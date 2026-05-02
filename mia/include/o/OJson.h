#ifndef O_OJSON_H
#define O_OJSON_H

/**
 * @file OJson.h
 *
 * Object
 *
 * A Json object holds the infos for a single json element.
 * The Json Api makes use of the OObj system,
 * in which a Json objects or arrays children are just allocated by it as parent.
 *     In additional to an optional external provided list.
 *
 * The name for object children will be the OObj_name
 *
 * CAUTION: name duplicates can easily occur and are NOT checked!
 *
 * Looking for a more convenient way to parse static json fields?
 * @sa OJsonwalker.h
 *
 * Operators:
 * o_num -> OJson_num
 * o_at -> OJson_at
 */


#include "OObj.h"


/** object id */
#define OJson_ID OObj_ID "OJson."

/** if name==NULL, this will be used */
#define OJson_NAME_DEFAULT "undefined"


enum OJson_type {
    OJson_NULL,
    OJson_BOOLEAN,
    OJson_NUMBER,
    OJson_STRING,
    OJson_OBJECT,
    OJson_ARRAY,
    OJson_ENUM_MAX
};

union OJson_data {
    // null needs no data
    bool boolean;
    double number;
    char *string;
    // object member stored in the resource list
    // array member stored in the resource list
};

typedef struct {
    OObj super;

    enum OJson_type type;
    union OJson_data data;

    // inherit from the parent, set with OJson_packed_set
    bool packed;
    
    // OList of additional external OJson children
    // use with caution, does not check lifetime
    oobj list_external;
} OJson;


/**
 * Initializes the object.
 * If the parent is an OJson object or array, this will get an element in that list (uses the resources list).
 * Also inherits the flag "packed" from the parent.
 * @param obj OJson object
 * @param parent to inherit from (may be a json object or array)
 * @param name if the parent is an object, NULL safe
 * @param type of this OJson element
 * @param data according to type
 *             if type == OJson_STRING, data.string will get cloned
 *             if not used it gets cleared
 * @return obj casted as OJson
 */
OJson *OJson_init(oobj obj, oobj parent, const char *name, enum OJson_type type, union OJson_data data);

/**
 * Creates a new OJson object
 * If the parent is an OJson object or array, this will get an element in that list (uses the resources list).
 * Also inherits the flag "packed" from the parent.
 * @param parent to inherit from (may be a json object or array)
 * @param name if the parent is an object, NULL safe
 * @param type of this OJson element
 * @param data according to type
 *             if type == OJson_STRING, data.string will get cloned
 *             if not used it gets cleared
 * @return The new object
 */
o_inline OJson *OJson_new(oobj parent, const char *name, enum OJson_type type, union OJson_data data)
{
    OObj_DECL_IMPL_NEW(OJson, parent, name, type, data);
}


/**
 * Creates a new OJson object
 * If the parent is an OJson object or array, this will get an element in that list (uses the resources list).
 * Also inherits the flag "packed" from the parent.
 * @param parent to inherit from (may be a json object or array)
 * @param name if the parent is an object, NULL safe
 * @param stream OStream object to read a json ascii name from
 * @return The new object, or NULL if parsing failed
 * @warning MAY RETURN NULL
 */
struct oobj_opt OJson_new_read_stream(oobj parent, const char *name, oobj stream);

/**
 * Creates a new OJson object
 * If the parent is an OJson object or array, this will get an element in that list (uses the resources list).
 * Also inherits the flag "packed" from the parent.
 * @param parent to inherit from (may be a json object or array)
 * @param name if the parent is an object, NULL safe
 * @param data, data_num dumped json string
 * @return The new object, or NULL if parsing failed
 * @warning MAY RETURN NULL
 */
struct oobj_opt OJson_new_read_data(oobj parent, const char *name, const char *restrict data, osize data_num);

/**
 * Creates a new OJson object
 * If the parent is an OJson object or array, this will get an element in that list (uses the resources list).
 * Also inherits the flag "packed" from the parent.
 * @param parent to inherit from (may be a json object or array)
 * @param name if the parent is an object, NULL safe
 * @param file filepath to an .json name to read in
 * @return The new object, or NULL if parsing failed
 * @note First reads in the whole file with o_file_read, then uses OJson_new_read_stream with an OStreamArray.
 *       For XXL json files, use OJson_new_read_stream instead with an OStream of o_file_open
 * @warning MAY RETURN NULL
 */
