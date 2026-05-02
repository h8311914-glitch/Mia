// curl is used on desktop versions and in CxxDroid
#if defined(MIA_OPTION_FETCH) && (defined(MIA_PLATFORM_DESKTOP) || defined(MIA_PLATFORM_CXXDROID))

#include "o/OEventFetch.h"
#include "o/OObj_builder.h"
#include "o/OObjRoot.h"
#include "o/OJoin.h"
#include "o/OThread.h"
#include "o/OArray.h"
#include "o/OList.h"
#include "o/ODict.h"
#include "o/str.h"
#include <curl/curl.h>

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


//
// protected
//

// protected
void o__fetch_init(void)
{
    curl_global_init(CURL_GLOBAL_ALL);
}


//
// private
//

struct FetchData {
    oobj root;

    // joined are the fetch thread root and the calling parent
    OEventFetch *event;

    char *url;
    char *method;
    char **opt_headers;
    void *opt_data;
    osize opt_data_size;

    // OArray of char
    oobj header_array;
    oobj body_array;

    struct OEventFetch_progress progress;
};


// built server header
static void fetch_create_headers(oobj server_headers, char *header_data)
{
    char *it = header_data;

    // ignore first line ( HTTP/1.1 200 OK )
    osize first_line_end = o_str_find(it, "\r\n");
    first_line_end = first_line_end < 0 ? o_strlen(it) : first_line_end + 2;
    it += first_line_end;

    while (*it) {
        osize strip;
        char *key, *value;

        // lstrip key
        strip = o_str_find_space_inv(it);
        if (strip < 0) {
            break;
        }
        it += strip;

        key = it;
        osize colon = o_str_find_char(it, ':');
        if (colon < 0) {
            break;
        }
        it += colon + 1;
        key[colon] = '\0';

        // rstrip key
        strip = o_str_find_back_space_inv(key);
        if (strip >= 0) {
            key[strip + 1] = '\0';
        }

        if (o_strlen(key) <= 0) {
            break;
        }
        // now key is valid

        // lstrip value
        strip = o_str_find_space_inv(it);
        if (strip < 0) {
            break;
        }
        it += strip;

        value = it;
        osize end = o_str_find(it, "\r\n");
        if (end < 0) {
            break;
        }
        it += end + 2;
        value[end] = '\0';

        // rstip value
        strip = o_str_find_back_space_inv(value);
        if (strip >= 0) {
            value[strip + 1] = '\0';
        }

        if (o_strlen(value) <= 0) {
            break;
        }
        // now value is valid

        // key is internally copied
        // value needs a clone
        oobj list = ODict_get(server_headers, key);
        if (!list) {
            list = OList_new(server_headers, NULL, 0);
            ODict_set(server_headers, key, list);
        }
        OList_push(list, o_str_clone(list, value));
    }
}


static size_t fetch_response_writer(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    oobj array = userdata;
    osize full_size = (osize) (size * nmemb);
    OArray_append(array, ptr, full_size);
    return full_size;
}


static int fetch_transfer_info(void *ptr,
                      curl_off_t dltotal, curl_off_t dlnow,
                      curl_off_t ultotal, curl_off_t ulnow)
{
    struct FetchData *data = ptr;
    o_lock_block(data->root)
    {
        data->progress.uploaded = o_max(0, ulnow);
        data->progress.upload_total = o_max(0, ultotal);
        data->progress.downloaded = o_max(0, dlnow);
        data->progress.download_total = o_max(0, dltotal);
    }
    return 0; // non-zero would abort transfer
}

