#ifndef O_ARRAY_H
#define O_ARRAY_H

/**
 * @file array.h
 *
 * Header only simplified version of OArray.
 * Purpose is to maximize performance for smth like string building.
 *
 * SPEEDUP: ~2x in constrast to OArray_raw (which itselfs speedsup OArray by ~2.5x)
 *
 * @sa OArray.h, OArray_raw.h
 */

#include "o/OObj.h"

struct o_array {
    obyte *data;
    osize num;
    osize capacity;
    oobj parent;
};

/**
 * @param parent OObj to allocate on
 * @param capacity start capacity, dont mind setting this high for max speedup
 * @return a new created o_array struct
 * @note call o_array_free on it to delete it manually (else deleted by parent)
 */
o_inline struct o_array o_array_new(oobj parent, osize capacity)
{
    assert(capacity > 0);
    struct o_array self;
    self.data = o_new(parent, obyte, capacity);
    self.num = 0;
    self.capacity = capacity;
    self.parent = parent;
    return self;
}

/**
 * Free / delete this array data
 * @param self struct o_array reference to free delete (NULL safe)
 */
o_inline void o_array_free(struct o_array *self)
{
    if (!self) {
        return;
    }
    if (self->parent) {
        // safe to call with NULL
        o_free(self->parent, self->data);
    }
    *self = (struct o_array) {0};
}

/**
 * Moves the internal memory buffer as resource into the object into.
 * Sets internal buffer to NULL and num|size to 0
 * @param obj OArray object
 * @param into OObj to move the array resource into
 * @return the moved data buffer
 */
o_inline void *o_array_move(struct o_array *self, oobj into)
{
    void *mem = self->data;
    self->data = NULL;
    self->num = 0;
    o_data_move(self->parent, into, mem);
    return mem;
}


//
// struct functions
//

/**
 * Grows the size and may renew the data
 * @param self struct o_array reference
 * @param size new size
 */
o_inline void o_array_resize(struct o_array *restrict self, osize num)
{
    if (o_unlikely(num > self->capacity)) {
        self->capacity = num*2;
        self->data = o_renew(self->parent, self->data, obyte, self->capacity);
    }
    self->num = num;
}

/**
 * Reserve some bytes to write on
 * @param self struct o_array reference
 * @param add_bytes number of bytes to reserve
 * @return the reserved pointer, valid as long as the array is beeing resized
 *         (which may happen due to its reserve and append functions)
 */
o_inline obyte *o_array_reserve(struct o_array *restrict self, osize add_bytes)
{
    assert(add_bytes>0);
    o_array_resize(self, self->num + add_bytes);
    return self->data + self->num - add_bytes;
}

/**
 * Appends an array of bytes to the array
 * @param self struct o_array reference
 * @param data array to be copied
 * @param n data array size
 */
o_inline void o_array_append(struct o_array *restrict self, const void *restrict data, osize n)
{
    assert(n>0);
    obyte *restrict append = o_array_reserve(self, n);
    o_memcpy(append, data, 1, n);
}

/**
 * Helper to append a string to the array (with o_strlen)
 * @param self struct o_array reference
 * @param str to be appended
 */
#define o_array_append_string(self, str) o_array_append((self), (const void *) (str), o_strlen((str)))

/**
 * Helper to append a static string to the array (with o_strlen_static)
 * @param self struct o_array reference
 * @param str to be appended (these that are embedded in code with "..", DO NOT PASS A POINTER)
 */
#define o_array_append_string_static(self, str) o_array_append((self), (const void *) (str), o_strlen_static(str))



#endif //O_ARRAY_H
