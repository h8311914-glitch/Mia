#ifdef MIA_PLATFORM_EMSCRIPTEN

#include "o/OEventFetch.h"
#include "o/OObj_builder.h"
#include "o/OObjRoot.h"
#include "o/OArray.h"
#include "o/OList.h"
#include "o/ODict.h"
#include "o/OJoin.h"
#include "o/str.h"
#include <emscripten/fetch.h>

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"

//
// protected
//

void o__fetch_init(void)
{
    // noop
}


//
// private
//

struct FetchData {
    oobj root;

    // joined are the fetch async root and the calling parent
    OEventFetch *event;

    char *url;
    void *opt_data;
    osize opt_data_size;

    struct OEventFetch_progress progress;
};

// built server header
static void fetch_create_headers(oobj server_headers, char *header_data)
{
    char *it = header_data;

    // ems does not provide the first http line (http ok 200...)

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
            key[strip+1] = '\0';
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
            value[strip+1] = '\0';
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

static void ems_fetch_success(emscripten_fetch_t *fetch)
{
    struct FetchData *data = fetch->userData;

    osize header_size = (osize) emscripten_fetch_get_response_headers_length(fetch);
    assert(header_size>0);
    char *header_data = o_new0(data->event, char, header_size + 1);
    emscripten_fetch_get_response_headers(fetch, header_data, header_size+1);

    o_log("header: (%i) <%s>", (int) header_size, header_data);

    oobj server_headers = ODict_new(data->event, 32);
    fetch_create_headers(server_headers, header_data);


    // set response (locking just in case... post should be enough, who knows :D 
    o_lock_block(data->event)  {
        struct OEventFetch_response *response = &data->event->response;
        response->error = false;
        response->api_code = fetch->status;
        response->headers = server_headers;
        response->body = OArray_new(data->event, fetch->data, 1, fetch->numBytes);
    }

    // set ready
    OEvent_post(data->event);

    o_log_trace_s("OEventFetch", "fetched %"osize_PRI" bytes from: <%s>",
                  (osize)fetch->numBytes, data->url);

    emscripten_fetch_close(fetch);

    // deletes one side of the join, if OFetch is also deleted, the OJoin gets deleted
    o_del(data->root);
}

static void ems_fetch_error(emscripten_fetch_t *fetch)
{
    struct FetchData *data = fetch->userData;
    o_lock_block(data->event)  {
        struct OEventFetch_response *response = &data->event->response;
        response->error = true;
        response->api_code = fetch->status;
        response->headers = ODict_new(data->event, 32);
        response->body = OArray_new_dyn(data->event, NULL, 1, 0, 8);
    }

    // set ready
    OEvent_post(data->event);

    o_log_warn_s("OFetch", "failed to fetch: <%s>, error code: %i", data->url, fetch->status);
    emscripten_fetch_close(fetch);
    
    // deletes one side of the join, if OFetch is also deleted, the OJoin gets deleted
    o_del(data->root);
}

void ems_fetch_progress(emscripten_fetch_t *fetch) {
    struct FetchData *data = fetch->userData;
    data->progress.downloaded = o_max(0, fetch->dataOffset + fetch->numBytes);
    data->progress.download_total = o_max(0, fetch->totalBytes);
    // just faked...
    data->progress.uploaded = data->progress.upload_total;
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
    assert((!opt_data || opt_data_size > 0) && "either NULL to opt_post_data or set a valid positive size");
    osize opt_header_num = o_list_num(opt_headers);
    assert(opt_header_num % 2 == 0 && "always pass key, value in the headers null terminated list");
    assert(o_strlen(opt_method) < 32);



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
    if(opt_data) {
        data->opt_data = o_alloc_clone(self, opt_data, 1, opt_data_size);
        data->opt_data_size = opt_data_size;
    }

    if (!opt_method) {
        // no need to clone static strings, stuff gets deleted here by the obj tree, not the pointers...
        if (data->opt_data) {
            opt_method = "POST";
            if (!opt_headers) {
                opt_headers = o_new0(self, char *, 3);
                opt_headers[0] = "Content-Type";
                opt_headers[1] = "application/octet-stream";
                // just to be sure...
                opt_headers[2] = NULL;

                opt_header_num = 2;
            }
        } else {
            opt_method = "GET";
        }
    }

   

    emscripten_fetch_attr_t attr;

    emscripten_fetch_attr_init(&attr);
    attr.userData = data;
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = ems_fetch_success;
    attr.onerror = ems_fetch_error;
    
    
    if(data->event->progress_show) {
        attr.onprogress = ems_fetch_progress;
    
        // just fake it...
        data->progress.upload_total = data->opt_data_size;
    }

    oobj header_list = OList_new(root, NULL, 6 + opt_header_num + (opt_data? 2 : 1));

    // disable caching via headers:
    OList_push(header_list, "Cache-Control");
    OList_push(header_list, "no-cache, no-store, must-revalidate");
    OList_push(header_list, "Pragma");
    OList_push(header_list, "no-cache");
    OList_push(header_list, "Expires");
    OList_push(header_list, "0");

    if(data->opt_data) {
        attr.requestData = data->opt_data;
        attr.requestDataSize = data->opt_data_size;
    }
    for (osize i = 0; i < opt_header_num; ++i) {
        OList_push(header_list, o_str_clone(header_list, opt_headers[i]));
    }

    attr.requestHeaders = (const char * const *) OList_list(header_list);

    snprintf(attr.requestMethod, sizeof(attr.requestMethod), "%s", opt_method);
    
    emscripten_fetch_t *fetch = emscripten_fetch(&attr, url);
    assert(fetch->userData == data);

    // holds the tree, which gets killed either by the detached thread or the objs tree
    self->impl = data;
}

struct OEventFetch_progress OEventFetch_progress(oobj obj)
{
    OObj_assert(obj, OEventFetch);
    OEventFetch *self = obj;
    assert(self->progress_show);
    struct FetchData *data = self->impl;
    return data->progress;
}


#endif
typedef int avoid_empty_translation_unit;
