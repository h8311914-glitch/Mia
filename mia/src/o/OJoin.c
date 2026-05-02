#include "o/OJoin.h"
#include "o/OObj_builder.h"
#include "o/OObjRoot.h"
#include "o/OList.h"
#include "o/ODelcallback.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


struct parent {
    OJoin *self;
    oobj parent;
    oobj delcb;
};


//
// OJoin and OWeakjoin share a lock via a new OObjRoot, only deleted if all references drop (cnt)
//

// protected
void OJoin__lock_add(oobj internal_lock)
{
    o_lock_block(internal_lock)
    {
        osize *cnt = o_user(internal_lock);
        *cnt += 1;
    }
}

// protected
void OJoin__lock_remove(oobj internal_lock)
{
    bool delete_lock = false;
    o_lock_block(internal_lock)
    {
        osize *cnt = o_user(internal_lock);
        *cnt -= 1;
        assert(*cnt >= 0);
        delete_lock = *cnt == 0;
    }
    if (delete_lock) {
        o_del(internal_lock);
    }
}

static void join__lock_create(OJoin *self)
{
    // new unregistered root; used as a lock
    self->lock = OObjRoot_new(NULL, o_allocator_heap_new(), false);
    osize *cnt = o_user_new0(self->lock, osize, 1);
    *cnt = 1;
}


//
// OWeakjoin adapter
//

static void OJoin__remove_weak(oobj obj, oobj weak, osize opt_idx)
{
    OObj_assert(obj, OJoin);
    OJoin *self = obj;

    o_lock_block(self->lock) {
        // check if it's the list
        osize idx = opt_idx;
        struct parent *p = NULL;
        if (idx>=0) {
            p = o_at(self->weaks, idx);
        } else {
            for (osize i = 0; i < o_num(self->weaks); i++) {
                p = o_at(self->weaks, i);
                if (p->parent == weak) {
                    idx = i;
                    // break out of for loop, not lock block (would be invalid)
                    break;
                }
            }
            if (idx < 0) {
                o_log_error_s(__func__, "parent is not in the weaks list?");
                // jump out of lock block
                continue;
            }
        }

        // protected
        // called locked
        void OWeakjoin__join_deleted(oobj obj, oobj join);
        OWeakjoin__join_deleted(weak, self);

        // only delete if not called by autodelete ODelcallback
        if(p->delcb) {
            o_user_set(p->delcb, NULL);
            o_del(p->delcb);
        }

        o_free(self, p);
        OList_pop_at(self->weaks, idx);
    }
}

static void autoremove_weak(oobj obj)
{
    struct parent *p = o_user(obj);
    if(p) {
        // will delete itself already after this function
        p->delcb = NULL;
        OJoin__remove_weak(p->self, p->parent, -1);
    }
}

// protected
void OJoin__add_weak(oobj obj, oobj weak)
{
    OObj_assert(obj, OJoin);
    OJoin *self = obj;

    o_lock_block(self->lock) {
        struct parent *p = o_new0(self, struct parent, 1);
        p->self = self;
        p->parent = weak;

        OList_push(self->weaks, p);

        // create a del callback that is called, when parent gets deleted
        p->delcb = ODelcallback_new(weak, autoremove_weak);
        o_user_set(p->delcb, p);
    }
}


static void autoremove_parent(oobj obj)
{
    struct parent *p = o_user(obj);
    if(p) {
        // will delete itself already after this function
        p->delcb = NULL;
        OJoin_remove(p->self, p->parent);
    }
}

static void join_init_base(OJoin *self, struct o_allocator_i allocator) {
    o_clear(self, sizeof *self, 1);

    // protected
    void OObj__init_base(oobj obj, struct o_allocator_i allocator);

    OObj__init_base(self, allocator);
    OObj_id_set(self, OJoin_ID);

    join__lock_create(self);

    // set deletor
    self->super.v_del = OJoin__v_del;
}

static void join_init_parents_weaks(OJoin *self, oobj *parents, osize parents_size)
{
    self->parents = OList_new(self, NULL, 0);
    if(parents_size<=0) {
        parents_size = o_list_num(parents);
    }
    assert(parents_size > 0 && parents && parents[0]);
    for (osize i = 0; i < parents_size; i++) {
        OJoin_add(self, parents[i]);
    }

    self->weaks = OList_new(self, NULL, 0);
}


