#ifdef MIA_OPTION_SDL2
#  include <SDL2/SDL_mutex.h>
#else
#  include <SDL3/SDL_mutex.h>
#endif

#include "o/OObj_builder.h"
#include "o/OObjRoot.h"
#include "o/str.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"

#include "o/log.h"


// pointer size should be maximal 8 bytes, so the default pool block size of 256
#define RESOURCES_START_CAPACITY 32

#define BREADCRUMPS 32

void OObj__init_base(oobj obj, struct o_allocator_i allocator)
{
    OObj *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_id_set(self, OObj_ID);
    self->allocator = allocator;

    // manually alloc data array
    self->data_capacity = RESOURCES_START_CAPACITY;

    self->data = o_allocator_i_realloc_try(self->allocator,
                                          NULL, sizeof *self->data,
                                          self->data_capacity);


#ifdef MIA_OPTION_THREAD
    self->thread_id = o_thread_id();
    self->thread_mutex = SDL_CreateMutex();
    o_assume(self->thread_mutex, "SDL_CreateMutex failed");
#endif

    self->v_del = OObj__v_del;

    // the data list is now normally functionable
    self->children = o_new(self, *self->children, RESOURCES_START_CAPACITY);
    self->children_capacity = RESOURCES_START_CAPACITY;
}

static void child_add(OObj *parent, OObj *child)
{
    osize old_num = parent->children_num++;
    if (o_unlikely(parent->children_num > parent->children_capacity)) {
        parent->children_capacity = old_num * 2;
        parent->children = o_renew(parent, parent->children, oobj, parent->children_capacity);
    }
    parent->children[old_num] = child;
}

static osize child_idx(OObj *parent, OObj *child)
{
    // search back, faster in most cases
    for (osize i = parent->children_num - 1; i >= 0; i--) {
        if (parent->children[i] == child) {
            return i;
        }
    }
    return -1;
}

// does NOT delete it
static void child_rem(OObj *parent, OObj *child)
{
    osize idx = child_idx(parent, child);
    assert(idx >= 0 && "invalid child remove");
    parent->children_num--;
    o_memmove(&parent->children[idx], &parent->children[idx + 1], sizeof *parent->children,
              (parent->children_num - idx));
}


static void data_add(OObj *self, void *data)
{
    osize old_num = self->data_num++;
    if (o_unlikely(self->data_num > self->data_capacity)) {
        self->data_capacity = old_num * 2;
        // manually relloc
        self->data = o_allocator_i_realloc_try(self->allocator, self->data, sizeof *self->data, self->data_capacity);
        o_assume(self->data, "data list allocation failed");
    }
    self->data[old_num] = data;
}

static osize data_idx(OObj *self, void *data)
{
    // search back, faster in most cases
    for (osize i = self->data_num - 1; i >= 0; i--) {
        if (self->data[i] == data) {
            return i;
        }
    }
    return -1;
}

// does NOT free it
static void data_rem(OObj *self, void *data)
{
    osize idx = data_idx(self, data);
    assert(idx >= 0 && "invalid data remove");
    self->data_num--;
    o_memmove(&self->data[idx], &self->data[idx + 1], sizeof *self->data, (self->data_num - idx));
}

#ifdef MIA_OPTION_THREAD
o_inline void safe_lock(OObj *self)
{
    if (o_unlikely(self->thread_safe)) {
        o_lock(self);
    } else {
        assert(self->thread_id == o_thread_id() && "thread safety violation"
            "Use OObj_threadsafe_set(obj, true) to enable thread safety for this object branch");
    }
}
o_inline void safe_unlock(OObj *self)
{
    if (o_unlikely(self->thread_safe)) {
        o_unlock(self);
    } else {
        assert(self->thread_id == o_thread_id() && "thread safety violation"
            "Use OObj_threadsafe_set(obj, true) to enable thread safety for this object branch");
    }
}
#else
#  define safe_lock(self) ((void)0)
#  define safe_unlock(self) ((void)0)
#endif

#define safe_lock_block(self) \
for(bool O_NAME_CONCAT(safe_lock_block__run_, __LINE__) = (safe_lock(self), true); \
O_NAME_CONCAT(safe_lock_block__run_, __LINE__); \
O_NAME_CONCAT(safe_lock_block__run_, __LINE__) = (safe_unlock(self), false))

//
// public
//


