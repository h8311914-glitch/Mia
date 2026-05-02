#ifndef M_IO_FLT_MATN_H
#define M_IO_FLT_MATN_H

/**
 * @file io/matn.h
 *
 * print float  matrices to buffer.
 * json parser.
 *
 * @note Functions with an `*_a` postfix are o_arena() allocated. So use with caution.
 *          o_arena() is typically set via a_app to be a frame based arena allocator with fixed bytes size.
 */

#include "m/common.h"



/**
 * prints into the given string buffer
 * In the style:
 * multiline && typed && n>=2 && n<=4:   "(mat<n>)
 *                                        {{ 1.1, 2.2,
 *                                           ... , n*n }}"
 * multiline ^^ else:                    "(float[4])
 *                                        { 1.1, 2.2,
 *                                          3.3, 4.4 }"
 *           typed && n>=2 && n<=4:      "(mat<n>) {{ 1.1, 2.2,   ... , n*n }}"
 *           else:                       "(float[4]) { 1.1, 2.2,   3.3, 4.4 }"
 * @param m the float square matrix
 * @param n the size (rows=n, cols=n)
 * @param colored true to use color codes for a terminal
 * @param typed if true, prints with the vec<n> type (2..4)
 * @param multiline if true, prints with newlines, else in a single line
 * @return characters written
 * @note printed transposed, so it matches C row major, even if this lib is column major!
 */
osize matn_buffer(char *str, osize size, const float *m, int n, bool colored, bool typed, bool multiline);

//
// print to stdout
//



/**
 * Calls matn_buffer and prints to stdout with colors (typed)
 * In the style:
 * multiline&& n>=2 && n<=4:   "(mat<n>)
 *                             {{ 1.1, 2.2,
 *                                ... , n*n }}"
 * multiline ^^ else:          "(float[4])
 *                              { 1.1, 2.2,
 *                                3.3, 4.4 }"
 *            n>=2 && n<=4:    "(mat<n>) {{ 1.1, 2.2,   ... , n*n }}"
 *           else:             "(float[4]) { 1.1, 2.2,   3.3, 4.4 }"
 * @param m the float vector
 * @param n the size
 * @param multiline if true, prints with newlines, else in a single line
 * @note printed transposed, so it matches C row major, even if this lib is column major!
 */
osize matn_print(const float *m, int n, bool multiline);



/**
 * Calls matn_buffer and prints to stdout with colors (typed)
 * Adds a newline at the end.
 * In the style:
 * multiline&& n>=2 && n<=4:   "(mat<n>)
 *                             {{ 1.1, 2.2,
 *                                ... , n*n }}"
 * multiline ^^ else:          "(float[4])
 *                              { 1.1, 2.2,
 *                                3.3, 4.4 }"
 *            n>=2 && n<=4:    "(mat<n>) {{ 1.1, 2.2,   ... , n*n }}"
 *           else:             "(float[4]) { 1.1, 2.2,   3.3, 4.4 }"
 * @param m the float square matrix
 * @param n the size
 * @param multiline if true, prints with newlines, else in a single line
 * @note printed transposed, so it matches C row major, even if this lib is column major!
 */
osize matn_println(const float *m, int n, bool multiline);

/**
 * prints into a new allocated string.
 * In the style:
 * multiline && typed && n>=2 && n<=4:   "(mat<n>)
 *                                        {{ 1.1, 2.2,
 *                                           ... , n*n }}"
 * multiline ^^ else:                    "(float[4])
 *                                        { 1.1, 2.2,
 *                                          3.3, 4.4 }"
 *           typed && n>=2 && n<=4:      "(mat<n>) {{ 1.1, 2.2,   ... , n*n }}"
 *           else:                       "(float[4]) { 1.1, 2.2,   3.3, 4.4 }"
 * @param parent object to allocate on
 * @param m the float square matrix
 * @param n the size (rows=n, cols=n)
 * @param colored true to use color codes for a terminal
 * @param typed if true, prints with the vec<n> type (2..4)
 * @param multiline if true, prints with newlines, else in a single line
 * @return characters written
 * @note printed transposed, so it matches C row major, even if this lib is column major!
 */
char *matn_str(oobj parent, const float *m, int n, bool colored, bool typed, bool multiline);
#define matn_str_a(m, n, colored, typed, multiline) \
    matn_str(o_arena(), (m), (n), (colored), (typed), (multiline))


//
// json stuff
//



/**
 * Tries to read the json hierarchy array to read n OJson_TYPE_ARRAY in which n float numbers are read
 * @param out_m float square matrix to read in
 * @param n size of the square matrix
 * @param json_array OJson object type OJson_TYPE_ARRAY
 * @return true if valid
 * @example json: [[1.1, 2.2], [3.3, 4.4]]
 * @note printed transposed, so it matches C row major, even if this lib is column major!
 */
bool matn_json_read(float *out_m, int n, oobj json_array);


/**
 * Creates a new OJson OJson_TYPE_ARRAY, representing the given matrix
 * @param parent to inherit from
 * @param name if the parent is an object, NULL safe
 * @param m float square matrix to create the json array from
 * @param n size of the matrix
 * @return new allocated OJson OJson_TYPE_ARRAY object as child of parent
 * @example json: [[1.1, 2.2], [3.3, 4.4]]
 * @note printed transposed, so it matches C row major, even if this lib is column major!
 */
oobj matn_json_new(oobj parent, const char *name, const float *m, int n);


#endif //M_IO_FLT_MATN_H
