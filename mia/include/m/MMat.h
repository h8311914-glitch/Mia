#ifndef M_MMAT_H
#define M_MMAT_H

/**
 * @file MMat.h
 *
 * Object
 *
 * Channel based matrix with different formats.
 * Acts as cpu based image class.
 *
 * Cloning or Allocating the matrix buffer may fail, test with MMat_valid() before using it!
 *
 * @sa MMat_ex.h for importing this header and all of its object extensions
 *
 * Operators:
 * o_num -> MMat_num
 * o_at -> MMat_at_idx
 */


#include "o/OObj.h"
#include "m/format.h"
#include "m/m_types/int.h"
#include "m/m_types/flt.h"

// forward
struct o_img;

/** object id */
#define MMat_ID OObj_ID "MMat."


typedef struct {
    OObj super;

    // either owned or a view.
    // may be of different types according to format and m/format.h
    // obyte pointer to do byte sized pointer arithmetics
    // even if its a subview, we declare it as restrict to boost performance
    obyte *restrict data;

    // this is always the start of the allocated buffer.
    // see MMat_data_root_share()
    const void *data_root;

    // true if data is owned / allocated by this MMat object.
    // false for an external reference or subview, etc.
    bool owning;

    // see m/format.h
    ou32 format;

    // const, create a new MMat to change
    m_ivec2 size;

    // data is row major.
    // next field is .x bytes from current.
    // full row jump is .y
    // strides allow for subviews, also with row or col jumps or single channel view.
    // also allows for negative numbers, to flip and rotate, etc. as a MMat view
    m_ivec2 stride;
} MMat;



/**
 * Initializes the object
 * @param obj MMat object
 * @param parent to inherit from
 * @param opt_data if not NULL, either cloned from to own or just referenced (set) according to clone_data
 * @param opt_data_root allocated root of opt_data (or NULL)
 * @param clone_data true to copy data into its own allocated buffer, false for simple reference
 *                   data is cloned using o_alloc_try, so may be NULL if allocation fails!
 *                   if opt_data is NULL, clone_data is ignored and always MMat tries to allocate
 * @param format used format for the matrix and opt_data to be copied from
 * @param size cols + rows (clamped to be minimal 1)
 * @param opt_stride if == 0:           automatically set (packed) according to format and size.
 *                   elif clone_data:   used for reading opt_data, after that automatically set to packed.
 *                   else:              just set to for the reference (may also be <0)
 * @return obj casted as MMat
 * @note MMat_data() may be NULL if allocation fails while cloning! Test with MMat_valid() before working with it!
 */
MMat *MMat_init_ex(oobj obj, oobj parent, void *opt_data, const void *opt_data_root, bool clone_data, ou32 format,
                   m_ivec2 size, m_ivec2 opt_stride);

/**
 * Creates a new MMat object
 * @param parent to inherit from
 * @param opt_data if not NULL, either cloned from to own or just referenced (set) according to clone_data
 * @param opt_data_root allocated root of opt_data (or NULL)
 * @param clone_data true to copy data into its own allocated buffer, false for simple reference
 * @param format used format for the matrix and opt_data to be copied from
 * @param size cols + rows
 * @param opt_stride if == 0:           automatically set (packed) according to format and size.
 *                   elif clone_data:   used for reading opt_data, after that automatically set to packed.
 *                   else:              just set to for the reference (may also be <0)
 * @return The new object
 * @note MMat_data() may be NULL if allocation fails while cloning! Test with MMat_valid() before working with it!
 */
o_inline MMat *MMat_new_ex(oobj parent, void *opt_data, const void *opt_data_root, bool clone_data, ou32 format,
                           m_ivec2 size, m_ivec2 opt_stride)
{
    OObj_DECL_IMPL_NEW_SPECIAL(MMat, ex, parent, opt_data, opt_data_root, clone_data, format, size, opt_stride);
}


/**
 * Creates a new MMat object.
 * @param parent to inherit from
 * @param opt_data if not NULL, cloned from (packed)
 * @param cols, rows: size of the matrix
 * @param format used format for the matrix
 * @return The new object
 * @note may return NULL if allocation fails
 */
struct oobj_opt MMat_new_try(oobj parent, const void *opt_data, int cols, int rows, ou32 format);


/**
 * Creates a new MMat object.
 * @param parent to inherit from
 * @param opt_data if not NULL, cloned from (packed)
 * @param cols, rows: size of the matrix
 * @param format used format for the matrix
 * @return The new object
 */
