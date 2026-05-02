/**
 * File uploading and downloading.
 * Opens the system file dialog.
 * For this example, an image can be uploaded from which a funny colored collage is created.
 * That collage can be downloaded.
 *
 * The collage function is also shown in README.md
 */


#include "mia.h"


/**
 * Size of the collage image and the render size
 */
static const vec2 IMG_SIZE = {{1024, 1024}};
static const vec2 IMG_AREA = {{128, 128}};

// for README.md
//static const vec2 IMG_SIZE = {{512, 128}};


/**
 * Num of collage items
 */
#define MANIPULATE_RUNS 128

/**
 * File up and download need a saved file.
 * The route "#..." saves as a temporary file (may or may not be available on next app start).
 * We use .image for the upload to enable both .png and .jpg formats.
 * .png would restrict to .png only on some devices.
 * The file format does not make a guarantee for the actual file
 */
#define TMP_DOWNLOAD_FILE "#ex09_upndownload_image.png"
#define TMP_UPLOAD_FILE "#ex09_upndownload_image.image"

struct context {
    oobj view;

    /**
     * Widget stuff
     */
    oobj theme;
    oobj gui;
    oobj img_area;
    oobj upload_btn;
    oobj download_btn;

    /**
     * RTex of the collage
     */
    oobj img;
};


/**
 * Creates the collage image into the context RTex C->img
 * @param img is the source item to render for the collage (RTex)
 */
static void create_collage(oobj view, oobj img)
{
    struct context *C = o_user(view);

    oobj res = C->img;
    const int runs = MANIPULATE_RUNS;

    //
    // This function is also displayed in the README.md
    //

    // For README.md
    //oobj img = [..]; // source image for the collage
    //const int runs = 128;

    // For README.md
    /**
     * Creates a new empty Texture (RTex) with a size of 512 cols * 128 rows.
     * img is used as a parent
     */
    //oobj res = RTex_new(img, NULL, 512, 128);

    /**
     * Clears the texture to black
     */
    RTex_clear_full(res, R_BLACK);

    /**
     * Constant sizes
     */
    vec2 img_size = RTex_size(img);
    vec2 res_size = RTex_size(res);
    float img_min_size = m_min(img_size.x, img_size.y);
    float res_min_size = m_min(res_size.x, res_size.y);

    /**
     * We create a render object that is able to draw the full collage,
     *      batched in a single draw call.
     * It has "runs" quads and the given img as RTex to render
     * This special "new" constructor takes in a tex (RTex) and uses the "color_hsva" shader
     */
    oobj ro = RObjQuad_new_tex_rgba_hsva(res, runs, img, false, 1, 1);

    /**
     * Setup each quad.
     * They are rendered on top of each other, so first is rendered first
     */
    for(int i=0; i<runs; i++) {

        /**
         * scaling from big (2.0f) to small (0.2f) according to the minimal result size
         */
        float t = (float) i / (float) (runs-1);
        float scale = m_mix(2.0f, 0.2f, t) * res_min_size / img_min_size;

        /**
         * Calc quad stuff like render size, pos and angle to create the pose.
         * pos should be random around the tex center (vec2_scale(res_size, 0.5)) with an amplitude of 66%
         */
        vec2 render_size = vec2_scale(img_size, scale);
        vec2 pos = vec2_random_noise_v(vec2_scale(res_size, 0.5), vec2_scale(res_size, 0.66));
        float angle = m_random() * 2 * m_PI;

        /**
         * Creates a 4x4 3D pose for this quad with the pos as center and a rotation around that
         */
        mat4 pose = u_pose_new_center_angle(m_2(pos), m_2(render_size), angle);

        /**
         * the color hsva shader uses the additional 'fx', 'fy' fields
         * 'fx' would be an override for the rgba color "albedo color"
         * 'fy' is a shift in the hsva color space (hue in this case)
         */
        vec4 hsva_shift = vec4_(m_random(), 0, 0, 0);

        /**
         * Set quad stuff
         */
        struct r_quad *q = o_at(ro, i);
        q->pose = pose;
        q->fy = hsva_shift;
    }

    /**
     * Batched draw call
     */
    RTex_ro(res, ro);

    // For README.md
    /**
     * Saves the texture as a temporary image file to disk (mia_tmp/collage.png)
     */
    //RTex_write_file(res, "#collage.png");

    // For README.md
    /**
     * Deletes the result texture image and all children, like the RObjQuad
     */
    //o_del(res);

    o_del(ro);
}


