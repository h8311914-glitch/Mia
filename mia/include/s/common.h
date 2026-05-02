#ifndef S_COMMON_H
#define S_COMMON_H

/**
 * @file common.h
 *
 * Common used | Basic includes, macros, types and functions.
 *
 * To play some sound fx or music:
 *   Ensure s_audio_device_open is called at least once (typically automatically done in a_app)
 * Load sounds with s/wav.h or s/ogg.h as SBuf buffer.
 * Play the fx or music with s_play()
 *
 * For a more complex workflow see STrack and STrackMix + STrackBuf
 *
 * In module "s" context, "sample" is a single float value
 *                        "frame" count samples * channels and are typically setup to play 48000 frames each second.
 *                        "block" are frames * S_BLOCK_SIZE, some filters and systems use them internally.
 */

#include "o/common.h"
#include "m/sca/dbl.h"


// just some imaginary value to work with, limited by the audio system
#define S_CHANNELS_MAX 32

// audio is processed with S_BLOCK_SIZE of frames
// a frame itself has the byte size: channels * sizeof(float) (channels * sample)
// 512 frames at 48000 Hz default frequency results in around 93 blocks/second
#define S_BLOCK_SIZE 512

// The audio is to play is pulled and buffered at frame end
// Depending on the last frame time, but atleast 0.1 s (10fps), at max 1s
// its also possible to fill the buffer manually with s_audio_buffer_fill()
#define S_BUFFER_MIN_TIME 0.1
#define S_BUFFER_MAX_TIME 1.0

/**
 * Mia always uses S_FORMAT_F32.
 * These types exist mostly for s_resample_ex.
 */
enum s_format {
    S_FORMAT_INVALID,
    S_FORMAT_U8,
    S_FORMAT_S8,
    S_FORMAT_S16LE,
    S_FORMAT_S16BE,
    S_FORMAT_S32LE,
    S_FORMAT_S32BE,
    S_FORMAT_F32LE,
    S_FORMAT_F32BE,
    S_FORMAT_S16,
    S_FORMAT_S32,
    S_FORMAT_F32,
    S_FORMAT_ENUM_MAX
};

struct s_spec {
    int freq;
    int channels;
};

struct s_spec_ex {
    struct s_spec spec;

    // Mia always uses S_FORMAT_F32. Just in case you need a different one
    enum s_format format;
};

/**
 * sizeof for a sample with that format in bytes
 */
osize s_format_size(enum s_format format);

/**
 * @return default values for the audio spec's.
 */
struct s_spec_ex s_spec_ex_default(void);

/**
 * @return default values for the audio spec's.
 */
o_inline struct s_spec s_spec_default(void)
{
    return s_spec_ex_default().spec;
}


/**
 * @param a, b specs to compare
 * @return true if both specs match and are equal
 */
o_inline bool s_spec_equals(struct s_spec a, struct s_spec b)
{
    return a.freq == b.freq && a.channels == b.channels;
}

/**
 * @param spec audio specification
 * @return simply smp * spec.channels
 */
o_inline osize s_spec_array_size(struct s_spec spec, oi64 frames)
{
    assert(frames>=0);
    return frames * spec.channels;
}

/**
 * @param spec audio specification
 * @return s_spec_array_size * sizeof(float)
 */
osize s_spec_ex_buffer_size(struct s_spec_ex spec, oi64 frames);

/**
 * @param spec audio specification
 * @return s_spec_array_size * sizeof(float)
 */
o_inline osize s_spec_buffer_size(struct s_spec spec, oi64 frames)
{
    return s_spec_array_size(spec, frames) * (osize) sizeof(float);
}

/**
 * @param spec audio specification
 * @param frames counted frames
 * @return time in seconds
 */
o_inline double s_spec_frames_as_seconds(struct s_spec spec, oi64 frames)
{
    return (double) frames / (double) spec.freq;
}

/**
 * @param spec audio specification
 * @return frames
 */
o_inline oi64 s_spec_frames_from_seconds(struct s_spec spec, double time_seconds)
{
    return (oi64) (time_seconds * spec.freq);
}





/**
 * Initializes the sound module, call this before attempting to open an audio device
 * Internally called in a_app_run.
 */
void s_init(bool audio_enable, const struct s_spec *opt_audio_spec,
            bool mic_enable, const struct s_spec *opt_mic_spec);


/**
 * @return A default allocator for the "s" module.
 * @note initialized as as a new OObjRoot with an heap allocator (to enable thread safety).
 */
oobj s_root(void);

/**
 * @returns used s_spec for audio playback
 */
struct s_spec s_audio_device_spec(void);


/**
 * @returns used s_spec for mic recording
 * @note should match s_audio_device_spec(void) except with a single channel (mono)
 */
struct s_spec s_mic_device_spec(void);


/**
 * Called by a_app to pull sounds and update microphone stuff.
 * Don't call it yourself if you are using full mia (with a_app).
 */
void s_update(void);


/**
 * returns true if audio was enabled via s_init
 */
bool s_audio_enabled(void);

/**
 * Call this to open the audio device.
 * Noop if already opened or !s_audio_enabled().
 * Internally may be called by a_app on first pointer event. (Cause emscripten else blocks it)
 */
void s_audio_device_open(void);

/**
 * @return true if s_audio_device_open() was called and the audio is audible
 */
bool s_audio_device_active(void);

/**
 * Pauses the audio device stream
 */
void s_audio_pause(void);

/**
 * Unpauses the audio device stream
 */
void s_audio_play(void);


/**
 * Will manually fill the audio output buffer.
 * Automatically called in a_app (frame end) with S_BUFFER_MIN_TIME
 * @param time in [s] clamped to [S_BUFFER_MIN_TIME : S_BUFFER_MAX_TIME]
 */