static void request_thread(oobj thread_obj)
{
    struct FetchData *data = o_user(thread_obj);

    oobj server_headers = ODict_new(data->event, 32);
    data->header_array = OArray_new_dyn(data->event, NULL, 1, 0, 1024);
    data->body_array = OArray_new_dyn(data->event, NULL, 1, 0, 1024);


    CURL *curl = curl_easy_init();
    struct curl_slist *headers = NULL;

    // disable caching via headers:
    curl_slist_append(headers, "Cache-Control: no-cache, no-store, must-revalidate");
    curl_slist_append(headers, "Pragma: no-cache");
    curl_slist_append(headers, "Expires: 0");

    curl_easy_setopt(curl, CURLOPT_URL, data->url);

    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, fetch_response_writer);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, data->header_array);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fetch_response_writer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data->body_array);

    if(data->event->progress_show) {
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, fetch_transfer_info);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, data);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    }
    
    // don't use signals in multi threads
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    if (data->opt_data) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data->opt_data);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data->opt_data_size);
    }

    if (data->opt_headers) {
        osize headers_num = o_list_num(data->opt_headers);
        // add headers
        for (osize i = 0; i < headers_num / 2; i++) {
            char *cat = o_strf(thread_obj, "%s: %s", data->opt_headers[i * 2], data->opt_headers[i * 2 + 1]);
            headers = curl_slist_append(headers, cat);
            o_free(thread_obj, cat);
        }
    }

    // set all headers
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // better after adding "post" data, cause may overwrite, don't know...
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, data->method);
    

    CURLcode perform_res = curl_easy_perform(curl);

    if (headers) {
        curl_slist_free_all(headers);
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);

    bool error = perform_res != CURLE_OK;


    if (error) {
        o_log_trace_s("OEventFetch", "failed to fetch: <%s>", data->url);
    } else {
        o_log_trace_s("OEventFetch", "fetched %"osize_PRI" bytes from: <%s>",
                      OArray_num(data->body_array), data->url);

        fetch_create_headers(server_headers, o_at(data->header_array, 0));
    }

    o_del(data->header_array);

    // set response (locking just in case... post should be enough, who knows :D )
    o_lock_block(data->event) {
        struct OEventFetch_response *response = &data->event->response;
        response->error = error;
        response->api_code = (int) http_code;
        response->headers = server_headers;
        response->body = data->body_array;
    }

    // set ready
    OEvent_post(data->event);

    // deletes one side of the join, if OEventFetch is also deleted, the OJoin gets deleted
    o_del(data->root);
}


//
// public
//

OEventFetch *OEventFetch_new_super(osize object_size, oobj parent, OObj__event_fn opt_on_ready, oobj opt_context)
{
    assert(object_size >= (osize) sizeof(OEventFetch));
    OEvent *super = OEvent_new_super(object_size, parent, opt_on_ready, opt_context);
    OEventFetch *self = (OEventFetch *) super;
    OObj_id_set(self, OEventFetch_ID);

    return self;
}

void OEventFetch_run(oobj obj, const char *url,
                     const char *opt_method, char **opt_headers,
                     const void *opt_data, osize opt_data_size)
{
    assert((!opt_data || opt_data_size>0) && "either NULL to opt_data or set a valid positive size");
    osize opt_header_num = o_list_num(opt_headers);
    assert(opt_header_num % 2 == 0 && "always pass key, value in the headers null terminated list");


    OObj_assert(obj, OEventFetch);
    OEventFetch *self = obj;

    assert(!self->impl && "only run once");

    // new root for the detached thread
    oobj root = OObjRoot_new_heap("OEventFetch");
    OJoin_add(self, root);
    OObj_threadsafe_set(self, true);

    struct FetchData *data = o_new0(self, *data, 1);
    data->root = root;
    data->event = self;

    data->url = o_str_clone(self, url);
    data->method = o_str_clone(self, opt_method);
    if (opt_header_num > 0) {
        data->opt_headers = o_new0(self, char *, opt_header_num + 1);
        for (osize i = 0; i < opt_header_num; ++i) {
            data->opt_headers[i] = o_str_clone(self, opt_headers[i]);
        }
        // just to be sure...
        data->opt_headers[opt_header_num] = NULL;
    }
    if(opt_data) {
        data->opt_data = o_alloc_clone(self, opt_data, 1, opt_data_size);
        data->opt_data_size = opt_data_size;
    }


    if (!data->method) {
        // no need to clone static strings, stuff gets deleted here by the obj tree, not the pointers...
        if (data->opt_data) {
            data->method = "POST";
            if (!data->opt_headers) {
                data->opt_headers = o_new0(self, char *, 2+1);
                data->opt_headers[0] = "Content-Type";
                data->opt_headers[1] = "application/octet-stream";
                // just to be sure...
                data->opt_headers[2] = NULL;
            }
        } else {
            data->method = "GET";
        }
    }

    // holds the tree, ith it's joined together data fields
    self->impl = data;

    oobj thread = OThread_new_run(self, request_thread, "OEventFetch", data);
    OThread_detach(thread);
}


struct OEventFetch_progress OEventFetch_progress(oobj obj)
{
    OObj_assert(obj, OEventFetch);
    OEventFetch *self = obj;
    assert(self->progress_show);
    struct FetchData *data = self->impl;
    struct OEventFetch_progress progress;
    o_lock_block(data->root) {
        progress = data->progress;
    }
    return progress;
}

#endif
typedef int avoid_empty_translation_unit;
