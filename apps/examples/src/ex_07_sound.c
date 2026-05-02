/**
 * Load and play sound effects and music.
 * Render a waveform into an RTex to be displayed.
 */


#include "mia.h"

struct context {
    /**
     * widget stuff
     */
    oobj theme;
    oobj gui;
    oobj wave_area;
    oobj play_btn;
    oobj echo_btn;


    /**
     * SBuf for the sound to be played
     */
    oobj sound;

    /**
     * RTex to render the generated waveform
     */
    oobj waveform;
};


static void setup(oobj view)
{
    struct context *C = o_user_new0(view, *C, 1);
    

    C->theme = WTheme_new_tiny(view);
    C->gui = WBox_new_v(view);
    WBox_spacing_set(C->gui, vec2_(8));

    oobj text = WTextShadow_new(C->gui, "BALLOON SOUND");
    WText_char_scale_set(text, vec2_(2));

    oobj frame = WFrame_new_title_shadow(C->gui, "WAVE", NULL);

    /**
     * Simple WObj which acts as a placeholder to render the RTex waveform on (see below).
     * The widget's are all rendered in a single draw call with the WTheme texture.
     * It is possible to render the waveform into the texture of WTheme (has custom fields).
     * But this way is more flexible.
     */
    C->wave_area = WObj_new(frame);
    WObj_fixed_size_set(C->wave_area, vec2_(128, 64));

    oobj btn_box = WBox_new_h(C->gui);
    WBox_spacing_set(btn_box, vec2_(32));

    /**
     * Button to play the sound
     */
    C->play_btn = WBtn_new(btn_box);
    oobj icon = WIcon_new(C->play_btn, WTheme_ICON_PLAY);
    WIcon_color_set(icon, vec4_(0.1, 0.8, 0.1, 1.0));
    WObj_padding_set(icon, vec4_(4));
    

    /**
     * Button to play the sound with an installed echo filter
     */
    C->echo_btn = WBtn_new(btn_box);
    icon = WIcon_new(C->echo_btn, WTheme_ICON_FF);
    WIcon_color_set(icon, vec4_(0.1, 0.8, 0.1, 1.0));
    WObj_padding_set(icon, vec4_(4));
    

    // on the emscripten web build, some (mobile) browsers
    //   have scratchy sound (even on the sdl3 examples...)
    // My last test was:
    // Android Chrome:              works fine
    // Android Firefox:             works fine
    // Android Samsung Internet:    scratchy
#ifdef MIA_PLATFORM_EMSCRIPTEN
    WText_new(C->gui, "NOTE:\n"
                      "  SOUND MAY BE SCRATCHY\n"
                      "  ON SOME BROWSERS!");
#endif

    /**
     * Loads a sound file (.wav also exists) into an OArray.
     * When only the STrack is needed and not the raw data, s_ogg_load_track is here for you.
     * As with other sound functions and objects, passing NULL as spec uses the system specifications.
     * Edited from:
     * License: Creative Commons 0
     * https://freesound.org/people/hellointernet02/sounds/321900/
     */
    oobj sound = s_ogg_load_array(view, "$ex/balloon.ogg", NULL).o;
    assert(sound && "failed to load sound");
    float *sound_data = o_at(sound, 0);
    osize sound_ticks = o_num(sound);

    /**
     * Create the SBuf from the raw data (s_ogg_load_track also exists, but needs an heap allocated array)
     */
    C->sound = SBuf_new(view, sound_data, sound_ticks, NULL);

    /**
     * Creates a texture for the waveform with 128 cols x 64 rows.
     */
    C->waveform = RTex_new(view, NULL, 128, 64);
    RTex_clear_full(C->waveform, R_BLUE);

    /**
     * Render the waveform lines in green onto the texture
     */
    u_waveform_render(C->waveform, sound_data, 0, 1, (int) sound_ticks,
                      RTex_rect(C->waveform, 0, 0), 1, R_GREEN);

    /**
     * We can also write textures to disk with this function. (As a temporary file: mia_tmp.ex_06_sound_wave.png)
     */
    RTex_write_file(C->waveform, "#ex_06_sound_wave.png");

    /**
     * Free / delete the OArray of the raw sound data. Not needed anymore
     */
    o_del(sound);
}

static void update(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    WTheme_update(C->theme, C->gui, vec2_(16, 32), vec2_(0));

    if(WBtn_clicked(C->play_btn)) {
        o_log("play balloon sound");

        /**
         * Plays the balloon sound on the hardware sound device.
         * In 0.0 seconds (immediatly).
         * With a gain of 1.0 (amplification) (default).
         */
        s_play(C->sound, 0.0f, 1.0f);
    
    }
    
    if(WBtn_clicked(C->echo_btn)) {
        o_log("play echo filtered sound");

        /**
         * s_play creates an STrackBuf installed on the s_audio_trackmix mixer (STrackMix).
         * After beeing played that returned STrackBuf is auto deleted by the mixer.
         * This system works like a tree, cause STrackMix just mixes all types of STrack (so also a submixer).
         * Filter and gain can be applied to STrackBuf and STrackMix.
         * SFilterFade for example can fade in or out the amplitude.
         * STrack's can also be used to filter and mix sound on the memory instead in the output audio channel.
         * Just build up the tree and run STrack_pull.
         *
         * In this example we take the returned STrackBuf and install an echo filter on it.
         *    (use the returned STrackBuf with caution, may be deleted by the mixer in the upcomming frames)
         * The echo is remixed into the track at 0.33 seconds later.
         * SFilter mainly can be applied on StrackMix or STrackBuf's.
         * But the SFilterEcho is an exception. Installing a filter on that filters the echo sound before mixing back in.
         * So we make use of that and apart from setting the echo gain to 50% amplitude,
         *     we create a convolution filter with a gauss kernel (sigma auto (-1)) as a low pass filter for the echo.
         *
         * If the track finished, all these filters get auto deleted as well.
         * For even more advanced audio api stuff (like using the microphone), see the mixer example.
         */
        oobj track = s_play(C->sound, 0.0f, 1.0f);
        oobj filter = SFilterEcho_new_time(track, NULL, 0.33);
        SFilterEcho_gain_set(filter, 0.5);
        oobj gauss = SFilterConv_new(filter, NULL, 16);
        /** function to setup a kernel from: "m/utils/kernel.h" */
        m_kernel_gauss(SFilterConv_kernel(gauss), -1, 1.0, 16);
    }
}

static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    RTex_clear_full(tex, R_GRAY_X(0.33));
    WTheme_render(C->theme, tex);

    /**
     * Retrieve the generated waveform area and blit the waveform tex onto the resulting view tex.
     */
    mat4 pose = WObj_gen_pose(C->wave_area);
    mat4 uv = RTex_pose(C->waveform, 0, 0);
    RTex_blit_pose(tex, C->waveform, pose, uv);
}


oobj ex_07_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_SAFE);
    return scene;
}

/**
 * Summary:
 *
 * This example shows how simple it is to play and filter audio files.
 * It also showed how to create a waveform and save it and mix widgets with plain render strategies.
 * For more advanced audio stuff, see thr mixer example.
 */
