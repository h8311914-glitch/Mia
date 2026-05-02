#ifdef MIA_OPTION_SDL2
#include <SDL2/SDL_audio.h>
#else
#include <SDL3/SDL_audio.h>
#endif

#include "s/wav.h"
#include "o/OObjRoot.h"
#include "o/OArray.h"
#include "o/OStreamArray.h"
#include "o/file.h"
#include "o/endian.h"
#include "s/SBuf.h"



#undef O_LOG_LIB
#define O_LOG_LIB "s"
#include "o/log.h"


// protected
SDL_AudioFormat s__format_to_sdl(enum s_format format);
enum s_format s__format_from_sdl(SDL_AudioFormat format);


struct oobj_opt s_wav_load_array(oobj parent, const char *file, const struct s_spec *opt_spec)
{
    char routed[O_FILE_PATH_MAX];
    o_file_route_resolve(routed, file);

    struct s_spec spec = opt_spec ? *opt_spec : s_spec_default();

    oobj dst = NULL;
    Uint8 *src_buffer = NULL;
    oobj container = OObj_new(parent);

    SDL_AudioSpec sdl_src_spec;
    Uint32 src_buffer_size;


#ifdef MIA_OPTION_SDL2
    bool wav_valid = SDL_LoadWAV(routed, &sdl_src_spec, &src_buffer, &src_buffer_size) != NULL;
#else
    bool wav_valid = SDL_LoadWAV(routed, &sdl_src_spec, &src_buffer, &src_buffer_size);
#endif

    if (!wav_valid) {
        o_log_error_s(__func__, "Failed to load WAV file: %s\n", SDL_GetError());
        return oobj_opt(NULL);
    }

    struct s_spec_ex src_spec = {0};
    src_spec.spec.freq = sdl_src_spec.freq;
    src_spec.spec.channels = sdl_src_spec.channels;
    src_spec.format = s__format_from_sdl(sdl_src_spec.format);

    struct s_spec_ex dst_spec = {0};
    dst_spec.spec = spec;
    dst_spec.format = S_FORMAT_F32;

    oi64 src_len = src_buffer_size / (s_format_size(src_spec.format) * src_spec.spec.channels);
    oi64 dst_len = s_resample_output_frames(dst_spec.spec, src_spec.spec, src_len);

    dst = OArray_new(parent, NULL, sizeof(float), dst_len * dst_spec.spec.channels);
    s_resample_ex(OArray_data_void(dst), dst_spec, src_buffer, src_spec, src_len);

    return oobj_opt(dst);
}


struct oobj_opt s_wav_load_buf(oobj parent, const char *file, const struct s_spec *opt_spec)
{
    struct s_spec spec = opt_spec ? *opt_spec : s_spec_default();

    struct oobj_opt array = s_wav_load_array(parent, file, &spec);
    if(!array.o) {
        return oobj_opt(NULL);
    }
    oobj buf = SBuf_new_array(parent, array.o, &spec);
    return oobj_opt(buf);
}


//
// write
//

static void write_le_ou32(oobj s, ou32 v)
{
    v = ou32_endian_to_lil(v);
    OStream_write(s, &v, sizeof v, 1);
}
static void write_le_ou16(oobj s, ou16 v)
{
    v = ou16_endian_to_lil(v);
    OStream_write(s, &v, sizeof v, 1);
}

bool s_wav_write(const char *file, const float *data, oi64 len, const struct s_spec *opt_spec)
{
    struct s_spec spec = opt_spec ? *opt_spec : s_spec_default();

    // new unregistered root
    oobj container = OObjRoot_new(NULL, o_allocator_heap_new(), false);
    bool ok = false;

    //
    // convert to 16 bit little endian pcm max stereo
    //
    struct s_spec_ex src_spec = {0};
    src_spec.spec = spec;
    src_spec.format = S_FORMAT_F32;

    struct s_spec_ex dst_spec = {0};
    dst_spec.spec = spec;
    dst_spec.format = S_FORMAT_S16LE;

    dst_spec.spec.channels = o_min(spec.channels, 2);

    int channels = o_min(spec.channels, 2);
    ou16 *pcm_data = o_alloc(container, 1, s_spec_ex_buffer_size(dst_spec, len));
    s_resample_ex(pcm_data, dst_spec, data, src_spec, len);

    //
    // create wav file:
    //

    oobj array = OArray_new_dyn(container, NULL, 1, 0, 1024*32);
    oobj s = OStreamArray_new(container, array, false, OStreamArray_SEEKABLE);

    // wav header
    OStream_print(s, "RIFF");

    // file size -8 (header size is 36, + 8 data header + data)
    ou32 data_size = len * channels * 2;
    ou32 file_size = 36 + data_size;
    write_le_ou32(s, file_size);

    // includes a space at end...
    OStream_print(s, "WAVEfmt ");

    // remaining format header length (32-16=16)
    write_le_ou32(s, 16);

    // format tag 1 for pcm
    write_le_ou16(s, 1);

    // channels
    write_le_ou16(s, channels);

    // freq
    write_le_ou32(s, spec.freq);

    // byte rate
    ou32 byte_rate = spec.freq * channels * 2;
    write_le_ou32(s, byte_rate);

    // block align
    ou16 block_align = channels * 2;
    write_le_ou16(s, block_align);

    // bits each sample
    write_le_ou16(s, 16);

    // begin of data section
    OStream_print(s, "data");

    // data length
    write_le_ou32(s, data_size);

    // actual data
    OStream_write(s, pcm_data, 1, data_size);




    // flush to file
    osize written = o_num(array);
    osize file_written = o_file_write(file, false, OArray_data_void(array), 1, written);
    ok = file_written == written;
    if (!ok) {
        o_log_error_s(__func__, "failed to write to file: %s", file);
    }

    o_del(container);
    return ok;
}