OObj *OObj_init(oobj obj, oobj parent)
{
    OObj_assert(parent, OObj);
    OObj *par = parent;
    OObj *self = obj;

    OObj__init_base(self, par->allocator);

    self->parent = parent;

#ifdef MIA_OPTION_THREAD
    // inherit thread safe flag
    self->thread_safe = par->thread_safe;
#endif

    child_add(parent, self);

    return self;
}

void OObj__v_del(oobj obj)
{
    OObj_assert(obj, OObj);
    OObj *self = obj;

    // delete children in recursion
    for (osize i = 0; i < self->children_num; i++) {
        OObj *child = self->children[i];
        child->v_del(child);
    }

#ifndef NDEBUG
    // rename id to DObj.* to mark as 'D'eleted
    self->id[0] = 'D';
#endif

#ifdef MIA_OPTION_THREAD
    // kill the mutex
    SDL_DestroyMutex(self->thread_mutex);
#endif

    //
    // free'ing resources:
    //

    if (self->allocator.type == O_ALLOCATOR_TYPE_ARENA) {
        // arena does not need to free resources
        return;
    }

    bool self_in_data = false;

    // free data (also contains the children list)
    for (osize i = 0; i < self->data_num; i++) {
        if (self->data[i] == self) {
            self_in_data = true;
            continue;
        }
        o_allocator_i_realloc_try(self->allocator, self->data[i], 0, 0);
    }
    // manually free the data list
    o_allocator_i_realloc_try(self->allocator, self->data, 0, 0);

    if (self_in_data) {
        // manually free self, if it was in the data list
        o_allocator_i_realloc_try(self->allocator, self, 0, 0);
    }
}


void OObj_id_mismatch_log_wtf(oobj obj, const char *id, const char *func)
{
    oobj *indirection = (oobj *) obj;
    
    // OObj, but wrong type
    if (OObj_check(obj, OObj)) {
        o_log_wtf_s(__func__, "OObj type error, wanted: \"%s\", got: \"%s\", at: %s, trace:", id, OObj_id(obj), func);
        OObj_trace_to_root(obj);
        return;
    }
    
    // use after o_del
#ifndef NDEBUG
    if (o_str_begins(obj, "DObj.")) {
        // revalid
        *(char *)obj = 'O';
        o_log_wtf_s(__func__, "OObj use after delete!, wanted: \"%s\", got: \"%s\", at: %s, trace:", id, OObj_id(obj), func);
        OObj_trace_to_root(obj);
        // reinvalid
        *(char *)obj = 'D';
        return;
    }
#endif
    
    // NULL
    if (!obj) {
        o_log_wtf_s(__func__, "OObj type error, wanted: \"%s\", got NULL!, at: %s, breadcrumps:", id, func);
        OObj_trace_breadcrumps();
        return;
    }
    
    
    // Garbage print
    char first_chars[32] = {0};
    strncpy(first_chars, obj, (sizeof first_chars) - 1);
    o_log_wtf_s(__func__, "OObj type error, wanted: \"%s\", got garbage: \"%s\", at: %s, breadcrumps:", id, first_chars, func);
    OObj_trace_breadcrumps();
    
    if(o_allocator_test_pointer_valid(indirection)) {
        o_log_wtf_s(__func__, "Trying OObj indirection (struct passed) may SEGFAULT...");
     
        if(OObj_check(*indirection, OObj)) {
            o_log_wtf_s(__func__, "OObj type error, wanted: \"%s\", got an indirection to: \"%s\", at: %s, (struct passed?) trace:", id, OObj_id(*indirection), func);
            OObj_trace_to_root(*indirection);
        }
    }
}

void OObj_name_set(oobj obj, const char *opt_name)
{
    OObj_assert(obj, OObj);
    OObj *self = obj;
    safe_lock_block(self) {
        self->opt_name = o_str_clone_realloc(obj, self->opt_name, opt_name);
    }
}


#ifdef MIA_OPTION_THREAD

bool OObj_threadsafe_set(oobj obj, bool set)
{
    OObj_assert(obj, OObj);
    OObj *self = obj;
    assert(!set || self->allocator.type == O_ALLOCATOR_TYPE_HEAP && "threadsafe can only be set on heap allocators, "
        "create an new OObj tree with OObjRoot or OJoin");
    self->thread_safe = set;
    return set;
}

void o_lock(oobj obj)
{
    OObj_assert(obj, OObj);
    OObj *self = obj;
    SDL_LockMutex(self->thread_mutex);
}

