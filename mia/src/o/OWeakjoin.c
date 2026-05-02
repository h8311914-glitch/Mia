#include "o/OWeakjoin.h"
#include "o/OJoin.h"
#include "o/OObj_builder.h"

// protected
// called locked
void OWeakjoin__join_deleted(oobj obj, oobj join)
{
    OObj_assert(obj, OWeakjoin);
    OWeakjoin *self = obj;
    assert(self->join == join);
    self->join = NULL;
}

OWeakjoin *OWeakjoin_init(oobj obj, oobj parent, oobj join)
{
    OObj_assert(join, OJoin);
    OJoin *join_obj = join;

    OWeakjoin *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, OWeakjoin_ID);

    self->lock = join_obj->lock;

    // protected
    void OJoin__lock_add(oobj internal_lock);
    OJoin__lock_add(self->lock);

    self->join = join;

    // protected
    void OJoin__add_weak(oobj obj, oobj weak);
    OJoin__add_weak(join, self);


    // set deletor
    self->super.v_del = OWeakjoin__v_del;

    return self;
}

//
// virtual implementations:
//

void OWeakjoin__v_del(oobj obj)
{
    OObj_assert(obj, OWeakjoin);
    OWeakjoin *self = obj;

    // protected
    void OJoin__lock_remove(oobj internal_lock);
    OJoin__lock_remove(self->lock);

    OObj__v_del(obj);
}

//
// object functions:
//

struct oobj_opt OWeakjoin_acquire(oobj obj)
{
    OObj_assert(obj, OWeakjoin);
    OWeakjoin *self = obj;

    OJoin *join = NULL;
    o_lock_block(self->lock) {
        join = self->join;
        if(join) {
            OJoin_add(join, self);
        }
    }
    return oobj_opt(join);
}

bool OWeakjoin_release(oobj obj)
{
    OObj_assert(obj, OWeakjoin);
    OWeakjoin *self = obj;

    bool ok = false;
    o_lock_block(self->lock) {
        ok = self->join && OJoin_is_parent(self->join, self);
        if(ok) {
            OJoin_remove(self->join, self);
        }
    }
    return ok;
}
