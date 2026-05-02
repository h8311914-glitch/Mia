#ifdef MIA_OPTION_SDL2
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_audio.h>
#else
#include <SDL3/SDL_audio.h>
#endif

#include "s/common.h"
#include "o/OObjRoot.h"
#include "o/ODelcallback.h"
#include "o/OArray.h"
#include "o/OList.h"
#include "o/OPtr.h"
#include "m/vec/vecn.h"
#include "s/SBuf.h"
#include "s/STrackBuf.h"
#include "s/STrackMix.h"
#include "s/SResampler.h"

#undef O_LOG_LIB
#define O_LOG_LIB "s"

#include "o/log.h"



static struct {
    bool init;
    oobj root;
    struct s_spec_ex audio_ex;
    struct s_spec_ex mic_ex;

    // from init
    bool audio_enabled;
    bool mic_enabled;

    // if device is open
    bool audio_active;
    bool mic_active;

    // in that array the mic callbacks writes blocks locked on that object
    // cleared on after update
    oobj mic_block_array;

    // Swap of the last mic_block_array on frame end, same as the stuff appended into all the mic bufs
    // (double buffered)
    oobj mic_block_array_frame;

    // OList of OPtr of SBuf's to copy the mic data into
    oobj mic_bufs;

#ifdef MIA_OPTION_SDL2
    SDL_AudioDeviceID audio_sdl_device;
    SDL_AudioDeviceID mic_sdl_device;
#else
    SDL_AudioStream *audio_sdl_stream;
    SDL_AudioStream *mic_sdl_stream;
#endif


    // STrackMix of the real hardware
    oobj trackmix;

    // output ring buffer
    // see S_BUFFER_MIN_TIME and S_BUFFER_MAX_TIME and s_audio_buffer_fill
    oobj audio_buffer_lock;
    float *audio_buffer_pull_tmp;
    float **audio_buffer_ring;
    int audio_buffer_ring_blks;
    int audio_buffer_ring_pos;
    int audio_buffer_ring_size;

} s_common_L;



#ifdef MIA_OPTION_SDL2
static void audio_callback(void *userdata, Uint8 *stream, int len);
static void mic_callback(void *userdata, Uint8 *stream, int len);
#else
static void audio_callback(void *userdata, SDL_AudioStream *astream, int additional_amount, int total_amount);
#endif


//
// protected
//

// protected
SDL_AudioFormat s__format_to_sdl(enum s_format format);
enum s_format s__format_from_sdl(SDL_AudioFormat format);

