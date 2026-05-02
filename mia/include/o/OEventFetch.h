#ifdef MIA_OPTION_FETCH
#ifndef O_OEVENTFETCH_H
#define O_OEVENTFETCH_H

/**
 * @file OEventFetch.h
 *
 * Object (derives OEvent which derives OJoin)
 *
 * An EventFetch creates a http(s) request.
 * Caching is always disabled via HTTP headers.
 *
 * @note As OEventFetch is an OJoin, o_del does not work.
 *       Call OEvent_done or delete its parent instead.
 *
 * @note Needs MIA_OPTION_FETCH -> MIA_USE_FETCH
 */


#include "OEvent.h"
#include "OArray.h"


/** object id */
#define OEventFetch_ID OEvent_ID "Fetch."


struct OEventFetch_response {
    bool error;
    int api_code;

    // server response headers ODict<header_key, OList<header_values for that key>> (maybe empty)
    oobj headers;

    // body bytes (maybe empty)
    OArray *body;
};

struct OEventFetch_progress {
    // inits with 0
    // MIA_PLATFORM_EMSCRIPTEN: uploaded (and upload_total) are faked as its not possible to progress it.
    //                          upload_total is set to opt_data_size
    //                          and on first download progress, uploaded is set to upload_total
    osize uploaded, downloaded;
    osize upload_total, download_total;
};

typedef struct {
    OEvent super;


    void *impl;
    
    // progress is only updated if this is set to true 
    //     (before run is called)
    bool progress_show;

    // valid if OEvent_ready()
    struct OEventFetch_response response;
} OEventFetch;

/**
* Creates a new OEventFetch object.
 * This function can be used to init a derived OJoin (for thread safe stuff as an example)
 * @param object_size size to allocate (asserts >= sizeof(OJoin)
 * @param parent to inherit from
 * @param opt_on_ready event on_ready function, or NULL to ignore
 * @param opt_context optional OContext applied before calling on_ready will be cloned, like o_context()
 *                    If apps need a_view, or a_cam, or a_pointer, or want to create an AScene's,
 *                        a valid app state is needed, pass o_context() to capture the current.
 * @return obj casted as OEventFetch
 * @note OEvent_post is called internally on a fetch result, DO NOT CALL IT
 *       Call OEventFetch_run to start fetching
 */
OEventFetch *OEventFetch_new_super(osize object_size, oobj parent, OObj__event_fn opt_on_ready, oobj opt_context);

/**
 * Creates a new OEventFetch object
 * @param parent to inherit from
 * @param opt_on_ready event on_ready function, or NULL to ignore
 * @param opt_context optional OContext applied before calling on_ready will be cloned, like o_context()
 *                    If apps need a_view, or a_cam, or a_pointer, or want to create an AScene's,
 *                        a valid app state is needed, pass o_context() to capture the current.
 * @return The new object
 * @note OEvent_post is called internally on a fetch result, DO NOT CALL IT
 *       Call OEventFetch_run to start fetching
 */
o_inline OEventFetch *OEventFetch_new(oobj parent, OObj__event_fn opt_on_ready, oobj opt_context)
{
    return OEventFetch_new_super(sizeof(OEventFetch), parent, opt_on_ready, opt_context);
}

//
// object functions:
//

/**
 * Starts fetching
 * @param obj OEventFetch object
 * @param url Full http url (use https://example.com instead of example.com!)
 * @param opt_method if NULL: opt_data==NULL? "GET" : "POST"
 * @param opt_headers a NULL terminated list of keys, values passed as headers or NULL
 *                    example: (char *[]) {"X-MIA-TOKEN", "123456", NULL}
 * @param opt_data request body data for POST, etc. with opt_data_size
 *                 if available and opt_headers==NULL -> will be set "Content-Type: application/octet-stream"
 * @return The new object
 * @note OEvent_post is called internally on a fetch result, DO NOT CALL IT
 */
void OEventFetch_run(oobj obj, const char *url,
                     const char *opt_method, char **opt_headers,
                     const void *opt_data, osize opt_data_size);

/**
 * @param obj OEventFetch object
 * @return the fetch response, only valid if OEvent_ready is true (which is true in the optional callback)
 */
OObj_DECL_GET(OEventFetch, struct OEventFetch_response, response)

/**
 * @param obj OEventFetch object
 * @return only updated progress if its set to true
 *         default: false
 * @note only valid if called before OEventFetch_run.
 *         default is false for less performancr overhead 
 *             Its not much but some cache invalidations,
 *             or calling C from Java again in Android.
 *             In MIA_PLATFORM_EMSCRIPTEN it wont matter at all.. 
 */
OObj_DECL_GETSET(OEventFetch, bool, progress_show)


/**
 * @param obj OEventFetch object
 * @return the current progress, only for visualizing.
 * @note for the fetch response (struct OEventFetch_response) use OEvent_ready, etc. instead.
 *       Keep in mind that OEvent_done() will delete this object...
 *       MIA_PLATFORM_EMSCRIPTEN: uploaded (and upload_total) are faked as its not possible to progress it.
 *       upload_total is set to opt_data_size
 *       and on first download progress, uploaded is set to upload_total
 *       asserts OEventFetch_progress_show (default false)
 */
struct OEventFetch_progress OEventFetch_progress(oobj obj);


#endif //O_OEVENTFETCH_H
#endif //MIA_OPTION_FETCH
