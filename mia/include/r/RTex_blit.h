#ifndef R_RTEX_BLIT_H
#define R_RTEX_BLIT_H

/**
 * @file RTex_blit.h
 *
 * Object extension
 *
 * RTex_ namespace
 *
 * Blitting, blending, casting, clone, resize, merge
 *
 * @sa RTex_ex.h to import all RTex object extensions
 */


#include "RTex.h"



enum RTex_merge_mode {
    RTex_merge_mode_DEFAULT,
    RTex_merge_mode_ADDITVE,
    RTex_merge_mode_SUBTRACTIVE,
};


//
// blit and blend tex on tex
//

/**
 * Overwrites (blend off) the tex onto obj
 * @param obj RTex object
 */
void RTex_blit_pose(oobj obj, oobj tex, mat4 pose, mat4 uv_pose);

/**
 * Overwrites (blend off) the tex onto obj
 * @param obj RTex object
 */
void RTex_blit_rect(oobj obj, oobj tex, vec4 rect, vec4 uv_rect);

/**
 * Overwrites (blend off) the tex onto obj
 * @param obj RTex object
 * @param x, y as left top position
 * @note calls RTex_blit_rect internally
 */
void RTex_blit(oobj obj, oobj tex, float x, float y);

/**
 * Overwrites (blend off) the colored tex onto obj
 * @param obj RTex object
 * @param rgba * color
 * @param hsva + color
 */
void RTex_blit_color_pose(oobj obj, oobj tex, mat4 pose, mat4 uv_pose, vec4 rgba, vec4 hsva);

/**
 * Overwrites (blend off) the colored tex onto obj
 * @param obj RTex object
 * @param rgba * color
 * @param hsva + color
 * @note calls RTex_blit_color_pose internally
 */
void RTex_blit_color_rect(oobj obj, oobj tex, vec4 rect, vec4 uv_rect, vec4 rgba, vec4 hsva);

/**
 * Overwrites (blend off) the tex onto obj
 * @param obj RTex object
 * @param rgba * color
 * @param hsva + color
 * @note calls RTex_blit_color_rect internally
 */
void RTex_blit_color(oobj obj, oobj tex, float x, float y, vec4 rgba, vec4 hsva);


/**
 * Overwrites (blend off) the tex onto obj.
 * Sprite size will be tex size / num_sprites.
 * So as it would be a full sprite grid (from left top to bottom right)
 * @param obj RTex object
 * @param x, y as left top position
 * @param sprite used as integer to select the grid sprite
 * @param num_sprites sprite atlas grid size
 * @param rgba * color
 * @param hsva + color
 */
void RTex_blit_sprite_color(oobj obj, oobj tex, float x, float y, vec2 sprite, ivec2 num_sprites, vec4 rgba, vec4 hsva);

/**
 * Overwrites (blend off) the tex onto obj
 * @param obj RTex object
 * @param x, y as left top position
 * @param sprite used as integer to select the grid sprite
 * @param num_sprites sprite atlas grid size
 * @note calls RTex_blit_sprite_color internally
 */
void RTex_blit_sprite(oobj obj, oobj tex, float x, float y, vec2 sprite, ivec2 num_sprites);

/**
 * Overwrites (blend off) the indexed tex onto obj using the palette tex.
 * Each ou8 value in tex is the index color column position from the colored palette texture.
 * @param obj RTex object as indexed image
 * @param palette RTex object for the palette
 * @param palette_offset palette grabbed texel is vec2(index+palette_offset.x, palette_offset.y)
 */
void RTex_blit_indexed_rect(oobj obj, oobj tex, vec4 rect, vec4 uv_rect, oobj palette, vec2 palette_offset);

/**
 * Overwrites (blend off) the indexed tex onto obj using the palette tex.
 * Each ou8 value in tex is the index color column position from the colored palette texture.
 * @param obj RTex object
 * @param palette RTex object for the palette
 * @param palette_offset palette grabbed texel is vec2(index+palette_offset.x, palette_offset.y)
 * @note calls RTex_blit_indexed_rect internally
 */
void RTex_blit_indexed(oobj obj, oobj tex, float x, float y, oobj palette, vec2 palette_offset);

/**
 * Blits the current back buffer into obj.
 * @param obj RTex object
 * @param viewport_obj lbwh in pixels to write to; ivec4_(-1) for full
 * @param viewport_back lbwh in pixels to read from; ivec4_(-1) for full
 */
void RTex_blit_back_ex(oobj obj, ivec4 viewport_obj, ivec4 viewport_back, enum RTex_filter_modes filter);

/**
 * Blits the current back buffer into obj.
 * @param obj RTex object
 */
o_inline void RTex_blit_back(oobj obj, enum RTex_filter_modes filter)
{
    RTex_blit_back_ex(obj, ivec4_(-1), ivec4_(-1), filter);
}

/**
 * Blends on tex onto obj
 * @param obj RTex object
 */
void RTex_blend_pose(oobj obj, oobj tex, mat4 pose, mat4 uv_pose);

/**
 * Blends on tex onto obj
 * @param obj RTex object
 */
void RTex_blend_rect(oobj obj, oobj tex, vec4 rect, vec4 uv_rect);

