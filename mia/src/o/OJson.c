#include "o/OJson.h"
#include "o/OObj_builder.h"
#include "o/OStreamCache.h"
#include "o/OStreamArray.h"
#include "o/OStreamMem.h"
#include "o/file.h"
#include "o/str.h"
#include "o/array.h"
#include "o/OList.h"
#include <locale.h>
#include <stdlib.h>

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"

//
// public
//

OJson *OJson_init(oobj obj, oobj parent, const char *name, enum OJson_type type, union OJson_data data)
{
    OJson *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, OJson_ID);

    if (!name) {
        name = OJson_NAME_DEFAULT;
    }

    if (type == OJson_STRING) {
        // clone string (NULL safe)
        data.string = o_str_clone(self, data.string);
    } else if (type != OJson_BOOLEAN && type != OJson_NUMBER) {
        // data not used, so clearing it
        o_clear(&data, sizeof data, 1);
    }

    self->type = type;
    self->data = data;
    self->packed = false;

    if (OObj_check(parent, OJson)) {
        OJson *parent_json = parent;
        self->packed = parent_json->packed;
    }

    OObj_name_set(self, name);


//    const char *pname = o_or(OObj_name(parent), "");
//
//    switch (type) {
//        case OJson_NULL:
//            o_log("OJSON null: %s (%s)", name, pname);
//            break;
//        case OJson_BOOLEAN:
//            o_log("OJSON boolean {%i}: %s (%s)", self->data.boolean, name, pname);
//            break;
//        case OJson_NUMBER:
//            o_log("OJSON number {%g}: %s (%s)", self->data.number, name, pname);
//            break;
//        case OJson_STRING:
//            o_log("OJSON string {%s}: %s (%s)", self->data.string, name, pname);
//            break;
//        case OJson_OBJECT:
//            o_log("OJSON object: %s (%s)", name, pname);
//            break;
//        case OJson_ARRAY:
//            o_log("OJSON array: %s (%s)", name, pname);
//            break;
//        default:
//            break;
//    }

    // vfuncs
    self->super.v_op_num = OJson__v_op_num;
    self->super.v_op_at = OJson__v_op_at;

    return self;
}

struct oobj_opt OJson_new_read_stream(oobj parent, const char *name, oobj stream)
{
    setlocale(LC_ALL, "C");
    
    // OJson__parse_stream_r creates the base object into the given "parent".
    // but our parent may already have OJson objects, so its hard to detect and return
    // that's why we create the move_container, pop out the tree in that.
    // The move_container should than have exactly one OJson object (if valid)
    // that is moved into the parent and returned
    oobj move_container = OObj_new(parent);

    // the stream may be not seekable, so we create a buffered one
    stream = OStreamCache_new(move_container, stream, 32, false);


    // protected
    bool OJson__parse_stream_r(oobj, oobj, const char *, int);

    bool valid = OJson__parse_stream_r(move_container, stream, name, 0);
    if(!valid) {
        o_del(move_container);
        return oobj_opt(NULL);
    }
    // OJson_list would be invalid as move_container is not an OJson
    OJson **list = (OJson**) OObj_list(move_container, NULL, OJson);
    if(list[0] == NULL) {
        o_log_wtf_s("OJson_new_read_stream",
                   "valid, but got no OJson objects?");
        o_del(move_container);
        return oobj_opt(NULL);
    }
    if(list[1] != NULL) {
        o_log_warn_s("OJson_new_read_stream",
                   "valid, but got multiple OJson objects?");
    }

    OJson *ret = list[0];
    o_move(ret, parent);
    o_del(move_container);
    return oobj_opt(ret);
}

struct oobj_opt OJson_new_read_data(oobj parent, const char *name, const char *restrict data, osize data_num)
{
    setlocale(LC_ALL, "C");

    assert(data[data_num] == '\0' && "data must be null terminated");

    // OJson__parse_stream_r creates the base object into the given "parent".
    // but our parent may already have OJson objects, so its hard to detect and return
    // that's why we create the move_container, pop out the tree in that.
    // The move_container should than have exactly one OJson object (if valid)
    // that is moved into the parent and returned
    oobj move_container = OObj_new(parent);

    // protected
    bool OJson__parse_raw(oobj, const char *restrict, osize, const char *);

    bool valid = OJson__parse_raw(move_container, data, data_num, name);
    if(!valid) {
        o_del(move_container);
        return oobj_opt(NULL);
    }
    
