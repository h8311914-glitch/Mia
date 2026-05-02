#include "o/OJson.h"
#include "o/file.h"
#include "o/str.h"
#include "o/array.h"
#include "o/timer.h"
#include <stdlib.h>
#include <ctype.h>

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"

#define INDENT_LEVEL "  "

#define LOG_PREV 16
#define LOG_NEXT 8

#define R_MAX_DEPTH 32


// protected
void OJson__write_raw_r(oobj container, OJson *json, struct o_array *array, int lvl);

// protected
bool OJson__parse_raw(oobj parent, const char *restrict data, osize data_num, const char *name);

//
// writing
//

#define arr_str o_array_append_string
#define arr_str_static o_array_append_string_static

static void write_raw_newline(struct o_array *array, int lvl)
{
    arr_str_static(array, "\n");
    for (int i = 0; i < lvl; i++) {
        arr_str_static(array, INDENT_LEVEL);
    }
}

static void write_raw_object(oobj container, OJson *json_object, struct o_array *array, int lvl)
{
    OJson **list = OJson_list(json_object, NULL);

    // not a single child found
    if (*list == NULL) {
        arr_str_static(array, "{}");
        o_free(json_object, list);

        return;
    }

    arr_str_static(array, "{");
    OJson **it = list;
    while (*it) {
        OJson *child = *it++;
        if (!json_object->packed) {
            write_raw_newline(array, lvl + 1);
        }
        if (OObj_name(child)) {
            char *escaped = o_str_escape(container, OObj_name(child));
            arr_str_static(array, "\"");
            arr_str(array, escaped);
            arr_str_static(array, "\":");
            o_free(container, escaped);
        } else {
            arr_str_static(array, "\"" OJson_NAME_DEFAULT "\": ");
        }
        OJson__write_raw_r(container, child, array, lvl + 1);
        if (*it != NULL) {
            arr_str_static(array, ",");
        }
    }

    if (!json_object->packed) {
        write_raw_newline(array, lvl);
    }
    arr_str_static(array, "}");

    o_free(json_object, list);
}

static void write_raw_array(oobj container, OJson *json_array, struct o_array *array, int lvl)
{
    OJson **list = OJson_list(json_array, NULL);

    // not a single child found, replacing with null
    if (*list == NULL) {
        arr_str_static(array, "[]");
        o_free(json_array, list);
        return;
    }

    arr_str_static(array, "[");
    OJson **it = list;
    while (*it) {
        OJson *child = *it++;
        if (!json_array->packed) {
            write_raw_newline(array, lvl + 1);
        }
        OJson__write_raw_r(container, child, array, lvl + 1);
        if (*it != NULL) {
            arr_str_static(array, ",");
        }
    }

    if (!json_array->packed) {
        write_raw_newline(array, lvl);
    }
    arr_str_static(array, "]");

    o_free(json_array, list);
}

// recursive function, returns the num written chars
void OJson__write_raw_r(oobj container, OJson *json, struct o_array *array, int lvl)
{
    char buffer[64];
    osize size;
    char *escaped;
    switch (json->type) {
        case OJson_NULL:
            arr_str_static(array, "null");
            return;
        case OJson_BOOLEAN:
            if (json->data.boolean) {
                arr_str_static(array, "true");
            } else {
                arr_str_static(array, "false");
            }
            return;
        case OJson_NUMBER:
            size = o_strf_buf(buffer, "%g", json->data.number);
            o_array_append(array, (obyte *) buffer, size);
            return;
        case OJson_STRING:
            escaped = o_str_escape(container, json->data.string);
            arr_str_static(array, "\"");
            arr_str(array, escaped);
            arr_str_static(array, "\"");
            o_free(container, escaped);
            return;
        case OJson_OBJECT:
            write_raw_object(container, json, array, lvl);
            return;
        case OJson_ARRAY:
            write_raw_array(container, json, array, lvl);
            return;
        default:
            o_log_warn_s("OJson_write_raw", "invalid type: %i", (int) json->type);
    }
}


#undef arr_str
#undef arr_str_static


