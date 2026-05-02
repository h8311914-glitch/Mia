/**
 * In this example we open a webcam and render its video stream.
 * We can also switch between cameras.
 * And mirror + rotate
 *
 * @note for the AndroidStudio port, 
 *       uncomment / enable camera support in the Android Manifest file.
 */

#include "mia.h"


struct context {
    
    /**
     * OWebcam
     */
    oobj webcam;

    /**
     * RTex created from a captured OWebcam frame
     */
    oobj webcam_tex;

    oobj theme;
    oobj gui;


    oobj cam_btn;
    oobj cam_btn_text;
    
    oobj mirror_btn;
    oobj rotate_btn;

    oobj position_text;
    
    /**
     * WObj as a placeholder to blit the webcam_tex
     */
    oobj webcam_area;
    
    /**
     * camera preview rotation [0:4) as angle quadrants (*= m_PI)
     */
    int rotation;
    
    /**
     * Platform emscripten needs a re'open on chnage
     */
    bool portrait_mode;
};


static void setup(oobj view)
{
    struct context *C = o_user_new0(view, *C, 1);

    /**
     * Creates the new webcam object.
     * Can switch between hardware cameras internally.
     * With auto_open (true) the first camera is automatically opened with its default format.
     */
    C->webcam = OWebcam_new(view, true);
    
    C->portrait_mode = RCam_is_portrait(AView_cam(view));

    /**
     * Setup gui for our little webcam example
     */
    C->theme = WTheme_new_tiny(view);
    C->gui = WAlign_new_center_h(view);

    oobj gui_box = WBox_new_v(C->gui);
    WObj_min_size_set(gui_box, vec2_(128));
    WObj_padding_ref(gui_box)->y = 16;
    WBox_spacing_set(gui_box, vec2_(8));
    
    oobj title = WTextShadow_new(gui_box, "WEBCAM");
    WText_char_scale_set(title, vec2_(1, 2));
    
    oobj btn_box = WBox_new_h(gui_box);
    WBox_spacing_set(btn_box, vec2_(8));
    
    C->cam_btn = WBtn_new_text(btn_box, "", &C->cam_btn_text);
    C->mirror_btn = WBtn_new_text(btn_box, "MIRROR", NULL);

    /**
     * Automatically calls WBtn_toggled
     */
    WBtn_auto_mode_set(C->mirror_btn, WBtn_auto_TOGGLED);
    
    C->rotate_btn = WBtn_new_text(btn_box, "ROTATE", NULL);

    C->position_text = WText_new(gui_box, "");
    
    oobj frame = WFrame_new(gui_box);
    C->webcam_area = WObj_new(frame);
    WObj_min_size_set(C->webcam_area, vec2_(64));
    
}

static void update(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);
    
    /**
     * index of the current used cameras and number of available hardware cameras.
     * camera_idx is [0:num_camera).
     */
    int cam_idx = OWebcam_camera_idx(C->webcam);
    int num_cams = OWebcam_num_cameras(C->webcam);
    
    char *cams = o_strf_a("CAM%2i /%2i", cam_idx+1, num_cams);
    WText_text_set(C->cam_btn_text, cams);

    /**
     * We can query the position of each hardware camera.
     * On a lot of systems it may be UNKNOWN.
     */
    switch (OWebcam_position(C->webcam, cam_idx)) {
        default:
        case OWebcam_position_UNKNOWN:
            WText_text_set(C->position_text, "POS: UNKNOWN");
            break;
        case OWebcam_position_FRONT_FACING:
            WText_text_set(C->position_text, "POS: FRONT_FACING");
            break;
        case OWebcam_position_BACK_FACING:
            WText_text_set(C->position_text, "POS: BACK_FACING");
            break;
    }

    if (!OWebcam_ready(C->webcam)) {
        WText_text_set(C->position_text, "NOT READY!");
    }


    if(C->webcam_tex) {
        /**
         * Target render width is 120 with height using its aspect ratio
         */
        vec2 tex_size = RTex_size(C->webcam_tex);
        float width = 120;
        float height = width * tex_size.y / tex_size.x;
        WObj_min_size_set(C->webcam_area, vec2_(width, height));
    }

    WTheme_update_full_tex(C->theme, C->gui, tex);
    
    
    if(num_cams <= 1) {
        WBtn_down_set(C->cam_btn, true);
    } else if(WBtn_clicked(C->cam_btn)) {
        cam_idx++;
        cam_idx%=num_cams;

        /**
         * Select and open another hardware camera.
         * We pass NULL as format (o_img) and use the default format.
         */
        OWebcam_open(C->webcam, cam_idx, 0);
    }
    
    if(WBtn_clicked(C->rotate_btn)) {
        C->rotation++;
        C->rotation%=4;
    }
    
    /**
     * Platform emscripten may need a re'open.
     */
    bool portrait = RCam_is_portrait(a_cam());
    if(portrait != C->portrait_mode) {
        C->portrait_mode = portrait;
        OWebcam_open(C->webcam, cam_idx, 0);
    }
}