    // OJson_list would be invalid as move_container is not an OJson
    OJson **list = (OJson**) OObj_list(move_container, NULL, OJson);
    if(list[0] == NULL) {
        o_log_wtf_s("OJson_new_read_data",
                   "valid, but got no OJson objects?");
        o_del(move_container);
        return oobj_opt(NULL);
    }
    if(list[1] != NULL) {
        o_log_warn_s("OJson_new_read_data",
                   "valid, but got multiple OJson objects?");
    }

    OJson *ret = list[0];
    o_move(ret, parent);
    o_del(move_container);
    return oobj_opt(ret);
}

struct oobj_opt OJson_new_read_file(oobj parent, const char *name, const char *file)
{
    struct oobj_opt data = o_file_read(parent, file, true, 1);
    if(!data.o) {
        o_log_warn_s("OJson_new_read_file",
                   "failed to open the file: %s", file);
        return oobj_opt(NULL);
    }
    struct oobj_opt res = OJson_new_read_data(parent, name, o_at(data.o, 0), o_num(data.o));
    o_del(data.o);
    return res;
}

//
// virtual implementations:
//

osize OJson__v_op_num(oobj obj)
{
    return OJson_num(obj);
}

void *OJson__v_op_at(oobj obj, osize idx)
{
    return OJson_at(obj, idx).o;
}

//
// object functions:
//


const char *OJson_string_set(oobj obj, const char *string)
{
    OObj_assert(obj, OJson);
    OJson *self = obj;
    if (self->type != OJson_STRING) {
        o_log_warn_s(__func__, "wrong type: %i", self->type);
        return NULL;
    }
    self->data.string = o_str_clone_realloc(self, self->data.string, string);
    return self->data.string;
}

void OJson_packed_set_tree(oobj obj, bool set, oi32 r_level)
{
    OObj_assert(obj, OJson);
    OJson *self = obj;
    self->packed = set;
    if(r_level<=0) {
        return;
    }
    r_level-=1;
    OJson **list = OJson_list(self, NULL);
    for(OJson **it = list; *it; it++) {
        OJson_packed_set_tree(*it, set, r_level);
    }
    o_free(self, list);
}

OJson **OJson_list(oobj obj, osize *opt_out_num)
{
    OObj_assert(obj, OJson);
    OJson *self = obj;
    osize tree_num;
    OJson **tree = (OJson **) OObj_list(obj, &tree_num, OJson);
    if(!self->list_external || OList_num(self->list_external) <= 0) {
        o_opt_set(opt_out_num, tree_num);
        return tree;
    }
    
    osize ext_num = OList_num(self->list_external);
    OJson **ext = (OJson **) OList_list(self->list_external);
    
    osize num = tree_num + ext_num;
    // concat lists, +1 for NULL termination
    OJson **list = o_new(self, OJson*, num+1);
    o_memcpy(list, tree, sizeof (OJson*), tree_num);
    o_memcpy(list+tree_num, ext, sizeof (OJson*), ext_num);
    list[num] = NULL;
    
    // dont forget to free the OObj_list
    o_free(self, tree);
    o_opt_set(opt_out_num, num);
    
    return list;
}

osize OJson_num(oobj obj)
{
    osize num;
    OJson **list = NULL;
    enum OJson_type type = OJson_type(obj);
    if(type != OJson_ARRAY && type != OJson_OBJECT) {
        return 0;
    }
    list = OJson_list(obj, &num);
    o_free(obj, list);
    return num;
}

struct oobj_opt OJson_at(oobj obj, osize idx)
{
    osize num;
    OJson **list = NULL;
    oobj res = NULL;
    enum OJson_type type = OJson_type(obj);
    if(type != OJson_ARRAY && type != OJson_OBJECT) {
        goto CLEAN_UP;
    }
    list = OJson_list(obj, &num);
    if(idx < 0 || idx>=num) {
        goto CLEAN_UP;
    }
    res = list[idx];

    CLEAN_UP:
    o_free(obj, list);
    return oobj_opt(res);
}

struct oobj_opt OJson_child(oobj obj, const char *name)
{
    assert(name);
    OJson **list = OJson_list(obj, NULL);
    OJson *ret = NULL;
    for(OJson **it = list; *it; it++) {
        OJson *child = *it;
        if (o_str_equals(OObj_name(child), name)) {
            ret = child;
        }
    }
    o_free(obj, list);
    return oobj_opt(ret);
}