//
// parsing
//     Tried A LOT of techniques to read from raw memory.
//     I once came close to it, was nearly like the OStream version, but without vfuncs.
//     But was slower on CxxDroid's clang.
//     This is really crazy. Tried a A LOT :D but something is bad for the caching, fascinating in its own :D
//


// data here is null terminated anyway , so *end=='\0'
struct parse {
    const char *head;
    const char *cur;
    const char *end;
    struct o_array strbuf;
    int depth;
};


static obyte json_obyte_from_hex(const char *hex_byte_chars)
{
    if (!hex_byte_chars || hex_byte_chars[0] == '\0' || hex_byte_chars[1] == '\0')
        return 0;
    char hex[3] = {hex_byte_chars[0], hex_byte_chars[1], '\0'};
    return (obyte) strtoul(hex, NULL, 16);
}

static void log_parse_error(oobj parent, struct parse parse, const char *cur, const char *msg) 
{
    oobj c = OObj_new(parent);
    const char *pre = o_max(parse.head, cur - 12);
    const char *post = o_min(parse.end, cur + 12);
    osize post_len = post - cur;
    
    char *pre_str = o_str_clone_sub(c, pre, 0, 12);
    char *post_str = o_str_clone_sub(c, cur, 0, post_len);
    
    
    int pos = parse.cur - parse.head;
    int line;
    {
        char *clone = o_str_clone_sub(c, parse.head, 0, pos);
        line = o_str_count(clone, "\n");
    }
    o_log_error_s("OJson_read_raw", "%s: [%i;%i] %s >>> \'%c\' <<< %s",
                  msg, pos, line, pre_str, *cur, post_str);
    
    o_del(c);
}


static struct parse json_read_null(oobj parent, const char *name, struct parse parse)
{
    // *cur == 'n'
    const char *restrict cur = parse.cur;
    const char *end = parse.end;
    if (o_unlikely(cur+4>end)) {
        log_parse_error(parent, parse, cur, "null failed, end reached");
        return (struct parse) {0};
    }
    cur++;
    if (o_unlikely(!o_equals(cur, "ull", 1, 3))) {
        log_parse_error(parent, parse, cur, "null failed to parse");
        return (struct parse) {0};
    }
    OJson_new_null(parent, name);
    cur += 3;
    parse.cur = cur;
    return parse;
}

static struct parse json_read_boolean_true(oobj parent, const char *name, struct parse parse)
{
    // *cur == 't'
    const char *restrict cur = parse.cur;
    const char *end = parse.end;
    if (o_unlikely(cur+4>end)) {
        log_parse_error(parent, parse, cur, "true failed, end reached");
        return (struct parse) {0};
    }
    cur++;
    if (o_unlikely(!o_equals(cur, "rue", 1, 3))) {
        log_parse_error(parent, parse, cur, "true failed to parse");
        return (struct parse) {0};
    }
    OJson_new_boolean(parent, name, true);
    cur += 3;
    parse.cur = cur;
    return parse;
}


static struct parse json_read_boolean_false(oobj parent, const char *name, struct parse parse)
{
    // *cur == 'f'
    const char *restrict cur = parse.cur;
    const char *end = parse.end;
    if (o_unlikely(cur+5>end)) {
        log_parse_error(parent, parse, cur, "false failed, end reached");
        return (struct parse) {0};
    }
    cur++;
    if (o_unlikely(!o_equals(cur, "alse", 1, 3))) {
        log_parse_error(parent, parse, cur, "false failed to parse");
        return (struct parse) {0};
    }
    OJson_new_boolean(parent, name, false);
    cur += 4;
    parse.cur = cur;
    return parse;
}


static struct parse json_read_number(oobj parent, const char *name, struct parse parse)
{
    const char *restrict cur = parse.cur;
    char *num_end;
    double number = strtod(cur, &num_end);
    if (o_unlikely(num_end <= cur)) {
        log_parse_error(parent, parse, cur, "number failed to parse");
        return (struct parse) {0};
    }
    OJson_new_number(parent, name, number);
    parse.cur = num_end;
    return parse;
}


