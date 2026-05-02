#ifndef O_ODICT_H
#define O_ODICT_H

/**
 * @file ODict.h
 *
 * Object
 *
 * The dictionary is a delegator to an OMap with string keys and void* values.
 * In contrast to the OMap functions, it is not based on references and can input strings and void*'s directly.
 * An ODict may be used with strings or oobj as value, or any other void* stuff
 *
 * ODict:
 * ```c
 * void *value = ODict_get(dict, "key");
 * ```
 *
 * OMap (with "char *" keys and "void*" value):
 * ```c
 * char *key = "key";
 * void **value_ref = OMap_get(map, &key, void*);
 * ```
 *
 * Performance tip:
 * As with OMap consider caching the (default) const pair index
 *     for upcomming get and set calls with ODict_*_at(..)
 *
 * Operators:
 * o_num -> ODict_num
 * o_at -> ODict_value_at (so void* is returned, AND NOT void**! )
 */


#include "OObj.h"

/** object id */
#define ODict_ID OObj_ID "ODict."




typedef struct {
    OObj super;

    // OMap with "char *" keys and "void *" values
    oobj map;
} ODict;

/**
 * Initializes the object
 * @param obj ODict object
 * @param parent to inherit from
 * @param approx_num estimated number of items
 * @return obj casted as ODict
 */
ODict *ODict_init(oobj obj, oobj parent, osize approx_num);

/**
 * Creates a new ODict object
 * @param parent to inherit from
 * @param approx_num estimated number of items
 * @return The new object
 */
o_inline ODict *ODict_new(oobj parent, osize approx_num)
{
    OObj_DECL_IMPL_NEW(ODict, parent, approx_num);
}

//
// virtual implementations:
//

/**
 * virtual operator function
 * @param obj ODict object
 * @return the number of key value pairs stored in the map
 */
osize ODict__v_op_num(oobj obj);

/**
 * virtual operator function
 * @param obj ODict object
 * @param idx key-value-pair index
 * @return (void *) of the idx associated value
 */
void *ODict__v_op_at(oobj obj, osize idx);

//
// object functions
//

/**
 * @param obj ODict object
 * @return the internal OMap object
 */
OObj_DECL_GET(ODict, oobj, map)

/**
 * @param obj ODict object
 * @return the number of key value pairs stored in the map
 */
osize ODict_num(oobj obj);


/**
 * Returns the key at the given index.
 * @param obj ODict object
 * @param idx key-value-pair index
 * @return (const char *) of the idx associated key (keys cant be changed...)
 * @note asserts idx bounds.
 *       the pointer is valid until the map is resized (ODict_set, ODict_remove).
 */
const char *ODict_key_at(oobj obj, osize idx);


/**
 * Returns a reference to the void* value at the given index.
 * @param obj ODict object
 * @param idx key-value-pair index
 * @return (void **) of the idx associated value void*
 * @note asserts idx bounds.
 *       the pointer is valid until the map is resized (ODict_set, ODict_remove).
 */
void **ODict_value_at_ref(oobj obj, osize idx);


/**
 * Returns the void* value at the given index.
 * @param obj ODict object
 * @param idx key-value-pair index
 * @return (void*) of the idx associated value void*
 * @note asserts idx bounds.
 *       the pointer is valid until the map is resized (ODict_set, ODict_remove).
 */
o_inline void *ODict_value_at(oobj obj, osize idx)
{
    return *ODict_value_at_ref(obj, idx);
}

/**
 * Will overwrite value on a pair in the map at the given index
 * @param obj ODict object
 * @param idx key-value-pair index
 * @param value void* to add with the key
 * @return (void**) of the idx associated value void*
 * @note asserts idx bounds.
 *       the pointer is valid until the map is resized (ODict_set, ODict_remove).
 */
o_inline void **ODict_set_at(oobj obj, osize idx, void *set)
{
    void **res = ODict_value_at_ref(obj, idx);
    *res = set;
    return res;
}

/**
 * Will return the index of key, or -1 if failed
 * @param obj ODict object
 * @param key to get the value from
 * @return the key associated index, or -1
 * @note Performance tip: consider caching the index for upcomming calls
 *           and use ODict_*_at(..)
 */
osize ODict_get_idx(oobj obj, const char *key);


/**
 * Will return a known reference void* value of key, or >NULL< if failed
 * @param obj ODict object
 * @param key to get the void* value from
 * @return (void* *) of the key associated value, or >NULL<
 * @note the pointer is valid until the map is resized (ODict_set, ODict_remove).
 *       Performance tip: consider caching the index for upcomming calls
 *           and use ODict_*_at(..)
 */
o_inline void **ODict_get_ref(oobj obj, const char *key)
{
    osize idx = ODict_get_idx(obj, key);
    if(idx < 0)
        return NULL;
    return ODict_value_at_ref(obj, idx);
}


/**
 * Will return a known value void* of key, or >NULL< if failed
 * @param obj ODict object
 * @param key to get the value from
 * @return (void*) of the key associated void* value, or >NULL<
 * @note the pointer is valid until the map is resized (ODict_set, ODict_remove).
 *       Performance tip: consider caching the index for upcomming calls
 *           and use ODict_*_at(..)
 */
o_inline void *ODict_get(oobj obj, const char *key)
{
    void **ref = ODict_get_ref(obj, key);
    if(!ref) {
        return NULL;
    }
    return *ref;
}


/**
 * Will set or overwrite a new key value pair in the map
 * @param obj ODict object
 * @param key to set the value to
 * @param value void* to add with the key
 * @return the key associated index
 * @note Performance tip: consider caching the index for upcomming calls
 *           and use ODict_*_at(..)
 */
osize ODict_set(oobj obj, const char *key, void *value);

/**
 * Reads and sets all entries from the given ODict or OMap into obj
 * @param obj OMap object
 * @param merge_from ODict or OMap to read from.
 *                   asserts for equal key and value sizes 
 *                   (should be same types...)
 */
void ODict_merge(oobj obj, oobj merge_from);

/**
 * Will remove the key pair from the map.
 * Heavy call, must check the whole hashmap and reduce the associated indices.
 * Breaks the pairs indices if there were cached for fast access!
 * @param obj ODict object
 * @param key to remove from the map
 * @return true if found and removed
 * @warning asserts for OMap_remove_able which is false by default.
 *          Turn on with OMap_remove_able_set(ODict_map(obj), true)
 */
bool ODict_remove(oobj obj, const char *key);


#endif //O_ODICT_H
