#ifndef O_OPATTERN_H
#define O_OPATTERN_H

/**
 * @file OPattern.h
 *
 * Object
 *
 * For matching a pattern on a string or a data array.
 * With lua-like pattern matching, see: Lua Patterns: https://www.lua.org/pil/20.2.html
 *
 *
 * -Patterns in short:-
 * They define a group of valid characters to be matched,
 * together with some additional rules.
 * The simple pattern "Hello World" has a single character in its 11 single groups.
 * Really just matches the exact string.
 * Defining a group can be done with valid characters or ranges inside square brackets.
 * To clearify, here a group is the smallest searchable unit, like a character or a set of characters as a group.
 * In the pattern "Ba[rz]" we defined a group for either 'r' or 'z'.
 * So it will match both, "Bar" and "Baz".
 * To match all digits we could either use:
 * "[0123456789]" or as range "[0-9]" or the pre defined group "%d".
 * ranges can also be mixed as with "[a-zA-Z]" ("%a") for all letters.
 * Groups can also be inverted by adding '^' at the begin.
 * Like "[^\n]" for matching all but a new line character.
 * Pre defined groups:
 *    .    all characters
 *   %a    letters
 *   %c    control characters (ascii)
 *   %d    digits
 *   %l    lower case letters
 *   %p    punctuation characters
 *   %s    space characters
 *   %u    upper case letters
 *   %w    alphanumeric characters
 *   %x    hexadecimal digits
 *   %z    the character with representation 0
 *
 * Some characters are called magic characters:
 *  ( ) . % + - * ? [ ^ $
 * To match them, use '%' to escape:
 * "%%" -> '%'
 * "%-" -> '-'
 * "%k" -> 'k' (escaping works for all characters)
 *
 * Each group can have a modifyier applied to it (as a reminder "Foo" has 3 groups, "Ba[rz]" has also 3).
 *   +    1 or more repetitions
 *   *    0 or more repetitions
 *   -    also 0 or more repetitions (same as *)
 *   ?    optional (0 or 1 occurrence)
 *
 * So the pattern "Fo+" matches "Fo", "Foo", "Foooooo", ....
 * The pattern "Apples?" matches both "Apple" and "Apples"
 * Modifyier can also be applied on custom groups of course.
 * The pattern ".*Foo" matches all characters followed by the string "Foo"
 * OPattern works in a few different modes:
 *   find     -> search a pattern in string / data
 *   match    -> is a string equal to the pattern?
 *   find_all -> searches and returns all valid substrings
 *   replace  -> find_all and replace
 * Patterns can be anchored with '^' for the begin and '$' for the end.
 * so matching is like using both "^Foo$" as the string needs to start with Foo and ends with it.
 *
 * Patterns also allow for capturing regions inside em.
 * A pattern can add multiple captures with braces '()'
 * As an example:
 * "(%d%d):(%d%d):%d%d" is equal to the pattern  "%d%d:%d%d:%d%d" and searches for a time in HH:MM:SS format (06:01:45).
 * But also captures HH and MM.
 * A match or find call will return those as captured ranges.
 *
 *
 *
 * @sa str.h o_str_pattern_* if you just want a single run
 */


#include "OObj.h"
#include "str_range.h"

/** object id */
#define OPattern_ID OObj_ID "OPattern."


/**
 * Group modifier
 * SINGLE is default.
 * PLUS: minimal one occurrence (+).
 * MUL: zero or more occurrences (* or -)
 * OPT: zero or one occurrence (?)
 */
enum OPattern_group_mode {
    OPattern_group_SINGLE,
    OPattern_group_PLUS,
    OPattern_group_MUL,
    OPattern_group_OPT,
    OPattern_group_ENUM_MAX
};

/**
 * A compiled group. A char can be tested with the "flag bit array"
 */
struct OPattern_group {
    // 32*8 = 256 bits|flags
    obyte flags[32];
    enum OPattern_group_mode mode;
};

/**
 * @param self pattern group to check a character | byte from
 * @param test character to test
 * @return true if the character is valid in the group
 */
o_inline bool OPattern_group_check(const struct OPattern_group *self, obyte test)
{
    obyte bitmask = 1 << (test % 8);
    return self->flags[test / 8] & bitmask;
}


typedef struct {
    OObj super;

    struct OPattern_group *groups;
    osize groups_size;

    bool anchored_start;
    bool anchored_end;
    osize min_groups;

    // list of captures "(..)", according to the group index
    struct o_str_range *captures;
    osize captures_size;

} OPattern;


/**
 * Initializes the object
 * @param obj OPattern object
 * @param parent to inherit from
 * @param pattern the lua-like pattern string to be compiled for matches, NULL safe
 * @return obj casted as OPattern
 */
OPattern *OPattern_init(oobj obj, oobj parent, const char *pattern);

/**
 * Creates a new OPattern object
 * @param parent to inherit from
 * @param fn the future execution function
 * @param pattern the lua-like pattern string to be compiled for matches, NULL safe
 * @return The new object
 */
o_inline OPattern *OPattern_new(oobj parent, const char *pattern)
{
    OObj_DECL_IMPL_NEW(OPattern, parent, pattern);
}

//
// object functions
//

/**
 * @param obj OPattern object
 * @return the maximal amount of captures in the compiled pattern
 */
o_inline osize OPattern_captures(oobj obj)
{
    OObj_assert(obj, OPattern);
    OPattern *self = obj;
    return self->captures_size;
}