osize OJson_write_stream(oobj obj, oobj stream)
{
    setlocale(LC_ALL, "C");
    OObj_assert(obj, OJson);
    OObj_assert(stream, OStream);
    oobj container = OObj_new(obj);

    // protected
    osize OJson__write_stream_r(oobj container, OJson *json, OStream *stream, int lvl);

    osize written = OJson__write_stream_r(container, obj, stream, 0);
    o_del(container);
    return written;
}

bool OJson_write_file(oobj obj, const char *file)
{
    osize written;
    char *dump = OJson_dump_ex(obj, &written, "\n");
    if(!dump) {
        o_log_error_s(__func__, "failed to dump json for file: %s", file);
        return false;
    }

    osize file_written = o_file_write(file, true, dump, 1, written);
    bool ok = file_written == written;
    if (!ok) {
        o_log_error_s(__func__, "failed to write to file: %s", file);
    }

    o_free(obj, dump);
    return ok;
}

char *OJson_dump_ex(oobj obj, osize *opt_out_len, const char *opt_end_string)
{
    setlocale(LC_ALL, "C");

    oobj container = OObj_new(obj);

    // 1 MB
    struct o_array array = o_array_new(container, 1024*1024);

    // protected
    void OJson__write_raw_r(oobj container, OJson *json, struct o_array *array, int lvl);

    OJson__write_raw_r(container, obj, &array, 0);

    osize written_json = array.num;
    
    if(opt_end_string) {
        o_array_append_string(&array, opt_end_string);
    }
    
    osize written = array.num;

    // add null terminator
    o_array_append_string_static(&array, "\0");


    char *result = NULL;
    if (written_json > 0) {
        result = o_array_move(&array, obj);
    }
    o_del(container);

    o_opt_set(opt_out_len, written);
    return result;
}

// protected for perf test in compersion with the new array version
char *OJson_dump__stream(oobj obj, osize *opt_out_len)
{
    oobj container = OObj_new(obj);
    oobj data = OArray_new_dyn(container, NULL, 1, 0, 1024);
    oobj stream = OStreamArray_new(container, data, false, OStreamArray_SEEKABLE);
    osize written = OJson_write_stream(obj, stream);
    char *result = NULL;
    if (written > 0) {
        result = OArray_data_void(data);
        o_data_move(container, obj, result);
    }
    o_del(container);

    o_opt_set(opt_out_len, written);
    return result;
}

//
// path api
//

struct oobj_opt OJson_path(oobj obj, const char *path)
{
    oobj current = obj;
    const char *start = path;
    struct o_array array = o_array_new(obj, 256);
    while (*start) {
        // skip slashes between objects, at begin or end (empty keys like {"": "empty_key"} are not allowed in the path
        while (*start == '/') {
            start++;
        }
        const char *it = start;
        osize it_len = 0;
        osize end = 0;
        for(;;) {
            it_len = o_strlen(it);
            if (it_len == 0) {
                return oobj_opt(current);
            }

            osize find = o_str_find_set(it, "\\/[");

            if(find<0) {
                o_array_append(&array, it, it_len);
                end += it_len;
                break;
            }
            if(find > 0) {
                o_array_append(&array, it, find);
            }

            char c = it[find];
            if(c == '\\') {
                find = o_min(it_len, find+2);
                end += find;
                it += find;
                continue;
            }
            end += find;
            break;
        }
        if (array.num == 0) {
            // array
            char *end_it;
            long idx = strtol(it+1, &end_it, 10);
            if (end_it < it+1 || *end_it != ']') {
                o_log_debug_s(__func__, "array parsing failed for path: %s", path);
                return oobj_opt(NULL);
            }
            oobj at = OJson_at(current, idx).o;
            if (!at) {
                o_log_debug_s(__func__, "array index failed for path: %s, index: %i", path, (int) idx);
                return oobj_opt(NULL);
            }
            current = at;
            start = end_it+1;
            continue;
        }
        o_array_append_string_static(&array, "\0");
        char *key = (char *) array.data;
        oobj child = OJson_child(current, key).o;
        if (!child) {
            o_log_debug_s(__func__, "object parsing failed for path: %s, key: %s", path, key);
            return oobj_opt(NULL);
        }
        o_array_resize(&array, 0);
        current = child;
        start = it+end;
    }

    return oobj_opt(current);
}