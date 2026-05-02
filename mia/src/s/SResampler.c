#include "s/SResampler.h"
#include "o/OObj_builder.h"

#ifdef MIA_OPTION_SDL2
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_audio.h>
#else
#include <SDL3/SDL_audio.h>
#endif

#undef O_LOG_LIB
#define O_LOG_LIB "s"

#include "o/log.h"

// format helper


// protected
SDL_AudioFormat s__format_to_sdl(enum s_format format);
enum s_format s__format_from_sdl(SDL_AudioFormat format);


struct resampler_context {
#ifdef MIA_OPTION_SDL2
    bool valid;
    SDL_AudioCVT cvt;
    void *cvt_buf;
    osize cvt_buf_cap;
#else
    // SDL3
    
    SDL_AudioStream *stream;
#endif
};


SResampler *SResampler_init_ex(oobj obj, oobj parent, struct s_spec_ex input_ex, struct s_spec_ex output_ex)
{
    SResampler *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, SResampler_ID);

    self->input_ex = input_ex;
    self->output_ex = output_ex;

    struct resampler_context *C = self->impl = o_new0(self, *C, 1);

#ifdef MIA_OPTION_SDL2
    int build_res;
    build_res = SDL_BuildAudioCVT(&C->cvt, s__format_to_sdl(input_ex.format),
                                  input_ex.spec.channels, input_ex.spec.freq,
                                  s__format_to_sdl(output_ex.format),
                                  output_ex.spec.channels, output_ex.spec.freq);
    C->valid = build_res >= 0;
    if (!C->valid) {
        o_log_error_s(__func__, "Failed to build audio CVT: %s", SDL_GetError());
    }
    
#else
    // SDL3

    SDL_AudioSpec sdl_output_spec = {0};
    sdl_output_spec.format = s__format_to_sdl(output_ex.format);
    sdl_output_spec.channels = output_ex.spec.channels;
    sdl_output_spec.freq = output_ex.spec.freq;
    SDL_AudioSpec sdl_input_spec = {0};
    sdl_input_spec.format = s__format_to_sdl(input_ex.format);
    sdl_input_spec.channels = input_ex.spec.channels;
    sdl_input_spec.freq = input_ex.spec.freq;
    // Create the SDL_AudioStream for conversion
    C->stream = SDL_CreateAudioStream(&sdl_input_spec, &sdl_output_spec);
    if (!C->stream) {
        o_log_error_s(__func__, "Failed to create SDL_AudioStream: %s\n", SDL_GetError());
    }

#endif

    // vfuncs
    self->super.v_del = SResampler__v_del;

    return self;
}

//
// virtual implementations
//

void SResampler__v_del(oobj obj)
{
    OObj_assert(obj, SResampler);
    SResampler *self = obj;

    struct resampler_context *C = self->impl;

#ifndef MIA_OPTION_SDL2
    if (C->stream) {
        SDL_DestroyAudioStream(C->stream);
    }
#endif


    OObj__v_del(self);
}

//
// object functions
//

void SResampler_resample(oobj obj, void *restrict out_output, const void *restrict input, oi64 input_smp)
{
    OObj_assert(obj, SResampler);
    SResampler *self = obj;
    struct resampler_context *C = self->impl;

    if (s_spec_equals(self->output_ex.spec, self->input_ex.spec)
        && self->output_ex.format == self->input_ex.format) {
        // no need to resample
        osize out_format_size = s_format_size(self->output_ex.format);
        o_memcpy(out_output, input, out_format_size, s_spec_array_size(self->input_ex.spec, input_smp));
        return;
    }

    osize output_smp = SResampler_output_smp(self, input_smp);
    osize output_buffer_size = s_spec_ex_buffer_size(self->output_ex, output_smp);

#ifdef MIA_OPTION_SDL2


    // Allocate buffer for conversion
    osize cvt_len = input_smp * self->input_ex.spec.channels * s_format_size(self->input_ex.format);
    osize cvt_size = cvt_len * C->cvt.len_mult;
    if (C->cvt_buf_cap < cvt_size) {
        C->cvt_buf_cap = cvt_size;
        C->cvt_buf = o_realloc(self, C->cvt_buf, 1, C->cvt_buf_cap);
    }
    C->cvt.len = cvt_len;
    C->cvt.buf = C->cvt_buf;

    // Copy the source audio data into the conversion buffer
    o_memcpy(C->cvt.buf, input, 1, C->cvt.len);

    // Perform the audio conversion
    if (SDL_ConvertAudio(&C->cvt) < 0) {
        o_log_error_s(__func__, "Failed to convert audio: %s", SDL_GetError());
        o_clear(out_output, output_buffer_size, 1);
        return;
    }

    // Calculate destination length and copy the converted audio data to the destination buffer
    o_memcpy(out_output, C->cvt.buf, s_format_size(self->output_ex.format), output_smp * self->output_ex.spec.channels);
    
#else
    // SDL3

    if (!C->stream) {
        o_log_warn_s(__func__, "No stream, bulding failed?");
        o_clear(out_output, output_buffer_size, 1);
        return;
    }

    osize input_buffer_size = s_spec_ex_buffer_size(self->input_ex, input_smp);
    if (!SDL_PutAudioStreamData(C->stream, input, (int) input_buffer_size)) {
        o_log_error_s(__func__, "Failed to put audio SDL_PutAudioStreamData: %s\n", SDL_GetError());
        o_clear(out_output, output_buffer_size, 1);
        return;
    }
    if (!SDL_FlushAudioStream(C->stream)) {
        o_log_error_s(__func__, "Failed to flush audio SDL_FlushAudioStream: %s\n", SDL_GetError());
        o_clear(out_output, output_buffer_size, 1);
        return;
    }
    // Determine the available data size in the stream
    int bytes_available = SDL_GetAudioStreamAvailable(C->stream);
    if (bytes_available <= 0) {
        o_log_error_s(__func__, "No audio data available in the stream for conversion.");
        o_clear(out_output, output_buffer_size, 1);
        return;
    }
    if (bytes_available != (int) output_buffer_size) {
        o_log_warn_s(__func__, "Invalid audio data size in the stream for conversion: %i<>%i (%i)",
            bytes_available, (int) output_buffer_size);
        bytes_available = o_min(bytes_available, (int) output_buffer_size);
    }
    // Get converted audio data from the stream
    if (SDL_GetAudioStreamData(C->stream, out_output, bytes_available) < 0) {
        o_log_error_s(__func__, "Failed to retrieve data from SDL_AudioStream: %s\n", SDL_GetError());
    }
    if (!SDL_ClearAudioStream(C->stream)) {
        o_log_error_s(__func__, "Failed to clear audio SDL_ClearAudioStream: %s\n", SDL_GetError());
    }
#endif
}
