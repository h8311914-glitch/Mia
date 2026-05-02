#ifdef MIA_PLATFORM_EMSCRIPTEN

#include "o/OEventFile.h"
#include "o/OObj_builder.h"
#include "o/str.h"
#include "o/file.h"
#include <emscripten.h>


#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"

// protected
bool o__ignore_pause_set(bool set);


static struct {
    OEventFile *event;
} event_file_L;

//
// public
//


OEventFile *OEventFile_new_super(osize object_size, oobj parent, OObj__event_fn opt_on_ready, oobj opt_context)
{  
    assert(object_size >= (osize) sizeof(OEventFile));
    OEvent *super = OEvent_new_super(object_size, parent, opt_on_ready, opt_context);
    OEventFile *self = (OEventFile *) super;
    OObj_id_set(self, OEventFile_ID);
    
    return self;
}

void OEventFile_download(oobj obj, const char *file)
{
    OObj_assert(obj, OEventFile);
    OEventFile *self = obj;
    assert(!self->impl && "already called up or download");

    char routed[O_FILE_PATH_MAX];
    o_file_route_resolve(routed, file);

    const char *filename = file;
    enum o_file_route_type type = o_file_route_type(file);
    switch (type) {
        case o_file_route_RES:
        case o_file_route_SAVE:
        case o_file_route_TMP:
            filename++;
        default:
            break;
    }

    char *script = o_strf(obj, "o__file_download(\'%s\', \'%s\');", routed, filename);
    emscripten_run_script(script);
    o_free(obj, script);

    self->impl = self;
    struct OEventFile_response *response = &self->response;
    response->error = false;
    response->file = o_str_clone(self, file);
    response->opt_user_file = o_str_clone(self, filename);
    OEvent_post(self);
}


void o_file_upload__done(const char *file, const char *user_file_name, bool success)
{
    if(!event_file_L.event) {
        o_log_warn_s("o_file_upload", "invalid callback call");
        return;
    }
    o_log_s(__func__, "file upload <%s> for: %s # %s", success? "done!" : "failed!",
                  file, user_file_name);
    struct OEventFile_response *response = &event_file_L.event->response;
    response->error = !success;
    response->opt_user_file = o_str_clone(event_file_L.event, user_file_name);
    OEvent_post(event_file_L.event);
    event_file_L.event = NULL;
    o__ignore_pause_set(false);
}

void OEventFile_upload(oobj obj, const char *file)
{
    OObj_assert(obj, OEventFile);
    OEventFile *self = obj;
    assert(!self->impl && "already called up or download");

    if (event_file_L.event) {
        o_log_info_s(__func__, "OEventFile got ignored");
        struct OEventFile_response *response = &event_file_L.event->response;
        response->error = true;
        response->opt_user_file = NULL;
        OEvent_post(event_file_L.event);
        event_file_L.event = NULL;
    }

    struct OEventFile_response *response = &event_file_L.event->response;
    response->file = o_str_clone(self, file);

    char routed[O_FILE_PATH_MAX];
    o_file_route_resolve(routed, file);

    self->impl = self;
    event_file_L.event = self;

    char script[O_FILE_PATH_MAX+64];
    o_strf_buf(script, "o__file_upload(\'%s\');", routed);
    o__ignore_pause_set(true);
    emscripten_run_script(script);
}

#endif
typedef int avoid_empty_translation_unit;