// resulting string is in strbuf (null terminated)
static struct parse json_next_string(oobj parent, struct parse parse)
{
    // *cur == '\"'
    o_array_resize(&parse.strbuf, 0);
    const char *restrict cur = parse.cur;
    cur++;
    const char *begin = cur;
    for (;;) {
        if (o_unlikely(cur >= parse.end)) {
            log_parse_error(parent, parse, cur, "text failed, end reached");
            return (struct parse) {0};
        }
        if (*cur == '\\') {
            osize len = cur - begin;
            if (len > 0) {
                o_array_append(&parse.strbuf, begin, len);
            }
            cur++;
            switch (*cur) {
                case '\'':
                    o_array_append_string_static(&parse.strbuf, "\'");
                break;
                case '\"':
                    o_array_append_string_static(&parse.strbuf, "\"");
                break;
                case '\\':
                    o_array_append_string_static(&parse.strbuf, "\\");
                break;
                case 'n':
                    o_array_append_string_static(&parse.strbuf, "\n");
                break;
                case 'r':
                    o_array_append_string_static(&parse.strbuf, "\r");
                break;
                case 't':
                    o_array_append_string_static(&parse.strbuf, "\t");
                break;
                case 'b':
                    o_array_append_string_static(&parse.strbuf, "\b");
                break;
                case 'f':
                    o_array_append_string_static(&parse.strbuf, "\f");
                break;
                case 'x': {
                    obyte hex = json_obyte_from_hex(cur+1);
                    o_array_append(&parse.strbuf, &hex, 1);
                    cur += 2;
                    break;
                }
                default:
                    // could be '\0' for string end as an example
                    return (struct parse) {0};
            }
            cur++;
            begin = cur;
            continue;
        }
        if (*cur == '\"') {
            osize len = cur - begin;
            if (len > 0) {
                o_array_append(&parse.strbuf, begin, len);
            }
            // null terminator
            o_array_append_string_static(&parse.strbuf, "\0");
            cur++;
            parse.cur = cur;
            return parse;
        }
        cur++;
    }

    // shouldn't get here
    return (struct parse) {0};
}

static struct parse json_read_string(oobj parent, const char *name, struct parse parse, struct o_array *strbuf)
{
    // *cur == '\"'
    parse = json_next_string(parent, parse);
    if (o_unlikely(parse.head == NULL)) {
        log_parse_error(parent, parse, parse.cur, "string failed to parse");
        return parse;
    }
    OJson_new_string(parent, name, (const char *) strbuf->data);
    return parse;
}

// forward
static struct parse json_read_r(oobj parent, const char *name, struct parse parse);

static struct parse json_read_object(oobj parent, const char *name, struct parse parse)
{
    // *cur == '{'
    oobj object_element = OJson_new_object(parent, name);
    const char *restrict cur = parse.cur;
    int depth = parse.depth;
    cur++;
    for (;;) {
        // ignore whitespace
        while (isspace(*cur)) {
            cur++;
        }
        if (o_unlikely(cur >= parse.end)) {
            log_parse_error(parent, parse, cur, "object failed, end reached");
            return (struct parse) {0};
        }

        if (*cur == '}') {
            // object closed
            parse.cur = cur+1;
            return parse;
        }

        if (o_unlikely(*cur != '\"')) {
            log_parse_error(parent, parse, cur, "object failed, expected key begin");
            return (struct parse) {0};
        }

        parse.cur = cur;
        parse.depth = depth;
        parse = json_next_string(parent, parse);
        if (o_unlikely(parse.head == NULL)) {
            log_parse_error(parent, parse, cur, "object failed");
            return (struct parse) {0};
        }
        char *child_name = o_new_clone(object_element, parse.strbuf.data, char, parse.strbuf.num);
        cur = parse.cur;

        // ignore whitespace
        while (isspace(*cur)) {
            cur++;
        }
        if (o_unlikely(*cur != ':' || cur+2>=parse.end)) {
            log_parse_error(parent, parse, cur, "object failed expected ':' ");
            o_free(object_element, child_name);
            return (struct parse) {0};
        }
        cur++;

        parse.cur = cur;
        parse = json_read_r(object_element, child_name, parse);
        o_free(object_element, child_name);
        if (o_unlikely(parse.head == NULL)) {
            //log_parse_error(parent, parse, cur, "object failed to parse child");
            return (struct parse) {0};
        }
        cur = parse.cur;

        // ignore whitespace
        while (isspace(*cur)) {
            cur++;
        }

        // ',' or '}'
        if (*cur == '}') {
            // object closed
            parse.cur = cur+1;
            return parse;
        }

        if (o_unlikely(*cur != ',')) {
            log_parse_error(parent, parse, cur, "object failed expected '}' or ',' ");
            return (struct parse) {0};
        }
        cur++;
    }