o_inline MMat *MMat_new(oobj parent, const void *opt_data, int cols, int rows, ou32 format)
{
    MMat *self = MMat_new_try(parent, opt_data, cols, rows, format).o;
    o_assume(self, "MMat allocation failed");
    return self;
}

/**
 * Creates a new MMat object.
 * All pixels are cleared to 0.
 * @param parent to inherit from
 * @param cols, rows: size of the matrix
 * @param format used format for the matrix
 * @return The new object
 * @note may return NULL if allocation fails
 */
struct oobj_opt MMat_new_0_try(oobj parent, int cols, int rows, ou32 format);

/**
 * Creates a new MMat object.
 * All pixels are cleared to 0.
 * @param parent to inherit from
 * @param cols, rows: size of the matrix
 * @param format used format for the matrix
 * @return The new object
 */
o_inline MMat *MMat_new_0(oobj parent, int cols, int rows, ou32 format)
{
    MMat *self = MMat_new_0_try(parent, cols, rows, format).o;
    o_assume(self, "MMat allocation failed");
    return self;
}


/**
 * Creates a new MMat object from an image file.
 * @param parent to inherit from
 * @param img to load the data from (asserts img valid)
 * @param move_img if true, the data is o_move'd into the new MMat, else its a view
 * @return The new object
 * @note uses the format of the o_img, so M_FORMAT__PRIMITIVE_U8 + img->channels
 */
MMat *MMat_new_img(oobj parent, struct o_img *img, bool move_img);


/**
 * Creates a new MMat object from an image file.
 * @param parent to inherit from
 * @param file to load the image from, either .jpg or .png
 * @return The new object, or NULL if failed
 * @note uses the format of the o_img, so M_FORMAT__PRIMITIVE_U8 + img->channels
 */
struct oobj_opt MMat_new_file_image(oobj parent, const char *file);


/**
 * Creates a new MMat object from an indexed image file.
 * @param parent to inherit from
* @param out_palette MMat object of the loaded palette, safe to pass NULL
 *                   Does not support loading transparency, .a == 255 (except for index 0, see below)
 * @param fixed_transparency if true, palette index 0 will be set to transparent
 * @param file to load the indexed image from (.png)
 * @return The new object, or NULL if failed
 * @note returned is the indexed image with M_FORMAT_1_UR8, palette as M_FORMAT_RGBA_U8
 */
struct oobj_opt MMat_new_file_image_indexed(oobj parent, oobj *out_palette, bool fixed_transparency, const char *file);


/**
 * Creates a new MMat object from a csv file.
 * @param parent to inherit from
 * @param file to load the csv data from, either .csv or .txt
 * @param format_primitive to be written into, internally uses double to parse.
 *                         channels is ignored and used what is present in the file
 * @param channel_sep, col_sep, row_sep: separators for channel, columns and rows
 *                                       default is: "," ";" "\n"
 * @param allow_inconsistency the first csv row col channel is used to determine size.
 *                            if true allow (ignore to 0) inconsistend sizes. 
 *                            false to return NULL in case
 * @return The new object, or NULL if failed
 */
struct oobj_opt MMat_new_file_csv_ex(oobj parent, const char *file, ou32 format_primitive,
    const char *channel_sep, const char *col_sep, const char *row_sep, 
    bool allow_inconsistency);

/**
 * Creates a new MMat object from a csv file.
 * @param parent to inherit from
 * @param file to load the csv data from, either .csv or .txt
 * @param format_primitive to be written into, internally uses double to parse.
 *                         channels is ignored and used what is present in tze file
 * @return The new object, or NULL if failed
 * @note uses the first row, col, channel to determine the size of the MMat aborts as failed on differences
 * @note uses the default  ",", ";", "\n" seperators
 */
o_inline struct oobj_opt MMat_new_file_csv(oobj parent, const char *file, ou32 format_primitive)
{
    return MMat_new_file_csv_ex(parent, file, format_primitive, ",", ";", "\n", false);
}


