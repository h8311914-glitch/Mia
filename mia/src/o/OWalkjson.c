#include "o/OWalkjson.h"
#include "o/OObj_builder.h"
#include "o/str.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"

//
// public
//


OWalkjson *OWalkjson_init(oobj obj, oobj parent)
{
    OWalkjson *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, OWalkjson_ID);

    OObj_assert(parent, OJson);

    self->valid = true;
    return self;
}


//
// object functions:
//

oobj OWalkjson_get(oobj obj, const char *path)
{
    OObj_assert(obj, OWalkjson);
    OWalkjson *self = obj;
    if (!self->valid) {
        return NULL;
    }
    oobj json = OObj_parent(self);
    OObj_assert(json, OJson);
    oobj ret = OJson_path(json, path).o;
    if (!ret) {
        self->valid = false;
        o_log_warn_s(__func__, "path \"%s\" not found", path);
    }
    return ret;
}

bool OWalkjson_boolean(oobj obj, const char *path)
{
    OObj_assert(obj, OWalkjson);
    OWalkjson *self = obj;
    if (!self->valid) {
        return false;
    }
    oobj json = OObj_parent(self);
    OObj_assert(json, OJson);
    bool *value = OJson_path_boolean(json, path);
    bool ret = false;
    if (value) {
        ret = *value;
    } else {
        self->valid = false;
        o_log_warn_s(__func__, "path \"%s\" not found", path);
    }
    return ret;
}


double OWalkjson_number(oobj obj, const char *path)
{
    OObj_assert(obj, OWalkjson);
    OWalkjson *self = obj;
    if (!self->valid) {
        return false;
    }
    oobj json = OObj_parent(self);
    OObj_assert(json, OJson);
    double *value = OJson_path_number(json, path);
    double ret = 0;
    if (value) {
        ret = *value;
    } else {
        self->valid = false;
        o_log_warn_s(__func__, "path \"%s\" not found", path);
    }
    return ret;
}

const char *OWalkjson_string(oobj obj, const char *path)
{
    OObj_assert(obj, OWalkjson);
    OWalkjson *self = obj;
    if (!self->valid) {
        return NULL;
    }
    oobj json = OObj_parent(self);
    OObj_assert(json, OJson);
    const char *value = OJson_path_string(json, path);
    if (!value) {
        self->valid = false;
        o_log_warn_s(__func__, "path \"%s\" not found", path);
    }
    return value;
}

void OWalkjson_vector(oobj obj, const char *path, double *out_numbers, osize n)
{
    OObj_assert(obj, OWalkjson);
    OWalkjson *self = obj;
    for (int i=0; i<n; i++) {
        if (!self->valid) {
            return;
        }
        char *subpath = o_strf(self, "%s[%d]", path, i);
        out_numbers[i] = OWalkjson_number(self, subpath);
        o_free(self, subpath);
    }
}

void OWalkjson_matrix(oobj obj, const char *path, double *out_numbers, osize cols, osize rows)
{
    OObj_assert(obj, OWalkjson);
    OWalkjson *self = obj;
    for (int c=0; c<cols; c++) {
        for (int r=0; r<rows; r++) {
            if (!self->valid) {
                return;
            }
            char *subpath = o_strf(self, "%s[%d][%d]", path, c, r);
            out_numbers[c*rows+r] = OWalkjson_number(self, subpath);
            o_free(self, subpath);
        }
    }
}