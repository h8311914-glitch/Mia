#if defined(MIA_OPTION_FETCH) && defined(MIA_PLATFORM_ANDROID)

#ifdef MIA_OPTION_SDL2
#  include <SDL2/SDL.h>
#else
#  include <SDL3/SDL.h>
#endif

#include "o/OEventFetch.h"
#include "o/OObj_builder.h"
#include "o/OObjRoot.h"
#include "o/OJoin.h"
#include "o/OThread.h"
#include "o/OArray.h"
#include "o/OList.h"
#include "o/str.h"
#include "o/endian.h"
#include <jni.h>



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

    // joined are the fetch thread root and the calling parent
    OEventFetch *event;

    char *url;
    char *method;
    char **opt_headers;
    void *opt_data;
    osize opt_data_size;

    struct OEventFetch_progress progress;
};

JNIEXPORT void JNICALL O_ANDROID_INTERFACE(oFetchProgress)(JNIEnv* env, jobject thisObject, jlong fetch_data,
                                                           jint uploaded, jint downloaded,
                                                           jint upload_total, jint download_total ) {
    struct FetchData *data = (struct FetchData *)(intptr_t)fetch_data;
    OObj_assert(data->event, OEventFetch);

    o_lock_block(data->event) {
        data->progress.uploaded = o_max(0, uploaded);
        data->progress.upload_total = o_max(0, upload_total);
        data->progress.downloaded = o_max(0, downloaded);
        data->progress.download_total = o_max(0, download_total);
    }
}

