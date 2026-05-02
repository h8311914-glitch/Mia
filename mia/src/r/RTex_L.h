#ifndef R_RTEX_L_H
#define R_RTEX_L_H

#include "o/common.h"
#include "m/types/int.h"
#include "r/RBufferBox.h"
#include "r/RBufferQuad.h"
#include "r/RShaderBoxIndexed.h"
#include "r/RShaderQuadChannelmerge.h"
#include "r/RShaderQuadKernel.h"
#include "r/RShaderQuadMerge.h"
#include "r/RShaderQuadOp.h"

struct RTex__L_t {
    oobj bound_tex;
    bool bound_valid;

    ou32 back_fbo;
    ivec4 back_gl_viewport;

    oobj o;

    oobj b_box;
    oobj b_quad;

    oobj s_box;
    oobj s_box_indexed;
    oobj s_quad;
    oobj s_quad_channelmerge;
    oobj s_quad_kernel;
    oobj s_quad_merge;
    oobj s_quad_op;

    oobj draw_boxes_array;
    oobj draw_dab_array;
};

#endif //R_RTEX_L_H