static void on_file_uploaded(oobj event)
{
    struct OEventFile_response response = OEventFile_response(event);
    if(response.error) {
        o_log("uploading failed");
        OEvent_done(event);
        return;
    }

    /**
     * Main parent of the OEvent (OJoin!) is the view object
     */
    oobj view = OEvent_parent(event);

    /**
     * Load uploaded file as RTex
     */
    oobj img = RTex_new_file(view, TMP_UPLOAD_FILE);
    if(!RTex_valid(img)) {
        o_log("upload failed, invalid image");
        o_del(img);
        OEvent_done(event);
        return;
    }

    /**
     * Create collage
     */
    create_collage(view, img);
    o_del(img);
    OEvent_done(event);
}

/**
 * This event is called if the upload button is clicked
 */
static void upload(oobj btn)
{
    struct context *C = o_user(btn);

    o_log("upload");

    /**
     * Create an OEvent that calls if a file got uploaded.
     * The selected file will be saved/copied under the given file path.
     *     Use o_context() as default to caputure the current app state.
     *     Needed to work with a_view, a_cam, a_pointer or to create new AScene's.
     *         (Not needed in this example and could be NULL instead)
     * @note noop on MIA_PLATFORM_CXXDROID
     */
    oobj event = OEventFile_new(C->view, on_file_uploaded, o_context());
    OEventFile_upload(event, TMP_UPLOAD_FILE);
}


static void on_file_downloaded(oobj event)
{
    struct OEventFile_response response = OEventFile_response(event);
    o_log("file %s downloaded as: %s", response.error? "failed to" : "successfully",  o_or(response.opt_user_file, "?"));
}
/**
 * This event is called when the download button is clicked
 */
static void download(oobj btn)
{
    struct context *C = o_user(btn);

    o_log("download");

    RTex_write_file(C->img, TMP_DOWNLOAD_FILE);

    /**
     * Like OEventFile_upload, this function opens the system file chooser.
     * @note just downloads the file on MIA_PLATFORM_EMSCRIPTEN (no file chooser)
     *       noop on MIA_PLATFORM_CXXDROID
     */
    oobj event = OEventFile_new(C->view, on_file_downloaded, o_context());
    OEventFile_download(event, TMP_DOWNLOAD_FILE);
}

//
// view functions
//

static void setup(oobj view)
{
    struct context *C = o_user_new0(view, *C, 1);
    C->view = view;


    C->theme = WTheme_new_tiny(view);
    C->gui = WBox_new_v(view);
    WBox_spacing_set(C->gui, vec2_(2));

    oobj title_align = WAlign_new_center_h(C->gui);
    oobj title = WTextShadow_new(title_align, "UP 'N DOWNLOAD\nCOLLAGE CREATOR");
    WText_char_scale_set(title, vec2_(2));

    oobj img_frame = WFrame_new(C->gui);
    C->img_area = WObj_new(img_frame);
    WObj_fixed_size_set(C->img_area, IMG_AREA);


    oobj btn_box_align = WAlign_new_center_h(C->gui);
    oobj btn_box = WBox_new_h(btn_box_align);
    WBox_spacing_set(btn_box, vec2_(8));

    C->upload_btn = WBtn_new(btn_box);
    oobj upload_text = WText_new(C->upload_btn, "UPLOAD");
    WObj_padding_set(upload_text, vec4_(4));
    WBtn_auto_mode_set(C->upload_btn, WBtn_auto_CLICKED);
    WBtn_auto_event_set(C->upload_btn, upload);
    o_user_set(C->upload_btn, C);

    C->download_btn = WBtn_new(btn_box);
    oobj download_text = WText_new(C->download_btn, "DOWNLOAD");
    WObj_padding_set(download_text, vec4_(4));
    WBtn_auto_mode_set(C->download_btn, WBtn_auto_CLICKED);
    WBtn_auto_event_set(C->download_btn, download);
    o_user_set(C->download_btn, C);


    C->img = RTex_new(view, NULL, m_2(IMG_SIZE));

    oobj img = RTex_new_file(view, "$ex/icon16.png");
    create_collage(view,img);
    o_del(img);
}

static void update(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    vec2 center = RCam_center_unit(AView_cam(view));

    WTheme_update(C->theme, C->gui, vec2_(32, center.y-88), vec2_(128,0));
}

static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    RTex_clear_full(tex, vec4_(0.4, 0.5, 0.4, 1.0));
    WTheme_render(C->theme, tex);

    /**
     * Renders the full image above the created area pose
     */
    mat4 pose = WObj_gen_pose(C->img_area);
    mat4 uv = RTex_pose(C->img, 0, 0);
    RTex_blit_pose(tex, C->img, pose, uv);
}


oobj ex_10_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_SAFE);
    return scene;
}

/**
 * Summary:
 *
 * This example shows how to up 'n download files from the user.
 * The uploaded image will be used to create a funny collage in a batched draw call.
 */
