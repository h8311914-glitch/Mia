#ifdef MIA_OPTION_SDL2
#  include <SDL2/SDL_stdinc.h>
#else
#  include <SDL3/SDL_stdinc.h>
#endif

#include "o/allocator.h"
#include "o/common.h"


#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


static struct {
   void *min, *max; 
} allocator_L;


static void allocator__update_minmax(void *ptr, osize bytes)
{
    obyte *end = ptr;
    end += (bytes-1);
    if(!allocator_L.min || allocator_L.min > ptr) {
        allocator_L.min = ptr;
    }
    if(!allocator_L.max || allocator_L.max < ptr) {
        allocator_L.max = ptr;
    }
}

static void *heap_realloc_try(struct o_allocator_i iface, void *mem, osize element_size, osize num)
{
    osize n = osize_mul(element_size, num);
    n = o_max(0, n);
    if(!mem && n==0) {
        return NULL;
    }
    // o_log_trace_s(__func__,"allocating: %p %i", mem, (int) n);
    mem = SDL_realloc(mem, n);
    if (o_unlikely(n > 0 && !mem)) {
        o_log_debug_s(__func__, "failed to allocate %i bytes", (int) n);
    }
    return mem;
}

struct o_allocator_i o_allocator_heap_new(void)
{
    return (struct o_allocator_i) {O_ALLOCATOR_TYPE_HEAP, heap_realloc_try, NULL};
}


bool o_allocator_test_pointer_valid(const void *ptr)
{
    return ptr && ptr >= allocator_L.min && ptr <= allocator_L.max;
}

//
// Pool
//

struct pool {
    int block_size;
    int blocks_in_pool;
    osize pool_byte_size;

    obyte **pools;
    int pools_num;

    // size of pools_num * blocks_in_pool
    obyte **stack;
    osize stack_num;

    void *mutex;
};

#define pool_assert(allocator) assert((allocator).type == O_ALLOCATOR_TYPE_POOL)

static void pool_stack_push(struct pool *p, void *mem)
{
    osize stack_size = (osize) p->pools_num * (osize) p->blocks_in_pool;
    if(o_unlikely(p->stack_num+1 >= stack_size)) {
        o_log_wtf_s(__func__, "free failed, already all free or another double free bug!");
        return;
    }
    p->stack[p->stack_num++] = mem;
}

static void *pool_stack_pop(struct pool *p)
{
    if(p->stack_num<=0) {
        // allocate a new pool
        p->pools_num++;
        o_log_trace_s("o_allocator_pool", "allocating a new pool! (pools now: %i)", p->pools_num);

        p->pools = SDL_realloc(p->pools, sizeof(obyte *) * p->pools_num);
        o_assume(p->pools, "pool list allocation failed?");
        obyte *pool = SDL_malloc(p->pool_byte_size);
        o_assume(pool, "pool allocation failed?");
        
        allocator__update_minmax(pool, p->pool_byte_size);
        
        p->pools[p->pools_num-1] = pool;

        osize stack_size = (osize) p->pools_num * (osize) p->blocks_in_pool;
        p->stack = SDL_realloc(p->stack, sizeof(obyte *) * stack_size);
        o_assume(p->stack, "pool stack allocation failed?");

        p->stack_num = p->blocks_in_pool;

        for(int i=0; i<p->stack_num; i++) {
            p->stack[i] = pool + p->block_size * i;
        }
    }

    return p->stack[--p->stack_num];
}

static void *pool_realloc_try(struct o_allocator_i iface, void *mem, osize element_size, osize num)
{
    pool_assert(iface);
    struct pool *p = iface.impl;

    osize n = osize_mul(element_size, num);
    n = o_max(0, n);
    if(!mem && n==0) {
        // noop
        mem = NULL;
        return mem;
    }
    if(n==0) {
        // free
        if(o_allocator_pool_pointer_pooled(iface, mem)) {
            pool_stack_push(p, mem);
            mem = NULL;
            return mem;
        }
        SDL_free(mem);
        mem = NULL;
        return mem;
    }
    // (re)alloc
    if(n<=p->block_size) {
        if(mem) {
            // realloc, either mem was in the pool before, so we keep the blok
            //          or it was not, but the size has shrunk and the old one can fit the values
            return mem;
        }
        // alloc
        mem = pool_stack_pop(p);
    return mem;
    }

    // (re)alloc using heap
    if(mem && o_allocator_pool_pointer_pooled(iface, mem)) {
        // if old was in pool, alloc, o_memcpy, free from pool
        void *old_mem = mem;
        mem = SDL_malloc(n);
        if (o_unlikely(!mem)) {
            o_log_debug_s(__func__, "failed to allocate %i bytes", n);
        } else {
            o_memcpy(mem, old_mem, 1, p->block_size);
        }
        pool_stack_push(p, old_mem);
    return mem;
    }
    // alloc
    mem = SDL_realloc(mem, n);
    if (o_unlikely(!mem)) {
        o_log_debug_s(__func__, "failed to allocate %i bytes", n);
    }

    return mem;
}

