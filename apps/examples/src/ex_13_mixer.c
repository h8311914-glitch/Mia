/**
 * This example demonstrates how to work with advacned audio stuff, that goes beyond a simple buf to be played.
 * Custom mixer with STrackMix.
 * Generating sounds with STrackGen.
 * Applying SFilters.
 * Recording from the microphone.
 *
 * @note for the AndroidStudio port, 
 *       uncomment / enable microphone support in the Android Manifest file.
 */

#include "mia.h"

/**
 * In the examples we have a bunch of generators such as wave, saw, noise, ...
 * Each one using this to store runtime settings.
 */
struct track_source {
    oobj track;
    oobj gain;
    oobj freq;
};

struct context {
    /**
     * This example is scrollable
     */
    struct u_scroll scroll;

    /**
     * STrackMix as mixer for this example.
     * Mixes all STrack children allocated on that (so also StrackMix children).
     * One could create a mixer for say a boss enemy and bundle music and sounds in it.
     * Using its filters and gain on it to amplify, etc. all its sound effects at once.
     */
    oobj mixer;

    /**
     * Custom SFilter that captures the last played audio block for rendering it
     */
    oobj capture_filter;

    oobj theme;
    oobj gui;

    /**
     * On that RTex the capture_filter renders the current played audio
     */
    oobj wave_tex;

    /**
     * WObj as placeholder where to blit the wave_tex above the gui
     */
    oobj wave_tex_container;

    /**
     * WNum (x_num) to control the master mixer gain
     */
    oobj master_gain;

    /**
     * Applied convultion filters on the mixer
     */
    oobj conv_filter;
    int conv_filter_mode;
    oobj conv_filter_btn;
    oobj conv_filter_btn_label;
    oobj conv_filter_val;
    oobj conv_filter_val2;
    bool conv_filter_update;

    /**
     * STrackGen runtime values
     */
    struct track_source srcs[STrackGen_ENUM_MAX];

    /**
     * Microphone record and playback SBuf and STrackBuf
     */
    oobj mic_resample;
    bool mic_recording;
    oobj mic_buf;
    oobj mic_track_ptr;
    oobj mic_btn;
    oobj mic_gain;
    oobj mic_speed;
};

/**
 * Custom SFilter function that renders the last S_BOCK_SIZE of samples into the RTex wave_tex
 */
void mixer_capture(oobj obj, float *restrict data, oi64 frames, struct s_spec spec)
{
    struct context *C = o_user(obj);

    RTex_clear_full(C->wave_tex, R_BLACK);
    oi64 shift_to_block = frames - S_BLOCK_SIZE;
    if (shift_to_block>0) {
        frames = S_BLOCK_SIZE;
        data += shift_to_block * spec.channels;
    }
    u_waveform_render(C->wave_tex, data, 0, spec.channels, frames, vec4_(-1), 1, R_WHITE);
}