#ifdef MIA_OPTION_SDL2
SDL_AudioFormat s__format_to_sdl(enum s_format format)
{
    switch (format) {
        case S_FORMAT_INVALID:
            break;
        case S_FORMAT_U8:
            return AUDIO_U8;
        case S_FORMAT_S8:
            return AUDIO_S8;
        case S_FORMAT_S16LE:
            return AUDIO_S16LSB;
        case S_FORMAT_S16BE:
            return AUDIO_S16MSB;
        case S_FORMAT_S16:
            return AUDIO_S16SYS;
        case S_FORMAT_S32LE:
            return AUDIO_S32LSB;
        case S_FORMAT_S32BE:
            return AUDIO_S32MSB;
        case S_FORMAT_S32:
            return AUDIO_S32SYS;
        case S_FORMAT_F32LE:
            return AUDIO_F32LSB;
        case S_FORMAT_F32BE:
            return AUDIO_F32MSB;
        case S_FORMAT_F32:
            return AUDIO_F32SYS;
        default:
            break;
    }
    return 0;
}
enum s_format s__format_from_sdl(SDL_AudioFormat format)
{
    switch (format) {
        case AUDIO_U8:
            return S_FORMAT_U8;
        case AUDIO_S8:
            return S_FORMAT_S8;
        case AUDIO_S16LSB:
            return S_FORMAT_S16LE;
        case AUDIO_S16MSB:
            return S_FORMAT_S16BE;
        case AUDIO_S32LSB:
            return S_FORMAT_S32LE;
        case AUDIO_S32MSB:
            return S_FORMAT_S32BE;
        case AUDIO_F32LSB:
            return S_FORMAT_F32LE;
        case AUDIO_F32MSB:
            return S_FORMAT_F32BE;
    }
    return S_FORMAT_INVALID;
}
#else
// SDL3
SDL_AudioFormat s__format_to_sdl(enum s_format format)
{
    switch (format) {
        case S_FORMAT_INVALID:
            break;
        case S_FORMAT_U8:
            return SDL_AUDIO_U8;
        case S_FORMAT_S8:
            return SDL_AUDIO_S8;
        case S_FORMAT_S16LE:
            return SDL_AUDIO_S16LE;
        case S_FORMAT_S16BE:
            return SDL_AUDIO_S16BE;
        case S_FORMAT_S16:
            return SDL_AUDIO_S16;
        case S_FORMAT_S32LE:
            return SDL_AUDIO_S32LE;
        case S_FORMAT_S32BE:
            return SDL_AUDIO_S32BE;
        case S_FORMAT_S32:
            return SDL_AUDIO_S32;
        case S_FORMAT_F32LE:
            return SDL_AUDIO_F32LE;
        case S_FORMAT_F32BE:
            return SDL_AUDIO_F32BE;
        case S_FORMAT_F32:
            return SDL_AUDIO_F32;
        default:
            break;
    }
    return SDL_AUDIO_UNKNOWN;
}
enum s_format s__format_from_sdl(SDL_AudioFormat format)
{
    switch (format) {
        case SDL_AUDIO_U8:
            return S_FORMAT_U8;
        case SDL_AUDIO_S8:
            return S_FORMAT_S8;
        case SDL_AUDIO_S16LE:
            return S_FORMAT_S16LE;
        case SDL_AUDIO_S16BE:
            return S_FORMAT_S16BE;
        case SDL_AUDIO_S32LE:
            return S_FORMAT_S32LE;
        case SDL_AUDIO_S32BE:
            return S_FORMAT_S32BE;
        case SDL_AUDIO_F32LE:
            return S_FORMAT_F32LE;
        case SDL_AUDIO_F32BE:
            return S_FORMAT_F32BE;
        default:
            break;
    }
    return S_FORMAT_INVALID;
}
#endif


//
// private
//


static void s__audio_device_open(void)
{
    if (s_common_L.audio_active) {
        // noop
        return;
    }

    // Open audio device with desired parameters
#ifdef MIA_OPTION_SDL2
    SDL_AudioSpec want = {0};
    want.freq = s_common_L.audio_ex.spec.freq;
    want.format = AUDIO_F32;
    want.channels = s_common_L.audio_ex.spec.channels;
    want.samples = S_BLOCK_SIZE;
    want.callback = audio_callback;
    SDL_AudioSpec have;
    s_common_L.audio_sdl_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    if (s_common_L.audio_sdl_device == 0) {
        o_log_error_s("s_open_devices", "Failed to open audio device: %s", SDL_GetError());
        return;
    }
    if (have.freq != want.freq) {
        o_log_error_s(__func__, "Invalid opened frequency!: %i/%i", have.freq, want.freq);
        return;
    }
    if (have.format != want.format) {
        o_log_error_s(__func__, "Invalid opened format!: %i/%i", have.format, want.format);
        return;
    }
    if (have.channels != want.channels) {
        o_log_error_s(__func__, "Invalid number of channels!: %i/%i", have.channels, want.channels);
        return;
    }
    if (have.samples != want.samples) {
        o_log_error_s(__func__, "Invalid number of samples!: %i/%i", have.samples, want.samples);
        return;
    }
    o_log_s("s_open_devices", "Opened audio with: %i Hz; %i channels; %i samples", have.freq, have.channels, have.samples);
#else
    // SDL3
    SDL_AudioSpec spec;
    spec.channels = s_common_L.audio_ex.spec.channels;
    spec.format = SDL_AUDIO_F32;
    spec.freq = s_common_L.audio_ex.spec.freq;
    s_common_L.audio_sdl_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audio_callback, NULL);
    if (!s_common_L.audio_sdl_stream) {
        o_log_error_s("s_open_devices", "Failed to open audio device: %s", SDL_GetError());
        return;
    }
    o_log_s("s_open_devices", "Opened audio with: %i Hz; %i channels", spec.freq, spec.channels);