static struct parent *join_parent_search(OJoin *self, oobj parent, osize *opt_out_idx)
{
    struct parent *p = NULL;
    osize idx = -1;
    o_lock(self->lock);
    {
        for (osize i = 0; i < o_num(self->parents); i++) {
            struct parent *at = o_at(self->parents, i);
            if (at->parent == parent) {
                idx = i;
                p = at;
                break;
            }
        }
    }
    o_unlock(self->lock);
    o_opt_set(opt_out_idx, idx);
    return p;
}

//
// public
//


OJoin *OJoin_new_super(osize object_size, oobj *parents, osize parents_size, struct o_allocator_i allocator)
{
    assert(object_size >= (osize) sizeof(OJoin));
    OJoin stacked_self;
    join_init_base(&stacked_self, allocator);
    OJoin *self = o_alloc0(&stacked_self, object_size, 1);
    *self = stacked_self;
    // needs a fixed OJoin object handle...
    join_init_parents_weaks(self, parents, parents_size);
    return self;
}


//
// virtual implementations:
//

void OJoin__v_del(oobj obj)
{
    OObj_assert(obj, OJoin);
    OJoin *self = obj;

    o_lock(self->lock);

    bool delete = o_num(self->parents) == 0;
    if (delete) {
        while(o_num(self->weaks)) {
            osize idx = o_num(self->weaks) - 1;
            struct parent *last = o_at(self->weaks, idx);
            OJoin__remove_weak(self, last->parent, idx);
        }
    }

    o_unlock(self->lock);

    if(delete) {
        OJoin__lock_remove(self->lock);
        OObj__v_del(self);
    }
}

//
// object functions:
//

void OJoin_del(oobj obj)
{
    OObj_assert(obj, OJoin);
    OJoin *self = obj;

    // manual locking, because OJoin_remove may delete self
    oobj lock = self->lock;
    o_lock(lock);

    // tmp push lock up, so OJoin_remove wont delete the lock
    OJoin__lock_add(lock);

    // remove all parents
    while (o_num(self->parents)) {
        struct parent *last = o_at(self->parents, o_num(self->parents) - 1);
        if (OJoin_remove(self, last->parent)) {
            //
            // got deleted, self is invalid now
            //
            break;
        }
    }

    // safe unlock, even if self hot deleted
    o_unlock(lock);
    // remove tmp push up
    OJoin__lock_remove(lock);
}

osize OJoin_num_parents(oobj obj)
{
    OObj_assert(obj, OJoin);
    OJoin *self = obj;
    osize num = 0;
    o_lock_block(self->lock) {
        num = o_num(self->parents);
    }
    return num;
}

bool OJoin_is_parent(oobj obj, oobj parent)
{
    OObj_assert(obj, OJoin);
    OJoin *self = obj;
    return join_parent_search(self, parent, NULL) != NULL;
}


void OJoin_add(oobj obj, oobj parent)
{
    OObj_assert(obj, OJoin);
    OJoin *self = obj;

    o_lock_block(self->lock) {

        // ignore doubles
        if (OJoin_is_parent(self, parent)) {
            continue;
        }

        struct parent *p = o_new0(self, struct parent, 1);
        p->self = self;
        p->parent = parent;

        OList_push(self->parents, p);

        // create a del callback that is called, when parent gets deleted
        p->delcb = ODelcallback_new(parent, autoremove_parent);
        o_user_set(p->delcb, p);
    }
}

bool OJoin_remove(oobj obj, oobj parent)
{
    OObj_assert(obj, OJoin);
    OJoin *self = obj;

    bool deleted = false;

    oobj lock = self->lock;
    o_lock(lock);

    // check if it's the list
    osize idx;
    struct parent *p = join_parent_search(self, parent, &idx);
    if (!p) {
        o_log_error_s(__func__, "parent is not joined?");
    } else {
        // only delete if not called by autodelete ODelcallback
        if(p->delcb) {
            o_user_set(p->delcb, NULL);
            o_del(p->delcb);
        }

        o_free(self, p);
        OList_pop_at(self->parents, idx);

        deleted = o_num(self->parents) <= 0;
    }

    if (deleted) {
        // OObj_del will remove its own lock, so we push it up before
        OJoin__lock_add(lock);
        OObj_del(self);
        //
        // self may be invalid now...
        //

        // safely unlock this lock, even after self may be invalid
        o_unlock(lock);
        // remove the tmp lock push
        OJoin__lock_remove(lock);
    } else {
        o_unlock(lock);
    }

    return deleted;
}

