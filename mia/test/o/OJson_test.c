#include "o/OJson.h"
#include "o/OWalkjson.h"

#include "o/str.h"
#include "o/OStreamArray.h"
#include "m/sca/flt.h"
#include "m/vec/vecn.h"
#include "o/log.h"

#define test(expr) o_assume(expr, "test failed")


static oobj json_tree_new(oobj parent)
{
    oobj j_root = OJson_new_object(parent, NULL);
    OJson_new_null(j_root, "null");
    OJson_new_boolean(j_root, "boolean", true);
    OJson_new_number(j_root, "number", 1.23);
    OJson_new_string(j_root, "string", "Hello, World!");
    oobj j_array = OJson_new_array(j_root, "array");
    for (int i = 0; i < 10; i++) {
        OJson_new_number(j_array, NULL, i);
    }
    OJson_new_array(j_root, "array_empty");
    OJson_new_object(j_root, "object_empty");
    
    OJson_new_string(j_root, "es\"cape", "Hello, \"World!\"");
    
    oobj j_path = OJson_new_object(OJson_new_array(OJson_new_object(j_root, "path"), "to"), NULL);
    OJson_new_boolean(j_path, "boolean", false);
    OJson_new_number(j_path, "number", 4.56);
    OJson_new_string(j_path, "string", "PATH");
    
    return j_root;
}

static void json_tree_test(oobj json)
{
    test(json);
    test(OJson_num(json) == 9);
    test(o_num(json) == 9);
    
    oobj j_null = OJson_child(json, "null").o;
    test(j_null);
    test(OJson_type(j_null) == OJson_NULL);
    
    oobj j_bool = OJson_child(json, "boolean").o;
    test(j_bool);
    bool *j_bool_val = OJson_boolean(j_bool);
    test(j_bool_val);
    test(*j_bool_val == true);
    
    oobj j_num = OJson_child(json, "number").o;
    test(j_num);
    double *j_num_val = OJson_number(j_num);
    test(j_num_val);
    test(m_equals_eps(*j_num_val, 1.23, 0.000001));
    
    oobj j_str = OJson_child(json, "string").o;
    test(j_str);
    const char *j_str_val = OJson_string(j_str);
    test(j_str_val);
    test(o_str_equals(j_str_val, "Hello, World!"));
    
    oobj j_arr = OJson_child(json, "array").o;
    test(j_arr);
    test(OJson_type(j_arr) == OJson_ARRAY);
    test(OJson_num(j_arr) == 10);
    test(o_num(j_arr) == 10);
    for (int i = 0; i < 10; i++) {
        oobj j_arr_i = OJson_at(j_arr, i).o;
        test(j_arr_i);
        oobj j_arr_i_op = o_at(j_arr, i);
        test(j_arr_i == j_arr_i_op);
        double *j_arr_i_val = OJson_number(j_arr_i);
        test(j_arr_i_val);
        test(m_equals_eps(*j_arr_i_val, i, 0.000001));
    }
    
    oobj j_arr_empty = OJson_child(json, "array_empty").o;
    test(j_arr_empty);
    test(OJson_type(j_arr_empty) == OJson_ARRAY);
    test(OJson_num(j_arr_empty) == 0);

    oobj j_obj_empty = OJson_child(json, "object_empty").o;
    test(j_obj_empty);
    test(OJson_type(j_obj_empty) == OJson_OBJECT);
    test(OJson_num(j_obj_empty) == 0);
    
    oobj j_esc = OJson_child(json, "es\"cape").o;
    test(j_esc);
    const char *j_esc_val = OJson_string(j_esc);
    test(j_esc_val);
    test(o_str_equals(j_esc_val, "Hello, \"World!\""));

    // Path stuff
    
    oobj j_path = OJson_path(json, "path/to[0]").o;
    test(j_path);
    test(OJson_type(j_path) == OJson_OBJECT);
    
    bool *j_path_bool_val = OJson_path_boolean(json, "path/to[0]boolean");
    test(j_path_bool_val);
    test(*j_path_bool_val == false);
    
    double *j_path_num_val = OJson_path_number(json, "path/to[0]number");
    test(j_path_num_val);
    test(m_equals_eps(*j_path_num_val, 4.56, 0.000001));

    const char *j_path_str_val = OJson_path_string(json, "path/to[0]string");
    test(j_path_str_val);
    test(o_str_equals(j_path_str_val, "PATH"));
    const char *j_path_str_val2 = OJson_path_string(json, "//path/////to////[0]//string///");
    test(j_path_str_val2 && j_path_str_val2 == j_path_str_val);

    for (int i = 0; i < 10; i++) {
        char path_buf[32];
        o_strf_buf(path_buf, "array[%i]", i);
        double *j_arr_i_val = OJson_path_number(json, path_buf);
        test(j_arr_i_val);
        test(m_equals_eps(*j_arr_i_val, i, 0.000001));
    }
    
    // not available path should return NULL
    // will log a debug msg, we'll mute it
    o_log_quiet_block {
        oobj invalid_path = OJson_path(json, "invalid/path/to/key[0]").o;
        test(!invalid_path);
    }
}

static void json_tree_walker(oobj json)
{
    // create OWalkjson to walk over the json, if smth is missing, valid will get false
    // so collect and single test for valid
    oobj walker = OWalkjson_new(json);
    bool walker_bool = OWalkjson_boolean(walker, "boolean");
    double walker_num = OWalkjson_number(walker, "number");
    const char *walker_str = OWalkjson_string(walker, "string");
    double walker_array[10];
    OWalkjson_vector(walker, "array", walker_array, 10);
    const char *walker_foo = OWalkjson_string(walker, "//path/////to////[0]//string///");

    test(OWalkjson_valid(walker));

    // test if data is correct, too
    test(walker_bool == true);
    test(m_equals_eps(walker_num, 1.23, 0.000001));
    test(o_str_equals(walker_str, "Hello, World!"));
    for (int i = 0; i < 10; i++) {
        test(m_equals_eps(walker_array[i], i, 0.000001));
    }
    test(o_str_equals(walker_foo, "PATH"));

    // not available path, will log a warning and debug msg, we'll mute it
    o_log_quiet_block {
        oobj invalid_path = OWalkjson_get(walker, "invalid/path/to/key[0]");
        test(!OWalkjson_valid(walker));
        test(!invalid_path);
    }
}

static oobj json_tree_recreate_raw(oobj parent, oobj json)
{
    char *dump = OJson_dump(json, NULL);
    json = OJson_new_read_string(parent, NULL, dump).o;
    test(json);
    return json;   
}

static oobj json_tree_recreate_stream(oobj parent, oobj json)
{
    // currently around 375 chars needed
    oobj array = OArray_new_dyn(parent, NULL, 1, 0, 512);
    oobj stream = OStreamArray_new(parent, array, false, OStreamArray_SEEKABLE);
    bool valid = OJson_write_stream(json, stream);
    test(valid);
    
    //o_log("stream num: %i", OArray_num(array));
    
    OStream_seek(stream, 0, OStream_SET);
    json = OJson_new_read_stream(parent, NULL, stream).o;
    test(json);
    return json;   
}

int OJson__test(oobj obj)
{
    oobj json = json_tree_new(obj);
    json_tree_test(json);
    json_tree_walker(json);
        
    oobj json_raw = json_tree_recreate_raw(obj, json);
    json_tree_test(json_raw);
    
    oobj json_stream = json_tree_recreate_stream(obj, json);
    json_tree_test(json_stream);

    return 0;
}