/**
 * Blends on tex onto obj
 * @param obj RTex object
 * @param x, y as left top position
 * @note calls RTex_blend_rect internally
 */
void RTex_blend(oobj obj, oobj tex, float x, float y);

/**
 * Blends on tex onto obj
 * @param obj RTex object
 * @param rgba * color
 * @param hsva + color
 */
void RTex_blend_color_pose(oobj obj, oobj tex, mat4 pose, mat4 uv_pose, vec4 rgba, vec4 hsva);

/**
 * Blends on tex onto obj
 * @param obj RTex object
 * @param rgba * color
 * @param hsva + color
 * @note calls RTex_blend_pose internally
 */
void RTex_blend_color_rect(oobj obj, oobj tex, vec4 rect, vec4 uv_rect, vec4 rgba, vec4 hsva);

/**
 * Blends on tex onto obj
 * @param obj RTex object
 * @param x, y as left top position
 * @param rgba * color
 * @param hsva + color
 * @note calls RTex_blend_rect internally
 */
void RTex_blend_color(oobj obj, oobj tex, float x, float y, vec4 rgba, vec4 hsva);


/**
 * Blends on tex onto obj
 * Sprite size will be tex size / num_sprites.
 * So as it would be a full sprite grid (from left top to bottom right)
 * @param obj RTex object
 * @param x, y as left top position
 * @param sprite used as integer to select the grid sprite
 * @param num_sprites sprite atlas grid size
 * @param rgba * color
 * @param hsva + color
 */
void RTex_blend_sprite_color(oobj obj, oobj tex, float x, float y, vec2 sprite, ivec2 num_sprites, vec4 rgba, vec4 hsva);

/**
 * Blends on tex onto obj
 * @param obj RTex object
 * @param x, y as left top position
 * @param sprite used as integer to select the grid sprite
 * @param num_sprites sprite atlas grid size
 * @note calls RTex_blit_sprite_color internally
 */
void RTex_blend_sprite(oobj obj, oobj tex, float x, float y, vec2 sprite, ivec2 num_sprites);

/**
 * Blends the indexed tex onto obj using the palette tex.
 * Each ou8 value in tex is the index color column position from the colored palette texture.
 * @param obj RTex object as indexed image
 * @param palette RTex object for the palette
 * @param palette_offset palette grabbed texel is vec2(index+palette_offset.x, palette_offset.y)
 */
void RTex_blend_indexed_rect(oobj obj, oobj tex, vec4 rect, vec4 uv_rect, oobj palette, vec2 palette_offset);

/**
 * Blends the indexed tex onto obj using the palette tex.
 * Each ou8 value in tex is the index color column position from the colored palette texture.
 * @param obj RTex object
 * @param palette RTex object for the palette
 * @param palette_offset palette grabbed texel is vec2(index+palette_offset.x, palette_offset.y)
 * @note calls RTex_blit_indexed_rect internally
 */
void RTex_blend_indexed(oobj obj, oobj tex, float x, float y, oobj palette, vec2 palette_offset);


//
// functions that generate RTex's (more in RTex_manip)
//


/**
 * @param obj RTex object
 * @param format for the resulting RTex
 * @return a new RTex allocated on obj
 * @note calls RTex_blit internally
 */
o_inline RTex *RTex_cast(oobj obj, int format)
{
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_blit(res, obj, 0, 0);
    return res;
}

/**
 * @param obj RTex object
 * @return a new RTex allocated on obj
 * @note calls RTex_cast internally
 */
o_inline RTex *RTex_clone(oobj obj)
{
    return RTex_cast(obj, RTex_format(obj));
}

/**
 * Resizes an RTex with the setup filtering options (default is nearest)
 * @param obj RTex object
 * @param cols resulting cols
 * @param rows resulting rows
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 */
RTex *RTex_resize(oobj obj, int cols, int rows, ou32 format);


/**
 * Merges tex onto obj, blended (alpha can just gain more opacity).
 * @param obj RTex object
 * @param tex RTex object to merge onto obj
 * @param opt_mask_a, opt_mask_b masks which use .r to apply as alpha, NULL -> r_tex_white
 * @param rgba additional color effect for tex
 * @param mode DEFAULT (0) justs merges.
 *             ADDITIVE is like color adding (max of old and new): RED + GREEN => YELLOW
 *             SUBTRACTIVE  is like painting (min of old and new): CYAN + YELLOW => GREEN
 */
void RTex_merge_into(oobj obj, oobj tex,
                     oobj opt_mask_a, oobj opt_mask_b, vec4 rgba, oobj into, enum RTex_merge_mode mode);

/**
 * Merges tex onto obj, blended (alpha can just gain more opacity).
 * @param obj RTex object
 * @param tex RTex object to merge onto obj
 * @param opt_mask_a, opt_mask_b masks which use .r to apply as alpha, NULL -> r_tex_white
 * @param rgba additional color effect for tex
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 */
o_inline RTex *RTex_merge(oobj obj, oobj tex, oobj opt_mask_a, oobj opt_mask_b, vec4 rgba, ou32 format,
                          enum RTex_merge_mode mode)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_merge_into(obj, tex, opt_mask_a, opt_mask_b, rgba, res, mode);
    return res;
}


#endif //R_RTEX_BLIT_H
