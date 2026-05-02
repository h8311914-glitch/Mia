#include "a/rumble.h"
#include "m/sca/flt.h"

#undef O_LOG_LIB
#define O_LOG_LIB "a"
#include "o/log.h"
#include "o/str.h"

#ifdef MIA_OPTION_GAMEPAD
#include "SDL3/SDL.h"
#endif

#ifdef MIA_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif

#ifdef MIA_PLATFORM_ANDROID
#include <jni.h>
#endif

static struct {
    bool init;
    bool available;
    bool mute;
    float intensity;
} rumble_L;


// protected
void *a_input__sdl_gamepad(void);

// protected
void a_rumble__init(void)
{
    if(rumble_L.init) {
        return;
    }
    rumble_L.intensity = 1.0;
    rumble_L.mute = false;
}

//
// public
//

bool a_rumble_available(void)
{
    // gamepad
#ifdef MIA_OPTION_GAMEPAD
    rumble_L.available = a_input__sdl_gamepad() != NULL;
#endif

#ifdef MIA_PLATFORM_EMSCRIPTEN
    rumble_L.available = true;
#endif

#ifdef MIA_PLATFORM_ANDROID
    rumble_L.available = true;
#endif

    return rumble_L.available;
}

bool a_rumble_mute(void)
{
    return rumble_L.mute;
}

bool a_rumble_mute_set(bool set)
{
    return rumble_L.mute = set;
}

float a_rumble_intensity(void)
{
    return rumble_L.intensity;
}

float a_rumble_intensity_set(float intensity)
{
    rumble_L.intensity = intensity;
    return intensity;
}

void a_rumble_ex(float intensity_low_freq, float intensity_high_freq, float time)
{
    if (rumble_L.mute) {
        return;
    }
    intensity_low_freq = m_clamp(intensity_low_freq * rumble_L.intensity, 0, 1);
    intensity_high_freq = m_clamp(intensity_high_freq * rumble_L.intensity, 0, 1);

#ifdef MIA_OPTION_GAMEPAD
    void *sdl_gamepad = a_input__sdl_gamepad();
    if (sdl_gamepad) {
        ou16 low = ou16_MAX * intensity_low_freq;
        ou16 high = ou16_MAX * intensity_high_freq;
        ou32 duration_ms = time * 1000.0f;
        SDL_RumbleGamepad(sdl_gamepad, low, high, duration_ms);
        return;
    }
#endif

#ifdef MIA_PLATFORM_EMSCRIPTEN
    {
        char script[128];
        ou32 duration_ms = time * 1000.0f;
        o_strf_buf(script, "a__rumble(%i);", duration_ms);
        emscripten_run_script(script);
        return;
    }
#endif

#ifdef MIA_PLATFORM_ANDROID
    {
        float intensity = m_mix(intensity_low_freq, intensity_high_freq, 0.5);

        JNIEnv *env = NULL;
        jobject activity = NULL;
        jclass clazz = NULL;

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
                                                        "aRumble",
                                                        "(FF)V");
        if (!method_id) {
            o_log_error_s(__func__, "method not found");
            goto JNI_CLEAN_UP;
        }

        (*env)->CallStaticVoidMethod(env, clazz, method_id, intensity, time);

        JNI_CLEAN_UP:
        if (env) {
            if (activity) (*env)->DeleteLocalRef(env, activity);
            if (clazz) (*env)->DeleteLocalRef(env, clazz);
        }
    }
#endif

}

/* Pseudo android function:

private static Vibrator aRumble_vibrator;

void aRumble(float intensity, float time) {
    if (aRumble_vibrator == null) {
        aRumble_vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        if (aRumble_vibrator == null) {
            return;
        }
    }

    int duration_ms = (int)(time * 1000.0f);
    int amplitude = (int)(intensity * 255.0f);

    // If amplitude is below 1 → stop vibration immediately
    if (amplitude < 1) {
        aRumble_vibrator.cancel();
        return;
    }

    if (amplitude > 255) {
        amplitude = 255; // clamp max
    }

    aRumble_vibrator.vibrate(
        VibrationEffect.createOneShot(duration_ms, amplitude)
    );
}
*/