/**
 * Creates a new MMat object from a numpy binary npy file.
 * @param parent to inherit from
 * @param file to load the numpy .npy data from
 *                 Dim 0: number -> rows=cols=channels=1
 *                 Dim 1: [0, 1, 2] -> cols=channels=1
 *                 Dim 2: [[0, 1], [2, 3]] -> channels=1
 *                 Dim 3: [[[0, 1], [2, 3]], [[3, 4], [5, 6]]]
 * @note dtype unsigned (or bool) are loaded as M_FORMAT__PRIMITIVE_TYPE_U.
 *             signed in contrast as raw with M_FORMAT__PRIMITIVE_TYPE_IR
 *             to change to UR, use MMat_format_raw_set(obj, true)
 */
struct oobj_opt MMat_new_file_npy(oobj parent, const char *file);

/**
 * Creates a new MMat object from the given OJson mat (array of array of array of numbers)
 * @param parent to inherit from
 * @param json_mat OJson object to read the numbers from
 *                 Dim 0: number -> rows=cols=channels=1
 *                 Dim 1: [0, 1, 2] -> cols=channels=1
 *                 Dim 2: [[0, 1], [2, 3]] -> channels=1
 *                 Dim 3: [[[0, 1], [2, 3]], [[3, 4], [5, 6]]]
 * @param format_primitive to be written into, internally uses double to read.
 *                         channels is ignored and used what is present in the json data
 * @return The new object, or NULL if failed
 * @note uses the first row, col, channel to determine the size of the MMat 
 *       and aborts as failed on differences
 */
struct oobj_opt MMat_new_json(oobj parent, oobj json_mat, ou32 format_primitive);



//
// virtual implementations:
//

/**
 * virtual operator function
 * @param obj MMat object
 * @return number of pixels (cols * rows)
 */
osize MMat__v_op_num(oobj obj);

/**
 * virtual operator function
 * @param obj MMat object
 * @param pixel data at given index
 */
void *MMat__v_op_at(oobj obj, osize idx);

//
// object functions
//


/**
 * @return true if data is not NULL
 * @note allocations are not asserted by default in MMat.
 *       Mostly only needed for init_ex and new_ex, cause others return struct oobj_opt if failed
 * @sa MMat_assert below
 */
o_inline bool MMat_valid(oobj obj)
{
    OObj_assert(obj, MMat);
    MMat *self = obj;
    return self->data != NULL;
}

/**
 * Asserts oobj type to by MMat and that it's valid
 */
#define MMat_assert(obj) assert(MMat_valid(obj));

/**
 * @param obj MMat object
 * @return data pixels of the matrix
 */
OObj_DECL_GET(MMat, void *, data)

/**
 * @param obj MMat object
 * @return allocated root of the data buffer (always ==data if owned)
 */
OObj_DECL_GET(MMat, const void *, data_root)

/**
 * @param obj MMat object
 * @return true if both MMat share the same buffer
 */
o_inline bool MMat_data_root_shared(oobj obj, oobj mat)
{
    return MMat_data_root(obj) == MMat_data_root(mat);
}

/**
 * @param obj MMat object
 * @return true if data is owned by this object, false if just a reference
 */
OObj_DECL_GET(MMat, bool, owning)

/**
 * @param obj MMat object
 * @return current pixel format
 */
OObj_DECL_GET(MMat, ou32, format)

/**
 * @param obj  MMat object
 * @return true if the format primtive type is raw (UR or IR)
 */
o_inline bool MMat_format_raw(oobj obj)
{
    ou32 type = m_format_primitive_type(MMat_format(obj));
    return type == M_FORMAT__PRIMITIVE_TYPE_IR || type == M_FORMAT__PRIMITIVE_TYPE_UR;
}

/**
 * @param obj  MMat object
 * @return true if the format primitive type is raw (UR or IR)
 */
bool MMat_format_raw_set(oobj obj, bool set);

/**
 * @param obj MMat object
 * @return amount of channels
 */
o_inline int MMat_channels(oobj obj)
{
    return m_format_channels(MMat_format(obj));
}

/**
 * @param obj MMat object
 * @return number of pixels (cols * rows)
 */
o_inline osize MMat_num(oobj obj)
{
    OObj_assert(obj, MMat);
    MMat *self = obj;
    return (osize) self->size.x * (osize) self->size.y;
}

/**
 * @param obj MMat object
 * @return size in bytes needed to hold the matrix data
 */
o_inline osize MMat_data_size(oobj obj)
{
    OObj_assert(obj, MMat);
    MMat *self = obj;
    return MMat_num(obj) * m_format_size(self->format);
}

/**
 * @param obj MMat object
 * @param c the column, 0 is left
 * @param r the row, 0 is top
 * @return the pointer to the pixel at the given pos (starting at first channel (red))
 */