#endif

    s_common_L.audio_active = true;

    // Start audio playback
    s_audio_play();
}


static void s__mic_device_open(void)
{
    if (s_common_L.mic_active) {
        // noop
        return;
    }

    // Open audio device with desired parameters
#ifdef MIA_OPTION_SDL2
    SDL_AudioSpec want = {0};
    want.freq = s_common_L.mic_ex.spec.freq;
    want.format = AUDIO_F32;
    want.channels = s_common_L.mic_ex.spec.channels;
    want.samples = S_BLOCK_SIZE;
    want.callback = mic_callback;
    SDL_AudioSpec have;
    s_common_L.mic_sdl_device = SDL_OpenAudioDevice(NULL, 1, &want, &have, 0);

    if (s_common_L.mic_sdl_device == 0) {
        o_log_error_s("s_open_devices", "Failed to open mic   device: %s", SDL_GetError());
        return;
    }
    if (have.freq != want.freq) {
        o_log_error_s(__func__, "Invalid opened frequency!: %i/%i", have.freq, want.freq);
        return;
    }
    if (have.format != want.format) {
        o_log_error_s(__func__, "Invalid opened format!: %i/%i", have.format, want.format);
        return;
    }
    if (have.channels != want.channels) {
        o_log_error_s(__func__, "Invalid number of channels!: %i/%i", have.channels, want.channels);
        return;
    }
    if (have.samples != want.samples) {
        o_log_error_s(__func__, "Invalid number of samples!: %i/%i", have.samples, want.samples);
        return;
    }
    o_log_s("s_open_devices", "Opened mic   with: %i Hz; %i channels; %i samples", have.freq, have.channels, have.samples);
#else
    // SDL3
    SDL_AudioSpec spec;
    spec.channels = s_common_L.mic_ex.spec.channels;
    spec.format = SDL_AUDIO_F32;
    spec.freq = s_common_L.mic_ex.spec.freq;
    // s_common_L.mic_sdl_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &spec, mic_callback, NULL);
    s_common_L.mic_sdl_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &spec, NULL, NULL);
    if (!s_common_L.mic_sdl_stream) {
        o_log_error_s("s_open_devices", "Failed to open mic   device: %s", SDL_GetError());
        return;
    }
    o_log_s("s_open_devices", "Opened mic   with: %i Hz; %i channels", spec.freq, spec.channels);
#endif

    s_common_L.mic_active = true;

    // Pauses audio recording
    s_mic_pause();
}


//
// public
//


osize s_format_size(enum s_format format)
{
    switch (format) {
        case S_FORMAT_U8:
        case S_FORMAT_S8:
            return 1;
        case S_FORMAT_S16LE:
        case S_FORMAT_S16BE:
        case S_FORMAT_S16:
            return 2;
        case S_FORMAT_S32LE:
        case S_FORMAT_S32BE:
        case S_FORMAT_S32:
        case S_FORMAT_F32LE:
        case S_FORMAT_F32BE:
        case S_FORMAT_F32:
            return 4;
        default:
            break;
    }
    assert(0 && "invalid audio format");
    return 0;
}

struct s_spec_ex s_spec_ex_default(void)
{
    return (struct s_spec_ex){
        {
            48000,
            1
        },
        S_FORMAT_F32
    };
}


osize s_spec_ex_buffer_size(struct s_spec_ex spec, osize len)
{
    return s_spec_array_size(spec.spec, len) * s_format_size(spec.format);
}

