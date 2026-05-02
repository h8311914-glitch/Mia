#include "u/waveform.h"
#include "o/OArray.h"
#include "r/RTex.h"
#include "r/tex.h"
#include "r/RShaderBox.h"
#include "u/rect.h"

oobj u_waveform_boxes(oobj parent, const float *audio_data, int channel, int channels_num, oi64 frames,
                      vec4 rect, float thickness, struct r_box init)
{
    int cols = (int) m_ceil(rect.v2 / thickness);
    if(cols <= 0 || frames <= 0) {
        return OArray_new(parent, NULL, sizeof(struct r_box), 0);
    }
    oobj array = OArray_new(parent, NULL, sizeof(struct r_box), cols);
    oi64 prev_frame = 0;
    struct r_box *array_v = OArray_data(array, struct r_box);
    for(int i=0; i<cols; i++) {
        struct r_box *b = &array_v[i];
        oi64 frame = (int) ((oi64) (i+1) * frames / cols);

        float min = m_MAX;
        float max = m_MIN;
        for(int t=prev_frame; t<frame; t++) {
            float sample = audio_data[t*channels_num + channel];
            min = o_min(min, sample);
            max = o_max(max, sample);
        }
        
        // -1 so that the windows all overlap a singlr sample
        //     else it may create a small hole
        prev_frame = o_max(0, frame-1);


        // range from min to max is 2.0f
        float h = (max - min) * rect.v3 * 0.5f;
        h = o_max(h, thickness);
        float w = thickness;
        float t = (1.0f-max) * rect.v3 * 0.5f;
        float l = rect.x + thickness * i;

        *b = init;
        b->rect = vec4_(l, t, w, h);
    }
    return array;
}

void u_waveform_render(oobj tex, const float *audio_data, int channel, int channels_num, oi64 frames,
                       vec4 rect, float thickness, vec4 color)
{
    if(rect.v2<=0) {
        rect.x = 0;
        rect.v2 = RTex_size(tex).x;
    }
    if(rect.v3<=0) {
        rect.y = 0;
        rect.v3 = RTex_size(tex).y;
    }
    oobj container = OObj_new(tex);

    struct r_box init = r_box_new(1, 1);
    init.fx = color;
    oobj boxes = u_waveform_boxes(container, audio_data, channel, channels_num, frames, rect, thickness, init);

    oobj shader = RShaderBox_new_rgba(container, r_tex_white(), false);
    RTex_boxes(tex, shader, o_at(boxes, 0), o_num(boxes));

    o_del(container);
}
