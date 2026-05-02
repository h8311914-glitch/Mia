#include "o/OJson.h"
#include "o/OStream.h"
#include "o/OList.h"
#include "o/file.h"
#include "o/str.h"
#include "o/timer.h"
#include <stdlib.h>

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"

#define INDENT_LEVEL "  "

#define LOG_PREV 16
#define LOG_NEXT 8

#define R_MAX_DEPTH 32

/**
 * This file parses and writes to an OStream.
 * array.c in contrast directly operates on ram and is MUCH faster.
 * The stream version was the initial parser and is left in the code base.
 */

// protected
bool OJson__parse_stream_r(oobj parent, oobj stream, const char *name, int depth);

// protected
osize OJson__write_stream_r(oobj container, OJson *json, OStream *stream, int lvl);


static void log_parse_stream_failed(OStream *stream, const char *msg)
{
    _Static_assert(LOG_PREV < 32 && LOG_NEXT < 32, "invalid buffer");
    char prev[32] = {0};
    char next[32] = {0};
    int prev_num = o_min(LOG_PREV, OStream_seek(stream, 0, OStream_CUR));
    OStream_seek(stream, -prev_num, OStream_CUR);
    OStream_read(stream, prev, 1, prev_num);
    OStream_read(stream, next, 1, LOG_NEXT);
    OStream_seek(stream, -LOG_NEXT, OStream_CUR);

    o_str_replace_char_this(prev, '\n', ' ');
    o_str_replace_char_this(next, '\n', ' ');

    o_log_warn_s("OJson_new_read_stream",
               "Parsing failed: %s at: [..]%s<!!!>%s[..]", msg, prev, next);
}

static void log_stream_failed(void)
{
    o_log_warn_s("OJson_new_read_stream",
               "stream failed to read");
}


//
// writing
//


static osize write_stream_newline(OStream *stream, int lvl)
{
    osize written = 0;
    written += OStream_print(stream, "\n");
    for (int i = 0; i < lvl; i++) {
        written += OStream_print(stream, INDENT_LEVEL);
    }
    return written;
}

static osize write_stream_object(oobj container, OJson *json_object, OStream *stream, int lvl)
{
    osize written = 0;
    OJson **list = OJson_list(json_object, NULL);

    // not a single child found
    if (*list == NULL) {
        written += OStream_print(stream, "{}");
        o_free(json_object, list);
        return written;
    }

    written += OStream_print(stream, "{");
    OJson **it = list;
    while (*it) {
        OJson *child = *it++;
        if (!json_object->packed) {
            written += write_stream_newline(stream, lvl + 1);
        }
        if (OObj_name(child)) {
            char *escaped = o_str_escape(container, OObj_name(child));
            written += OStream_printf(stream, "\"%s\": ", escaped);
            o_free(container, escaped);
        } else {
            OStream_print(stream, "\"" OJson_NAME_DEFAULT "\": ");
        }
        written += OJson__write_stream_r(container, child, stream, lvl + 1);
        if (*it != NULL) {
            written += OStream_print(stream, ",");
        }
    }

    if (!json_object->packed) {
        written += write_stream_newline(stream, lvl);
    }
    written += OStream_print(stream, "}");

    o_free(json_object, list);
    return written;
}

static osize write_stream_array(oobj container, OJson *json_array, OStream *stream, int lvl)
{
    osize written = 0;
    OJson **list = OJson_list(json_array, NULL);

    // not a single child found, replacing with null
    if (*list == NULL) {
        written += OStream_print(stream, "[]");
        o_free(json_array, list);
        return written;
    }

    written += OStream_print(stream, "[");
    OJson **it = list;
    while (*it) {
        OJson *child = *it++;
        if (!json_array->packed) {
            written += write_stream_newline(stream, lvl + 1);
        }
        written += OJson__write_stream_r(container, child, stream, lvl + 1);
        if (*it != NULL) {
            written += OStream_print(stream, ",");
        }
    }

    if (!json_array->packed) {
        written += write_stream_newline(stream, lvl);
    }
    written += OStream_print(stream, "]");

    o_free(json_array, list);
    return written;
}