void s_init(bool audio_enable, const struct s_spec *opt_audio_spec,
            bool mic_enable, const struct s_spec *opt_mic_spec)
{
    if (s_common_L.init) {
        o_log_error_s(__func__, "s_init called already!");
        return;
    }
    s_common_L.init = true;

    s_common_L.audio_enabled = audio_enable;
    s_common_L.mic_enabled = mic_enable;



    s_common_L.audio_ex = s_spec_ex_default();

    if (opt_audio_spec) {
        s_common_L.audio_ex.spec = *opt_audio_spec;
    }

    s_common_L.mic_ex = s_common_L.audio_ex;
    s_common_L.mic_ex.spec.channels = 1;
    if (opt_mic_spec) {
        s_common_L.mic_ex.spec = *opt_mic_spec;
    }

    if (s_common_L.audio_ex.spec.channels > S_CHANNELS_MAX) {
        o_log_warn_s(__func__, "limiting channels from %i to %i", s_common_L.audio_ex.spec.channels, S_CHANNELS_MAX);
        s_common_L.audio_ex.spec.channels = S_CHANNELS_MAX;
    } else if (s_common_L.audio_ex.spec.channels < 1) {
        s_common_L.audio_ex.spec.channels = 1;
    }
    if (s_common_L.mic_ex.spec.channels > S_CHANNELS_MAX) {
        o_log_warn_s(__func__, "limiting mic channels from %i to %i", s_common_L.mic_ex.spec.channels, S_CHANNELS_MAX);
        s_common_L.mic_ex.spec.channels = S_CHANNELS_MAX;
    } else if (s_common_L.mic_ex.spec.channels < 1) {
        s_common_L.mic_ex.spec.channels = 1;
    }
    
    s_common_L.root = OObjRoot_new_heap("[mia]sound");
    ODelcallback_new_assert(s_common_L.root, "[mia]sound", "deleted!");

    if (audio_enable) {
        s_common_L.trackmix = STrackMix_new(s_common_L.root, NULL, NULL);
        OObj_name_set(s_common_L.trackmix, "s_audio_trackmix");
        ODelcallback_new_assert(s_common_L.trackmix, "s_audio_trackmix", "deleted!");
        OObj_threadsafe_set(s_common_L.trackmix, true);

        int ring_blks = m_ceil(S_BUFFER_MAX_TIME * s_common_L.audio_ex.spec.freq / S_BLOCK_SIZE);
        osize ring_floats = ring_blks * s_common_L.audio_ex.spec.channels * S_BLOCK_SIZE;
        float *ring_buffer = o_new0(s_common_L.root, float, ring_floats * 2);
        s_common_L.audio_buffer_lock = OObj_new(s_common_L.root);
        s_common_L.audio_buffer_pull_tmp = ring_buffer + ring_floats;
        s_common_L.audio_buffer_ring = o_new(s_common_L.root, float*, ring_blks);
        for (int i=0; i<ring_blks; i++) {
            s_common_L.audio_buffer_ring[i] = ring_buffer + i * s_common_L.audio_ex.spec.channels * S_BLOCK_SIZE;
        }
        s_common_L.audio_buffer_ring_blks = ring_blks;
    }

    if (mic_enable) {
        s_common_L.mic_block_array = OArray_new_dyn(s_common_L.root, NULL, sizeof(float)*s_common_L.mic_ex.spec.channels, 0, 32 * S_BLOCK_SIZE);
        s_common_L.mic_block_array_frame = OArray_new_dyn(s_common_L.root, NULL, sizeof(float)*s_common_L.mic_ex.spec.channels, 0, 32 * S_BLOCK_SIZE);
        s_common_L.mic_bufs = OList_new(s_common_L.root, NULL, 0);
    }

}

oobj s_root(void)
{
    return s_common_L.root;
}

struct s_spec s_audio_device_spec(void)
{
    return s_common_L.audio_ex.spec;
}

struct s_spec s_mic_device_spec(void)
{
    return s_common_L.mic_ex.spec;
}

