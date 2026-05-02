#ifndef O_OLIST_H
#define O_OLIST_H

/**
 * @file OList.h
 *
 * Object
 *
 * A list is a delegator to an OArray with void* as element.
 * In contrast to the OArray functions, it is not based on references and can input strings and void*'s directly.
 * An OList may be used as a list of strings or oobj's, or any other void* stuff.
 *
 * For sorting and searching or other advanced functions, use the internal OArray with OList_array
 *
 * Operators:
 * o_num -> OList_num
 * o_at -> OList_at (so void* is returned, AND NOT void**! )
 */


#include "OObj.h"

/** object id */
#define OList_ID OObj_ID "OList."



typedef struct {
    OObj super;

    // OArray with "void *" elements
    oobj array;
} OList;


/**
 * Initializes the object.
 * @param obj OList object
 * @param parent to inherit from
 * @param opt_list to copy from, or NULL (allocated by zeros)
 * @param num elements to create (or copied from data, if not NULL)
*            if <0 opt_list is copied as NULL terminated list
 * @param start_capacity = o_max(start_capacity, num)
 * @param array_dyn_mode dynamic realloc mode, like OArray_REALLOC_DOUBLED_FRONT
 * @return obj casted as OList
 */
OList *OList_init_ex(oobj obj, oobj parent, void **opt_list, osize num,
                     osize start_capacity, int array_dyn_mode);

/**
 * Creates a new OList.
 * @param parent to inherit from
 * @param opt_list to copy from, or NULL (allocated by zeros)
 * @param num elements to create (or copied from data, if not NULL)
*            if <0 opt_list is copied as NULL terminated list
 * @param start_capacity = o_max(start_capacity, num)
 * @param array_dyn_mode dynamic realloc mode, like OArray_REALLOC_DOUBLED_FRONT
 * @return The new object
 */
o_inline OList *OList_new_ex(oobj parent, void **opt_list, osize num,
                    osize start_capacity, int array_dyn_mode)
{
    OObj_DECL_IMPL_NEW_SPECIAL(OList, ex, parent, opt_list, num, start_capacity, array_dyn_mode);
}

/**
 * Creates a new OList.
 * The internal OArray's mode is OArray_REALLOC_DOUBLED.
 * The start capacity is num*2, but at least 32.
 * @param parent to inherit from
 * @param opt_list to copy from, or NULL (allocated by zeros)
 * @param num elements to create (or copied from data, if not NULL)
 *            if <0 opt_list is copied as NULL terminated list
 * @return The new object
 */
OList *OList_new(oobj parent, void **opt_list, osize num);

//
// virtual implementations:
//

/**
 * virtual operator function
 * @param obj OList object
 * @return the number of elements stored in the array
 */
osize OList__v_op_num(oobj obj);

/**
 * virtual operator function
 * @param obj OList object
 * @param idx element index
 * @return void * of the element data
 */
void *OList__v_op_at(oobj obj, osize idx);


//
// object functions:
//

/**
 * @param obj OList object
 * @return the internal OArray object
 */
OObj_DECL_GET(OList, oobj, array)


/**
 * @param obj OList object
 * @return the data list pointer as void **
 *         always null terminated (num+1 is allocated with zeros)
 * @note the pointer is valid until the array is resized
 */
void **OList_list(oobj obj);


/**
 * @param obj OList object
 * @return the number of elements stored in the array
 * @note or use the o_num operator
 */
osize OList_num(oobj obj);


/**
 * @param obj OList object
 * @return the capacity for the array memory, counted in elements
 */
osize OList_capacity(oobj obj);

/**
 * Returns a void * of the element at the given index
 * @param obj OList object
 * @param idx element index
 * @return reference to the void * of the element data
 * @note asserts idx bounds.
 *       the pointer is valid until the array is resized.
 * @note or use the o_at operator
 */
void **OList_at_ref(oobj obj, osize idx);

/**
 * Returns a void * of the element at the given index
 * @param obj OList object
 * @param idx element index
 * @return void * of the element data
 * @note asserts idx bounds.
 *       the pointer is valid until the array is resized.
 * @note or use the o_at operator
 */
o_inline void *OList_at(oobj obj, osize idx)
{
    return *OList_at_ref(obj, idx);
}


/**
 * Resets the capacity
 * @param obj OList object
 * @param capacity new capacity, if < num, num will be reset to the new capacity
 * @param reserve_front from allocated memory to data, counted in elements
 */
void OList_realloc(oobj obj, osize capacity, osize reserve_front);

/**
 * Resets the capacity to the current number of elements.
 * @param obj OList object
 */
o_inline void OList_capacity_fit(oobj obj)
{
    OList_realloc(obj, OList_num(obj), 0);
}

/**
 * Resizes the array, makes use of empty/unused capacity at back.
 * Else uses the virtual realloc function.
 * @param obj OList object
 * @param num new element size
 */
void OList_resize(oobj obj, osize num);

/**
 * Resizes the array in the front, makes use of empty/unused capacity at front.
 * Else uses the virtual realloc function.
 * @param obj OList object
 * @param num new element size
 */
void OList_resize_front(oobj obj, osize num);


/**
 * Just calls OList_resize(..., 0) to remove all elements.
 * @ param obj OList object
 */
o_inline void OList_clear(oobj obj)
{
    OList_resize(obj, 0);
}


