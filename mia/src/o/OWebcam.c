#include "o/OWebcam.h"
#include "o/OObj_builder.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"

#ifndef MIA_OPTION_SDL2
#include <SDL3/SDL.h>
struct webcam_impl {
    SDL_Camera *camera;
    SDL_CameraID *camera_ids;
};
#endif


//
// public
//

OWebcam *OWebcam_init(oobj obj, oobj parent, bool auto_open)
{
    OWebcam *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, OWebcam_ID);

    // v del override
    self->super.v_del = OWebcam__v_del;

    self->camera_idx = -1;

#ifndef MIA_OPTION_SDL2
    struct webcam_impl *impl = self->impl = o_new0(self, *impl, 1);

    int ids_num = 0;
    SDL_CameraID *ids = SDL_GetCameras(&ids_num);
    if (ids && ids_num > 0) {
        self->num_cameras = ids_num;
        impl->camera_ids = o_new_clone(self, ids, SDL_CameraID, ids_num);
    }
    SDL_free(ids);

    if (ids_num > 0 && auto_open) {
        OWebcam_open(self, 0, 0);
    }

#endif

    return self;
}

//
// virtual implementations:
//

void OWebcam__v_del(oobj obj)
{
    OWebcam_release(obj);
    OObj__v_del(obj);
}

//
// object functions:
//

bool OWebcam_ready(oobj obj)
{
#ifdef MIA_OPTION_SDL2
    return false;
#else
    OObj_assert(obj, OWebcam);
    OWebcam *self = obj;
    struct webcam_impl *impl = self->impl;
    if (!impl->camera) {
        return false;
    }
    return SDL_GetCameraPermissionState(impl->camera) > 0;
#endif
}


struct o_img *OWebcam_capture(oobj obj)
{
#ifdef MIA_OPTION_SDL2
    return NULL;
#else
    OObj_assert(obj, OWebcam);
    OWebcam *self = obj;
    struct webcam_impl *impl = self->impl;

    if (!impl->camera) {
        return NULL;
    }
    
    int ids_num = 0;
    SDL_CameraID *ids = SDL_GetCameras(&ids_num);
    if (ids && ids_num > self->num_cameras) {
        self->num_cameras = ids_num;
        impl->camera_ids = o_renew(self, impl->camera_ids, SDL_CameraID, ids_num);
        o_memcpy(impl->camera_ids, ids, sizeof(SDL_CameraID), ids_num);
    }
    SDL_free(ids);

    ou64 time_ns;
    SDL_Surface *surf = SDL_AcquireCameraFrame(impl->camera, &time_ns);
    if (!surf) {
        return NULL;
    }
    o_img_free(&self->frame);
    self->frame = o_img_new_sdl_surface(self, surf, o_img_KEEP);
    SDL_ReleaseCameraFrame(impl->camera, surf);
    return &self->frame;
#endif
}


enum OWebcam_position OWebcam_position(oobj obj, int camera_idx)
{
#ifdef MIA_OPTION_SDL2
    return OWebcam_position_UNKNOWN;
#else
    OObj_assert(obj, OWebcam);
    OWebcam *self = obj;
    struct webcam_impl *impl = self->impl;

    if (camera_idx < 0 || camera_idx >= self->num_cameras) {
        return OWebcam_position_UNKNOWN;
    }
    SDL_CameraPosition pos = SDL_GetCameraPosition(impl->camera_ids[camera_idx]);
    switch (pos) {
        default:
        case SDL_CAMERA_POSITION_UNKNOWN:
            return OWebcam_position_UNKNOWN;
        case SDL_CAMERA_POSITION_FRONT_FACING:
            return OWebcam_position_FRONT_FACING;
        case SDL_CAMERA_POSITION_BACK_FACING:
            return OWebcam_position_BACK_FACING;
    }
#endif
}


bool OWebcam_open(oobj obj, int camera_idx, int max_resolution)
{
#ifdef MIA_OPTION_SDL2
    return false;
#else
    OObj_assert(obj, OWebcam);
    OWebcam *self = obj;
    struct webcam_impl *impl = self->impl;

    if (impl->camera) {
        SDL_CloseCamera(impl->camera);
        impl->camera = NULL;
        self->camera_idx = -1;
    }
    
    o_img_free(&self->frame);
    
    if(camera_idx < 0) {
        // just close the camera
        return false;
    }

    if (camera_idx >= self->num_cameras) {
        o_log_error_s(__func__, "camera_idx out of bounds");
        return false;
    }

    if(max_resolution<=0) {
        max_resolution = 1280;
    }

    /*
     * Selecting a target max resolution for the webcam.
     * Defaults to 1280.
     * Browser may deny querying any webcam infos if not allowed
     *     (and the question is asked after that call)
     *     So may not work in browsers.
     *     But they seem to not capture in > HD anyway
     */

    SDL_CameraSpec *spec_used = NULL;
    int specs_num = 0;
    SDL_CameraSpec **specs = SDL_GetCameraSupportedFormats(impl->camera_ids[camera_idx], &specs_num);
    if(specs && specs_num>1) {
        for(int i=0; i<specs_num; i++) {
            int w = specs[i]->width;
            int h = specs[i]->height;
            if(w > max_resolution || h > max_resolution) {
                continue;
            }
            if(spec_used && (w < spec_used->width || h < spec_used->height)) {
                continue;
            }
            spec_used = specs[i];
        }
    }
    if(spec_used) {
        o_log_info("opening webcam with resolution: %i x %i", spec_used->width, spec_used->height);
    } else {
        o_log_info("opening default webcam as querying formats failed");
    }

    impl->camera = SDL_OpenCamera(impl->camera_ids[camera_idx], spec_used);
    SDL_free(specs);

    if (!impl->camera) {
        o_log_error_s(__func__, "failed to open the webcam");
        return false;
    }

    self->camera_idx = camera_idx;
    o_log_s(__func__, "webcam opened");
    return true;
#endif
}