o_inline void *MMat_at(oobj obj, int c, int r)
{
    MMat_assert(obj);
    MMat *self = obj;
    assert(self->data && 0<=c && c<self->size.x && 0<=r && r < self->size.y && "idx out of o_img bounds");
    return &self->data[r * self->stride.y + c * self->stride.x];
}

/**
 * UNCHECKED RAW MACRO MODE
 * @param obj MMat object
 * @param c the column, 0 is left
 * @param r the row, 0 is top
 * @return the pointer to the pixel at the given pos (starting at first channel (red))
 */
#define MMat_at_raw(obj, c, r) ((void*) &((MMat*)(obj))->data[\
    (r) * ((MMat*)(obj))->stride.y\
    + (c) * ((MMat*)(obj))->stride.x])

/**
 * @param obj MMat object
 * @param idx of a pixel
 * @return the pointer to the pixel at the given pos (starting at first channel (red))
 */
o_inline void *MMat_at_idx(oobj obj, osize idx)
{
    MMat_assert(obj);
    MMat *self = obj;
    return MMat_at(self, idx % self->size.x, idx / self->size.x);
}

/**
 * UNCHECKED RAW MODE
 * @param obj MMat object
 * @param idx of a pixel
 * @return the pointer to the pixel at the given pos (starting at first channel (red))
 */
#define MMat_at_idx_raw(obj, idx) MMat_at_raw((obj), (idx) % ((MMat*)(obj))->size.x, (idx) / ((MMat*)(obj))->size.x)


/**
 * @param obj MMat object
 * @param c the column, 0 is left
 * @param r the row, 0 is top
 * @return Casts a pixel up to 4 channels to a vec4, if channels < 4, remaining is set to 0
 * @note For max performance, consider using MMat_at(_raw)
 */
o_inline m_vec4 MMat_at_vec4(oobj obj, int c, int r)
{
    return m_format_cast_to_vec4(MMat_at(obj, c, r), MMat_format(obj));
}

/**
 * @param obj MMat object
 * @param c the column, 0 is left
 * @param r the row, 0 is top
 * @param set vec4 to cast from up to 4 channels, channels > 4 are untouched / ignored
 * @note For max performance, consider using MMat_at(_raw)
 */
o_inline m_vec4 MMat_at_vec4_set(oobj obj, int c, int r, m_vec4 set)
{
    m_format_cast_from_vec4(MMat_at(obj, c, r), MMat_format(obj), set);
    return set;
}

/**
 * @param obj MMat object
 * @return size of the allocated matrix
 */
o_inline m_ivec2 MMat_size_int(oobj obj)
{
    OObj_assert(obj, MMat);
    MMat *self = obj;
    return self->size;
}

/**
 * @param obj MMat object
 * @returnsize of the allocated matrix
 * @note integer values
 */
o_inline m_vec2 MMat_size(oobj obj)
{
    m_ivec2 size = MMat_size_int(obj);
    return (m_vec2){{size.x, size.y}};
}


/**
 * Creates a rect containing the full mat
 * @param obj MMat object
 * @param x position (left)
 * @param y position (top)
 * @return rect (ltwh) containing the full mat on position xy (lt)
 */
o_inline m_vec4 MMat_rect(oobj obj, float x, float y)
{
    m_ivec2 size = MMat_size_int(obj);
    return (m_vec4){{x, y, size.x, size.y}};
}


/**
 * Creates a rect containing the full mat
 * @param obj MMat object
 * @param x position (left)
 * @param y position (top)
 * @return rect (ltwh) containing the full mat on position xy (lt)
 */
o_inline m_ivec4 MMat_rect_int(oobj obj, int x, int y)
{
    m_ivec2 size = MMat_size_int(obj);
    return (m_ivec4){{x, y, size.x, size.y}};
}

/**
 * @param obj MMat object
 * @return true if data is packed
 */
o_inline m_ivec2 MMat_packed_stride(oobj obj)
{
    OObj_assert(obj, MMat);
    MMat *self = obj;
    ou32 format_size = m_format_size(self->format);
    return (m_ivec2) {{format_size, format_size * self->size.x}};
}

/**
 * @param obj MMat object
 * @return true if data is packed
 */
o_inline bool MMat_packed(oobj obj)
{
    OObj_assert(obj, MMat);
    MMat *self = obj;
    m_ivec2 packed_stride = MMat_packed_stride(obj);
    return (self->stride.x == packed_stride.x) && (self->stride.y == packed_stride.y);
}