static void setup(oobj view)
{
    struct context *C = o_user_new0(view, *C, 1);

    /**
     * Opens the mic device manually.
     * Another option is to change the app options in main to auto open the mic as with the audio device.
     * But that way the browser only asks when this example is first called.
     * (Ensure that this is called after the first pointer down event of the app. In this case its guaranteed)
     * Noop if already opened.
     */
    s_mic_device_open();

    C->theme = WTheme_new_tiny(view);
    C->gui = WAlign_new_center_h(view);

    oobj box = WBox_new_v(C->gui);
    WObj_min_size_set(box, vec2_(128));
    WBox_spacing_set(box, vec2_(8));

    /**
     * parent of our mixer is s_audio_trackmix() to play that on the real hardware.
     * parent2 is our view, so if this example is closed, that mixer also gets deleted.
     * NULL as opt_spec to use system spec
     */
    C->mixer = STrackMix_new(s_audio_trackmix(), view, NULL);

    /**
     * we capture from s_audio_trackmix, case the "master gain" is applied after filtering on C->mixer
     * else if C->mixer would be the parent, it wouldn't affect the wave tex
     */
    C->capture_filter = SFilter_new(s_audio_trackmix(), view, mixer_capture);
    o_user_set(C->capture_filter, C);

    WTextShadow_new(box, "PREVIEW:");

    C->wave_tex = RTex_new(view, NULL, 128, 32);
    C->wave_tex_container = WObj_new(box);
    WObj_fixed_size_set(C->wave_tex_container, vec2_(128, 32));

    /**
     * on the emscripten web build, some (mobile) browsers
     *   have scratchy sound (even on the sdl3 examples...)
     * My last test was:
     * Android Chrome:              works fine
     * Android Firefox:             works fine
     * Android Samsung Internet:    scratchy
     */
#ifdef MIA_PLATFORM_EMSCRIPTEN
    WText_new(box, "NOTE:\n"
                      "  SOUND MAY BE SCRATCHY\n"
                      "  ON SOME BROWSERS!");
#endif

    /**
     * x_num is a WNum that opens an XViewText to edit the value if pressed on it
     */
    C->master_gain = x_num(box, 0.0, 1.0, 0.01, "MASTER GAIN");
    WNum_num_set(C->master_gain, 0.5);
    WDrag_progress_color_x_set(WNum_drag(C->master_gain), vec4_(0.8, 0.1, 0.1, 1.0));

    oobj frame = WFrame_new_title(box, "CONV FILTER", NULL);
    oobj frame_box = WBox_new_v(frame);
    WObj_padding_set(frame_box, vec4_(4));
    WBox_spacing_set(frame_box, vec2_(2));

    C->conv_filter = SFilterConv_new(C->mixer, NULL, 128);
    C->conv_filter_btn = WBtn_new(frame_box);
    C->conv_filter_btn_label = WText_new(C->conv_filter_btn, "");
    WObj_padding_set(C->conv_filter_btn_label, vec4_(2));
    C->conv_filter_val = x_num(frame_box, 0.1, 10.0, 0.1, "VAL");
    WNum_num_set(C->conv_filter_val, 1.0);
    C->conv_filter_val2 = x_num(frame_box, 0.1, 10.0, 0.1, "VAL 2");
    WNum_num_set(C->conv_filter_val2, 1.0);
    C->conv_filter_update = true;

    WTextShadow_new(box, "SOURCES:\n"
                    "  USE GAIN TO ENABLE");

    for (int i = 0; i < STrackGen_ENUM_MAX; i++) {
        struct track_source *src = &C->srcs[i];
        src->track = STrackGen_new(C->mixer, NULL, i, 200, NULL);

        const char *title = (const char *[]){
            "WAVE",
            "RAMP",
            "SAW",
            "BLOCK",
            "NOISE"
        }[i];
        frame = WFrame_new_title(box, title, NULL);
        frame_box = WBox_new_v(frame);
        WObj_padding_set(frame_box, vec4_(4));
        WBox_spacing_set(frame_box, vec2_(2));

        src->gain = x_num(frame_box, 0.0, 1.0, 0.01, "GAIN");
        src->freq = x_num(frame_box, 20.0, 2000.0, 0.1, "FREQ");

        WDrag_progress_color_x_set(WNum_drag(src->gain), vec4_(0.6, 0.1, 0.1, 1.0));
        WNum_num_set(src->freq, m_random_range(20, 2000));
    }

    /**
     * noise ignores freq(uency)
     */
    WObj_hide_set(C->srcs[STrackGen_NOISE].freq, true);

    /**
     * If mic is available, show record button and stuff
     */
    if (s_mic_device_active()) {
        /**
         * Between our STrackMix and our played STrackBuf (from the recorded audio).
         * We use an STrackResample object to change playback speed from resampling from fake frequencies.
         */
        C->mic_resample = STrackResample_new(C->mixer, NULL, NULL);
        STrack_endable_set(C->mic_resample, false);

        frame = WFrame_new_title(box, "MIC", NULL);
        frame_box = WBox_new_v(frame);
        WObj_padding_set(frame_box, vec4_(4));
        WBox_spacing_set(frame_box, vec2_(4));

        C->mic_btn = WBtn_new(frame_box);
        WBtn_auto_mode_set(C->mic_btn, WBtn_auto_CLICKED);
        oobj mic_btn_box = WBox_new_h(C->mic_btn);
        WObj_padding_set(mic_btn_box, vec4_(2));
        WBox_spacing_set(mic_btn_box, vec2_(8));
        oobj icon = WIcon_new(mic_btn_box, WTheme_ICON_RECORD);
        WIcon_color_set(icon, R_RED);
        oobj btn_label = WText_new(mic_btn_box, "HOLD TO RECORD");
        // padding top to center with the icon
        WObj_padding_ref(btn_label)->v1 = 1;

        C->mic_gain = x_num(frame_box, 0.0, 1.0, 0.01, "GAIN");
        C->mic_speed = x_num(frame_box, 0.5, 2.0, 0.01, "SPEED");
        WNum_num_set(C->mic_speed, 1.0);
    } else {
        WTextShadow_new(box, "MIC NOT ACTIVE");
    }

    C->scroll = u_scroll_new(u_scroll_Y, AView_cam(view));
}

