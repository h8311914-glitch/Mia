#include "w/WBox.h"
#include "o/OObj_builder.h"
#include "o/str.h"
#include "o/OArray.h"
#include "m/vec/vec2.h"
#include "u/pose.h"
#include "w/WTheme.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"

#include "o/log.h"

//
// public
//

float WBox_child_weight(oobj obj_child, float default_weight)
{
    const char *string = WObj_option(obj_child, WBox_weight_KEY);
    if (!string) {
        return default_weight;
    }
    float weight;
    int parsed = sscanf(string, "%f", &weight);
    if (parsed != 1) {
        return default_weight;
    }
    return o_max(weight, 0);
}

float WBox_child_weight_set(oobj obj_child, float weight)
{
    // faster than creating and dumping a json...
    char *string = o_strf(obj_child, "%f", weight);
    WObj_option_set(obj_child, WBox_weight_KEY, string);
    o_free(obj_child, string);
    return weight;
}


static void box_weights_cache_update(WBox* self, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn, 
        WObj **list, osize list_num)
{
    o_log_debug_s("WBox_update", "weights cache update");
    if(!self->weights_cached_min_size) {
        self->weights_cached_min_size = OArray_new_dyn(self, NULL, sizeof(vec2), list_num, list_num);
    } else {
        OArray_resize(self->weights_cached_min_size, list_num);
    }
    // save theme num
    int prev_num = WTheme_num(theme);
    
    // run update on all with min_size 0; to set cache
    for (osize i = 0; i < list_num; i++) {
        vec2 child_size = WObj_update(list[i], lt, vec2_(0), enabled, theme, pointer_fn);
        vec2 *cache = OArray_at(self->weights_cached_min_size, i, vec2);
        *cache = child_size;
    }
    
    // reset theme num
    WTheme_reset_to(theme, prev_num);
}


WBox *WBox_init(oobj obj, oobj parent, enum WBox_layout layout)
{
    WObj *super = obj;
    WBox *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WBox_ID);

    self->layout = layout;
    self->spacing = vec2_(0);
    self->weight_default = -1.0f;

    // vfuncs
    super->v_update = WBox__v_update;

    return self;
}

//
// vfuncs
//


vec2 WBox__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WBox);
    WObj *super = obj;
    WBox *self = obj;

    // list needs to be o_free'd
    osize list_num;
    WObj **list = WObj_list_direct(self, &list_num);

    // check weight min_size cache
    if(self->layout == WBox_H_WEIGHTS || self->layout == WBox_V_WEIGHTS) {
        if(!self->weights_cached_min_size || OArray_num(self->weights_cached_min_size) != list_num) {
            box_weights_cache_update(self, lt, min_size, enabled, theme, pointer_fn,
                list, list_num);
        }
    }

    float weight_sum = 0;
    float weight_room = self->layout == WBox_H_WEIGHTS ? min_size.x : min_size.y;
    float weight_target = weight_room;

    vec2 size = vec2_(0);
    vec2 offset = vec2_(0);

    // prepare weights
    if (self->layout == WBox_H_WEIGHTS || self->layout == WBox_V_WEIGHTS) {
        for (osize i = 0; i < list_num; i++) {
            float w = WBox_child_weight(list[i], self->weight_default);
            if (w > 0) {
                weight_sum += w;
                vec2 cache = *OArray_at(self->weights_cached_min_size, i, vec2);
                weight_room -= self->layout == WBox_H_WEIGHTS? cache.x : cache.y;
            } else {
                vec2 used_size = WObj_gen_padding_size(list[i]);
                weight_room -= self->layout == WBox_H_WEIGHTS ? used_size.x : used_size.y;
            }
        }
    }
    
    weight_target /= weight_sum;

    if(self->layout == WBox_H_WEIGHTS) {
        weight_room -= o_max(0,list_num-1) * self->spacing.x;
    } else {
        weight_room -= o_max(0,list_num-1) * self->spacing.y;
    }

    vec2 line_size = vec2_(0);

    for (osize i = 0; i < list_num; i++) {

        vec2 child_min_size = vec2_(0);
        if(self->layout == WBox_H) {
            child_min_size.y = min_size.y;
        }
        if(self->layout == WBox_V) {
            child_min_size.x = min_size.x;
        }
        if (self->layout == WBox_H_V) {
            float right = offset.x + WObj_gen_padding_size(list[i]).x;
            if (right > min_size.x) {
                offset.x = 0;
                offset.y += line_size.y + self->spacing.y;
                line_size = vec2_(0);
            }
        }
        if (self->layout == WBox_V_H) {
            float bottom = offset.y + WObj_gen_padding_size(list[i]).y;
            if (bottom > min_size.y) {
                offset.y = 0;
                offset.x += line_size.x + self->spacing.x;
                line_size = vec2_(0);
            }
        }
        if(self->layout == WBox_H_WEIGHTS) {
            float w = WBox_child_weight(list[i], self->weight_default);
            if (w > 0 && weight_room > 0) {
                float cache = OArray_at(self->weights_cached_min_size, i, vec2)->x;
                float add = w*weight_target - cache;
                add = o_clamp(add, 0, weight_room);
                child_min_size.x = m_floor(cache + add);
                weight_room -= (child_min_size.x-cache);
            }
            child_min_size.y = min_size.y;
        }
        if(self->layout == WBox_V_WEIGHTS) {
            float w = WBox_child_weight(list[i], self->weight_default);
            if (w > 0 && weight_room > 0) {
                float cache = OArray_at(self->weights_cached_min_size, i, vec2)->y;
                float add = w*weight_target - cache;
                add = o_clamp(add, 0, weight_room);
                child_min_size.y = m_floor(cache + add);
                weight_room -= (child_min_size.y-cache);
            }
            child_min_size.x = min_size.x;
        }

        vec2 child_lt = vec2_(lt.x + offset.x, lt.y + offset.y);
        // round to next pixel, so a fraction'ed size does not ruin all of our other objects
        child_lt = vec2_round(child_lt);
        vec2 child_size = WObj_update(list[i], child_lt, child_min_size, enabled, theme, pointer_fn);
        
        // update cache if min_size <= 0
        if(self->layout == WBox_H_WEIGHTS || self->layout == WBox_V_WEIGHTS) {
            vec2 *cache = OArray_at(self->weights_cached_min_size, i, vec2);
            if(child_min_size.x<=0) {
                cache->x = child_size.x;
            }
            if(child_min_size.y<=0) {
                cache->y = child_size.y;
            }
        }

        vec2 s = vec2_add_v(child_size, offset);
        size = vec2_max_v(size, s);

        line_size = vec2_max_v(line_size, child_size);

        if (self->layout == WBox_H || self->layout == WBox_H_V || self->layout == WBox_H_WEIGHTS) {
            offset.x += child_size.x + self->spacing.x;
        }
        if (self->layout == WBox_V || self->layout == WBox_V_H || self->layout == WBox_V_WEIGHTS) {
            offset.y += child_size.y + self->spacing.y;
        }
    }

    o_free(self, list);
    
    return size;
}


//
// object functions:
//


void WBox_weights_cached_min_size_clear(oobj obj)
{
    OObj_assert(obj, WBox);
    WBox *self = obj;
    if(self->weights_cached_min_size) {
        OArray_clear(self->weights_cached_min_size);
    }
}