/**
 * Moves the internal memory buffer as resource into the object into.
 * Sets internal buffer to NULL and num|size to 0
 * @param obj OList object
 * @param into OObj to move the array resource into
 * @return the moved data buffer
 */
void *OList_move(oobj obj, oobj into);


/**
 * Clones the data (not capacity and settings!) of another OList
 * @param obj OList to be cloned from, also acts as parent
 * @return a new OList with data cloned
 */
o_inline OList *OList_clone(oobj obj)
{
    return OList_new(obj, OList_list(obj), OList_num(obj));
}


/**
 * Append an array of elements at the end
 * @param obj OList object
 * @param opt_list elements to append, or NULL (new allocated elements will be cleared to 0)
 * @param num of elements to append
 *            if <0 opt_list is copied as NULL terminated list
 * @return a pointer to the copied element in the data memory (or NULL id num <= 0)
 * @note the returned pointer is valid until the array is resized.
 */
void *OList_append(oobj obj, void **opt_list, osize num);

/**
 * Takes out an array of elements from the OList at the end
 * @param obj OList object
 * @param out_opt_list elements to take, or NULL
 * @param num of elements to take
 * @note asserts that the OList_num(obj) >= num.
 */
void OList_take(oobj obj, void **out_opt_list, osize num);


/**
 * Append an array of elements at the front, shifting the rest back
 * @param obj OList object
 * @param opt_list elements to append, or NULL (new allocated elements will be cleared to 0)
 * @param num of elements to append
 *            if <0 opt_list is copied as NULL terminated list
 * @return a pointer to the copied element in the data memory (or NULL id num <= 0)
 * @note the returned pointer is valid until the array is resized.
 */
void *OList_append_front(oobj obj, void **opt_list, osize num);

/**
 * Takes out an array of elements from the OList at the beginning, shifting the rest front
 * @param obj OList object
 * @param out_opt_list elements to take, or NULL
 * @param num of elements to take
 * @note asserts that the OList_num(obj) >= num.
 */
void OList_take_front(oobj obj, void **out_opt_list, osize num);

/**
 * Append an array of elements at the given index, shifting the rest back
 * @param obj OList object
 * @param idx
 * @param opt_list elements to append, or NULL (new allocated elements will be cleared to 0)
 * @param num of elements to append
 *            if <0 opt_list is copied as NULL terminated list
 * @return a pointer to the copied element in the data memory (or NULL id num <= 0)
 * @note asserts idx in bounds [0 : size]
 *       the returned pointer is valid until the array is resized.
 */
void *OList_append_at(oobj obj, osize idx, void **opt_list, osize num);


/**
 * Takes out an array of elements at the given index, shifting the rest front
 * @param obj OList object
 * @param idx
 * @param out_opt_list elements to take, or NULL
 * @param num of elements to take
 * @note asserts that the OList_num(obj) >= num.
 */
void OList_take_at(oobj obj, osize idx, void **out_opt_list, osize num);


/**
 * Appends a single element at the end
 * @param obj OList object
 * @param element to append, or NULL (new allocated element will be cleared to 0)
 * @return a pointer to the copied element in the data memory
 * @note element_size should match OList_element_size(obj)
 *       the returned pointer is valid until the array is resized.
 */
o_inline void *OList_push(oobj obj, void *element)
{
    return OList_append(obj, &element, 1);
}

/**
 * Takes out a single element at the end
 * @param obj OList object
 * @return element to take out, or NULL
 * @note element_size should match OList_element_size(obj).
 */
o_inline void *OList_pop(oobj obj)
{
    void *pop;
    OList_take(obj, &pop, 1);
    return pop;
}

/**
 * Appends a single element at the given index, shifting the rest back
 * @param obj OList object
 * @param idx the start index at which the element should be appended
 * @param element to append, or NULL (new allocated element will be cleared to 0)
 * @return a pointer to the copied element in the data memory
 * @note element_size should match OList_element_size(obj)
 *       the returned pointer is valid until the array is resized.
 */
o_inline void *OList_push_at(oobj obj, osize idx, void *element)
{
    return OList_append_at(obj, idx, &element, 1);
}

/**
 * Takes out a single element at the given index, shifting the rest front
 * @param obj OList object
 * @param idx the start index at which the element should be taken
 * @return element to take out, or NULL
 * @note element_size should match OList_element_size(obj)
 * @note asserts idx in bounds [0 : size).
 */
o_inline void *OList_pop_at(oobj obj, osize idx)
{
    void *pop;
    OList_take_at(obj, idx, &pop, 1);
    return pop;
}

/**
 * Appends a single element at the beginning, shifting the rest back
 * @param obj OList object
 * @param element to append, or NULL (new allocated element will be cleared to 0)
 * @return a pointer to the copied element in the data memory
 * @note element_size should match OList_element_size(obj)
 *       the returned pointer is valid until the array is resized.
 */
o_inline void *OList_push_front(oobj obj, void *element)
{
    return OList_append_front(obj, &element, 1);
}

/**
 * Takes out a single element at the beginning, shifting the rest front
 * @param obj OList object
 * @return element to take out, or NULL
 * @note element_size should match OList_element_size(obj).
 */
o_inline void *OList_pop_front(oobj obj)
{
    void *pop;
    OList_take_front(obj, &pop, 1);
    return pop;
}


#endif //O_OLIST_H