static void update(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    vec2 size = WTheme_update_full_tex(C->theme, C->gui, tex);

    /**
     * Set scroll limits to used gui size
     */
    C->scroll.cam_limits_rect = u_rect_new(0, -32, 0, size.y + 64);
    u_scroll_update(&C->scroll, dt);

    float master_gain = WNum_num(C->master_gain);
    STrackMix_gain_set(C->mixer, master_gain);

    for (int i = 0; i < STrackGen_ENUM_MAX; i++) {
        struct track_source *src = &C->srcs[i];

        float gain = WNum_num(src->gain);
        float freq = WNum_num(src->freq);

        STrackGen_gain_set(src->track, gain);
        STrackGen_freq_set(src->track, freq);
    }

    if (WBtn_clicked(C->conv_filter_btn)) {
        C->conv_filter_mode = (C->conv_filter_mode + 1) % 6;
        C->conv_filter_update = true;
    }

    if (C->conv_filter_update) {
        WText_text_set(C->conv_filter_btn_label, (const char *[]){
                           "EYE",
                           "BLUR",
                           "BLUR_HIGH",
                           "GAUSS",
                           "GAUSS_HIGH",
                           "GAUSS_BAND"
                       }[C->conv_filter_mode]);
        WObj_hide_set(C->conv_filter_val, C->conv_filter_mode == 0);
        WObj_hide_set(C->conv_filter_val2, C->conv_filter_mode != 5);
        float val = WNum_num(C->conv_filter_val);
        float val2 = WNum_num(C->conv_filter_val2);
        float *kernel = SFilterConv_kernel(C->conv_filter);
        int kernel_size = SFilterConv_kernel_size(C->conv_filter);

        /** functions of: "m/utils/kernel.h" */
        switch (C->conv_filter_mode) {
            default:
            case 0:
                m_kernel_eye(kernel, kernel_size);
                break;
            case 1:
                m_kernel_blur(kernel, val / 10.0f * kernel_size, 1.0, kernel_size);
                break;
            case 2:
                m_kernel_blur_high(kernel, val / 10.0f * kernel_size, 1.0, kernel_size);
                break;
            case 3:
                m_kernel_gauss(kernel, val, 1.0, kernel_size);
                break;
            case 4:
                m_kernel_gauss_high(kernel, val, 1.0, kernel_size);
                break;
            case 5:
                /** the math band function needs an additional set of floats as tmp for calculation */
                m_kernel_gauss_band(kernel, o_new(o_arena(), float, kernel_size), val, val2, kernel_size);
                break;
        }
    }

    if (C->mic_btn) {
        /**
         * The mixer may delete tracks, so always use OPtr or search in the tree!
         * In our special case with inf loops, that won`t happen,
         *      but its best practice to just don`t depend on that fact
         */
        oobj mic_track = C->mic_track_ptr ? OPtr_get(C->mic_track_ptr).o : NULL;

        if (WBtn_down(C->mic_btn) && !C->mic_recording) {
            C->mic_recording = true;
            o_log("start recording...");
            o_del(C->mic_buf);
            C->mic_buf = s_mic_buf_new(view);
            s_mic_record();
        }
        if (!WBtn_down(C->mic_btn) && C->mic_recording) {
            C->mic_recording = false;
            o_log("stop recording and play...");
            s_mic_pause();

            /**
             * Normalizing the recorded buffer.
             * Using rms = root mean squared so its less single peak dependent
             */
            oobj buf_arr = SBuf_array(C->mic_buf);
            s_normalize_rms(
                    OArray_data_void(buf_arr),
                    SBuf_spec(C->mic_buf),
                    o_num(buf_arr), 0, 1);

            /**
             * o_del is NULL safe...
             */
            o_del(mic_track);
            mic_track = STrackBuf_new(C->mic_resample, NULL, C->mic_buf, 0);
            STrackBuf_loops_set(mic_track, -1);
            o_del(C->mic_track_ptr);

            /**
             * Played tracks may be deleted by the mixer.
             * So
             */
            C->mic_track_ptr = OPtr_new(view, mic_track);
        }

        if (mic_track) {
            float gain = WNum_num(C->mic_gain);
            STrackBuf_gain_set(mic_track, gain);
        }

        /**
         * Speed up or slow down the recorded buffer with setting a different frequency in the inbetween resampler track
         */
        float speed = WNum_num(C->mic_speed);
        int freq = s_spec_default().freq;
        freq *= speed;
        STrackResample_freq_set(C->mic_resample, freq);
    }
}

static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    RTex_clear_full(tex, vec4_(0.2, 0.3, 0.6, 1.0));

    WTheme_render(C->theme, tex);

    vec2 wave_lt = WObj_gen_lt(C->wave_tex_container);
    RTex_blit(tex, C->wave_tex, m_2(wave_lt));
}


oobj ex_13_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_UNSAFE);
    return scene;
}


/**
 * Summary:
 * This example had shown some more advanced audio techniques like:
 * - mixing
 * - using filters
 * - recording from mic
 */