bool o_lock_try(oobj obj)
{
    OObj_assert(obj, OObj);
    OObj *self = obj;
#ifdef MIA_OPTION_SDL2
    int ret = SDL_TryLockMutex(self->thread_mutex);
    o_assume(ret != -1, "SDL_TryLockMutex failed");
    return ret == 0;
#else
    return SDL_TryLockMutex(self->thread_mutex);
#endif
}

void o_unlock(oobj obj)
{
    OObj_assert(obj, OObj);
    OObj *self = obj;
    SDL_UnlockMutex(self->thread_mutex);
}

#endif // MIA_OPTION_THREAD


// recursice function
// returns NULL if not found, else the object in which data exists in the data list
static OObj *data_search_r(OObj *self, void *data, oi32 r_level)
{
    // data is in the data list?
    if (data_idx(self, data) >= 0) {
        return self;
    }

    if (r_level <= 0) {
        return NULL;
    }

    // recursion
    for (int i = 0; i < self->children_num; i++) {
        OObj *found = data_search_r(self->children[i], data, r_level - 1);
        if (found) {
            return found;
        }
    }
    return NULL;
}


// recursice function
// returns NULL if not found, else the object in which data exists in the child is parent
static OObj *child_search_r(OObj *self, OObj *child, oi32 r_level)
{
    // child is in the children list?
    if (child_idx(self, child) >= 0) {
        return self;
    }

    if (r_level <= 0) {
        return NULL;
    }

    // recursion
    for (int i = 0; i < self->children_num; i++) {
        OObj *found = child_search_r(self->children[i], child, r_level - 1);
        if (found) {
            return found;
        }
    }
    return NULL;
}

struct oobj_opt OObj_data_search_parent(oobj obj, void *data, oi32 r_level)
{
    OObj_assert(obj, OObj);
    OObj *self = obj;

    OObj *owner;
    safe_lock_block(self) {
        owner = data_search_r(self, data, o_max(0, r_level));
    }
    return oobj_opt(owner);
}


void o_data_move(oobj obj, oobj into, void *data)
{
    if (data == NULL) {
        // noop
        return;
    }
    OObj_assert(obj, OObj);
    OObj_assert(into, OObj);
    OObj *self = obj;
    OObj *in = into;

    assert(self->allocator.impl == in->allocator.impl
           && self->allocator.realloc_try == in->allocator.realloc_try
           && "Must share the same allocator! Consider creating a bridge using an OJoin to move into your tree.");

    safe_lock_block(self) {
        // get the object which owns the resource
        struct oobj_opt owner_opt = OObj_data_search_parent(self, data, oi32_MAX);
        OObj *owner = owner_opt.o;
        assert(owner != NULL && "data not found in the hierarchy?");

        // move
        data_rem(owner, data);
        data_add(in, data);
    }
}


struct oobj_opt OObj_child_search_parent(oobj obj, oobj child, oi32 r_level)
{
    OObj_assert(obj, OObj);
    OObj *self = obj;

    OObj *owner;
    safe_lock_block(self) {
        owner = child_search_r(self, child, o_max(0, r_level));
    }
    return oobj_opt(owner);
}


void o_move(oobj obj, oobj into)
{
    if (o_unlikely(obj == NULL)) {
        // noop
        return;
    }

    assert(OObj_child_search_parent(obj, into, oi32_MAX).o == NULL
           && "into must not be inside the obj children hierarchy");

    OObj_assert(obj, OObj);
    OObj_assert(into, OObj);
    OObj *self = obj;
    OObj *in = into;

    OObj_assert(self->parent, OObj);
    OObj *parent = self->parent;


    assert(parent->allocator.impl == in->allocator.impl
           && parent->allocator.realloc_try == in->allocator.realloc_try
           && "Must share the same allocator! Consider creating a bridge using an OJoin to move into your tree.");

    safe_lock_block(self) {
        // move
        child_rem(parent, self);
        child_add(in, self);
        self->parent = in;
    }

}


oobj *OObj_list_id(oobj obj, osize *opt_out_num, const char *id)
{
    OObj_assert(obj, OObj);
    OObj *self = obj;

    oobj *list = NULL;
    osize num = 0;
    safe_lock_block(self) {
        list = o_new(obj, oobj, self->children_num + 1);
        for (osize i = 0; i < self->children_num; i++) {
            if (OObj_id_check(self->children[i], id)) {
                list[num++] = self->children[i];
            }
        }
        // null terminated list
        list[num] = NULL;
    }

    o_opt_set(opt_out_num, num);
    return list;
}





