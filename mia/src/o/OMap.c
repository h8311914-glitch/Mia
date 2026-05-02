#include "o/OMap.h"
#include "o/OObj_builder.h"
#include "o/OArray.h"
#include "o/OList.h"
#include "o/str.h"


#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"

//
// key string implementations
//

static ou32 key_string_hash(const void *key)
{
    const char * const * str = key;
    return o_str_hash(*str);
}

static bool key_string_equals(const void *key_a, const void *key_b)
{
    const char * const * str_a = key_a;
    const char * const * str_b = key_b;
    return o_str_equals(*str_a, *str_b);
}

static void *key_string_clone(oobj obj, const void *key)
{
    const char * const * str = key;
    void *cloned = o_str_clone(obj, *str);
    return cloned;
}

static void key_string_del(oobj obj, void *key)
{
    char ** str = key;
    o_free(obj, *str);
}


OMap *OMap_init(oobj obj, oobj parent, osize key_size, osize value_size, osize approx_num,
                OMap__key_hash_fn hash_fn, OMap__key_equals_fn equals_fn,
                OMap__key_clone_fn clone_fn, OMap__key_del_fn del_fn)
{
    OMap *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, OMap_ID);

    self->key_size = key_size;
    self->value_size = value_size;

    // pairs of key and value elements
    osize key_size_aligned = o_align_size_max(key_size);
    osize value_size_aligned = o_align_size_max(value_size);
    self->pairs = OArray_new_dyn(self, NULL, key_size_aligned + value_size_aligned, 0, approx_num);

    // the hashmap is at first a list of possible DataDyn arrays
    self->hashmap = OList_new_ex(self, NULL, approx_num, 0, OArray_REALLOC_DEFAULT);
    
    self->collisions_max_log = 8;

    // vfuncs
    self->super.v_op_num = OMap__v_op_num;
    self->super.v_op_at = OMap__v_op_at;
    self->v_hash = hash_fn;
    self->v_equals = equals_fn;
    self->v_clone = clone_fn;
    self->v_del = del_fn;
    return self;
}

osize OMap__v_op_num(oobj obj)
{
    return OMap_num(obj);
}

void *OMap__v_op_at(oobj obj, osize idx)
{
    return OMap_value_at_void(obj, idx);
}


OMap *OMap_new_string_keys(oobj parent, osize value_size, osize approx_num)
{
    return OMap_new(parent, sizeof(char *), value_size, approx_num,
                     key_string_hash, key_string_equals,
                     key_string_clone, key_string_del);
}

osize OMap_num(oobj obj)
{
    OObj_assert(obj, OMap);
    OMap *self = obj;
    return OArray_num(self->pairs);
}

const void *OMap_key_at_void(oobj obj, osize idx)
{
    OObj_assert(obj, OMap);
    OMap *self = obj;
    return OArray_at_void(self->pairs, idx);
}

void *OMap_value_at_void(oobj obj, osize idx)
{
    OObj_assert(obj, OMap);
    OMap *self = obj;
    obyte *kv_pair = OArray_at_void(self->pairs, idx);
    obyte *value = kv_pair + o_align_size_max(self->key_size);
    return value;
}

void *OMap_set_at(oobj obj, osize idx, const void *value)
{
    OObj_assert(obj, OMap);
    OMap *self = obj;
    void *dst = OMap_value_at_void(self, idx);
    o_memcpy(dst, value, 1, self->value_size);
    return dst;
}

void OMap_clear(oobj obj)
{
    OObj_assert(obj, OMap);
    OMap *self = obj;
    OArray_clear(self->pairs);
    osize num = OList_num(self->hashmap);
    for (osize i = 0; i < num; i++) {
        oobj array = o_at(self->hashmap, i);
        if (array) {
            OArray_clear(array);
        }
    }
}


osize OMap_get_idx(oobj obj, const void *key)
{
    OObj_assert(obj, OMap);
    OMap *self = obj;
    
    ou32 hash = self->v_hash(key);

    // the first hashmap returns an array of keys that may fit
    // array is an OArray of osize elements, which are indices into the self->pairs list
    oobj array = o_at(self->hashmap, hash % o_num(self->hashmap));
    if (!array) {
        // list under that hash% not available, so pair not available, too
        return -1;
    }

    // search in the list for the key
    osize idx = -1;
    for (osize i = 0; i < OArray_num(array); i++) {
        osize test_idx = *OArray_at(array, i, osize);
        if (self->v_equals(key, OMap_key_at_void(self, test_idx))) {
            idx = test_idx;
            break;
        }
    }
    return idx;
}