/**
 * @param obj MMat object
 * @return new allocated buffer with packed data (or NULL if failed to allocate)
 * @note call o_free to free it
 */
void *MMat_data_packed(oobj obj);

/**
 * @param obj MMat object
 * @param data packed to be copied into the MMat
 */
void MMat_data_packed_set(oobj obj, const void *restrict data);


/**
 * Will (try to) make this MMat owning.
 * If not owning, will allocate and clone values from its reference.
 * If already owning, this is a noop.
 * If failed, MMat_data gets NULL.
 * @param obj MMat object
 * @note MMat_data() may be NULL if allocation fails while cloning! Test with MMat_valid() before working with it!
 */
void MMat_owning_make_try(oobj obj);

/**
 * Will make this MMat owning.
 * If not owning, will allocate and clone values from its reference.
 * If already owning, this is a noop.
 * @param obj MMat object
 */
o_inline void MMat_owning_make(oobj obj)
{
    MMat_owning_make_try(obj);
    MMat_assert(obj);
}


/**
 * @param obj MMat object
 * @param mat MMat object to compare with
 * @return true if all: format + size + stride equal in obj and mat
 */
o_inline bool MMat_layout_equals(oobj obj, oobj mat)
{
    MMat_assert(obj);
    MMat *self = obj;
    MMat_assert(mat);
    MMat *mat_self = mat;
    return self->format == mat_self->format
            && self->size.x == mat_self->size.x
            && self->size.y == mat_self->size.y
            && self->stride.x == mat_self->stride.x
            && self->stride.y == mat_self->stride.y;
}

/**
 * @param obj MMat object
 * @param mat MMat object to compare with
 * @return true if fields equal exactly
 *              (same size, same format, same pixel values)
 *              stride does not need to be the same
 */
bool MMat_equals(oobj obj, oobj mat);


/**
 * Sorts the data matrix with qsort (along the indices).
 * @param obj MMat object
 * @param comperator a comperator function, return ("a" - "b") to sort in ascending order
 *                   <0: a is less than b.
 *                   0:  equal.
 *                   >0: a is greater than b.
 *                   The o__compare_fn user_data is set to obj MMat
 * @return true on success (always if packed, may fail to allocate on none packed data)
 * @note if the data is not packed, a clone is created and sort with that comperator.
 *       The user_data of the comperator will still be obj, but the sorted data is from the clone instead.
 *       After sorting the clone, the data is copied back to obj.
 *
 *       Simply returning ala: "return a<b" is not enough! 
 *       use "return a<b? -1:+1" instead.
 *       or better "return a-b" for integers.
 *       or "return m_sign(a-b)" for floats.
 */
bool MMat_sort_try(oobj obj, o__compare_fn comperator);

/**
 * Sorts the data matrix with qsort (along the indices).
 * @param obj MMat object
 * @param comperator a comperator function, return ("a" - "b") to sort in ascending order (integer only)
 *                   <0: a is less than b.
 *                   0:  equal.
 *                   >0: a is greater than b.
 *                   The o__compare_fn user_data is set to obj MMat.
 * @note simply returning ala: "return a<b" is not enough! 
 *       use "return a<b? -1:+1" instead.
 *       or better "return a-b" for integers.
 *       or "return m_sign(a-b)" for floats.
 */
o_inline void MMat_sort(oobj obj, o__compare_fn comperator)
{
    bool valid = MMat_sort_try(obj, comperator);
    o_assume(valid, "MMat allocation failed");
}

/**
 * Searches in the data matrix with brute force
 * @param obj MMat object
 * @param search_element the element to search the index for
 * @param comperator a comperator function, return ("a" - "b") to sort in ascending order (integer only)
 *                   <0: a is less than b.
 *                   0:  equal.
 *                   >0: a is greater than b.
 *                   The o__compare_fn user_data is set to obj MMat.
 * @return the index in the array for the searched element, or -1 if not found
 */
osize MMat_search(oobj obj, const void *search_element, o__compare_fn comperator);

//
// write io
//

/**
 * @param obj MMat object
 * @param out_img image view from this MMat data
 * @return true on success, false if format is invalid
 *         needs to be packed M_FORMAT__PRIMITIVE_U8 + channels [1:4]
 */
bool MMat_img(oobj obj, struct o_img *out_img);

