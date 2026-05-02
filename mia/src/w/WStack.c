#include "w/WStack.h"
#include "o/OObj_builder.h"
#include "o/OArray.h"
#include "o/str.h"
#include "m/vec/vec2.h"
#include "w/WWindow.h"
#include "r/RTex.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include "o/log.h"

//
// public
//

osize WStack_child_order(oobj obj_child)
{
    const char *string = WObj_option(obj_child, WStack_order_KEY);
    if (!string) {
        return 0;
    }
    osize order;
    osize parsed = sscanf(string, "%"osize_PRI, &order);
    if (parsed != 1) {
        return 0;
    }
    return order;
}

osize WStack_child_order_set(oobj obj_child, osize order)
{
    // faster than creating and dumping a json...
    char *string = o_strf(obj_child, "%"osize_PRI, order);
    WObj_option_set(obj_child, WStack_order_KEY, string);
    o_free(obj_child, string);
    return order;
}


WStack *WStack_init(oobj obj, oobj parent)
{
    WObj *super = obj;
    WStack *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WStack_ID);

    self->window_auto_mode = false;

    // vfuncs
    super->v_update = WStack__v_update;
    super->v_list = WStack__v_list;

    return self;
}

//
// vfuncs
//

vec2 WStack__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    osize list_num;
    WObj **list = WObj_list(obj, &list_num);

    vec2 child_size = WObj__update_list_stacked(list, list_num, lt, min_size, enabled, theme, pointer_fn);

    if (WStack_window_auto_mode(obj)) {
        osize order_max = -1;
        osize order_max_idx = 0;
        osize items_at_max = 0;
        osize active_idx = -1;
        for (osize i = 0; i < list_num; i++) {
            int order = WStack_child_order(list[i]);
            if (order > order_max) {
                order_max = order;
                order_max_idx = i;
                items_at_max = 1;
            } else if (order == order_max) {
                items_at_max++;
            }
            if (OObj_check(list[i], WWindow) && WWindow_active(list[i])) {
                active_idx = i;
            }
        }
        if (active_idx >= 0 && (active_idx != order_max_idx || items_at_max > 1)) {
            o_log_info_s("WStack", "new active top window: %"osize_PRI
                         " with new order: %"osize_PRI,
                         active_idx, order_max+1);
            WStack_child_order_set(list[active_idx], order_max + 1);
        }
    }

    o_free(obj, list);

    return child_size;
}

oobj *WStack__v_list(oobj obj, osize *opt_out_num)
{
    osize list_num;
    WObj **list = WObj_list_direct(obj, &list_num);

    // ivec2_(order, idx)
    oobj data = OArray_new_dyn(obj, NULL, sizeof(ivec2), 0, list_num);

    for (int idx = 0; idx < list_num; idx++) {
        int order = WStack_child_order(list[idx]);

        // if order is bigger than all, it should be set to the last position
        osize data_num = OArray_num(data);
        const ivec2 *data_v = OArray_data(data, const ivec2);
        int pos = (int) data_num;
        for (int sort_i = 0; sort_i < data_num; sort_i++) {
            const ivec2 *item = &data_v[sort_i];
            if (item->v0 > order) {
                pos = sort_i;
                break;
            }
        }

        ivec2 item_set = {{order, idx}};
        OArray_push_at(data, pos, &item_set);
    }

    assert(o_num(data) == list_num);

    // create the resulting sorted list
    WObj **sorted = o_new(obj, WObj *, list_num+1);
    const ivec2 *data_v = OArray_data(data, const ivec2);
    for (int i = 0; i < list_num; i++) {
        const ivec2 *item = &data_v[i];
        sorted[i] = list[item->v1];
    }
    // NULL terminator...
    sorted[list_num] = NULL;

    o_free(obj, list);
    o_del(data);

    o_opt_set(opt_out_num, list_num);
    return (oobj *) sorted;
}