// recursive find child
static OObj *find_child_r(OObj *self, const char *id, const char *opt_name, oi32 r_level)
{
    // check first if a child is found, before entering recursion
    for (osize i = 0; i < self->children_num; i++) {
        if (!OObj_id_check(self->children[i], id)) {
            continue;
        }
        OObj *child = self->children[i];
        if (!opt_name || o_str_equals(child->opt_name, opt_name)) {
            return child;
        }
    }

    // now check for recursion
    if (r_level <= 0) {
        return NULL;
    }
    for (osize i = 0; i < self->children_num; i++) {
        OObj *found = find_child_r(self->children[i], id, opt_name, r_level - 1);
        if (found) {
            return found;
        }
    }
    return NULL;
}


struct oobj_opt OObj_find_id(oobj obj, const char *id, const char *opt_name, oi32 r_level)
{
    if (o_unlikely(!obj)) {
        return oobj_opt(NULL);
    }
    OObj_assert(obj, OObj);
    OObj *self = obj;
    OObj *search = NULL;
    safe_lock_block(self) {
        search = find_child_r(self, id, opt_name, o_max(0, r_level));
    }
    return oobj_opt(search);
}


// recursive find parent (if full is true)
static OObj *find_parent_r(OObj *self, const char *id, const char *opt_name, oi32 r_level)
{
    OObj *parent = self->parent;
    if (o_unlikely(parent == NULL)) {
        return NULL;
    }
    OObj *found = NULL;
    if (OObj_id_check(parent, id)) {
        found = parent;
        if (opt_name && !o_str_equals(parent->opt_name, opt_name)) {
            found = NULL;
        }
    }
    if (found) {
        return found;
    }

    if (r_level <= 0) {
        return NULL;
    }
    return find_parent_r(parent, id, opt_name, r_level - 1);
}

struct oobj_opt OObj_find_parent_id(oobj obj, const char *id, const char *opt_name, oi32 r_level)
{
    if (o_unlikely(!obj)) {
        return oobj_opt(NULL);
    }
    OObj_assert(obj, OObj);
    OObj *self = obj;
    OObj *search = NULL;
    safe_lock_block(self) {
        search = find_parent_r(self, id, opt_name, o_max(0, r_level));
    }
    return oobj_opt(search);
}

void OObj_tree_metrics(oobj obj, osize *out_oobjs, osize *out_datas, osize *out_depth) {
    *out_oobjs = *out_datas = *out_depth = 0;
    if (o_unlikely(!obj)) {
        return;
    }
    OObj_assert(obj, OObj);
    OObj *self = obj;

    // we don't want to edit the current obj's tree, so we create a new simple root
    // this root will not get registered to the root list
    oobj root = OObjRoot_new(NULL, o_allocator_heap_new(), false);

    struct item {
        OObj *obj;
        osize depth;
        bool unlock;
    };

    osize stack_num = 1;
    osize stack_capacity = 512;
    struct item *stack = o_new(root, struct item, stack_capacity);
    stack[0].obj = self;
    stack[0].depth = 0;
    stack[0].unlock = false;

    while (stack_num > 0) {
        struct item cur = stack[--stack_num];
        OObj_assert(cur.obj, OObj);

        if (cur.unlock) {
            safe_unlock(cur.obj);
            continue;
        }
        safe_lock(cur.obj);

        *out_oobjs += cur.obj->children_num;
        *out_datas += cur.obj->data_num;
        *out_depth = o_max(*out_depth, cur.depth);

        osize rem = stack_capacity - stack_num;
        osize add = cur.obj->children_num + 1;
        if (rem < add) {
            stack_capacity = o_max(stack_capacity * 2, stack_num + add);
            stack = o_renew(root, stack, struct item, stack_capacity);
        }

        struct item unlock;
        unlock = cur;
        unlock.unlock = true;
        stack[stack_num++] = unlock;

        for (osize i = 0; i < cur.obj->children_num; i++) {
            struct item put;
            put.obj = cur.obj->children[i];
            put.depth = cur.depth + 1;
            put.unlock = false;
            stack[stack_num++] = put;
        }

    }

    o_del(root);
}

static void print_indent(int lvl)
{
    for (int ident = 0; ident < 2 * lvl; ident++) {
        printf(" ");
    }
}

