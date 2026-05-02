#ifndef O_OARRAY_RAW_H
#define O_OARRAY_RAW_H

/**
 * @file OArray_raw.h
 *
 * Object extension
 *
 * USE WITH CAUTION (unchecked)
 *
 * Adds unsafe raw access for pushing and appending.
 * Great for a little speedup in bulk operations like string building.
 *     Needs proper setup of the OArray for the speedup, with a dynamic mode and pre allocated capacity.
 *     The _raw functions do not speedup the actual reallaction, but work faster in using the capacity
 * Call OArray_raw_done if finished to set the cleared termination object.
 *
 * SPEEDUP: 2.5x in contrast to default checked OArray_append/OArray_push
 *          in both release and debug (where release may be 10x faster than debug)
 *          Compared to the trivial C struct "o/array.h" its nevertheless 2x slower than that.
 *              So if performance is the main concern, use struct o_array instead for may speed.
 */


#include "OArray.h"


/**
 * Use this after dealing with OArray_raw stuff (not each time, but after bulking all together)
 * It only sets the termination object (cleared element), which is in the OArray design.
 * @param obj OArray object unchecked
 */
o_inline void OArray_raw_done(oobj obj)
{
    OArray *self = obj;
    obyte *term_start = &self->data[self->num * self->element_size];
    o_clear(term_start, self->element_size, 1);
}


/**
 * RAW mode, unchecked for more speedups.
 * Resizes the array, makes use of empty/unused capacity at back.
 * Else uses the virtual realloc function.
 * @param obj OArray object unchecked
 * @param num new element size
 * @note DOES NOT SET THE TERMINATION ELEMENT, after bulking, call OArray_raw_done()
 */
o_inline void OArray_raw_resize(oobj obj, osize num)
{
    OArray *self = obj;

    // adding elements in capacity or popping or do nothing (do nothing occurs rarely in raw)
    osize add = num - self->num;
    osize available = self->capacity - self->num - self->available_front;
    if(o_unlikely(add <= available)) {
        self->num = num;
        return;
    }

    // actual resize
    osize append = add - available;
    self->v_dyn_realloc(self, num, 0, append);
    self->num = num;
}

/**
 * RAW mode, unchecked for more speedups.
 * Resizes the array in the front, makes use of empty/unused capacity at front.
 * Else uses the virtual realloc function.
 * @param obj OArray object unchecked
 * @param num new element size
 * @note DOES NOT SET THE TERMINATION ELEMENT, after bulking, call OArray_raw_done()
 */
o_inline void OArray_raw_resize_front(oobj obj, osize num)
{
    OArray *self = obj;

    // adding elements in capacity or popping or do nothing (do nothing occurs rarely in raw)
    osize add = num - self->num;
    osize available = self->available_front;
    if (o_unlikely(add <= available)) {
        self->num = num;
        self->available_front -= add;
        self->data = &self->memory[self->available_front * self->element_size];
        return;
    }

    // actual resize
    osize append = add - available;
    self->v_dyn_realloc(self, num, append, 0);
    self->num = num;
    self->available_front -= add;
    self->data = &self->memory[self->available_front * self->element_size];
}



/**
 * RAW mode, unchecked for more speedups.
 * Append an array of elements at the end
 * @param obj OArray object unchecked
 * @param opt_elements_data elements to append, or NULL (new allocated elements will be empty!)
 * @param num of elements to append
 * @return a pointer to the copied element in the data memory (or NULL id num <= 0)
 * @note the returned pointer is valid until the array is resized.
 *       DONT FORGET TO PASS A REFERENCE OF oobj ELEMENTS (would need oobj *)
 *       DOES NOT SET THE TERMINATION ELEMENT, after bulking, call OArray_raw_done()
 */
o_inline void *OArray_raw_append(oobj obj, const void *opt_elements_data, osize num)
{
    OArray *self = obj;
    osize old_num = self->num;
    OArray_raw_resize(obj, self->num + num);
    obyte *new_element_data = &self->data[old_num * self->element_size];
    if (opt_elements_data) {
        o_memcpy(new_element_data, opt_elements_data, self->element_size, num);
    }
    return new_element_data;
}
/**
 * RAW mode, unchecked for more speedups.
 * Takes out an array of elements from the OArray at the end
 * @param obj OArray object unchecked
 * @param out_opt_elements_data elements to take, or NULL
 * @param num of elements to take
 * @note asserts that the OArray_num(obj) >= num.
 *       DONT FORGET TO PASS A REFERENCE OF oobj ELEMENTS (would need oobj *)
 *       DOES NOT SET THE TERMINATION ELEMENT, after bulking, call OArray_raw_done()
 */
o_inline void OArray_raw_take(oobj obj, void *out_opt_elements_data, osize num)
{
    OArray *self = obj;
    if (out_opt_elements_data) {
        obyte *taken_element_data = &self->data[(self->num - num) * self->element_size];
        o_memcpy(out_opt_elements_data, taken_element_data, self->element_size, num);
    }
    OArray_resize(obj, self->num - num);
}