struct oobj_opt OJson_new_read_file(oobj parent, const char *name, const char *file);

/**
 * Creates a new OJson object
 * If the parent is an OJson object or array, this will get an element in that list (uses the resources list).
 * Also inherits the flag "packed" from the parent.
 * @param parent to inherit from (may be a json object or array)
 * @param name if the parent is an object, NULL safe
 * @param string dumped json string
 * @return The new object, or NULL if parsing failed
 * @warning MAY RETURN NULL
 */
o_inline struct oobj_opt OJson_new_read_string(oobj parent, const char *name, const char *string)
{
    return OJson_new_read_data(parent, name, string, o_strlen(string));
}


//
// type specific initializers
//


/**
 * Creates a new OJson object as OJson_BOOLEAN element
 * If the parent is an OJson object or array, this will get an element in that list (uses the resources list).
 * Also inherits the flag "packed" from the parent.
 * @param parent to inherit from (may be a json object or array)
 * @param name if the parent is an object, NULL safe
 * @return The new object
 */
o_inline OJson *OJson_new_null(oobj parent, const char *name)
{
    union OJson_data data = {0};
    return OJson_new(parent, name, OJson_NULL, data);
}

/**
 * Creates a new OJson object as OJson_BOOLEAN element
 * If the parent is an OJson object or array, this will get an element in that list (uses the resources list).
 * Also inherits the flag "packed" from the parent.
 * @param parent to inherit from (may be a json object or array)
 * @param name if the parent is an object, NULL safe
 * @param boolean the bool value
 * @return The new object
 */
o_inline OJson *OJson_new_boolean(oobj parent, const char *name, bool boolean)
{
    union OJson_data data = {0};
    data.boolean = boolean;
    return OJson_new(parent, name, OJson_BOOLEAN, data);
}


/**
 * Creates a new OJson object as OJson_NUMBER element
 * If the parent is an OJson object or array, this will get an element in that list (uses the resources list).
 * Also inherits the flag "packed" from the parent.
 * @param parent to inherit from (may be a json object or array)
 * @param name if the parent is an object, NULL safe
 * @param number the double value
 * @return The new object
 */
o_inline OJson *OJson_new_number(oobj parent, const char *name, double number)
{
    union OJson_data data = {0};
    data.number = number;
    return OJson_new(parent, name, OJson_NUMBER, data);
}


/**
 * Creates a new OJson object as OJson_STRING element
 * If the parent is an OJson object or array, this will get an element in that list (uses the resources list).
 * Also inherits the flag "packed" from the parent.
 * @param parent to inherit from (may be a json object or array)
 * @param name if the parent is an object, NULL safe
 * @param string null terminated and NULL safe (results in element OJson_NULL)
 * @return The new object
 * @warning THIS DOES NOT PARSE A JSON DUMPED STRING, USE OJson_new_read_string FOR THAT
 */
o_inline OJson *OJson_new_string(oobj parent, const char *name, const char *string)
{
    union OJson_data data = {0};
    // will get cloned in init
    data.string = (char*) string;
    return OJson_new(parent, name, OJson_STRING, data);
}

/**
 * Creates a new OJson object as OJson_NUMBER element
 * If the parent is an OJson object or array, this will get an element in that list (uses the resources list).
 * Also inherits the flag "packed" from the parent.
 * @param parent to inherit from (may be a json object or array)
 * @param name if the parent is an object, NULL safe
 * @return The new object
 */
o_inline OJson *OJson_new_object(oobj parent, const char *name)
{
    union OJson_data data = {0};
    return OJson_new(parent, name, OJson_OBJECT, data);
}

/**
 * Creates a new OJson object as OJson_ARRAY element
 * If the parent is an OJson object or array, this will get an element in that list (uses the resources list).
 * Also inherits the flag "packed" from the parent.
 * @param parent to inherit from (may be a json object or array)
 * @param name if the parent is an object, NULL safe
 * @return The new object
 */
o_inline OJson *OJson_new_array(oobj parent, const char *name)
{
    union OJson_data data = {0};
    return OJson_new(parent, name, OJson_ARRAY, data);
}

//
// virtual implementations:
//

/**
 * virtual operator function
 * @param obj OJson object
 * @return the number of children if obj type is OJson_OBJECT or OJson_ARRAY
 * @note calls OJson_num.
 */
osize OJson__v_op_num(oobj obj);