static void children_trace_r(OObj *self, int lvl, oi32 r_level)
{
    for (osize i = 0; i < self->children_num; i++) {
        OObj *child = self->children[i];
        print_indent(lvl);
        printf("%s \"%s\" at: %p\n", child->id, o_or(child->opt_name, ""), (void *) child);
        if (r_level > 0) {
            children_trace_r(child, lvl + 1, r_level - 1);
        }
    }
}

void OObj_trace_children(oobj obj, oi32 r_level)
{
    OObj_assert(obj, OObj);
    OObj *self = obj;
    safe_lock_block(self) {
        children_trace_r(self, 0, o_max(0, r_level));
    }
    fflush(stdout);
}

void OObj_trace_to_root(oobj obj)
{
    OObj_assert(obj, OObj);
    OObj *step = obj;
    do {
        printf("\\ %s \"%s\" C%i D%i U%i\n", 
            step->id, 
            o_or(step->opt_name, ""), 
            (int) step->children_num,
            (int) step->data_num,
            step->userdata!=NULL);
        step = step->parent;
    } while(step);
}

#if defined (MIA_OPTION_BREADCRUMPS) && !defined(NDEBUG) 
static _Thread_local struct {
    char ring[BREADCRUMPS][32];
    int pos;
} OObj_breadcrumps_L;
void OObj__breadcrump(OObj *self)
{
    // protected function
    OObj_breadcrumps_L.pos++;
    OObj_breadcrumps_L.pos%=BREADCRUMPS;
    char *ring = OObj_breadcrumps_L.ring[OObj_breadcrumps_L.pos];
    snprintf(ring, 32, "%s \"%s\" C%i D%i U%i", 
            self->id, 
            o_or(self->opt_name, ""), 
            (int) self->children_num,
            (int) self->data_num,
            self->userdata!=NULL);
}
void OObj_trace_breadcrumps(void)
{
    for(int i=0; i<BREADCRUMPS; i++) {
        int pos = OObj_breadcrumps_L.pos - i;
        pos = o_mod(pos, BREADCRUMPS);
        printf("%3i %s\n", -i-1, OObj_breadcrumps_L.ring[pos]);
    }
}
#else
void OObj_trace_breadcrumps(void)
{
    puts("USE MIA_OPTION_BREADCRUMPS");
}
#endif


void *o_realloc_try(oobj obj, void *mem, osize element_size, osize num)
{
    OObj_assert(obj, OObj);
    OObj *self = obj;

    void *ret = NULL;
    safe_lock_block(self) {

        // new allocation -> new entry in the data list
        if (!mem) {
            mem = o_allocator_i_realloc_try(self->allocator, NULL, element_size, num);
            if (mem) {
                data_add(self, mem);
            }
            ret = mem;
            continue;
        }

        // search object which holds the data
        struct oobj_opt owner_opt = OObj_data_search_parent(obj, mem, oi32_MAX);
        OObj *owner = owner_opt.o;
        assert(owner && "allocation not found in the object hierarchy tree");

        safe_lock_block(owner) {

            osize idx = data_idx(owner, mem);
            assert(idx >= 0 && "allocation not found in the object");

            osize n = osize_mul(element_size, num);

            // realloc or free
            void *old_mem = mem;
            mem = o_allocator_i_realloc_try(owner->allocator, mem, element_size, num);

            if (n <= 0) {
                // free
                data_rem(owner, old_mem);
                ret = NULL;
            } else {
                // realloc
                owner->data[idx] = mem;
                ret = mem;
            }

        } //lock owner
    } //lock self

    return ret;
}

void OObj_del(oobj obj)
{
    if (o_unlikely(!obj)) {
        return;
    }
    OObj_assert(obj, OObj);
    OObj *self = obj;

    if (self->parent) {
        OObj_assert(self->parent, OObj);
        OObj *parent = self->parent;
        child_rem(parent, self);
    }

    self->v_del(self);
}


//
// OObjRoot.h stuff
//


static struct {
    struct o_allocator_i allocator;
    oobj *list;
    osize list_capacity;
    osize list_num;
    void *mutex;
} OObjRoot_L;