void s_update(void)
{
    if (s_common_L.audio_active) {
        s_audio_buffer_fill(S_BUFFER_MIN_TIME);
    }

    if (s_common_L.mic_active) {

        // we lock both here, case we swap...
        // only needed for SDL2 callback style
#ifdef MIA_OPTION_SDL2
        o_lock(s_common_L.mic_block_array_frame);
        o_lock(s_common_L.mic_block_array);
#else
        // sdl3 does not need a callback
        int stream_bytes = SDL_GetAudioStreamAvailable(s_common_L.mic_sdl_stream);
        int block_size = sizeof(float) * s_common_L.mic_ex.spec.channels * S_BLOCK_SIZE;
        int stream_blks = stream_bytes / block_size;
        if (stream_blks > 0) {
            float *data = OArray_append(s_common_L.mic_block_array, NULL, stream_blks);
            int blks_bytes = stream_blks * block_size;
            int read_bytes = SDL_GetAudioStreamData(s_common_L.mic_sdl_stream, data, blks_bytes);
            if (o_unlikely(read_bytes != blks_bytes)) {
                o_log_error_s(__func__, "failed to read blocks from mic stream! (%i/%i)", read_bytes, blks_bytes);
            }
        }
#endif

        // swap buffers
        oobj tmp = s_common_L.mic_block_array;
        s_common_L.mic_block_array = s_common_L.mic_block_array_frame;
        s_common_L.mic_block_array_frame = tmp;

        // clear to 0
        OArray_clear(s_common_L.mic_block_array);

        osize blks = OArray_num(s_common_L.mic_block_array_frame);
        if (blks>0) {
            float *data = OArray_data_void(s_common_L.mic_block_array_frame);

            // back to front to easily pop in the loop
            for (osize b=OList_num(s_common_L.mic_bufs)-1; b>=0; b--) {
                oobj ptr = OList_at(s_common_L.mic_bufs, b);
                oobj buf = OPtr_get(ptr).o;
                if (!buf) {
                    o_del(ptr);
                    OList_pop_at(s_common_L.mic_bufs, b);
                    continue;
                }
                oobj array = SBuf_array(buf);

                OArray_append(array, data, S_BLOCK_SIZE*blks);
            }
        }

        // unlock both (case swapped)
#ifdef MIA_OPTION_SDL2
        o_unlock(s_common_L.mic_block_array);
        o_unlock(s_common_L.mic_block_array_frame);
#endif
    }
}

bool s_audio_enabled(void)
{
    return s_common_L.audio_enabled;
}


void s_audio_device_open(void)
{
    if (s_common_L.audio_enabled) {
        s__audio_device_open();
    }
}

bool s_audio_device_active(void)
{
    return s_common_L.audio_active;
}


void s_audio_pause(void)
{
    if (!s_common_L.audio_active) {
        return;
    }
#ifdef MIA_OPTION_SDL2
    SDL_PauseAudioDevice(s_common_L.audio_sdl_device, 1);
#else
    SDL_PauseAudioStreamDevice(s_common_L.audio_sdl_stream);
#endif
}

void s_audio_play(void)
{
    if (!s_common_L.audio_active) {
        return;
    }
#ifdef MIA_OPTION_SDL2
    SDL_PauseAudioDevice(s_common_L.audio_sdl_device, 0);
#else
    SDL_ResumeAudioStreamDevice(s_common_L.audio_sdl_stream);
#endif
}

void s_audio_buffer_fill(float time)
{
    if (!s_common_L.audio_enabled) {
        return;
    }

    time = m_clamp(time, S_BUFFER_MIN_TIME, S_BUFFER_MAX_TIME);
    int wanted_blks = m_ceil(time * s_common_L.audio_ex.spec.freq / S_BLOCK_SIZE);
    wanted_blks = o_clamp(wanted_blks, 0, s_common_L.audio_buffer_ring_blks);

    // we need to lock against the thread that consumes the buffer
    o_lock(s_common_L.audio_buffer_lock);

    int needed_blocks = wanted_blks - s_common_L.audio_buffer_ring_size;
    if (needed_blocks>0) {

        // actual pulling from the trackmix
        // done cached so the mixer only is pulled a single time each frame
        STrack_pull(s_common_L.trackmix, s_common_L.audio_buffer_pull_tmp, needed_blocks * S_BLOCK_SIZE);

        osize block_samples = S_BLOCK_SIZE * s_common_L.audio_ex.spec.channels;

        float *restrict src_it = s_common_L.audio_buffer_pull_tmp;

        while (wanted_blks>s_common_L.audio_buffer_ring_size) {
            int ring_pos = s_common_L.audio_buffer_ring_pos + s_common_L.audio_buffer_ring_size;
            ring_pos %= s_common_L.audio_buffer_ring_blks;
            o_memcpy(s_common_L.audio_buffer_ring[ring_pos], src_it, sizeof(float), block_samples);
            src_it+=block_samples;
            s_common_L.audio_buffer_ring_size++;
            assert(s_common_L.audio_buffer_ring_size <= s_common_L.audio_buffer_ring_blks);
        }
    }

    o_unlock(s_common_L.audio_buffer_lock);
}