struct o_allocator_i o_allocator_pool_new(int block_size, int blocks_in_pool, int start_pools)
{
    if(block_size <= 0) {
        block_size = 512;
    }
    if(blocks_in_pool <=0) {
        // so with 512 as block size, a pool is around 1MB
        blocks_in_pool = 2048;
    }
    if(start_pools <=0) {
        start_pools = 4;
    }
    osize pool_byte_size = (osize) block_size * (osize) blocks_in_pool;

    obyte **pools = SDL_malloc(sizeof(obyte*) * start_pools);
    o_assume(pools, "pool list allocation failed?");

    int num_pools = 0;

    for(int i=0; i<start_pools; i++) {
        pools[i] = SDL_malloc(pool_byte_size);
        
        if(o_unlikely(!pools[i])) {
            o_log_error_s(__func__, "failed to allocate a pool");
            break;
        }
        num_pools = i+1;
        
        allocator__update_minmax(pools[i], pool_byte_size);
        
    }

    o_assume(num_pools>0, "failed to allocate a single pool");

    osize stack_num = (osize) num_pools * (osize) blocks_in_pool;
    obyte **stack = SDL_malloc(sizeof (obyte *) * stack_num);
    o_assume(stack, "failed to allocate the fifo list of available pool memory");
    osize stack_i = 0;
    for(int i=0; i<num_pools; i++) {
        for(int j=0; j<blocks_in_pool; j++) {
            stack[stack_i++] = pools[i] + block_size * j;
        }
    }

    struct pool *p = SDL_calloc(sizeof *p, 1);
    o_assume(p, "failed to create pool struct");
    p->block_size = block_size;
    p->blocks_in_pool = blocks_in_pool;
    p->pool_byte_size = pool_byte_size;
    p->pools = pools;
    p->pools_num = num_pools;
    p->stack = stack;
    p->stack_num = stack_num;

    o_log_trace_s("o_allocator_pool", "created %i pools with %i blocks and %i block_size",
                  p->pools_num, p->blocks_in_pool, p->block_size);

    return (struct o_allocator_i) {O_ALLOCATOR_TYPE_POOL, pool_realloc_try,p};
}

void o_allocator_pool_del(struct o_allocator_i *self)
{
    if(!self || !self->impl) {
        return;
    }
    pool_assert(*self);
    struct pool *p = self->impl;

    SDL_free(p->pools);
    SDL_free(p);
    o_clear(self, sizeof *self, 1);
}

int o_allocator_pool_block_size(struct o_allocator_i self)
{
    pool_assert(self);
    struct pool *p = self.impl;
    return p->block_size;
}

int o_allocator_pool_blocks_in_pool(struct o_allocator_i self)
{
    pool_assert(self);
    struct pool *p = self.impl;
    return p->blocks_in_pool;
}

int o_allocator_pool_pools_num(struct o_allocator_i self)
{
    pool_assert(self);
    struct pool *p = self.impl;
    return p->pools_num;
}

int o_allocator_pool_blocks_available(struct o_allocator_i self)
{
    pool_assert(self);
    struct pool *p = self.impl;
    return p->stack_num;
}


bool o_allocator_pool_pointer_pooled(struct o_allocator_i self, const void *mem)
{
    pool_assert(self);
    struct pool *p = self.impl;
    
    const obyte *ptr = mem;
    for(osize i=0; i<p->pools_num; i++) {
        if(p->pools[i] <= ptr && ptr < p->pools[i]+p->pool_byte_size) {
            ou64 pool_mod = ((ou64) p->pools[i]) % p->block_size;
            ou64 ptr_mod = ((ou64) ptr) % p->block_size;
            if(pool_mod == ptr_mod) {
                return true;
            }
            o_assume(false, "invalid ptr that is in the pool! did you manipulate it?");
        }
    }
    return false;
}

//
// Arena
//

struct arena {
    obyte *region;
    osize size;
    osize used;
    osize last;

    void *mutex;
};

