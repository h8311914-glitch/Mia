#ifndef O_OWALKJSON_H
#define O_OWALKJSON_H

/**
 * @file OWalkjson.h
 *
 * Object
 *
 * This object helps to extract static json fields.
 * So if you know which attributes/fields you should have, consider using this.
 * Safes a lot of boilerplate checks and bundles it into OWalkjson_valid().
 * If OWalkjson_valid() is true, all extracting worked fine.
 *
 */

#include "o/OJson.h"

/** object id */
#define OWalkjson_ID OObj_ID "OWalkjson."


typedef struct {
    OObj super;

    bool valid;
} OWalkjson;


/**
 * Initializes the object.
 * @param obj OWalkjson object
 * @param parent OJson object to inherit from and to extract from
 * @return obj casted as OWalkjson
 */
OWalkjson *OWalkjson_init(oobj obj, oobj parent);

/**
 * Creates a new OJson object
 * @param parent OJson object to inherit from and to extract from
 * @return The new object
 */
o_inline OWalkjson *OWalkjson_new(oobj parent)
{
    OObj_DECL_IMPL_NEW(OWalkjson, parent);
}

//
// object functions:
//

/**
 * @param obj OWalkjson object
 * @return true if extracting was valid
 */
OObj_DECL_GETSET(OWalkjson, bool, valid)

/**
 * @param obj OWalkjson object
 * @param path as described in OJson_path()
 * @return the OJson object (maybe NULL if invalid)
 * @note only if valid and will set it if failed
 */
oobj OWalkjson_get(oobj obj, const char *path);


/**
 * @param obj OWalkjson object
 * @param path as described in OJson_path()
 * @return boolean of that keypath. check OWalkjson_valid after collecting
 * @note only if valid and will set it if failed
 */
bool OWalkjson_boolean(oobj obj, const char *path);

/**
 * @param obj OWalkjson object
 * @param path as described in OJson_path()
 * @return number of that keypath. check OWalkjson_valid after collecting
 * @note only if valid and will set it if failed
 */
double OWalkjson_number(oobj obj, const char *path);
/**
 * @param obj OWalkjson object
 * @param path as described in OJson_path()
 * @return string of that keypath. check OWalkjson_valid after collecting
 * @note only if valid and will set it if failed
 */
const char *OWalkjson_string(oobj obj, const char *path);

//
// array of numbers
//

/**
 * @param obj OWalkjson object
 * @param path as described in OJson_path()
 * @param numbers as a list of numbers for that keypath. check OWalkjson_valid after collecting
 * @note only if valid and will set it if failed
 */
void OWalkjson_vector(oobj obj, const char *path, double *out_numbers, osize n);


/**
 * @param obj OWalkjson object
 * @param path as described in OJson_path()
 * @param numbers as a list of numbers for that keypath. check OWalkjson_valid after collecting
 * @note only if valid and will set it if failed
 *       As with the math library, this is column_major
 */
void OWalkjson_matrix(oobj obj, const char *path, double *out_numbers, osize cols, osize rows);




#endif //O_OWALKJSON_H