/**
 * Finds the next valid pattern on the given data.
 * @param obj OPattern object
 * @param data to be matched, NULL safe
 * @param data_size byte size of data, <=0 safe
 * @param out_opt_captures if not NULL, expected as array of (at least) size: OPatter_captures.
 *                         If some patterns are not found, they will be cleared to 0
 * @return the range of the next match, or a cleared element (.len = 0) for not found
 * @sa o_str_pattern_find
 */
struct o_str_range OPattern_find_data(oobj obj, const void *data, osize data_size,
                                      struct o_str_range *out_opt_captures);


/**
 * Finds the next valid pattern on the string.
 * @param obj OPattern object
 * @param str to be matched, NULL safe
 * @param out_opt_captures if not NULL, expected as array of (at least) size: OPattern_captures
 *                         If some patterns are not found, they will be cleared to 0
 * @return the range of the next match, or a cleared element (.len = 0) for not found
 * @sa o_str_pattern_find
 */
o_inline struct o_str_range OPattern_find(oobj obj, const char *str, struct o_str_range *out_opt_captures)
{
    return OPattern_find_data(obj, str, o_strlen(str), out_opt_captures);
}

/**
 * Matches a pattern with the given data (the full data must match to the pattern).
 * @param obj OPattern object
 * @param data to be matched, NULL safe
 * @param data_size byte size of data, <=0 safe
 * @param out_opt_captures if not NULL, expected as array of (at least) size: OPatter_captures.
 *                         If some patterns are not found, they will be cleared to 0
 * @return true if matched
 * @sa o_str_pattern_match
 */
o_inline bool OPattern_match_data(oobj obj, const void *data, osize data_size, struct o_str_range *out_opt_captures)
{
    struct o_str_range res = OPattern_find_data(obj, data, data_size, out_opt_captures);
    return res.start == 0 && res.len == data_size;
}


/**
 * Matches a pattern with the string (the full string must match to the pattern).
 * @param obj OPattern object
 * @param str to be matched, NULL safe
 * @param out_opt_captures if not NULL, expected as array of (at least) size: OPattern_captures
 *                         If some patterns are not found, they will be cleared to 0
 * @return true if matched
 * @sa o_str_pattern_match
 */
o_inline bool OPattern_match(oobj obj, const char *str, struct o_str_range *out_opt_captures)
{
    return OPattern_match_data(obj, str, o_strlen(str), out_opt_captures);
}


/**
 * Searches the pattern on the given data and creates a list of all findings (and captures)
 * @param obj OPattern object
 * @param out_opt_matches if not NULL, a list of ranges is created, containing all matches
 * @param out_opt_captures_list if not NULL, a list of a list of captures is created
 * @param data to be matched, NULL safe
 * @param data_size byte size of data, <=0 safe
 * @return a cleared element (.len = 0) terminated list of matches (allocated ob obj)
 * @sa o_str_pattern_find_all
 */
osize OPattern_find_all_data(oobj obj,
                        struct o_str_range_list *out_opt_matches,
                        struct o_str_range_list_list *out_opt_captures_list,
                        const void *data, osize data_size);


/**
 * Searches the pattern on the string and creates a list of all findings (and captures)
 * @param obj OPattern object
 * @param out_opt_matches if not NULL, a list of ranges is created, containing all matches
 * @param out_opt_captures_list if not NULL, a list of a list of captures is created
 * @param str to be matched, NULL safe
 * @return findings counter, equal to the generated lists sizes for: out_opt_matches, out_opt_captures_list
 * @sa o_str_pattern_find_all
 */
o_inline osize OPattern_find_all(oobj obj,
                        struct o_str_range_list *out_opt_matches,
                        struct o_str_range_list_list *out_opt_captures_list,
                        const char *str)
{
    return OPattern_find_all_data(obj, out_opt_matches, out_opt_captures_list, str, o_strlen(str));
}


/**
 * Replaces all findings with the replace data.
 * @param obj OPattern object
 * @param out_data A new allocated data array with all matches replaces (allocated on obj)
 *                 always null terminated (out_data[returned_size] == '\0')
 * @param data to be matched, NULL safe
 * @param data_size byte size of data, <=0 safe
 * @param replace substitutes all matches with this data
 *                (may use capture positions "%X", to replace "%" escape it with "%%"
 * @param data_size byte size of replace, <=0 safe
 * @param out_opt_replaced if not NULL, will be set to the amount of replaced matches
 * @return the size of the allocated data array
 * @sa o_str_pattern_replace
 */
osize OPattern_replace_data(oobj obj, void **out_data, const void *data, osize data_size,
                            const void *replace, osize replace_size, osize *out_opt_replaced);


/**
 * Replaces all findings with the replace string.
 * @param obj OPattern object
 * @param str to be matched, NULL safe
 * @param replace substitutes all matches with this string
 *                (may use capture positions "%X", to replace "%" escape it with "%%"
 * @param out_opt_replaced if not NULL, will be set to the amount of replaced matches
 * @return A new allocated string with all matches replaces (allocated on obj)
 * @sa o_str_pattern_replace
 */
o_inline char *OPattern_replace(oobj obj, const char *str, const char *replace, osize *out_opt_replaced)
{
    void *data = NULL;
    OPattern_replace_data(obj, &data, str, o_strlen(str), replace, o_strlen(replace), out_opt_replaced);
    return (char *) data;
}

#endif //O_OPATTERN_H