oobj s_audio_trackmix(void)
{
    assert(s_common_L.audio_enabled);
    return s_common_L.trackmix;
}


bool s_mic_enabled(void)
{
    return s_common_L.mic_enabled;
}

void s_mic_device_open(void)
{
    if (s_common_L.mic_enabled) {
        s__mic_device_open();
    }
}

bool s_mic_device_active(void)
{
    return s_common_L.mic_active;
}

void s_mic_pause(void)
{
    if (!s_common_L.mic_active) {
        return;
    }
#ifdef MIA_OPTION_SDL2
    SDL_PauseAudioDevice(s_common_L.mic_sdl_device, 1);
#else
    SDL_PauseAudioStreamDevice(s_common_L.mic_sdl_stream);
#endif
}

void s_mic_record(void)
{
    if (!s_common_L.mic_active) {
        return;
    }
#ifdef MIA_OPTION_SDL2
    SDL_PauseAudioDevice(s_common_L.mic_sdl_device, 0);
#else
    SDL_ResumeAudioStreamDevice(s_common_L.mic_sdl_stream);
#endif
}


int s_mic_current_blocks_num(void)
{
    // no need to o_lock, cause its on the main thread
    o_thread_assert_main();
    assert(s_common_L.mic_enabled);
    return OArray_num(s_common_L.mic_block_array_frame);
}

float *s_mic_current_blocks_at(int current_blk)
{
    // no need to o_lock, cause its on the main thread
    o_thread_assert_main();
    assert(s_common_L.mic_enabled);
    return OArray_at_void(s_common_L.mic_block_array_frame, current_blk);
}

oobj s_mic_buf_new(oobj parent)
{
    o_thread_assert_main();
    assert(s_common_L.mic_enabled);
    oobj array = OArray_new_dyn(parent, NULL, sizeof(float)*s_common_L.mic_ex.spec.channels, 0, 128 * S_BLOCK_SIZE);
    oobj buf = SBuf_new_array(parent, array, &s_common_L.mic_ex.spec);
    oobj ptr = OPtr_new(s_common_L.mic_bufs, buf);
    OList_push(s_common_L.mic_bufs, ptr);
    return buf;
}

oobj s_play(oobj buf, double delay_seconds, float gain)
{
    o_thread_assert_main();
    assert(s_audio_device_active() && "device not opened yet!");
    oobj track;
    track = STrackBuf_new(s_common_L.trackmix, NULL, buf,
        s_spec_frames_from_seconds(s_common_L.audio_ex.spec, delay_seconds));
    STrackBuf_gain_set(track, gain);
    return track;
}


//
// audio utils
//


void s_mix_into(float *restrict in_out_data, const float *restrict mix_data, float mix_amp, int channels, oi64 frames)
{
    for (osize i = 0; i < channels * frames; i++) {
        float a = in_out_data[i];
        float b = mix_data[i];
        float mixed = a + b * mix_amp;
        mixed = o_clamp(mixed, -1.0f, +1.0f);
        in_out_data[i] = mixed;
    }
}


void s_resample_ex(void *restrict out_output, struct s_spec_ex output_spec,
                   const void *restrict input, struct s_spec_ex input_spec, oi64 input_frames)
{
    oobj resampler = SResampler_new_ex(s_common_L.root, input_spec, output_spec);
    SResampler_resample(resampler, out_output, input, input_frames);
    o_del(resampler);
}


float s_amplitude_peak(const float *restrict data, struct s_spec spec, oi64 frames)
{
    oi64 samples = frames * spec.channels;
    float amp = 0;
    for(oi64 i=0; i<samples; i++) {
        float a = m_abs(data[i]);
        amp = m_max(amp, a);
    }
    return amp;
}

float s_amplitude_rms(const float *restrict data, struct s_spec spec, oi64 frames)
{
    oi64 samples = frames * spec.channels;
    float amp = 0;
    for(oi64 i=0; i<samples; i++) {
        amp += data[i]*data[i];
    }
    return m_sqrt(amp/samples);
}