void s_audio_buffer_fill(float time);

/**
 * @return STrackMix for the real hardware
 * @note NULL if !s_audio_enabled()
 */
oobj s_audio_trackmix(void);


/**
 * returns true if audio was enabled via s_init
 */
bool s_mic_enabled(void);

/**
 * Call this to open the mic device.
 * Noop if already opened or !s_mic_enabled().
 * Internally may be called by a_app on first pointer event. (Cause emscripten else blocks it)
 */
void s_mic_device_open(void);

/**
 * @return true if s_mic_device_open() was called and the audio is recordable
 */
bool s_mic_device_active(void);

/**
 * Pauses the mic device stream
 */
void s_mic_pause(void);

/**
 * Unpauses the mic device stream
 */
void s_mic_record(void);

/**
 * @return the current recorded amount of blocks for the last frame (cleared each frame)
 * @note asserts main thread
 */
int s_mic_current_blocks_num(void);





////////////////////////////////

/**
 * @param current_block in range [ 0:s_mic_current_blocks_num() ),
 *                      where s_mic_current_blocks_num()-1 is the newest recorded bpvl
 * @return one of the current recorded blocks for the last visual app frame (cleared each each app frame)
 *         a block has the size of S_BLOCK_SIZE audio frames
 * @note asserts blk in bounds [ 0:s_mic_current_blocks_num() )
 *       the blocks are packed in a series, so blk_0 + block_floats == blk_1
 * @note asserts main thread
 */
float *s_mic_current_blocks_at(int current_block);

/**
 * @param parent to allocate on
 * @return a new SBuf which will push back the current recorded audio
 *         Internally gets updates on visual app frame end caused by a_app (calls s__update)
 * @note asserts main thread
 */
oobj s_mic_buf_new(oobj parent);

/**
 * Plays a sound/music once on the system track (s_audio_trackmix)
 * @param buf SBuf object to be played
 * @param delay_seconds start in seconds, added to the current time
 * @param gain amplification, 1.0f for simple copy
 * @return STrackBuf valid for this frame
 * @note asserts s_audio_device_active() and main thread
 *       If the audio device is not opened yet,
 *       either the call to s_audio_device_open()
 *           or a_app_audio_device_open is missing
 *       Or you played >before< first pointer user input.
 *           In the latter considering to create smth like an intro screen and play after first acrive pointer.
 */
oobj s_play(oobj buf, double delay_seconds, float gain);



//
// audio utils
//


/**
 * Mixes two audio buffers into the first
 * @param in_out_data first audio buffer and also the result
 * @param mix_data to add into in_out_data
 * @param mix_gain amplification for the mix data (default is 1.0f)
 * @param channels from the spec
 * @param frames: number of samples*channels
 * @note not to be confused with math mix functions. This mix works by adding and clamping
 */
void s_mix_into(float *restrict in_out_data, const float *restrict mix_data, float mix_gain, int channels, oi64 frames);


/**
 * @return frames for dst if used for resampling
 */
o_inline oi64 s_resample_output_frames(struct s_spec dst_spec, struct s_spec src_spec, oi64 src_frames)
{
    return (oi64) md_ceil((double) src_frames * (double) dst_spec.freq / (double) src_spec.freq);
}

/**
 * Resamples a track, with different frequencies.
 * Have a look at s_spec_ex_buffer_size()
 * Mia always uses S_FORMAT_F32 else, could be used for loading and saving of files.
 * @param out_output needs to be of size: s_resample_output_frames(...) -> s_spec_ex_buffer_size(...)
 */
void s_resample_ex(void *restrict out_output, struct s_spec_ex output_spec,
                   const void *restrict input, struct s_spec_ex input_spec, oi64 input_frames);


/**
 * Resamples a track
 * Have a look at s_spec_buffer_size()
 * @param out_output needs to be of size: s_resample_output_frames(...) -> s_spec_buffer_size(...)
 */
o_inline void s_resample(void *restrict out_output, struct s_spec output_spec,
                        const void *restrict input, struct s_spec input_spec, oi64 input_frames)
{
    struct s_spec_ex output_spec_ex = {0};
    output_spec_ex.spec = output_spec;
    output_spec_ex.format = S_FORMAT_F32;
    struct s_spec_ex input_spec_ex = {0};
    input_spec_ex.spec = input_spec;
    input_spec_ex.format = S_FORMAT_F32;
    s_resample_ex(out_output, output_spec_ex, input, input_spec_ex, input_frames);
}


/**
 * @param data audio buffer with spec(ification)
 * @return the amplitude calced by peak.
 *         which is max(abs(data))
 */
float s_amplitude_peak(const float *restrict data, struct s_spec spec, oi64 frames);

/**
 * @param data audio buffer with spec(ification)
 * @return the amplitude calced by rms (root mean square).
 *         which is sqrt(sum(data**2)/n)
 */
float s_amplitude_rms(const float *restrict data, struct s_spec spec, oi64 frames);

/**
 * Normalizes data to the s_amplitude_peak of it.
 * peak is like max(abs(data)).
 * @param amp_min clamps calced amplitude to a minumum of that, <=0 to auto set it to 0.1
 * @param gain additional applied gain
 * @return same as s_amolitude_peak
 */
float s_normalize_peak(float *restrict data, struct s_spec spec, oi64 frames, float amp_min, float gain);

/**
 * Normalizes data to the s_amplitude_rms of it.
 * rms is like sqrt(sum(data**2)/n)).
 * @param amp_min clamps calced amplitude to a minumum of that, <=0 to auto set it to 0.1
 * @param gain additional applied gain
 * @return same as s_amolitude_rms
 */
float s_normalize_rms(float *restrict data, struct s_spec spec, oi64 frames, float amp_min, float gain);

#endif //S_COMMON_H