// recursive function, returns the num written chars
osize OJson__write_stream_r(oobj container, OJson *json, OStream *stream, int lvl)
{
    char *escaped;
    bool ret;
    switch (json->type) {
        case OJson_NULL:
            return OStream_print(stream, "null");
        case OJson_BOOLEAN:
            return OStream_print(stream, json->data.boolean ? "true" : "false");
        case OJson_NUMBER:
            return OStream_printf(stream, "%g", json->data.number);
        case OJson_STRING:
            escaped = o_str_escape(container, json->data.string);
            ret = OStream_printf(stream, "\"%s\"", escaped);
            o_free(container, escaped);
            return ret;
        case OJson_OBJECT:
            return write_stream_object(container, json, stream, lvl);
        case OJson_ARRAY:
            return write_stream_array(container, json, stream, lvl);
        default:
            o_log_warn_s("OJson_write_stream", "invalid type: %i", (int) json->type);
    }
    return 0;
}


//
// parsing
//


static bool parse_stream_null(oobj parent, oobj stream, const char *name)
{
    // n was already read
    char buf[4] = {0};
    OStream_read(stream, buf, 1, 3);
    if (!o_str_equals(buf, "ull")) {
        log_parse_stream_failed(stream, "expected null");
        return false;
    }
    OJson_new_null(parent, name);
    return true;
}


static bool parse_stream_boolean(oobj parent, oobj stream, const char *name, int first_char)
{
    ou64 timer = o_timer();
    char buf[16] = {0};
    buf[0] = (char) first_char;

    osize read = OStream_read_until_into(stream, NULL, &buf[1], sizeof buf - 2, O_ISSPACE_SET ",}]");
    if (read < 0) {
        log_stream_failed();
        return false;
    }

    // backseek the character to not drop it
    OStream_seek(stream, -1, OStream_CUR);

    bool boolean = false;
    if (o_str_equals(buf, "true")) {
        boolean = true;
    } else if (!o_str_equals(buf, "false")) {
        log_parse_stream_failed(stream, "failed to parse the boolean");
        return false;
    }
    OJson_new_boolean(parent, name, boolean);
    return true;
}


static bool parse_stream_number(oobj parent, oobj stream, const char *name, int first_char)
{
    ou64 timer = o_timer();
    char buf[64] = {0};
    buf[0] = (char) first_char;

    osize read = OStream_read_until_into(stream, NULL, &buf[1], sizeof buf - 2, O_ISSPACE_SET ",}]");
    if (read < 0) {
        log_stream_failed();
        return false;
    }

    // backseek the character to not drop it
    OStream_seek(stream, -1, OStream_CUR);

    char *end;
    double number = strtod(buf, &end);
    if (end == buf) {
        log_parse_stream_failed(stream, "failed to parse the number");
        return false;
    }
    OJson_new_number(parent, name, number);
    return true;
}

// needed by parse_stream_string and parse_stream_object
static char *parse_stream_read_string(oobj parent, oobj stream)
{
    ou64 timer = o_timer();
    // we read in until char '\"', but it could be escaped, so we rerun and put these strings in a list
    oobj list = OList_new(parent, NULL, 0);
    char *ret = NULL;

    for (;;) {
        char *str = OStream_read_until(stream, NULL, osize_MAX, "\"");
        if (!str) {
            log_parse_stream_failed(stream, "failed to read string");
            ret = NULL;
            goto CLEAN_UP;
        }

        // maybe directly closed
        osize len = o_strlen(str);
        if (len <= 1) {
            break;
        }

        // check if not escaped
        if (str[len - 2] != '\\') {
            // remove last '\"'
            str[len - 1] = '\0';

            // add end to list
            OList_push(list, str);
            break;
        }

        // add escaped to list
        OList_push(list, str);
    }

    char *cat = o_str_join(list, (char**) OList_list(list), o_num(list), NULL);
    // NULL -> empty string
    cat = o_or(cat, "");
    char *unescaped = o_str_unescape(parent, cat);
    ret = unescaped;

    CLEAN_UP:
    // clear list and cat (list is parent)
    o_del(list);
    return ret;
}