float s_normalize_peak(float *restrict data, struct s_spec spec, oi64 frames, float amp_min, float gain)
{
    amp_min = amp_min>0? amp_min : 0.1; 
    float amp = s_amplitude_peak(data, spec, frames);
    amp = o_max(amp, amp_min);
    oi64 samples = frames * spec.channels;
    vecn_scale(data, data, gain/amp, samples);
    return amp;
}

float s_normalize_rms(float *restrict data, struct s_spec spec, oi64 frames, float amp_min, float gain)
{
    amp_min = amp_min>0? amp_min : 0.1; 
    float amp = s_amplitude_rms(data, spec, frames);
    amp = o_max(amp, amp_min);
    oi64 samples = frames * spec.channels;
    vecn_scale(data, data, gain/amp, samples);
    return amp;
}


//
// callbacks
//


// forwarded
#ifdef MIA_OPTION_SDL2
void audio_callback(void *userdata, Uint8 *restrict stream, int stream_bytes)
{
    if (o_thread_id() != o_thread_main_id()) {
        o_log_level_set(O_LOG_INFO);
    }
    int channels = s_common_L.audio_ex.spec.channels;
    float *data = (float *) stream;
    oi64 frames = (oi64) stream_bytes / (oi64) (channels* sizeof(float));


    if(o_unlikely(frames != S_BLOCK_SIZE)) {
        o_log_wtf_s(__func__, "invalid blocksize? %i / %i", frames, S_BLOCK_SIZE);
    }

    o_lock(s_common_L.audio_buffer_lock);

    if (s_common_L.audio_buffer_ring_size > 0) {
        o_memcpy(data, s_common_L.audio_buffer_ring[s_common_L.audio_buffer_ring_pos], channels*sizeof(float), S_BLOCK_SIZE);

        s_common_L.audio_buffer_ring_pos++;
        s_common_L.audio_buffer_ring_pos%=s_common_L.audio_buffer_ring_blks;
        s_common_L.audio_buffer_ring_size--;
    } else {
        o_clear(data, channels*sizeof(float), S_BLOCK_SIZE);
    }

    o_unlock(s_common_L.audio_buffer_lock);
}
#else
//SDL3
void audio_callback(void *userdata, SDL_AudioStream *restrict astream, int additional_amount, int total_amount)
{
    if (o_thread_id() != o_thread_main_id()) {
        o_log_level_set(O_LOG_INFO);
    }
    const osize block_bytes = S_BLOCK_SIZE * s_common_L.audio_ex.spec.channels * sizeof(float);

    o_lock(s_common_L.audio_buffer_lock);

    // as ticks
    additional_amount /= (sizeof(float) * s_common_L.audio_ex.spec.channels);
    while (additional_amount > 0 && s_common_L.audio_buffer_ring_size > 0) {
        additional_amount -= S_BLOCK_SIZE;

        float *ring = s_common_L.audio_buffer_ring[s_common_L.audio_buffer_ring_pos];
        bool valid = SDL_PutAudioStreamData(s_common_L.audio_sdl_stream, ring, block_bytes);
        if (o_unlikely(!valid)) {
            o_log_error_s(__func__, "Failed to put audio SDL_PutAudioStreamData: %s", SDL_GetError());
            break;
        }

        s_common_L.audio_buffer_ring_pos++;
        s_common_L.audio_buffer_ring_pos%=s_common_L.audio_buffer_ring_blks;
        s_common_L.audio_buffer_ring_size--;
    }

    o_unlock(s_common_L.audio_buffer_lock);
}
#endif



// forwarded
#ifdef MIA_OPTION_SDL2
void mic_callback(void *userdata, Uint8 *restrict stream, int stream_bytes)
{
    o_lock(s_common_L.mic_block_array);
    const float *block = (float *) stream;
    osize frames = (osize) stream_bytes / (osize) (s_common_L.mic_ex.spec.channels * sizeof(float));

    if(o_unlikely(frames != S_BLOCK_SIZE)) {
        o_log_wtf_s(__func__, "invalid blocksize? %i / %i", frames, S_BLOCK_SIZE);
    }

    OArray_push(s_common_L.mic_block_array, block);
    o_unlock(s_common_L.mic_block_array);
}

// note: sdl3 does not need a callback for mic in contrast to audio.
//       pulling in s__update
#endif