static void OObjRoot__list_init(void) {
    if (o_likely(OObjRoot_L.list)) {
        return;
    }
    o_init_assert();

    OObjRoot_L.allocator = o_allocator_heap_new();

    // no need to clean up / free this list here, it remains as long as the app runs...
    OObjRoot_L.list_capacity = 32;
    OObjRoot_L.list_num = 0;
    OObjRoot_L.list = o_allocator_i_realloc_try(OObjRoot_L.allocator, NULL, sizeof(oobj), OObjRoot_L.list_capacity);
    o_assume(OObjRoot_L.list, "OObjRoot_L.list allocation failed");

#ifdef MIA_OPTION_THREAD
    OObjRoot_L.mutex = SDL_CreateMutex();
    o_assume(OObjRoot_L.mutex, "SDL_CreateMutex failed");
#endif
}
OObj *OObjRoot_init(oobj obj, const char *name, struct o_allocator_i allocator)
{
    OObjRoot__list_init();

    OObj *self = obj;
    OObj__init_base(self, allocator);
    OObj_id_set(self, OObjRoot_ID);
    OObj_name_set(self, name);
#ifdef MIA_OPTION_THREAD
    self->thread_safe = false;
#endif
    return self;
}

OObj *OObjRoot_new(const char *name, struct o_allocator_i allocator, bool add_to_root_list)
{
    OObj stacked_self;
    OObjRoot_init(&stacked_self, name, allocator);
    OObj *self = o_new(&stacked_self, OObj, 1);
    *self = stacked_self;
    if (add_to_root_list) {
        OObjRoot__list_add(self);
    }
    return self;
}

void OObjRoot__v_del(oobj obj)
{
    OObjRoot__list_rem(obj);
    OObj__v_del(obj);
}

void OObjRoot__list_lock(void)
{
    OObjRoot__list_init();
#ifdef MIA_OPTION_THREAD
    SDL_LockMutex(OObjRoot_L.mutex);
#endif
}


void OObjRoot__list_unlock(void)
{
    OObjRoot__list_init();
#ifdef MIA_OPTION_THREAD
    SDL_UnlockMutex(OObjRoot_L.mutex);
#endif
}


void OObjRoot__list_add(oobj obj) {
    OObj_assert(obj, OObjRoot);
    OObj *self = obj;

    OObjRoot__list_lock_block {
        for (osize i = 0; i < OObjRoot_L.list_num; i++) {
            if (OObjRoot_L.list[i] == obj) {
                o_log_s(__func__, "OObjRoot_L.list already contains %p", obj);
                return;
            }
        }

        if (OObjRoot_L.list_num >= OObjRoot_L.list_capacity) {
            OObjRoot_L.list_capacity = o_max(OObjRoot_L.list_capacity * 2, OObjRoot_L.list_num+1);
            OObjRoot_L.list = o_renew(obj, OObjRoot_L.list, oobj, OObjRoot_L.list_capacity);
            OObjRoot_L.list = o_allocator_i_realloc_try(OObjRoot_L.allocator, OObjRoot_L.list, sizeof(oobj), OObjRoot_L.list_capacity);
            o_assume(OObjRoot_L.list, "OObjRoot_L.list reallocation failed");
        }
        OObjRoot_L.list[OObjRoot_L.list_num++] = obj;
    }

    // install v del to remove
    self->v_del = OObjRoot__v_del;
}

void OObjRoot__list_rem(oobj obj) {
    OObj_assert(obj, OObjRoot);
    OObj *self = obj;

    bool found = false;
    OObjRoot__list_lock_block {
        for (osize i = 0; i < OObjRoot_L.list_num; i++) {
            if (OObjRoot_L.list[i] == obj) {
                osize rem = OObjRoot_L.list_num - i - 1;
                if (rem>0) {
                    o_memmove(OObjRoot_L.list[i], OObjRoot_L.list[i+1], sizeof(oobj), rem);
                }
                found = true;
                OObjRoot_L.list_num--;
                break;
            }
        }
    }

    if (found) {
        // uninstall v del
        self->v_del = OObj__v_del;
    } else {
        o_log_warn_s(__func__, "OObjRoot_L.list not found %p", obj);
    }
}



osize OObjRoot__list_num(void)
{
    OObjRoot__list_init();
    return OObjRoot_L.list_num;
}

struct oobj_opt OObjRoot__list_at(osize index)
{
    OObjRoot__list_init();
    oobj ret = NULL;
    OObjRoot__list_lock_block {
        if (0 <= index &&  index < OObjRoot_L.list_num) {
            ret = OObjRoot_L.list[index];
        }
    }
    return oobj_opt(ret);
}