/**
 * virtual operator function
 * @param obj OJson object
 * @param idx element index
 * @return void * (oobj) of the OJson_OBJECT or OJson_ARRAY indexed child (or NULL if failed)
 * @note calls OJson_at.
 *       Fastest way to iterate is OJson_list traversal!
 */
void *OJson__v_op_at(oobj obj, osize idx);

//
// object functions:
//

/**
 * @param obj OJson object
 * @return the type of this json element
 */
OObj_DECL_GET(OJson, enum OJson_type, type)

/**
 * @param obj OJson object
 * @return the data union of this json element
 */
OObj_DECL_GET(OJson, union OJson_data, data)

/**
 * @param obj OJson object
 * @return a pointer to the OJson boolean data, or NULL if type!=OJson_BOOLEAN
 */
o_inline bool *OJson_boolean(oobj obj)
{
    OObj_assert(obj, OJson);
    OJson *self = obj;
    if(self->type != OJson_BOOLEAN) {
        return NULL;
    }
    return &self->data.boolean;
}

/**
 * @param obj OJson object
 * @return a pointer to the OJson number data, or NULL if type!=OJson_NUMBER
 */
o_inline double *OJson_number(oobj obj)
{
    OObj_assert(obj, OJson);
    OJson *self = obj;
    if(self->type != OJson_NUMBER) {
        return NULL;
    }
    return &self->data.number;
}

/**
 * @param obj OJson object
 * @return the string of the OJson, or NULL if type!=OJson_STRING
 */
o_inline const char *OJson_string(oobj obj)
{
    OObj_assert(obj, OJson);
    OJson *self = obj;
    if(self->type != OJson_STRING) {
        return NULL;
    }
    return self->data.string;
}

/**
 * @param obj OJson object
 * @string the string for the OJson element
 * @return the string of the OJson, or NULL if type!=OJson_STRING
 * @note does an o_log_warn if type!=OJson_STRING
 */
const char *OJson_string_set(oobj obj, const char *string);


/**
 * @param obj OJson object
 * @return true if the element should be written in packed mode (no newlines)
 */
OObj_DECL_GETSET(OJson, bool, packed)

/**
 * Calls OJson_packed_set in recursion for all OJson children.
 * @param obj OJson object
 * @param true if the element should be written in packed mode (no newlines).
 * @param r_level: <=0 to only search in the given objects children list, without recursion. 
                   else maximal recursion level, use oi32_MAX for all
 */
void OJson_packed_set_tree(oobj obj, bool set, oi32 r_level);



/**
 * Get the children of a OJson_OBJECT or OJson_ARRAY.
 * Combines OObj_list (default) and OJson_list_external.
 * @param obj OJson object
 * @param opt_out_num if not NULL, returns the list size/num
 * @return A null terminated list of the OJson children, call o_free(obj, list) to free it manually
 * @note the list is allocated on obj.
 *       Create an OJson **it=list to traverse through the list
 */
OJson **OJson_list(oobj obj, osize *opt_out_num);

/**
 * @param obj OJson object
 * @return OList of additional external OJson children
 * @note use with caution, does not check lifetime,
 *       nor does it check OJson as children!
 *       use case: An app wants to keep the OJson objects but needs to resort an OJson array / object
 * @warning USE WITH CAUTION!
 *          Must be an OList of OJson!
 *          Does NOT check lifetimes!
 */
OObj_DECL_GETSET(OJson, oobj, list_external)


/**
 * Gets the array size of an OJson_OBJECT or OJson_ARRAY
 * @param obj OJson object
 * @return number of elements in the object or array (0 on type missmatch)
 */
osize OJson_num(oobj obj);

/**
 * Get an indexed child of an OJson_OBJECT or OJson_ARRAY
 * @param obj OJson object
 * @param idx in the OJson_list
 * @return the OJson object with that index or NULL if not in range or wrong type
 * @note Fastest way to iterate is OJson_list traversal!
 */
struct oobj_opt OJson_at(oobj obj, osize idx);

/**
 * Get a specific child of an OJson_OBJECT
 * @param obj OJson object
 * @param name the name to search
 * @return the OJson object with that name or NULL if not found
 * @note asserts name!=NULL
 */
struct oobj_opt OJson_child(oobj obj, const char *name);

/**
 * writes the json element tree into the given stream
 * @param obj OJson object
 * @param stream OStream object
 * @return the number of written characters
 */
