#ifndef O_OMAP_H
#define O_OMAP_H

/**
 * @file OMap.h
 *
 * Object
 *
 * The map object represents a hash map with generic key and value types
 * @note do not use OMap_set("NO direct string keys", "NO direct string values!");
 *       Always pass a pointer to the key and values (so a pointer to char *)
 *
 * Want a map with key as char* and value as void* (or char*, oobj, ...)?
 * > Have a look at ODict <
 *
 * Performance tip:
 * The OMap has constant key-value pair indices by default (see OMap_remove_able).
 * So if the index of a key to get or set is known,
 * its faster to call OMap_key_at, OMap_value_at, OMap_set_at.
 * Cause internally the OMap stores key-value pairs in an OArray 
 *     and removing (so popping) is not allowed by default.
 * Using the default hashmap does:
 *   - calcing key hash
 *   - get OArray for each colliding hash for valid key indiced
 *   - check that list until the key matches.
 * This is all not needed if the index is already known.
 * 
 *
 * Operators:
 * o_num -> OMap_num
 * o_at -> OMap_value_at
 */


#include "OObj.h"

/** object id */
#define OMap_ID OObj_ID "OMap."


/**
 * Virtual hashing function.
 * @param key the key to create the hash from
 * @return an unsinged 32 int hash hey
 */
typedef ou32 (*OMap__key_hash_fn)(const void *key);

/**
 * Virtual key equals function.
 * @return true if the keys are equal
 */
typedef bool (*OMap__key_equals_fn)(const void *key_a, const void *key_b);


/**
 * Virtual cloning function.
 * @param obj OMap obj for allocations
 * @param key the key to clone
 * @return a clone of the key, may be allocated by obj
 */
typedef void *(*OMap__key_clone_fn)(oobj obj, const void *key);


/**
 * Virtual delete function.
 * @param obj OMap obj for allocations
 * @param key the key to delete
 * @note called by OMap_remove
 */
typedef void (*OMap__key_del_fn)(oobj obj, void *key);


typedef struct {
    OObj super;

    // const
    osize key_size, value_size;

    // internal OArray array of key value pairs
    oobj pairs;

    // internal OList hashmap, key -> pair idx
    oobj hashmap;
    
    // OMap_remove is only valid on true (asserts)
    bool remove_able;
    
    // counts hash collisions
    ou32 collisions;
    
    ou32 collisions_max_log;

    // vfuncs
    OMap__key_hash_fn v_hash;
    OMap__key_equals_fn v_equals;
    OMap__key_clone_fn v_clone;
    OMap__key_del_fn v_del;
} OMap;

/**
 * Initializes the object
 * @param obj OMap object
 * @param parent to inherit from
 * @param key_size size of a key (sizeof(...))
 * @param value_size size of a value (sizeof(...))
 * @param approx_num estimated number of items
 * @param hash_fn a hash function for the keys
 * @param equals_fn an equals function for the keys
 * @param clone_fn a clone function for the keys
 * @param del_fn a delete function for the keys
 * @return obj casted as OMap
 */
OMap *OMap_init(oobj obj, oobj parent, osize key_size, osize value_size, osize approx_num,
                OMap__key_hash_fn hash_fn, OMap__key_equals_fn equals_fn,
                OMap__key_clone_fn clone_fn, OMap__key_del_fn del_fn);

/**
 * Creates a new OMap object
 * @param parent to inherit from
 * @param key_size size of a key (sizeof(...))
 * @param value_size size of a value (sizeof(...))
 * @param approx_num estimated number of items
 * @param hash_fn a hash function for the keys
 * @param equals_fn an equals function for the keys
 * @param del_fn a delete function for the keys
 * @return The new object
 */
o_inline OMap *OMap_new(oobj parent, osize key_size, osize value_size, osize approx_num,
               OMap__key_hash_fn hash_fn, OMap__key_equals_fn equals_fn,
               OMap__key_clone_fn clone_fn, OMap__key_del_fn del_fn)
{
    OObj_DECL_IMPL_NEW(OMap, parent, key_size, value_size, approx_num, hash_fn, equals_fn, clone_fn, del_fn);
}


/**
 * Creates a new OMap object with strings as keys
 * @param parent to inherit from
 * @param value_size size of a value (sizeof(...))
 * @param approx_num estimated number of items
 * @return The new object
 * @note keys must be passed as (char**)!
 */
OMap *OMap_new_string_keys(oobj parent, osize value_size, osize approx_num);

//
// virtual implementations:
//

/**
 * virtual operator function
 * @param obj OMap object
 * @return the number of key value pairs stored in the map
 */
osize OMap__v_op_num(oobj obj);

/**
 * virtual operator function
 * @param obj OMap object
 * @param idx key-value-pair index
 * @return (void *) of the idx associated value
 */
void *OMap__v_op_at(oobj obj, osize idx);

//
// object functions
//

/**
 * @param obj OMap object
 * @return The size of a key element (sizeof(...))
 */
OObj_DECL_GET(OMap, osize, key_size)

/**
 * @param obj OMap object
 * @return The size of a value element (sizeof(...))
 */
OObj_DECL_GET(OMap, osize, value_size)

/**
 * @param obj OMap object
 * @return OMap_remove asserts for that, default is false
 */
OObj_DECL_GETSET(OMap, bool, remove_able)

/**
 * @param obj OMap object
 * @return Amount of hash collisions. 
 *         If to much, use a bigger approx_num.
 * @warning invalid if OMap_remove is called
 */
OObj_DECL_GET(OMap, ou32, collisions)

/**
 * @param obj OMap object
 * @return if collisions >=, an o_log_warn is logged on a new collision. (Default 8)
 *         Simply set ou32_MAX to turn off
 * @warning invalid if OMap_remove is called
 */
