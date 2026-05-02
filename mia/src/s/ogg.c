#include "s/ogg.h"
#include "o/OArray.h"
#include "o/file.h"
#include "s/SBuf.h"

#define STB_VORBIS_HEADER_ONLY
#include "ext_stb_vorbis.c"

#undef O_LOG_LIB
#define O_LOG_LIB "s"
#include "o/log.h"

// Function to load and decode an Ogg Vorbis from memory using stb_vorbis.c
// chatgpt version...
static float* loadOggMemory(oobj parent, const void* data, int data_len, ou32* audioLength, ou16* channels, ou32* sampleRate) {
    int error;

    // this function would not run on android, because it uses the plain c file loader, instead of sdl's
    //stb_vorbis* vorbis = stb_vorbis_open_filename(filename, &error, NULL);

    stb_vorbis *vorbis = stb_vorbis_open_memory(data, data_len, &error, NULL);
    if (vorbis == NULL) {
        fprintf(stderr, "Error opening Ogg Vorbis file: %d\n", error);
        return NULL;
    }

    stb_vorbis_info info = stb_vorbis_get_info(vorbis);
    *channels = info.channels;
    *sampleRate = info.sample_rate;

    // Calculate total PCM data size
    int num_samples = stb_vorbis_stream_length_in_samples(vorbis);
    int num_channels = info.channels;
    int num_floats = num_samples * num_channels;

    // Allocate buffer for decoded PCM data (as floats)
    float* pcm = (float*)o_alloc(parent, sizeof(float), num_floats);
    if (pcm == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        stb_vorbis_close(vorbis);
        return NULL;
    }

    // Decode Ogg Vorbis data into PCM buffer
    int samples = stb_vorbis_get_samples_float_interleaved(vorbis, info.channels, pcm, num_samples * num_channels);
    if (samples != num_samples) {
        fprintf(stderr, "Failed to decode Ogg Vorbis file\n");
        o_free(parent, pcm);
        stb_vorbis_close(vorbis);
        return NULL;
    }

    // Close the Ogg Vorbis file
    stb_vorbis_close(vorbis);

    *audioLength = num_floats * sizeof(float);
    return pcm;
}

struct oobj_opt s_ogg_load_array(oobj parent, const char *file, const struct s_spec *opt_spec)
{
    struct s_spec spec = opt_spec ? *opt_spec : s_spec_default();

    oobj container = OObj_new(parent);
    oobj dst = NULL;
    float *ogg_data = NULL;

    ou32 ogg_audio_length;
    ou16 ogg_channels;
    ou32 ogg_sample_rate;

    struct oobj_opt memory = o_file_read(container, file, false, 1);
    if(!memory.o) {
        o_log_error_s(__func__, "Failed to load file to memory %s", file);
        goto CLEAN_UP;
    }

    ogg_data = loadOggMemory(container, o_at(memory.o, 0), (int) o_num(memory.o),
                             &ogg_audio_length, &ogg_channels, &ogg_sample_rate);
    if(!ogg_data) {
        o_log_error_s(__func__, "Failed to parse ogg file %s", file);
        goto CLEAN_UP;
    }


    // convert to desired spec
    struct s_spec src_spec = {0};
    src_spec.channels = ogg_channels;
    src_spec.freq = ogg_sample_rate;

    oi64 src_len = (oi64) ogg_audio_length / ((oi64)sizeof(float) * src_spec.channels);

    oi64 dst_len = s_resample_output_frames(spec, src_spec, src_len);

    dst = OArray_new(parent, NULL, sizeof(float), dst_len * spec.channels);
    s_resample(OArray_data_void(dst), spec, ogg_data, src_spec, src_len);

    CLEAN_UP:
    o_del(container);

    return oobj_opt(dst);
}



struct oobj_opt s_ogg_load_buf(oobj parent, const char *file, const struct s_spec *opt_spec)
{
    struct s_spec spec = opt_spec ? *opt_spec : s_spec_default();

    struct oobj_opt array = s_ogg_load_array(parent, file, &spec);
    if(!array.o) {
        return oobj_opt(NULL);
    }
    oobj buf = SBuf_new_array(parent, array.o, &spec);
    return oobj_opt(buf);
}