// built server header
static void fetch_create_headers(oobj server_headers, char *header_data)
{
    char *it = header_data;

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

static void request_thread(oobj thread_obj)
{
    struct FetchData *data = o_user(thread_obj);

    oobj server_headers = ODict_new(data->event, 32);

    // keeps beeing NULL on error
    int fetch_api_code = 0;
    char *fetch_headers = NULL;
    oobj fetch_array = NULL;

    // JNI CALL
    {
        JNIEnv *env = NULL;
        jobject activity = NULL;
        jclass clazz = NULL;
        jstring url = NULL;
        jstring method = NULL;
        jobjectArray headers = NULL;
        jbyteArray post_data = NULL;
        jobjectArray result = NULL;
        jobject result_response = 0;
        jint result_response_code = 0;
        jstring result_headers = NULL;
        const char *result_headers_C = NULL;
        jbyteArray result_body = NULL;
        jbyte *result_bytes = NULL;
        jsize result_bytes_len = 0;

        env = (JNIEnv *) SDL_GetAndroidJNIEnv();
        if (!env) {
            o_log_error_s(__func__, "failed to get jni env");
            goto JNI_CLEAN_UP;
        }

        activity = (jobject) SDL_GetAndroidActivity();
        if (!activity) {
            o_log_error_s(__func__, "failed to get activity");
            goto JNI_CLEAN_UP;
        }

        clazz = (*env)->GetObjectClass(env, activity);
        if (!clazz) {
            o_log_error_s(__func__, "failed to get clazz");
            goto JNI_CLEAN_UP;
        }

        jmethodID method_id = (*env)->GetStaticMethodID(env, clazz,
                                                        "oEventFetchBlocking",
                                                        "(JLjava/lang/String;Ljava/lang/String;[Ljava/lang/String;[B)[Ljava/lang/Object;");
        if (!method_id) {
            o_log_error_s(__func__, "method not found");
            goto JNI_CLEAN_UP;
        }

        url = (*env)->NewStringUTF(env, data->url);
        method = (*env)->NewStringUTF(env, data->method);
        if(data->opt_headers) {
            osize headers_size = o_list_num(data->opt_headers);
            jclass string_class = (*env)->FindClass(env, "java/lang/String");
            headers = (*env)->NewObjectArray(env, (jsize) headers_size, string_class, NULL);
            if(!headers) {
                o_log_error_s(__func__, "failed to create headers array");
                goto JNI_CLEAN_UP;
            }
            for(osize i=0; i<headers_size; i++) {
                (*env)->SetObjectArrayElement(env, headers, 0, (*env)->NewStringUTF(env, data->opt_headers[i]));
            }
        }
        if(data->opt_data && data->opt_data_size>0) {
            post_data = (*env)->NewByteArray(env, (jsize) data->opt_data_size);
            if(!post_data) {
                o_log_error_s(__func__, "failed to create post data array");
                goto JNI_CLEAN_UP;
            }
            (*env)->SetByteArrayRegion(env, post_data, 0, (jsize) data->opt_data_size,
                                       (const jbyte*) data->opt_data);
        }

        o_log_s("OEventFetch", "calling android fetch");
        jlong opt_fetch_data = 0;
        if(data->event->progress_show) {
            opt_fetch_data = (jlong)(intptr_t)data;
        }
        result = (jobjectArray) (*env)->CallStaticObjectMethod(env, clazz, method_id, opt_fetch_data, url, method, headers, post_data);
        o_log_s("OEventFetch", "calling android fetch done");
        if(!result) {
            goto JNI_CLEAN_UP;
        }

        jclass integerClass = (*env)->FindClass(env, "java/lang/Integer");
        jmethodID intValueMethod = (*env)->GetMethodID(env, integerClass, "intValue", "()I");

        result_response = (*env)->GetObjectArrayElement(env, result, 0);
        result_response_code = (*env)->CallIntMethod(env, result_response, intValueMethod);

        result_headers = (*env)->GetObjectArrayElement(env, result, 1);
        result_headers_C = (*env)->GetStringUTFChars(env, result_headers, NULL);

        result_body = (*env)->GetObjectArrayElement(env, result, 2);
        result_bytes_len = (*env)->GetArrayLength(env, result_body);
        result_bytes = (*env)->GetByteArrayElements(env, result_body, NULL);
        obyte *data_bytes = (obyte*) result_bytes;

        // clone result
        fetch_api_code = result_response_code;
        fetch_headers = o_str_clone(data->event, result_headers_C);
        fetch_array = OArray_new(data->event, data_bytes, 1, result_bytes_len);


        JNI_CLEAN_UP:
        if (env) {
            if(result_bytes) (*env)->ReleaseByteArrayElements(env, result_body, result_bytes, JNI_ABORT);
            if (result_body) (*env)->DeleteLocalRef(env, result_body);
            if (result_headers_C) (*env)->ReleaseStringUTFChars(env, result_headers, result_headers_C);
            if (result_headers) (*env)->DeleteLocalRef(env, result_headers);
            if (result_response) (*env)->DeleteLocalRef(env, result_response);
            if (post_data) (*env)->DeleteLocalRef(env, post_data);
            if (headers) {
                jsize size = (*env)->GetArrayLength(env, headers);
                for (int i = 0; i < size; i++) {
                    jobject str = (*env)->GetObjectArrayElement(env, headers, i);  // Get each string from array
                    (*env)->DeleteLocalRef(env, str);
                }
                (*env)->DeleteLocalRef(env, headers);
            }
            if (method) (*env)->DeleteLocalRef(env, method);
            if (url) (*env)->DeleteLocalRef(env, url);
            if (activity) (*env)->DeleteLocalRef(env, activity);
            if (clazz) (*env)->DeleteLocalRef(env, clazz);
        }
    }

    bool error = !fetch_array || !fetch_headers;

    if (error) {
        o_log_trace_s("OEventFetch", "failed to fetch: <%s>", data->url);
    } else {
        o_log_trace_s("OEventFetch", "fetched %"osize_PRI" bytes from: <%s>",
                      OArray_num(fetch_array), data->url);

        fetch_create_headers(server_headers, fetch_headers);
    }

    // set response (locking just in case... post should be enough, who knows :D )
    o_lock_block(data->event) {
        struct OEventFetch_response *response = &data->event->response;
        response->error = error;
        response->api_code = fetch_api_code;
        response->headers = server_headers;
        response->body = fetch_array;
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
    assert((!opt_data || opt_data_size>0) && "either NULL to opt_post_data or set a valid positive size");
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
                data->opt_headers = o_new0(self, char *, 3);
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