/**
 * Writes the (image) matrix to the given image file.
 * Currently only possible with M_FORMAT_RGBA_U8 output format.
 * @param obj MMat object
 * @param file to save the image into
 * @return false if failed
 * @note if the format of obj does not match packed M_FORMAT_[1:4]_U8, it gets casted down
 */
bool MMat_write_file_image(oobj obj, const char *file);

/**
 * Writes the indexed image matrix and its palette to the given indexed image png file.
 * @param obj MMat object of the indexed image, assertde to be either M_FORMAT_1_UR8 or M_FORMAT_1_U8
 * @param palette MMat object of the palette
 * @param file to save the indexed image into
 * @return false if failed
 * @note if the format of obj does not match packed M_FORMAT_[1:4]_U8, it gets casted down
 */
bool MMat_write_file_image_indexed(oobj obj, oobj palette, const char *file);

/**
 * Writes the matrix to the given csv file.
 * @param obj MMat object
 * @parem file to save the csv mat into
 * @param channel_sep, col_sep, row_sep: separators for channel, columns and rows
 *                                       default is: "," ";" "\n"
 * @return false if failed
 */
bool MMat_write_file_csv_ex(oobj obj, const char *file, const char *channel_sep, const char *col_sep, const char *row_sep);

/**
 * Writes the matrix to the given csv file.
 * @param obj MMat object
 * @parem file to save the csv mat into
 * @return false if failed
 * @note uses the default ",", ";", "\n" seperators
 */
o_inline bool MMat_write_file_csv(oobj obj, const char *file)
{
    return MMat_write_file_csv_ex(obj, file, ",", ";", "\n");
}

/**
 * Writes as a numpy .npy binary file
 * Dim 0: [number] <- rows=cols=channels=1
 * Dim 1: [0, 1, 2] <- cols=channels=1
 * Dim 2: [[0, 1], [2, 3]] <- channels=1
 * Dim 3: [[[0, 1], [2, 3]], [[3, 4], [5, 6]]]
 * @param obj MMat object
 * @param file .npy file
 * @return false if failed
 */
bool MMat_write_file_npy(oobj obj, const char *file);


/**
 * Creates an OJson tree containing the mat fields.
 * According to the MMat sizes, its structured as:
 * Dim 0: number <- rows=cols=channels=1
 * Dim 1: [0, 1, 2] <- cols=channels=1
 * Dim 2: [[0, 1], [2, 3]] <- channels=1
 * Dim 3: [[[0, 1], [2, 3]], [[3, 4], [5, 6]]]
 * Invalid: OJson_NULL
 * @param obj MMat object
 * @param parent OObj to allocate the OJson on
 * @param name for json if the parent is a json object, NULL safe
 * @return OJson tree allocated on parent
 */
oobj MMat_json(oobj obj, oobj parent, const char *name);


/**
 * @param obj MMat object
 * @return new allocated string with the MMat fields, free with o_free(obj, ...)
 * @note uses OJson_dump and MMat_json internally
 */
char *MMat_str(oobj obj);

/**
 * @param obj MMat object to print to the console
 * @note uses MMat_str internally
 */
void MMat_print(oobj obj);

/**
 * @param obj MMat object to print to the console with a newline at the end
 * @note uses MMat_str internally
 */
void MMat_println(oobj obj);




//
// Copies
//


void MMat_clone_into(oobj obj, oobj into);
struct oobj_opt MMat_clone_try(oobj obj);
/**
 * Clones the MMat
 * @param obj MMat object
 * @return MMat allocated on obj (packed)
 */
o_inline MMat *MMat_clone(oobj obj)
{
    MMat *self = MMat_clone_try(obj).o;
    o_assume(self, "MMat allocation failed");
    return self;
}


void MMat_cast_into(oobj obj, oobj into);
struct oobj_opt MMat_cast_try(oobj obj, ou32 format);
/**
 * Clones and casts the MMat.
 * Casts internally using double as an immediate format, so >may< lose precision.
 * @param obj MMat object
 * @param format for the new generated MMat, channels must match!
 * @return MMat allocated on obj (packed)
 * @note Casts only at max M_FORMAT_MAX_SIZE channels.
 */
o_inline MMat *MMat_cast(oobj obj, ou32 format)
{
    MMat *self = MMat_cast_try(obj, format).o;
    o_assume(self, "MMat allocation failed");
    return self;
}



#endif //M_MMAT_H