struct arena_alloc_info {
    osize data_size;
    osize full_chunk_size;
};
_Static_assert(sizeof(struct arena_alloc_info) <= O_ALIGN_SYSTEM_MAX, "align error");

#define arena_assert(allocator) assert((allocator).type == O_ALLOCATOR_TYPE_ARENA)


static void *arena_realloc_try(struct o_allocator_i iface, void *mem, osize element_size, osize num)
{
    arena_assert(iface);
    struct arena *a = iface.impl;

    // for each new entry, set the
    //      struct arena_alloc_info
    //      and then (aligned to O_ALIGN_SYSTEM_MAX) the memory
    //      in the buffer

    // free and realloc may replace and work if the last allocated mem is equal to mem
    // else for free: noop, realloc reallocates new and uses o_memcpy


    osize n = osize_mul(element_size, num);
    n = o_max(0, n);
    osize full_chunk = n + O_ALIGN_SYSTEM_MAX;
    if(full_chunk % O_ALIGN_SYSTEM_MAX != 0) {
        // align
        full_chunk = (full_chunk/O_ALIGN_SYSTEM_MAX + 1) * O_ALIGN_SYSTEM_MAX;
    }
    if(!mem && n==0) {
        // noop
        mem = NULL;
        return mem;
    }
    if(n==0) {
        // free
        if(mem == &a->region[a->last + O_ALIGN_SYSTEM_MAX]) {
            // free possible
            a->used = a->last;
        }
        mem = NULL;
        return mem;
    }

    if (o_unlikely(full_chunk > (a->size - a->used))) {
        o_log_debug(__func__, "failed to allocate %i bytes", n);
        mem = NULL;
        return mem;
    }


    if(mem == &a->region[a->last + O_ALIGN_SYSTEM_MAX]) {
        // realloc directly possible
        a->used = a->last;
    }

    void *old_mem = mem;
    a->last = a->used;
    struct arena_alloc_info *new_info = (struct arena_alloc_info *) &a->region[a->last];
    new_info->data_size = n;
    new_info->full_chunk_size = full_chunk;
    mem = &a->region[a->last + O_ALIGN_SYSTEM_MAX];
    a->used += full_chunk;

    if(old_mem && mem != old_mem) {
        // realloc needs o_memcpy, so first find mem, and second o_memcpy that thing
        struct arena_alloc_info *from = NULL;
        osize pos = 0;
        do {
            struct arena_alloc_info *info = (struct arena_alloc_info *) &a->region[pos];
            if(info->full_chunk_size <= 0) {
                break;
            }
            assert(info->data_size>0);
            if(old_mem == &a->region[pos + O_ALIGN_SYSTEM_MAX]) {
                from = info;
                break;
            }
            pos += info->full_chunk_size;
        } while(pos < a->size);

        if(o_unlikely(!from)) {
            o_log_error_s(__func__, "realloc on invalid memory!", n);
        } else {
            // actual o_memcpy
            o_memcpy(mem, old_mem, 1, from->full_chunk_size);
        }
    }

    return mem;
}

struct o_allocator_i o_allocator_arena_new(osize size)
{
    obyte *region;
    do {
        region = SDL_malloc(size);
        if(!region) {
            o_assume(size>=1024, "arena region allocation failed?");
            o_log_error_s(__func__, "failed to allocate arena region, "
          "retrying with half the size: %" osize_PRI"/%" osize_PRI, size/2, size);
            size /= 2;
            continue;
        }
    } while(!region);
    
    allocator__update_minmax(region, size);
    
    struct arena *a = SDL_calloc(sizeof *a, 1);
    o_assume(a, "failed to create arena struct");
    a->region = region;
    a->size = size;

    return (struct o_allocator_i) {O_ALLOCATOR_TYPE_ARENA, arena_realloc_try, a};
}


void o_allocator_arena_del(struct o_allocator_i *self)
{
    if(!self || !self->impl) {
        return;
    }
    arena_assert(*self);
    struct arena *a = self->impl;

    SDL_free(a->region);
    SDL_free(a);
    o_clear(self, sizeof *self, 1);
}


void o_allocator_arena_clear(struct o_allocator_i self) 
{
    arena_assert(self);
    struct arena *a = self.impl;
    a->used = 0;
}


osize o_allocator_arena_size(struct o_allocator_i self)
{
    arena_assert(self);
    struct arena *a = self.impl;
    return a->size;
}


osize o_allocator_arena_used(struct o_allocator_i self)
{
    arena_assert(self);
    struct arena *a = self.impl;
    return a->used;
}
