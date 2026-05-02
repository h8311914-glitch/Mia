#include "o/OList.h"
#include "o/OObj_builder.h"
#include "o/OArray.h"


static osize list_num(void **opt_list, osize num)
{
    if (num<0) {
        num = 0;
        if (opt_list) {
            while (opt_list[num]) {
                num++;
            }
        }
    }
    return num;
}

//
// public
//

OList *OList_init_ex(oobj obj, oobj parent, void **opt_list, osize num,
                     osize start_capacity, int array_dyn_mode)
{
    OList *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, OList_ID);

    self->array = OArray_new_dyn_mode(self, opt_list, sizeof(void*), num, start_capacity, array_dyn_mode);

    // vfuncs
    self->super.v_op_num = OList__v_op_num;
    self->super.v_op_at = OList__v_op_at;

    return self;
}

OList *OList_new(oobj parent, void **opt_list, osize num)
{
    num = list_num(opt_list, num);
    osize capacity = num*2;
    capacity = o_max(capacity, 32);
    return OList_new_ex(parent, opt_list, num, capacity, OArray_REALLOC_DOUBLED);
}


//
// virtual implementations
//

osize OList__v_op_num(oobj obj)
{
    return OList_num(obj);
}

void *OList__v_op_at(oobj obj, osize idx)
{
    return OList_at(obj, idx);
}


//
// object functions
//

void **OList_list(oobj obj)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    return (void **) OArray_data_void(self->array);
}


osize OList_num(oobj obj)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    return OArray_num(self->array);
}

osize OList_capacity(oobj obj)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    return OArray_capacity(self->array);
}

void **OList_at_ref(oobj obj, osize idx)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    return OArray_at(self->array, idx, void*);
}

void OList_realloc(oobj obj, osize capacity, osize reserve_front)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    OArray_realloc(self->array, capacity, reserve_front);
}

void OList_resize(oobj obj, osize num)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    OArray_resize(self->array, num);
}

void OList_resize_front(oobj obj, osize num)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    OArray_resize_front(self->array, num);
}

void *OList_move(oobj obj, oobj into)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    return OArray_move(self->array, into);
}

void *OList_append(oobj obj, void **opt_list, osize num)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    num = list_num(opt_list, num);
    return OArray_append(self->array, opt_list, num);
}

void OList_take(oobj obj, void **out_opt_list, osize num)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    OArray_take(self->array, out_opt_list, num);
}


void *OList_append_front(oobj obj, void **opt_list, osize num)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    num = list_num(opt_list, num);
    return OArray_append_front(self->array, opt_list, num);
}

void OList_take_front(oobj obj, void **out_opt_list, osize num)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    OArray_take_front(self->array, out_opt_list, num);
}

void *OList_append_at(oobj obj, osize idx, void **opt_list, osize num)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    num = list_num(opt_list, num);
    return OArray_append_at(self->array, idx, opt_list, num);
}


void OList_take_at(oobj obj, osize idx, void **out_opt_list, osize num)
{
    OObj_assert(obj, OList);
    OList *self = obj;
    OArray_take_at(self->array, idx, out_opt_list, num);
}