static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);
    
    RTex_clear_full(tex, vec4_(0.4, 0.2, 0.3, 1.0));
    
    WTheme_render(C->theme, tex);

    bool mirror = WBtn_down(C->mirror_btn);

    /**
     * In a minimal example its enough to create the OWebcam with auto_open=true and capture frames.
     * The resulting frame may be NULL and is only !=NULL if a new frame arrived.
     */
    struct o_img *frame = OWebcam_capture(C->webcam);
    if(frame) {
        /**
         * struct o_img is a like plain old data.
         * MMat in contrast works like the opencv::Mat so its a channel based matrix.
         * Allows strides and views and a lot of image manipulation functions.
         * Here we create a view on the image (not a deep copy!).
         * Then rotate and mirror (also as view)
         * If we created stride changed views, the MMat data may nor be packed anymore.
         * RTex creation needs packed data in a valid format.
         * We manually make it packed to avoid RTex warnings. 
         */
        oobj mat = MMat_new_img(a_tmp(), frame, false);
        for (int rot=0; rot<C->rotation; rot++){
            mat = MMat_rotate_right(mat);
        }
        if (mirror) {
            mat = MMat_mirror_v(mat);
        }
        
        /** make it packed for RTex to turn of warnings */
        if (!MMat_packed(mat)) {
            mat = MMat_clone(mat);
        }
        
        /**
         * If RTex size equals, only update data
         */
        ivec2 tex_size = RTex_size_int(C->webcam_tex);
        ivec2 mat_size = MMat_size_int(mat);
        if(C->webcam_tex && ivec2_equals_v( tex_size, mat_size)) {
            RTex_set_from_mat(C->webcam_tex, mat);
        } else {
            o_del(C->webcam_tex);
            C->webcam_tex = RTex_new_mat(view, mat, R_FORMAT_KEEP);
        }
    }
    
    if(C->webcam_tex) {
        /**
         * Renders the webcam frame to its area.
         */
        mat4 pose = WObj_gen_pose(C->webcam_area);
        mat4 uv = RTex_pose(C->webcam_tex, 0, 0);
        RTex_blit_pose(tex, C->webcam_tex, pose, uv);
    }
}


oobj ex_14_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_SAFE);
    return scene;
}


/**
 * Summary:
 * In this small example we setup and rendered webcam frames.
 * With the option to switch hardware cameras, 
 *     to mirror, or to rotate the frame.
 * We also made use of some simple MMat image cpu manipulations.
 * The most important functions are simply:
 * - OWebcam_new(parent, true);
 * - OWebcam_capture(cam);
 * But with the following caveats:
 * - Native mobile frames may not be rotated if the phone is rotated.
 * - In the mobile webapp the camera stream may change on rotation, but the resolution not.
 * Thats both case the webcam returns raw camera frames.
 * - On Native Android the rotation (portrait mode) just ignores camera rotation.
 * - In mobile web rotation, the browser in contrast rotates the fames, but sdl keeps the resolution.
 * So to fix that, we both: re'open the camera on portrait <> landscape changes.
 * But also allow the user to rotate the image (we could also automatically rotate)
 */
