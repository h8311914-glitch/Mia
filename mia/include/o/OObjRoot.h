#ifndef O_OOBJ_ROOT_H
#define O_OOBJ_ROOT_H

/**
 * @file ORoot.h
 *
 * Object
 *
 * The root object type is the only one to have no parent at all.
 * They're the roots of the managed hierarchy.
 * Deleting them (o_del) will delete the whole managed resource tree
 *      and all its allocated memory and objects.
 */

#include "OObj.h"

/** object id */
#define OObjRoot_ID OObj_ID "ORoot."


/**
 * Initializes a object root, (so no parent).
 * Call its deletor to delete the full managed object tree
 * @param obj The object to initialize
 * @param name The OObj_name for this root (for debugging metrics) ("[mia]*" is reserved for internal stuff)
 * @param allocator the allocator to allocate from in the whole tree
 * @return obj casted as OObj*
 */
OObj *OObjRoot_init(oobj obj, const char *name, struct o_allocator_i allocator);


/**
 * Creates a object root, (so no parent).
 * Call its deletor to delete the full managed object tree
 * @param allocator the allocator to allocate from in the whole tree
 * @param name The OObj_name for this root (for debugging metrics) ("[mia]*" is reserved for internal stuff)
 * @param add_to_root_list if true (default) this root will be part of the global list of roots
 * @return The new allocated root OObj
 */
OObj *OObjRoot_new(const char *name, struct o_allocator_i allocator, bool add_to_root_list);


/**
 * Creates a object root, (so no parent) with the (default) heap allocator.
 * Call its deletor to delete the full managed object tree
 * @param name The OObj_name for this root (for debugging metrics) ("[mia]*" is reserved for internal stuff)
 * @return The new allocated root OObj
 */
o_inline OObj *OObjRoot_new_heap(const char *name)
{
    return OObjRoot_new(name, o_allocator_heap_new(), true);
}

/**
 * Creates a object root, (so no parent) with a pool allocator, which is faster for small allocations
 * Call its deletor to delete the full managed object tree
 * @param name The OObj_name for this root (for debugging metrics) ("[mia]*" is reserved for internal stuff)
 * @param block_size maximal size for an allocation to make use of a pool (<=0 for default)
 * @param blocks_in_pool number of blocks in a pool (<=0 for default)
 * @param start_pools number of pools to be created now (<=0 for default)
 * @return The new allocated root OObj
 * @note when deleting this root object, the allocator is >NOT< deleted, see "o_allocator_pool_del".
 */
o_inline OObj *OObjRoot_new_pool_ex(const char *name, int block_size, int blocks_in_pool, int start_pools)
{
    return OObjRoot_new(name, o_allocator_pool_new(block_size, blocks_in_pool, start_pools), true);
}

/**
 * Creates a object root, (so no parent) with a pool allocator, which is faster for small allocations
 * Call its deletor to delete the full managed object tree
 * @param name The OObj_name for this root (for debugging metrics) ("[mia]*" is reserved for internal stuff)
 * @return The new allocated root OObj
 * @note uses the pool size defaults.
 *       when deleting this root object, the allocator is >NOT< deleted, see "o_allocator_pool_del".
 */
o_inline OObj *OObjRoot_new_pool(const char *name)
{
    return OObjRoot_new_pool_ex(name, -1, -1, -1);
}

//
// virtual implementations:
//

/**
 * Object deletor that unregisters from the list of roots
 * @param obj ODelcallback object
 * @note only installed if add_to_root_list=true (default)
 */
void OObjRoot__v_del(oobj obj);


//
// utility functions
//

/**
 * Locks the access to the root list
 */
void OObjRoot__list_lock(void);

/**
 * Unlocks the access to the root list
 */
void OObjRoot__list_unlock(void);

/**
 * Create a block in which the mutex is locked and unlocked.
 * Use continue to leave the block.
 * @threadsafe
 * @note May be nested, but needs to be in another line (uses __LINE__ internally).
 *       DO NOT:
 *          return in the block!
 *          break in the block! (this block in a for loop...)
 */
#define OObjRoot__list_lock_block \
for(bool O_NAME_CONCAT(OObjRoot__list_lock_block__run_, __LINE__) = (OObjRoot__list_lock(), true); \
O_NAME_CONCAT(OObjRoot__list_lock_block__run_, __LINE__); \
O_NAME_CONCAT(OObjRoot__list_lock_block__run_, __LINE__) = (OObjRoot__list_unlock(), false))


/**
 * @param obj OObjRoot object to add to the list of roots, normally automatically done
 */
void OObjRoot__list_add(oobj obj);

/**
 * @param obj OObjRoot object to remove from the list of roots, normally automatically done
 */
void OObjRoot__list_rem(oobj obj);

/**
 * @return Number of roots in the root list
 * @note Consider an OObjRoot__list_lock, cause else it may change from other threads
 */
osize OObjRoot__list_num(void);


/**
 * @return Root from the root list at the given index or NULL if out of bounds
 * @note Consider an OObjRoot__list_lock while using the resulting root
 */
struct oobj_opt OObjRoot__list_at(osize index);


#endif //O_OOBJ_ROOT_H