/**
 * RAW mode, unchecked for more speedups.
 * Append an array of elements at the front, shifting the rest back
 * @param obj OArray object unchecked
 * @param opt_elements_data elements to append, or NULL (new allocated elements will be empty!)
 * @param num of elements to append
 * @return a pointer to the copied element in the data memory (or NULL id num <= 0)
 * @note the returned pointer is valid until the array is resized.
 *       DONT FORGET TO PASS A REFERENCE OF oobj ELEMENTS (would need oobj *)
 *       DOES NOT SET THE TERMINATION ELEMENT, after bulking, call OArray_raw_done()
 */
o_inline void *OArray_raw_append_front(oobj obj, const void *opt_elements_data, osize num)
{
    OArray *self = obj;
    OArray_resize_front(obj, self->num + num);
    if (opt_elements_data) {
        o_memcpy(self->data, opt_elements_data, self->element_size, num);
    }
    return self->data;
}

/**
 * RAW mode, unchecked for more speedups.
 * Takes out an array of elements from the OArray at the beginning, shifting the rest front
 * @param obj OArray object unchecked
 * @param out_opt_elements_data elements to take, or NULL
 * @param num of elements to take
 * @note asserts that the OArray_num(obj) >= num.
 *       DONT FORGET TO PASS A REFERENCE OF oobj ELEMENTS (would need oobj *)
 *       DOES NOT SET THE TERMINATION ELEMENT, after bulking, call OArray_raw_done()
 */
o_inline void OArray_raw_take_front(oobj obj, void *out_opt_elements_data, osize num)
{
    OArray *self = obj;
    if (out_opt_elements_data) {
        o_memcpy(out_opt_elements_data, self->data, self->element_size, num);
    }
    OArray_resize_front(obj, self->num - num);
}


/**
 * RAW mode, unchecked for more speedups.
 * Appends a single element at the end
 * @param obj OArray object unchecked
 * @param opt_element_data element to append, or NULL (new allocated element will be empty!)
 * @return a pointer to the copied element in the data memory
 * @note element_size should match OArray_element_size(obj)
 *       the returned pointer is valid until the array is resized.
 *       DONT FORGET TO PASS A REFERENCE OF oobj ELEMENTS (would need oobj *)
 *       DOES NOT SET THE TERMINATION ELEMENT, after bulking, call OArray_raw_done()
 */
o_inline void *OArray_raw_push(oobj obj, const void *opt_element_data)
{
    return OArray_raw_append(obj, opt_element_data, 1);
}

/**
 * RAW mode, unchecked for more speedups.
 * Takes out a single element at the end
 * @param obj OArray object unchecked
 * @param out_opt_element_data element to take out, or NULL
 * @note element_size should match OArray_element_size(obj).
 *       DONT FORGET TO PASS A REFERENCE OF oobj ELEMENTS (would need oobj *)
 *       DOES NOT SET THE TERMINATION ELEMENT, after bulking, call OArray_raw_done()
 */
o_inline void OArray_raw_pop(oobj obj, void *out_opt_element_data)
{
    OArray_raw_take(obj, out_opt_element_data, 1);
}

/**
 * RAW mode, unchecked for more speedups.
 * Appends a single element at the beginning, shifting the rest back
 * @param obj OArray object unchecked
 * @param opt_element_data element to append, or NULL (new allocated element will be empty!)
 * @return a pointer to the copied element in the data memory
 * @note element_size should match OArray_element_size(obj)
 *       the returned pointer is valid until the array is resized.
 *       DONT FORGET TO PASS A REFERENCE OF oobj ELEMENTS (would need oobj *)
 *       DOES NOT SET THE TERMINATION ELEMENT, after bulking, call OArray_raw_done()
 */
o_inline void *OArray_raw_push_front(oobj obj, const void *opt_element_data)
{
    return OArray_raw_append_front(obj, opt_element_data, 1);
}

/**
 * RAW mode, unchecked for more speedups.
 * Takes out a single element at the beginning, shifting the rest front
 * @param obj OArray object unchecked
 * @param out_opt_element_data element to take out, or NULL
 * @note element_size should match OArray_element_size(obj).
 *       DONT FORGET TO PASS A REFERENCE OF oobj ELEMENTS (would need oobj *)
 *       DOES NOT SET THE TERMINATION ELEMENT, after bulking, call OArray_raw_done()
 */
o_inline void OArray_raw_pop_front(oobj obj, void *out_opt_element_data)
{
    OArray_raw_take_front(obj, out_opt_element_data, 1);
}


//
// string stuff
//


/**
 * Helper to append a string to the OArray in raw mode (with o_strlen)
 * @param obj OArray object uncecked
 * @param str to be appended
 */
#define OArray_raw_append_string(obj, str) OArray_raw_append((obj), (const void *) (str), o_strlen((str)))

/**
 * Helper to append a static string to the OArray in raw mode (with o_strlen_static)
 * @param obj OArray object uncecked
 * @param str to be appended (these that are embedded in code with "..")
 */
#define OArray_raw_append_string_static(obj, str) OArray_raw_append((obj), (const void *) (str), o_strlen_static(str))



#endif //O_OARRAY_RAW_H
