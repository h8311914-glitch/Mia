#include "o/ODict.h"
#include "o/OObj_builder.h"
#include "o/OMap.h"


ODict *ODict_init(oobj obj, oobj parent, osize approx_num)
{
    ODict *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, ODict_ID);

    self->map = OMap_new_string_keys(self, sizeof(oobj), approx_num);

    // vfuncs
    self->super.v_op_num = ODict__v_op_num;
    self->super.v_op_at = ODict__v_op_at;

    return self;
}

//
// virtual implementations:
//

osize ODict__v_op_num(oobj obj)
{
    return ODict_num(obj);
}

void *ODict__v_op_at(oobj obj, osize idx)
{
    return ODict_value_at(obj, idx);
}


//
// object functions
//

osize ODict_num(oobj obj)
{
    OObj_assert(obj, ODict);
    ODict *self = obj;
    return OMap_num(self->map);
}

const char *ODict_key_at(oobj obj, osize idx)
{
    OObj_assert(obj, ODict);
    ODict *self = obj;
    const char * const * key = OMap_key_at_void(self->map, idx);
    return *key;
}

void **ODict_value_at_ref(oobj obj, osize idx)
{
    OObj_assert(obj, ODict);
    ODict *self = obj;
    void **val = OMap_value_at_void(self->map, idx);
    return val;
}

osize ODict_get_idx(oobj obj, const char *key)
{
    OObj_assert(obj, ODict);
    ODict *self = obj;
    return OMap_get_idx(self->map, &key);
}

osize ODict_set(oobj obj, const char *key, void *value)
{
    OObj_assert(obj, ODict);
    ODict *self = obj;
    return OMap_set(self->map, &key, &value);
}

void ODict_merge(oobj obj, oobj merge_from)
{
    OObj_assert(obj, ODict);
    ODict *self = obj;
    oobj map;
    if(OObj_check(merge_from, OMap)) {
        map = merge_from;
    } else {
        map = ODict_map(merge_from);
    }
    OMap_merge(self->map, map);
}

bool ODict_remove(oobj obj, const char *key)
{
    OObj_assert(obj, ODict);
    ODict *self = obj;
    return OMap_remove(self->map, &key);
}
