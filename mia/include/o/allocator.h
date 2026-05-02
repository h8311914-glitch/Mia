#ifndef O_ALLOCATOR_H
#define O_ALLOCATOR_H

/**
 * @file allocator.h
 *
 * Allocator abstraction and implementations.
 * Each object (OObj) has an allocator and inherits it from the parents.
 * A typical allocator is the heap allocator.
 *
 * @note Apart from ALL other objects, etc.. An allocator is NOT an OObj.
 *       Because all objects need an allocator.
 *
 * @sa o_allocator_heap_new, OObjRoot_new
 */

#include "common.h"

enum o_allocator_type {
    O_ALLOCATOR_TYPE_INVALID,
    O_ALLOCATOR_TYPE_HEAP,
    O_ALLOCATOR_TYPE_POOL,
    O_ALLOCATOR_TYPE_ARENA,
    O_ALLOCATOR_TYPE_ENUM_MAX
};

// forward declaration for the virtual function
struct o_allocator_i;

/**
 * Virtual allocator function.
 * The realloc serves for all necessary allocator functions.
 *
 * @param iface Allocator interface.
 * @param mem  mem == NULL -> alloc.
 *             mem != NULL -> realloc | free.
 * @param element_size Size of a single element (sizeof(...)).
 * @param num  num <= 0 -> free (if mem != NULL).
 *
 * @return the (re)allocated memory or NULL, if it failed, or (element_size*num)==0
 */
typedef void *(*o_allocator_i__realloc_try_fn)(struct o_allocator_i iface, void *restrict mem, osize element_size,
                                             osize num);

/**
 * Allocator interface.
 * Consisting only of an implementation pointer and the virtual realloc function.
 * name is just for debugging purpose, pointer to static memory
 */
struct o_allocator_i {
    enum o_allocator_type type;
    o_allocator_i__realloc_try_fn realloc_try;
    void *impl;
};

/**
 * handy wrapper for the realloc_try function
 *
 * @param iface Allocator interface.
 * @param mem  mem == NULL -> alloc.
 *             mem != NULL -> realloc | free.
 * @param element_size Size of a single element (sizeof(...)).
 * @param num  num <= 0 -> free (if mem != NULL).
 *
 * @return the (re)allocated memory or NULL, if it failed, or (element_size*num)==0
 */
o_inline void *o_allocator_i_realloc_try(struct o_allocator_i iface, void *mem, osize element_size, osize num)
{
    return iface.realloc_try(iface, mem, element_size, num);
}

/**
 * Creates an allocator, that allocates on the heap (may be the default allocator).
 * @threadsafe (this function and the returned allocator)
 * @return the allocator interface.
 */
struct o_allocator_i o_allocator_heap_new(void);


/**
 * @return true if ptr may be in a valid range.
 * @note just tests if ptr >= min and ptr <= max.
 *       those min and max are collected by pool and arena allocations.
 * @warn no garantuee that the ptr wont segfault
 */
bool o_allocator_test_pointer_valid(const void *ptr);

//
// Pool
//

/**
 * Creates an allocator, that allocates pools of the given size for small objects, bigger fall bag to the heap.
 * @param block_size maximal size for an allocation to make use of a pool (<=0 for default)
 * @param blocks_in_pool number of blocks in a pool (<=0 for default)
 * @param start_pools number of pools to be created now (<=0 for default)
 * @return the allocator interface.
 * @warning This allocator type is NOT THREADSAFE
 */
struct o_allocator_i o_allocator_pool_new(int block_size, int blocks_in_pool, int start_pools);

/**
 * Deletes the allocator and its allocated pools, but NOT the heap allocated pointers!
 * @param self a reference to the pool interface, which will he cleared
 * @note >NOT< thread safe
 */
void o_allocator_pool_del(struct o_allocator_i *self);


//
// struct functions
//

/**
 * @param self a reference to the pool interface
 * @return number of the size of a block (maximal byte size to be pooled)
 */
int o_allocator_pool_block_size(struct o_allocator_i self);

/**
 * @param self a reference to the pool interface
 * @return number of blocks in a pool
 */
int o_allocator_pool_blocks_in_pool(struct o_allocator_i self);

/**
 * @param self a reference to the pool interface
 * @return number of allocated pools
 */
int o_allocator_pool_pools_num(struct o_allocator_i self);


/**
 * @param self a reference to the pool interface
 * @return number of total allocated blocks
 */
o_inline int o_allocator_pool_blocks_num(struct o_allocator_i self)
{
    return o_allocator_pool_blocks_in_pool(self)
             * o_allocator_pool_pools_num(self);
}

/**
 * @param self a reference to the pool interface
 * @return number of available blocks
 */
int o_allocator_pool_blocks_available(struct o_allocator_i self);


/**
 * @param self a reference to the pool interface
 * @return number of total allocated blocks
 */
o_inline int o_allocator_pool_blocks_used(struct o_allocator_i self)
{
    return o_allocator_pool_blocks_num(self)
             - o_allocator_pool_blocks_available(self);
}


/**
 * @param self a reference to the pool interface
 * @param mem a pointer which was allocated with this allocator
 * @return true if the given pointer was allocated in a pool
 * @note assumes that the pointer is the returned allocated pointer, when in the pool
 */
bool o_allocator_pool_pointer_pooled(struct o_allocator_i self, const void *mem);



//
// Arena
//

/**
 * Creates an allocator, that allocates an arena.
 * An arena is a pre allocated region, which results in a super fast allocation.
 * Just the arena position is checked and updated on an allocation.
 *      Ok. plus saving some allocation infos in the buffer to enable reallocs o_memcpy...
 *      The position is also updated on an aligned chunk O_ALIGN_SYSTEM_MAX
 * data can only be actually freed, if it was the last one allocated (as with realloc...)
 * The arena can be completely "freed" with the call to o_allocator_arena_clear
 * OObj's check if the allocator is of type arena and won't call free on the resources to save some time
 *
 * @param size in bytes for the arena memory region, allocated on the heap
 * @return the allocator interface.
 * @note see the o_allocator_arena_* functions
 *       delete with o_allocator_arena_del     
 * @sa OObjRoot_new to use the object system with this allocator
 *     If you know what you do, just call o_allocator_arena_clear
 *     If unsure, call o_del first
 * @warning This allocator type is NOT THREADSAFE
 */
struct o_allocator_i o_allocator_arena_new(osize size);

/**
 * Deletes the allocator and its allocated region
 * @param self a reference to the arena interface, which will he cleared
 */
void o_allocator_arena_del(struct o_allocator_i *self);


//
// struct functions
//

/**
 * frees all memory allocated by the arena.
 * Simply resets the internal used size to 0
 * @param self arena interface
 */
void o_allocator_arena_clear(struct o_allocator_i self);

/**
 * @param self arena interface
 * @return the size allocated by o_allocator_arena_new
 */
osize o_allocator_arena_size(struct o_allocator_i self);

/**
 * @param self arena interface
 * @return the used bytss in the arena
 */
osize o_allocator_arena_used(struct o_allocator_i self);

/**
 * @param self arena interface
 * @return the remaining bytes in the arena
 */
o_inline osize o_allocator_arena_remaining(struct o_allocator_i self)
{
    return o_allocator_arena_size(self) - o_allocator_arena_used(self);
}


#endif //O_ALLOCATOR_H
