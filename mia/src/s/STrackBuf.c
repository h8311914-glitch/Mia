#include "s/STrackBuf.h"
#include "o/OObj_builder.h"
#include "o/OPtr.h"
#include "m/vec/vecn.h"
#include "s/SFilter.h"
#include "s/SBuf.h"


#undef O_LOG_LIB
#define O_LOG_LIB "s"
#include "o/log.h"


static void trackbuf_apply_filters(STrackBuf *self, float *restrict data, oi64 frames)
{
    SFilter **list = STrackBuf_filter_list(self, NULL);
    for(SFilter **it=list; *it; it++) {
        SFilter_apply(*it, data, frames, self->super.spec);
        if (SFilter_ended(*it)) {
            o_del(*it);
        }
    }
    o_free(self, list);
}


//
// public
//


STrackBuf *STrackBuf_init(oobj obj, oobj parent, oobj opt_parent2, oobj buf, oi64 frame_delay)
{
    STrackBuf *self = obj;
    o_clear(self, sizeof *self, 1);

    struct s_spec spec = SBuf_spec(buf);
    STrack_init(obj, parent, opt_parent2, STrackBuf__v_pull, &spec);
    OObj_id_set(self, STrackBuf_ID);

    self->super.endable = true;

    OObj_assert(buf, SBuf);

    self->buf_ptr = OPtr_new(self, buf);

    self->frame_time = 0;
    self->frame_delay = frame_delay;

    self->loops = 0;
    self->loop_start_frame_offset = 0;
    self->loop_end_frame_offset = 0;

    self->gain = 1.0f;

    return self;
}

//
// virtual implementations:
//

bool STrackBuf__v_pull(oobj obj, float *restrict out_data, oi64 frames)
{
    OObj_assert(obj, STrackBuf);
    STrackBuf *self = obj;

    int channels = self->super.spec.channels;

    SBuf *buf = OPtr_get(self->buf_ptr).o;
    if (!buf) {
        o_clear(out_data, sizeof(float) * channels, frames);
        return true;
    }

    bool ended = false;
    
    assert(s_spec_equals(buf->spec, self->super.spec));
    assert(OArray_element_size(buf->array) == sizeof(float) * channels);

    osize array_samples = OArray_num(buf->array);

    float *data_it = out_data;
    oi64 frames_rem = frames;
    
    // silence at start?
    if (self->frame_delay > self->frame_time) {
        osize silence_frames = self->frame_delay - self->frame_time;
        silence_frames = m_min(silence_frames, frames_rem);
        o_clear(data_it, sizeof(float) * channels, silence_frames);
        data_it += silence_frames * channels;
        frames_rem -= silence_frames;
        self->frame_time += silence_frames;
    }

    // while loop cause the loop may be smaller as the block size...
    while (!ended && frames_rem>0) {
        osize wnd_start = self->frame_time - self->frame_delay;
        osize wnd_end = wnd_start + frames_rem;
        if (self->loops!=0) {
            osize loop_end = self->loop_end_frame_offset>0? self->loop_end_frame_offset : array_samples;
            if (wnd_end > loop_end) {
                osize overrun = wnd_end - loop_end;
                wnd_end = loop_end;
                self->frame_time = self->frame_delay + self->loop_start_frame_offset;

                if (self->loops>0) {
                    self->loops--;
                }

                // in case start was already at loop end...
                if (overrun == frames_rem) {
                    continue;
                }
            }
        } else if (wnd_end >= array_samples) {
            // loops == 0, sound has ended
            ended = true;
        }
        osize used_start = o_max(wnd_start, 0);
        osize used_end = o_min(wnd_end, array_samples);
        osize wnd_size = used_end - used_start;
        if (wnd_size <= 0) {
            break;
        }

        // actual data to be copied
        o_memcpy(data_it, OArray_at_void(buf->array, used_start),
            sizeof(float)*channels, wnd_size);
        data_it += wnd_size * channels;
        frames_rem -= wnd_size;
        self->frame_time += wnd_size;
    }

    if (frames_rem>0) {
        // silent end cause sound is over
        o_clear(data_it, sizeof(float) * channels, frames_rem);
        ended =  true;
    }

    // apply filters and gain
    trackbuf_apply_filters(self, out_data, frames);
    vecn_scale(out_data, out_data, self->gain, channels * frames);

    return ended;

}


//
// object functions
//