OObj_DECL_GETSET(OMap, ou32, collisions_max_log)


/**
 * @param obj OMap object
 * @return the number of key value pairs stored in the map
 */
osize OMap_num(oobj obj);


/**
 * Returns the key at the given index.
 * @param obj OMap object
 * @param idx key-value-pair index
 * @return (const void *) of the idx associated key (keys cant be changed...)
 * @note asserts idx bounds.
 *       the pointer is valid until the map is resized (OMap_set, OMap_remove).
 *       DON'T FORGET TO DEREFERENCE STRING KEYS (return would be char **)
 */
const void *OMap_key_at_void(oobj obj, osize idx);


/**
 * Returns a (const key_type *) of the key at the given index.
 * @param obj OMap object
 * @param idx key-value-pair index
 * @param key_type to cast into (not checked for type equality)
 * @return key_type * of the idx associated key (keys cant be changed...)
 * @note asserts idx bounds.
 *       the pointer is valid until the data is resized.
 *       DON'T FORGET TO DEREFERENCE STRING KEYS (return would be char **)
 */
#define OMap_key_at(obj, idx, key_type)                 \
(assert(((OMap *)obj)->key_size == sizeof(key_type)),   \
(const key_type*) OMap_key_at_void((obj), (idx)))


/**
 * Returns the value at the given index.
 * @param obj OMap object
 * @param idx key-value-pair index
 * @return (void *) of the idx associated value
 * @note asserts idx bounds.
 *       the pointer is valid until the map is resized (OMap_set, OMap_remove).
 */
void *OMap_value_at_void(oobj obj, osize idx);


/**
 * Returns a (volue_type *) of the value at the given index.
 * @param obj OMap object
 * @param idx key-value-pair index
 * @param value_type to cast into (not checked for type equality)
 * @return value_type * of the idx associated value
 * @note asserts idx bounds.
 *       the pointer is valid until the data is resized
 */
#define OMap_value_at(obj, idx, value_type)                 \
(assert(((OMap *)obj)->value_size == sizeof(value_type)),   \
(value_type*) OMap_value_at_void((obj), (idx)))

/**
 * Will overwrite the value of a pair in the map at given index
 * @param obj OMap object
 * @param idx key-value-pair index
 * @param value to set (do not add strings directly, that would be an error...)
 * @return value_type * of the given idx
 * @note DON'T FORGET TO PASS A REFERENCE OF STRING KEYS (would need char **)
 */
void *OMap_set_at(oobj obj, osize idx, const void *value);

/**
 * Removes all items.
 * Breaks (of course) the pairs indices if there were cached for fast access!
 * @param obj OMap object
 * @note ignores OMap_remove_able
 */
void OMap_clear(oobj obj);

/**
 * Will return the index of key, or -1 if failed
 * @param obj OMap object
 * @param key to get the value from
 * @return the key associated index, or -1
 * @note DON'T FORGET TO PASS A REFERENCE OF STRING KEYS (would need char **)
 *       Performance tip: consider caching the index for upcomming calls
 *           and use OMap_*_at(..)
 */
osize OMap_get_idx(oobj obj, const void *key);

/**
 * Will return a known value of key, or NULL if failed
 * @param obj OMap object
 * @param key to get the value from
 * @return (void *) of the key associated value, or NULL
 * @note the pointer is valid until the map is resized (OMap_set, OMap_remove).
 *       DON'T FORGET TO PASS A REFERENCE OF STRING KEYS (would need char **)
 *       Performance tip: consider caching the index for upcomming calls
 *           and use OMap_*_at(..)
 */
o_inline void *OMap_get_void(oobj obj, const void *key)
{
    osize idx = OMap_get_idx(obj, key);
    if(idx < 0)
        return NULL;
    return OMap_value_at_void(obj, idx);
}


/**
 * Returns a (value_type *) of the known value at key, or NULL if failed
 * @param obj OMap object
 * @param key to get the value from
 * @param value_type to cast into (not checked for type equality)
 * @return value_type * of the key associated value, or NULL
 * @note the pointer is valid until the data is resized.
 *       DON'T FORGET TO PASS A REFERENCE OF STRING KEYS (would need char **)
 *       Performance tip: consider caching the index for upcomming calls
 *           and use OMap_*_at(..)
 */
#define OMap_get(obj, key, value_type)                      \
(assert(((OMap *)obj)->value_size == sizeof(value_type)),   \
(value_type*) OMap_get_void((obj), (key)))


/**
 * Will set or overwrite a new key value pair in the map
 * @param obj OMap object
 * @param key to set the value to
 * @param value to add with the key (do not add strings directly, that would be an error...)
 * @return the key associated index
 * @note DON'T FORGET TO PASS A REFERENCE OF STRING KEYS (would need char **)
 *       Performance tip: consider caching the index for upcomming calls
 *           and use OMap_*_at(..)
 */
osize OMap_set(oobj obj, const void *key, const void *value);


/**
 * Reads and sets all entries from the given OMap into obj
 * @param obj OMap object
 * @param map_to_merge_from OMap to read from.
 *                          asserts for equal key and value sizes 
 *                          (should be same types...)
 */
void OMap_merge(oobj obj, oobj map_to_merge_from);


/**
 * Will remove the key pair from the map.
 * Heavy call, must check the whole hashmap and reduce the associated indices.
 * Breaks the pairs indices if there were cached for fast access!
 * @param obj OMap object
 * @param key to remove from the map
 * @return true if found and removed
 * @note DON'T FORGET TO PASS A REFERENCE OF STRING KEYS (would need char **)
 * @warning asserts for OMap_remove_able which is false by default.
 */
bool OMap_remove(oobj obj, const void *key);


#endif //O_OMAP_H
