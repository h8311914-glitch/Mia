#include "u/USamplerVec.h"
#include "o/OObj_builder.h"
#include "o/OArray.h"
#include "m/sca/flt.h"

#undef O_LOG_LIB
#define O_LOG_LIB "u"
#include "o/log.h"

//
// public
//

USamplerVec *USamplerVec_init(oobj obj, oobj parent, osize vec_size,
                              oobj frames_array, bool move_frames_array, enum USamplerVec_mode mode)
{
    USamplerVec *self = obj;
    o_clear(self, sizeof *self, 1);

    USampler_init(obj, parent, vec_size * sizeof(float), USampleVec__v_interpolate, USamplerVec__v_times);
    OObj_id_set(self, USamplerVec_ID);

    OObj_assert(frames_array, OArray);
    self->frames = frames_array;
    if (move_frames_array) {
        o_move(frames_array, self);
    }

    self->mode = mode;

    assert(OArray_element_size(frames_array) == (vec_size*3+1)*4 && "invalid frames element_size");

    return self;
}

USamplerVec *USamplerVec_new_frames(oobj parent, osize vec_size,
                                    const void *frames, int num, enum USamplerVec_mode mode)
{
    osize element_size = (vec_size * 3 + 1) * 4;
    oobj array = OArray_new(parent, frames, element_size, num);
    return USamplerVec_new(parent, vec_size, array, true, mode);
}

//
// virtual implementations:
//

void USampleVec__v_interpolate(oobj obj, void *out_interpolated_element, float time)
{
    OObj_assert(obj, USamplerVec);
    USamplerVec *self = obj;

    float *out_floats = out_interpolated_element;


    osize num_frames = OArray_num(self->frames);
    osize vec_size = USamplerVec_vec_size(self);

    assert(num_frames > 0 && "need atleast 1 frame");
    assert(OArray_element_size(self->frames) == (vec_size*3+1)*4 && "invalid frames element_size");

    float *item_a, *item_b;
    item_a = item_b = OArray_at_void(self->frames, 0);

    int state = -1;
    // 0 is default, 1 is left out of bounds, 2 is right out of bounds
    for (osize i = 1; i < num_frames; i++) {
        item_a = item_b;
        item_b = OArray_at_void(self->frames, i);

        // perfect time match
        if (item_a[0] <= time && time <= item_b[0]) {
            state = 0;
            break;
        }

        // time "out of bounds" check
        if (i == 1 && time < item_a[0]) {
            state = 1;
            break;
        }
        if (i == (num_frames - 1) && time > item_b[0]) {
            state = 2;
            break;
        }
    }
    if (o_unlikely(state == -1)) {
        o_log_error_s(__func__, "invalid keypoint times!");
    }

    if (state == 1 || state == -1) {
        // left out of bounds
        for (osize i = 0; i < vec_size; i++) {
            float a = item_a[1 + i];
            out_floats[i] = a;
        }
        return;
    }
    if (state == 2) {
        // right out of bounds
        for (osize i = 0; i < vec_size; i++) {
            float b = item_b[1 + i];
            out_floats[i] = b;
        }
        return;
    }


    if (self->mode == USamplerVec_LINEAR) {
        for (osize i = 0; i < vec_size; i++) {
            float t = time - item_a[0];
            float dtime = item_b[0] - item_a[0];
            float a = item_a[1 + i];
            float b = item_b[1 + i];
            t /= dtime;
            out_floats[i] = m_mix(a, b, t);
        }
    } else if (self->mode == USamplerVec_STEP) {
        for (osize i = 0; i < vec_size; i++) {
            float a = item_a[1 + i];
            out_floats[i] = a;
        }
    } else {
        // SPLINE
        for (osize i = 0; i < vec_size; i++) {
            // item is defined as [time, value1, value2, ..., tangent_in1, tangent_in2, ..., tangent_out1, tangent_out2, ...]
            float t = time - item_a[0];
            float dtime = item_b[0] - item_a[0];
            float a = item_a[1 + i];
            float b = item_b[1 + i];
            float tangent_a = item_a[1 + i + vec_size * 2];
            float tangent_b = item_b[1 + i + vec_size];
            out_floats[i] = m_spline(a, tangent_a, b, tangent_b, t, dtime);
        }
    }
}

float *USamplerVec__v_times(oobj obj, osize *opt_out_num)
{
    OObj_assert(obj, USamplerVec);
    USamplerVec *self = obj;

    osize num_frames = OArray_num(self->frames);
    assert(num_frames > 0 && "need atleast 1 frame");
    float *times = o_new(self, float, num_frames+1);
    for (osize i = 0; i < num_frames; i++) {
        times[i] = *(float *) OArray_at_void(self->frames, i);
    }
    times[num_frames] = -1.0;
    if (opt_out_num) {
        *opt_out_num = num_frames;
    }
    return times;
}
