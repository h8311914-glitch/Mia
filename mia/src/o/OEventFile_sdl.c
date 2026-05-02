#if !defined(MIA_PLATFORM_EMSCRIPTEN) && !defined(MIA_OPTION_SDL2)

#include <SDL3/SDL_dialog.h>

#include "o/OEventFile.h"
#include "o/OObj_builder.h"
#include "o/str.h"
#include "o/file.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"

// protected
bool o__ignore_pause_set(bool set);


struct event_file_context {
    OEventFile *self;
    bool download; // else upload...
    char *file;
};

static void event_file_dialog_cb(void *userdata, const char *const *filelist, int filter)
{
    struct event_file_context *C = userdata;
    osize written = 0;
    char *user_file = NULL;
    if (filelist && filelist[0] && *filelist[0]!='\0') {
        user_file = o_str_clone(C->self, filelist[0]);
        o_file_route_quiet_block {
            if (C->download) {
                written = o_file_clone(user_file, C->file);
            } else {
                written = o_file_clone(C->file, user_file);
            }
        }
    }

    struct OEventFile_response *response = &C->self->response;
    response->error = written <= 0;
    response->file = C->file;
    response->opt_user_file = user_file;
    OEvent_post(C->self);
    o__ignore_pause_set(false);
}

static SDL_DialogFileFilter event_file_dialog_filter(oobj parent, const char *file)
{
    SDL_DialogFileFilter filter = {0};
    char *lower = o_str_tolower(parent, file);
    if (o_str_ends(lower, ".png")) {
        filter.name = "Image PNG File";
        filter.pattern = "png";
    } else if (o_str_ends(lower, ".jpg") || o_str_ends(lower, ".jpeg")) {
        filter.name = "Image JPG File";
        filter.pattern = "jpg;jpeg";
    } else if (o_str_ends(lower, ".image")) {
        filter.name = "Image File";
        filter.pattern = "png;jpg;jpeg";
    } else if (o_str_ends(lower, ".gif")) {
        filter.name = "GIF File";
        filter.pattern = "gif";
    } else if (o_str_ends(lower, ".json")) {
        filter.name = "Json File";
        filter.pattern = "json";
    } else if (o_str_ends(lower, ".tar")) {
        filter.name = "Tape Archive";
        filter.pattern = "tar";
    } else {
        filter.name = "All Files";
        filter.pattern = "*";
    }
    o_free(parent, lower);
    return filter;
}


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

    struct event_file_context *C = o_new(self, *C, 1);
    self->impl = C;
    C->self = self;
    C->download = true;
    C->file = o_str_clone(self, file);
    SDL_DialogFileFilter filter = event_file_dialog_filter(self, C->file);
    o__ignore_pause_set(true);
    SDL_ShowSaveFileDialog(event_file_dialog_cb, C, NULL, &filter, 1, NULL);
}

void OEventFile_upload(oobj obj, const char *file)
{
    OObj_assert(obj, OEventFile);
    OEventFile *self = obj;
    assert(!self->impl && "already called up or download");

    struct event_file_context *C = o_new(self, *C, 1);
    self->impl = C;
    C->self = self;
    C->download = false;
    C->file = o_str_clone(self, file);
    SDL_DialogFileFilter filter[2];
    filter[0] = event_file_dialog_filter(self, C->file);
    filter[1] = (SDL_DialogFileFilter) {"All files", "*"};
    o__ignore_pause_set(true);
    SDL_ShowOpenFileDialog(event_file_dialog_cb, C, NULL, filter, 2, NULL, 0);
}

#endif

#ifdef MIA_OPTION_SDL2
//
// noop fallback for SDL2
//


#include "o/OEventFile.h"
#include "o/OObj_builder.h"
#include "o/str.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


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
    o_log_info_s(__func__, "not implemented in SDL2");
    self->impl = self;
    struct OEventFile_response *response = &self->response;
    response->error = true;
    response->file = o_str_clone(self, file);
    response->opt_user_file = NULL;
    OEvent_post(self);
}

void OEventFile_upload(oobj obj, const char *file)
{
    OObj_assert(obj, OEventFile);
    OEventFile *self = obj;
    assert(!self->impl && "already called up or download");
    o_log_info_s(__func__, "not implemented in SDL2");
    self->impl = self;
    struct OEventFile_response *response = &self->response;
    response->error = true;
    response->file = o_str_clone(self, file);
    response->opt_user_file = NULL;
    OEvent_post(self);
}

#endif
typedef int avoid_empty_translation_unit;
