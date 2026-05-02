#ifndef R_PROGRAM_H
#define R_PROGRAM_H

/**
 * @file program.h
 *
 * RProgram's shipped with the 'r' module.
 * All loaded lazy on first need.
 */


#include "r/common.h"


/**
 * Combines all shader_code sources and also replaces the includes with their contents.
 * #include "$r/common_quad_vertex.glsl" would be replaced with the content of it.
 *     But that file is not looked up for includes! (so not recursive)
 * @param parent OObj to allocate on
 * @param shader_codes null terminated list of:
 *                     glsl code string to compile, containing both vertex and fragment shader.
 * @param opt_shader_codes_num if NULL, uses shader_codes as NULL terminated list, else uses this size
 */
char *r_program_merge_code(oobj parent, char **shader_codes, const osize *opt_shader_codes_num);

/**
 * @return RProgram of the 'r' module, lazy loaded
 */
#define r_program_DECL(file, num_draw_buffer)\
oobj r_program_ ## file (void);


r_program_DECL(Box, 1)
r_program_DECL(Box_rgba, 1)
r_program_DECL(Box_hsva, 1)
r_program_DECL(BoxIndexed, 1)
r_program_DECL(Quad, 1)
r_program_DECL(Quad_addscaled, 1)
r_program_DECL(Quad_rgba, 1)
r_program_DECL(Quad_rgba_hsva, 1)
r_program_DECL(Quad_hsva_from_rgba, 1)
r_program_DECL(Quad_hsva_into_rgba, 1)
r_program_DECL(Quad_mixer, 1)
r_program_DECL(QuadChannelmerge, 1)
r_program_DECL(QuadDab, 2)
r_program_DECL(QuadKernel_bloom, 1)
r_program_DECL(QuadKernel_contour, 1)
r_program_DECL(QuadKernel_contour_inv, 1)
r_program_DECL(QuadKernel_conv, 1)
r_program_DECL(QuadMerge, 1)
r_program_DECL(QuadMerge_additive, 1)
r_program_DECL(QuadMerge_subtractive, 1)
r_program_DECL(QuadOp_min, 1)


#undef r_program_DECL


#endif //R_PROGRAM_H