osize OJson_write_stream(oobj obj, oobj stream);

/**
 * writes the json element tree into the given name
 * @param obj OJson object
 * @param file filepath
 * @return true if succeeded
 * @note first uses OJson_dump and then writes the whole string to file (o_file_write).
 *       For XXL json files, use OJson_write_stream with an file OStream instead (o_file_open)
 */
bool OJson_write_file(oobj obj, const char *file);


/**
 * Writes the json element tree into the resulting allocated string.
 * Extended edition which may append an optional end string
 * @param obj OJson object
 * @param opt_out_len if not NULL, will be set to the string length
 * @param opt_end_string appended if not NULL
 * @return json ascii string null terminated, or NULL if failed
 */
char *OJson_dump_ex(oobj obj, osize *opt_out_len, const char *opt_end_string);

/**
 * Writes the json element tree into the resulting allocated string
 * @param obj OJson object
 * @param opt_out_len if not NULL, will be set to the string length
 * @return json ascii string null terminated, or NULL if failed
 */
o_inline char *OJson_dump(oobj obj, osize *opt_out_len)
{
    return OJson_dump_ex(obj, opt_out_len, NULL);
}


//
// path api
//

/**
 * Get a specific child in the OJson tree using a path.
 * The path can traverse through objects and arrays to reach nested objects.
 * Some examples:
 *
 *   "foo" -> child foo of the given json (type object)
 *   "foo/bar" -> foo itself is of type object and get child bar
 *   "foo/bar[4]" -> bar is an array (or object) and we get the fiths (like in C) object of it
 *   "" -> return passed obj (empty keys are not allowed such as { "": "empty key" } )
 *   "[0]" -> passed obj is an array (or object) and we get the first object of it
 *   "[0][1][2]" -> nested arrays
 *   "[7]foo[8]bar" -> array, object, array, object
 *   "/[7]/foo/[8]/bar/" -> as above, correctly fine to use a slash between each node
 *   "/////[7]////foo///[8]//bar//////" -> as above, correctly fine to use multiple slashes (empty keys are not allowed)
 *
 * The given path may also be escaped for some obscure key names:
 *   "esc\/ape" -> for the key "esc/ape" instead of esc -> ape
 *   "foo\[6\]" -> for the key "foo[6]"
 *   "bar\\" ‐> for key "bar\"
 *
 * @note Even if valid for json, empty "" keys are not allowed in the path
 *
 * If the path fails, NULL is returned.
 * @param obj OJson obj as path root
 * @param path as described above
 * @return OJson object from the path of obj, or NULL if failed
 */
struct oobj_opt OJson_path(oobj obj, const char *path);


/**
 * Path api for nested access.
 * @param obj OJson object
 * @param path as described in OJson_path
 * @return a pointer to the OJson boolean data, or NULL if type!=OJson_BOOLEAN
 */
o_inline bool *OJson_path_boolean(oobj obj, const char *path)
{
    oobj itm = OJson_path(obj, path).o;
    if(!itm) {
        return NULL;
    }
    return OJson_boolean(itm);
}

/**
 * Path api for nested access.
 * @param obj OJson object
 * @param path as described in OJson_path
 * @return a pointer to the OJson number data, or NULL if type!=OJson_NUMBER
 */
o_inline double *OJson_path_number(oobj obj, const char *path)
{
    oobj itm = OJson_path(obj, path).o;
    if(!itm) {
        return NULL;
    }
    return OJson_number(itm);
}

/**
 * Path api for nested access.
 * @param obj OJson object
 * @param path as described in OJson_path
 * @return the string of the OJson, or NULL if type!=OJson_STRING
 */
o_inline const char *OJson_path_string(oobj obj, const char *path)
{
    oobj itm = OJson_path(obj, path).o;
    if(!itm) {
        return NULL;
    }
    return OJson_string(itm);
}

/**
 * Path api for nested access.
 * @param obj OJson object
 * @param path as described in OJson_path
 * @string the string for the OJson element
 * @return the string of the OJson, or NULL if type!=OJson_STRING
 * @note does an o_log_warn if type!=OJson_STRING
 */
o_inline const char *OJson_path_string_set(oobj obj, const char *path, const char *string)
{
    oobj itm = OJson_path(obj, path).o;
    if(!itm) {
        return NULL;
    }
    return OJson_string_set(itm, string);
}



#endif //O_OJSON_H