osize OMap_set(oobj obj, const void *key, const void *value)
{
    OObj_assert(obj, OMap);
    OMap *self = obj;
    
    ou32 hash = self->v_hash(key);

    // the first hashmap returns an array of keys that may fit
    // arrray is an OArray of osize elements, which are indices into the self->pairs list
    oobj *array_ptr = OList_at_ref(self->hashmap, hash % o_num(self->hashmap));
    if (!*array_ptr) {
        // list under that hash% not created yet:
        *array_ptr = OArray_new_dyn(self, NULL, sizeof(osize), 0, 4);
    }

    // search in the array for the key
    osize idx = -1;
    for (osize i = 0; i < OArray_num(*array_ptr); i++) {
        osize test_idx = *OArray_at(*array_ptr, i, osize);
        if (self->v_equals(key, OMap_key_at_void(self, test_idx))) {
            idx = test_idx;
            break;
        }
    }

    // key not found, add it
    if (idx < 0) {
        // add a pair and copy the key to it
        void *kv_pair = OArray_push(self->pairs, NULL);
        char **new_key_ptr = kv_pair;
        *new_key_ptr = self->v_clone(self, key);
        // update idx to the new pair and add it to the list
        idx = OArray_num(self->pairs) - 1;
        OArray_push(*array_ptr, &idx);
        
        if(OArray_num(*array_ptr) > 1) {
            self->collisions++;
            if(self->collisions >= self->collisions_max_log) {
                o_log_warn_s(__func__, "New hash collision: Currently %i collsions", (int) self->collisions);
            }
        }
    }

    // set the value
    o_memcpy(OMap_value_at_void(self, idx), value, 1, self->value_size);
    return idx;
}

void OMap_merge(oobj obj, oobj map_to_merge_from)
{
    OObj_assert(obj, OMap);
    OMap *self = obj;
    OObj_assert(map_to_merge_from, OMap);
    OMap *map = map_to_merge_from;
    assert(self->key_size == map->key_size);
    assert(self->value_size == map->value_size);
    osize num = OArray_num(map->pairs);
    for(osize i=0; i<num; i++) {
        OMap_set(self, OMap_key_at_void(map, i), OMap_value_at_void(map, i));
    }
}

bool OMap_remove(oobj obj, const void *key)
{
    OObj_assert(obj, OMap);
    OMap *self = obj;
    assert(self->remove_able && "not allowed");
    ou32 hash = self->v_hash(key);

    // the first hashmap returns an array of keys that may fit
    // array is an OArray of osize elements, which are indices into the self->pairs list
    oobj array = o_at(self->hashmap, hash % o_num(self->hashmap));
    if (!array) {
        // list under that hash% not available, so pair not available, too
        return false;
    }

    // search in the list for the key
    osize idx = -1;
    osize array_i = 0;
    for (osize i = 0; i < OArray_num(array); i++) {
        osize test_idx = *OArray_at(array, i, osize);
        if (self->v_equals(key, OMap_key_at_void(self, test_idx))) {
            idx = test_idx;
            array_i = i;
            break;
        }
    }

    // key not found
    if (idx < 0) {
        return false;
    }

    // delete the key
    const void *key_object = OMap_key_at_void(self, idx);
    self->v_del(self, (void*) key_object);

    // remove the key pair
    OArray_pop_at(self->pairs, idx, NULL);


    // remove the idx entry in the list
    OArray_pop_at(array, array_i, NULL);

    // traverse the whole hashmap to reduce indices, greater the removed idx
    osize hashmap_num = OList_num(self->hashmap);
    oobj *hashmap_v = OList_list(self->hashmap);
    for (osize i = 0; i < hashmap_num; i++) {
        array = hashmap_v[i];
        if (!array) {
            continue;
        }
        osize array_num = OArray_num(array);
        osize *array_v = OArray_data(array, osize);
        for (osize l = 0; l < o_num(array); l++) {
            osize *index = &array_v[l];

            // check if index>idx and reduce it by one
            if (*index > idx) {
                (*index)--;
            }
        }
    }


    return true;
}