static bool parse_stream_string(oobj parent, oobj stream, const char *name)
{
    char *str = parse_stream_read_string(parent, stream);
    if (str) {
        // will be cloned...
        OJson_new_string(parent, name, str);
        o_free(parent, str);
        return true;
    }
    return false;
}

static bool parse_stream_object(oobj parent, oobj stream, const char *name, int depth)
{

    // create the object
    oobj object_element = OJson_new_object(parent, name);

    for (;;) {
        // ignore whitespace and return the first not isspace character
        int first_char;
        OStream_read_isspace_into(stream, &first_char, NULL, osize_MAX);
        if (first_char == -1) {
            log_stream_failed();
            return false;
        }

        if (first_char == '}') {
            // object closed
            return true;
        }

        // should be '\"' for the name
        if (first_char != '\"') {
            log_parse_stream_failed(stream, "expected beginning of an object name");
            return false;
        }

        char *child_name = parse_stream_read_string(parent, stream);
        if(!child_name) {
            return false;
        }

        // reading up to the colon ":"
        osize read = OStream_read_until_into(stream, NULL, NULL, osize_MAX, ":");
        if (read < 0) {
            log_stream_failed();
            o_free(parent, child_name);
            return false;
        }

        // recursively read the childs data
        bool ok = OJson__parse_stream_r(object_element, stream, child_name, depth);
        o_free(parent, child_name);

        if(!ok) {
            return false;
        }

        // ',' or '}' ?
        OStream_read_isspace_into(stream, &first_char, NULL, osize_MAX);
        if (first_char == -1) {
            log_stream_failed();
            return false;
        }

        if(first_char == '}') {
            return true;
        }

        // should be ','
        if(first_char != ',') {
            log_parse_stream_failed(stream, "expected a \',\'");
            return false;
        }
    }

    // should not get here
    return false;
}

static bool parse_stream_array(oobj parent, oobj stream, const char *name, int depth)
{

    // create the object
    oobj array_element = OJson_new_array(parent, name);

    for (;;) {

        // recursively read the childs data
        bool ok = OJson__parse_stream_r(array_element, stream, NULL, depth);
        if(!ok) {
            return false;
        }

        // ',' or ']' ?
        int first_char;
        OStream_read_isspace_into(stream, &first_char, NULL, osize_MAX);
        if (first_char == -1) {
            log_stream_failed();
            return false;
        }

        if(first_char == ']') {
            return true;
        }

        // should be ','
        if(first_char != ',') {
            log_parse_stream_failed(stream, "expected a \',\'");
            return false;
        }
    }

    // should not get here
    return false;
}

// protected
bool OJson__parse_stream_r(oobj parent, oobj stream, const char *name, int depth)
{
    depth++;
    if(depth>R_MAX_DEPTH) {
        log_parse_stream_failed(stream, "max depth reached");
    }
    
    // we start at the value of an element, like the roots opening object "{" or just null, 3.1415, "message", [

    // ignore whitespace and return the first not isspace character
    int first_char;
    OStream_read_isspace_into(stream, &first_char, NULL, osize_MAX);

    if (first_char == 'n') {
        return parse_stream_null(parent, stream, name);
    }
    if (first_char == 't' || first_char == 'f') {
        return parse_stream_boolean(parent, stream, name, first_char);
    }
    if ((first_char >= '0' && first_char <= '9')
        || first_char == '+'
        || first_char == '-'
        || first_char == '.') {
        return parse_stream_number(parent, stream, name, first_char);
    }
    if (first_char == '\"') {
        return parse_stream_string(parent, stream, name);
    }
    if (first_char == '{') {
        return parse_stream_object(parent, stream, name, depth);
    }
    if(first_char == '}') {
        // empty object let the parse_stream_object detect it:
        OStream_seek(stream, -1, OStream_CUR);
        return true;
    }
    if (first_char == '[') {
        return parse_stream_array(parent, stream, name, depth);
    }
    if(first_char == ']') {
        // empty array let the parse_stream_array detect it:
        OStream_seek(stream, -1, OStream_CUR);
        return true;
    }

    // error
    if (first_char == -1) {
        log_stream_failed();
        return false;
    }

    log_parse_stream_failed(stream, "invalid element start");
    return false;
}
