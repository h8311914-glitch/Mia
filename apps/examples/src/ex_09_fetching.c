/**
 * This example shows fetching via http.
 * Pattern finding in the response.
 * Parsing a json string and render it.
 * We fetch >this< example source file via githubs raw api,
 *     find the payload pattern (FIND...ME) in a comment
 *     and extract its data.
 */


#include "mia.h"

struct context {
    /**
     * RObjText
     */
    oobj text;
};


static void fetch_ready(oobj fetch)
{
    struct context *C = o_user(fetch);

    struct OEventFetch_response response = OEventFetch_response(fetch);

    char *text = "";
    bool failed = false;

    if (response.error) {
        text = "ERROR!";
        goto SET_TEXT;
    }
    failed = true;

    /**
     * http api code (200)
     */
    o_log("Received: %i api_code", response.api_code);

    /**
     * Headers send by the server
     * Its an ODict of an OList of c strings.
     * So why an OList of strings? The server may sent multiple lines with the same key:
     *   <KEY_A>: <VALUE_A>
     *   <KEY_B>: <VALUE_B>
     *   <KEY_C>: <VALUE_C>
     *   <KEY_C>: <VALUE_C2>
     */
    for (osize k=0; k<ODict_num(response.headers); k++) {
        const char *key = ODict_key_at(response.headers, k);
        oobj values = ODict_value_at(response.headers, k);
        for (osize v=0; v<o_num(values); v++) {
            const char *value = o_at(values, v);
            o_log("Header: %s: %s", key, value);
        }
    }

    /**
     * response.body is now an OArray of the received data.
     * OArray_data returns the begin address of the data / received string.
     */
    const char *received = OArray_data(response.body, char);
    
    
    /**
     * Mia's OPattern matcher works like lua's pattern matcher.
     * We use the o_str_pattern_ wrapper which internally builds an OPattern temporarily
     * Our pattern is built up like:
     * 'FIND' must match the beginning
     * '(' ... ')' define a group to capture (what we want to extract) 
     * '{' nothing special, just match exactly '{'
     * '.+' the period means match all characters and the plus means one or more of those (our json data)
     * '}' like above nothing special, just match it
     * ')' as described above, group end
     * 'ME' must match the end
     */
    const char *pattern = "FIND({.+})ME";
    
    
    /**
     * As we fetch >this< example source file via github, here is the payload to find:
     *
     * FIND{ "msg": "HELLO", "nested": {"path": "WEB" }}ME
     *
     */
    
    struct o_str_range_list capture;
    o_str_pattern_find(fetch, pattern, received, &capture);
    if(capture.len != 1) {
        o_log_error("failed to capture pattern");
        goto SET_TEXT;
    }
    char *json_str = o_str_clone_range(fetch, received, capture.list[0]);
    
    
    /**
     * Creates an OJson object tree from the given string.
     * See "o/OJson.h" for more details on how to build or parse the tree.
     * We allocate on the fetch oobj, cause it will delete itself, once we call OEvent_done (below)
     */
    struct oobj_opt json = OJson_new_read_string(fetch, "root", json_str);
    if (!json.o) {
        o_log_error("failed to parse json");
        goto SET_TEXT;
    }

    
    /**
     * The utility class OWalkjson helps in parsing a json and expect multiple components.
     * If one of the parsed strings is missing or type is missmatching, the walker gets invalid.
     * Each of the walker parse functions, such as _string, or _boolean, _number
     *      take a path, see the OJson header for more infos.
     * You could also directly use the OJson api and check each member if its available.
     */
    oobj walker = OWalkjson_new(json.o);

    const char *msg = OWalkjson_string(walker, "msg");
    const char *path =  OWalkjson_string(walker, "nested/path");

    if (!OWalkjson_valid(walker)) {
        o_log_error("json walker invalid");
        goto SET_TEXT;
    }

    text = o_strf(fetch, "GOT INTERNET!\n\n %s, %s",
                  msg, path);
    failed = false;
                
    
    SET_TEXT:

    if (failed) {
        text = "FAILED!";
    }

    /**
     * Update the text (which is cloned internally, so fine to pass a temporary like we do here)
     */
    RObjText_text_set(C->text, text, R_WHITE);

    /**
     * The event system works on top of OJoins, with a call to OEvent_done we remove us as a parent.
     *   So may delete the fetch after that call.
     * This also deletes our temporary oobj's such as the OJson tree
     */
    OEvent_done(fetch);
}


static void setup(oobj view)
{
    struct context *C = o_user_new0(view, *C, 1);

    /**
     * Creating a text render object and set its pose to 8,48 (lt) with x1,x1 scaling
     */
    C->text = RObjText_new_font35_shadow(view, "LOADING", NULL);
    RObjText_pose_set(C->text, u_pose_new(8, 48, 1, 1));

    /**
     * Start a fetch in async as GET request (the api wont cache).
     * We will fetch >this< example source file.
     *
     * An OEvent (or the inherit OEventFetch) can either be polled using OEvent_ready()
     *     or a callback function (fetch_ready) is passed.
     *         The callback will be called on the main thread at frame start (if the OEvent is post'ed)
     *         For the callback, an app context may be passed which is applied before calling.
     *             Use o_context() as default to caputure the current app state.
     *             Needed to work with a_view, a_cam, a_pointer or to create new AScene's.
     *                 (Not needed in this example and could be NULL instead)
     * To actually start fetching, call OEventFetch_run.
     *     The NULL's are:
     *     opt_method ("GET")
     *     opt_headers (NULL terminated list of key value pairs to send as headers)
     *     opt_data (Additional data like for POST)
     *     opt_data_size (...)
     */
    oobj fetch = OEventFetch_new(view, fetch_ready, o_context());
    o_user_set(fetch, C);
    OEventFetch_run(fetch, "https://raw.githubusercontent.com/renehorstmann/Mia/refs/heads/main/apps/examples/src/ex_09_fetching.c", NULL, NULL, NULL, 0);
}

static void update(oobj view, oobj tex, float dt)
{
    /**
     * Nothing to do here
     */
}

static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    /**
     * Render the text
     */
    RTex_ro(tex, C->text);
}


oobj ex_09_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_SAFE);
    return scene;
}

/**
 * Summary:
 *
 * This example demonstrates the use of fetching http(s) requests with mia.
 * In the reaponse we searched for a payload with OPattern (via its o_str_pattern_ wrapper)
 * The result is parsed as OJson with the OJsonwalker helper.
 * And then rendered in the app.
 */