    // should not get here
    return (struct parse) {0};
}

static struct parse json_read_array(oobj parent, const char *name, struct parse parse)
{
    // *cur == '['
    oobj object_element = OJson_new_array(parent, name);
    const char *restrict cur = parse.cur;
    int depth = parse.depth;
    cur++;
    for (;;) {
        // ignore whitespace
        while (isspace(*cur)) {
            cur++;
        }
        if (o_unlikely(cur >= parse.end)) {
            log_parse_error(parent, parse, cur, "array failed, end reached");
            return (struct parse) {0};
        }

        if (*cur == ']') {
            // array closed
            parse.cur = cur+1;
            return parse;
        }

        parse.cur = cur;
        parse.depth = depth;
        parse = json_read_r(object_element, NULL, parse);
        if (o_unlikely(parse.head == NULL)) {
            // log_parse_error(parent, parse, cur, "array failed to parse child");
            return (struct parse) {0};
        }
        cur = parse.cur;

        // ignore whitespace
        while (isspace(*cur)) {
            cur++;
        }

        // ',' or ']'
        if (*cur == ']') {
            // array closed
            parse.cur = cur+1;
            return parse;
        }

        if (o_unlikely(*cur != ',')) {
            log_parse_error(parent, parse, cur, "array failed, expected ']' or ',' ");
            return (struct parse) {0};
        }
        cur++;
    }

    // should not get here
    return (struct parse) {0};
}

static struct parse json_read_r(oobj parent, const char *name, struct parse parse)
{
    parse.depth++;
    if (o_unlikely(parse.depth > R_MAX_DEPTH)) {
        log_parse_error(parent, parse, parse.cur, "recursion failed, max depth reached");
        return (struct parse) {0};
    }

    const char *restrict cur = parse.cur;
    // ignore whitespace
    while (isspace(*cur)) {
        cur++;
    }
    if (o_unlikely(cur >= parse.end)) {
        log_parse_error(parent, parse, cur, "parse failed, end reached");
        return (struct parse) {0};
    }

    parse.cur = cur;
    switch (*cur) {
        case 'n':
            parse = json_read_null(parent, name, parse);
            break;
        case 't':
            parse = json_read_boolean_true(parent, name, parse);
            break;
        case 'f':
            parse = json_read_boolean_false(parent, name, parse);
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '+':
        case '-':
        case '.':
            parse = json_read_number(parent, name, parse);
            break;
        case '\"':
            parse = json_read_string(parent, name, parse, &parse.strbuf);
            break;
        case '{':
            parse = json_read_object(parent, name, parse);
            break;
        case '}':
            break;
        case '[':
            parse = json_read_array(parent, name, parse);
            break;
        case ']':
            break;
        default:
            log_parse_error(parent, parse, cur, "parse failed, invalid character");
            return (struct parse) {0};
    }
    return parse;
}


// protected
bool OJson__parse_raw(oobj parent, const char *restrict data, osize data_num, const char *name)
{
    struct parse parse;
    parse.head = parse.cur = data;
    parse.end = parse.head + data_num;
    parse.strbuf = o_array_new(parent, 1024);
    parse.depth = 0;

    parse = json_read_r(parent, name, parse);

    o_array_free(&parse.strbuf);
    return parse.head != NULL;
}
