#include "m/MMat_view.h"
#include "o/img.h"
#include "o/file.h"
#include "o/str.h"
#include "o/OJson.h"
#include "o/OArray_raw.h"
#include "o/endian.h"
#include "m/int.h"

#include <stdlib.h> // strtod, strtol

#undef O_LOG_LIB
#define O_LOG_LIB "m"

#include "o/log.h"



MMat *MMat_new_img(oobj parent, struct o_img *img, bool move_img)
{
    assert(img->data && img->channels>0 && img->channels<o_img_ENUM_MAX);
    ou32 format = M_FORMAT_X(img->channels, M_FORMAT__PRIMITIVE_U8);
    MMat *self = MMat_new_ex(parent, img->data, NULL, false, format,
                                 ivec2_(img->cols, img->rows), ivec2_(0));
    if (move_img) {
        o_img_move(img, self);
        self->owning = true;
    }
    return self;
}

struct oobj_opt MMat_new_file_image(oobj parent, const char *file)
{
    struct o_img img = o_img_new_file(parent, file, 0);
    if (!img.data) {
        o_log_warn_s(__func__, "failed to create MMat from image file: %s", file);
        return oobj_opt(NULL);
    }
    MMat *self = MMat_new_img(parent, &img, true);
    return oobj_opt(self);
}

struct oobj_opt MMat_new_file_image_indexed(oobj parent, oobj *out_palette, bool fixed_transparency, const char *file)
{
    struct o_img indexed, palette;
    indexed = o_img_indexed_new_file(parent, &palette, fixed_transparency, file);
    if (!indexed.data) {
        o_log_warn_s(__func__, "failed to create MMat from indexed image file: %s", file);
        *out_palette = NULL;
        return oobj_opt(NULL);
    }
    MMat *self = MMat_new_img(parent, &indexed, true);
    assert(self->format == M_FORMAT_1_U8);
    self->format = M_FORMAT_1_UR8;
    *out_palette = MMat_new_img(parent, &palette, true);
    return oobj_opt(self);
}

struct oobj_opt MMat_new_file_csv_ex(oobj parent, const char *file, ou32 format_primitive,
    const char *channel_sep, const char *col_sep, const char *row_sep, 
    bool allow_inconsistency)
{
    oobj res = NULL;
    oobj container = OObj_new(parent);
    
    oobj csv_file_array = o_file_read(container, file, true, 1).o;
    if (!csv_file_array) {
        o_log_warn_s(__func__, "failed to create MMat from csv file (failed to read): %s", file);
        goto CLEAN_UP;
    }
    if (OArray_num(csv_file_array) <= 0) {
        o_log_warn_s(__func__, "failed to create MMat from csv file (empty): %s", file);
        goto CLEAN_UP;
    }
    const char *csv_string = OArray_data_void(csv_file_array);
    
    ou32 dbl_format = 0;
    ou32 res_format = 0;

    MMat *mat = NULL;
    oi32 max_rows = oi32_MAX;
    oi32 max_cols = oi32_MAX;
    oi32 max_channels = oi32_MAX;
    bool consistend = true;
    struct o_str_range_list rows = o_str_split_ranges(container, csv_string, row_sep);
    for (osize r=0; r<o_min(rows.len, max_rows); r++) {
        char *row_string = o_str_clone_range(container, csv_string, rows.list[r]);
        struct o_str_range_list cols = o_str_split_ranges(container, row_string, col_sep);
        if(mat && cols.len != max_cols) {
            consistend = false;
        }
        for (osize c=0; c<o_min(cols.len, max_cols); c++) {
            char *col_string = o_str_clone_range(container, row_string, cols.list[c]);
            struct o_str_range_list channels = o_str_split_ranges(container, col_string, channel_sep);

            if (o_unlikely(!mat)) {
                dbl_format = M_FORMAT_X(channels.len, M_FORMAT__PRIMITIVE_F64);
                res_format = M_FORMAT_X(channels.len, format_primitive);
                
                if(m_format_size(dbl_format) > M_FORMAT_MAX_SIZE 
                    || m_format_size(res_format) > M_FORMAT_MAX_SIZE) {
                    o_log_warn_s(__func__, "failed to create MMat from csv file (invalid format size): %s", file);
                    goto CLEAN_UP;
                }
                
                max_rows = rows.len;
                max_cols = cols.len;
                max_channels = channels.len;
                
                o_log_debug("parsing: %i rows, %i cols, %i channels", max_rows, max_cols, max_channels);
                
                mat = MMat_new_0_try(container, cols.len, rows.len, dbl_format).o;
                if(!mat) {
                    o_log_warn_s(__func__, "failed to create MMat from csv file (allocation failed): %s", file);
                    goto CLEAN_UP;
                }
            }
            
            if(channels.len != max_channels) {
                consistend = false;
            }
            
            double *at = MMat_at_raw(mat, c, r);
                
            for(osize i=0; i<o_min(channels.len, max_channels); i++) {
                char *value_string = o_str_clone_range(container, col_string, channels.list[i]);
                
                double val = strtod(value_string, NULL);
                at[i] = val;
                o_free(container, value_string);
            }

            o_free(container, col_string);
        }

        o_free(container, row_string);
    }
    
    if(!consistend) {
        if(!allow_inconsistency) {
            o_log_warn_s(__func__, "failed to create MMat from csv file (inconsistend): %s", file);
            goto CLEAN_UP;
        } else {
            o_log_info_s(__func__, "csv was inconsistend");
        }
    }
    
    if(res_format != dbl_format) {
        mat = MMat_cast_try(mat, res_format).o;
        if(!mat) {
            o_log_warn_s(__func__, "failed to create MMat from csv file (cast failed): %s", file);
            goto CLEAN_UP;
        }
    }
    
    o_move(mat, parent);
    res = mat;
    
    CLEAN_UP:
    o_del(container);
    return oobj_opt(res);
}

struct oobj_opt MMat_new_file_npy(oobj parent, const char *file)
{
    MMat *res = NULL;
    oobj container = OObj_new(parent);

    oobj npy_file_array = o_file_read(container, file, false, 1).o;
    if (!npy_file_array) {
        o_log_warn_s(__func__, "failed to create MMat from npy file (failed to read): %s", file);
        goto CLEAN_UP;
    }
    if (OArray_num(npy_file_array) < 10) {
        o_log_warn_s(__func__, "failed to create MMat from npy file (empty): %s", file);
        goto CLEAN_UP;
    }
    char *npy_root = OArray_data_void(npy_file_array);
    osize npy_num = OArray_num(npy_file_array);

    if (!o_str_begins(npy_root, "\x93NUMPY")) {
        o_log_warn_s(__func__, "failed to create MMat from npy file (magic string wrong): %s", file);
        goto CLEAN_UP;
    }
    ou8 version_major = npy_root[6];
    ou8 version_minor = npy_root[7];

    if (version_major<=0 || version_major>3 || version_minor != 0) {
        o_log_info_s(__func__, "unknown .npy version: %i.%i, from file: %s", version_major, version_minor, file);
    }

    char *header;
    ou32 header_len;
    osize header_len_full;

    if (version_major<=1) {
        header = npy_root+10;
        header_len = *((ou16*) (npy_root+8));
        header_len_full = 10 + header_len;
    } else {
        header = npy_root+12;
        header_len = *((ou32*) (npy_root+8));
        header_len_full = 12 + header_len;
    }

    if (npy_num < header_len_full) {
        o_log_warn_s(__func__, "failed to create MMat from npy file (header too small): %s", file);
        goto CLEAN_UP;
    }

    // the header is terminated by a newlined padded with spaces
    // changing that last byte to \0 as string for OJson parse safety
    npy_root[header_len_full-1] = '\0';

    // python dict to valid json:
    {
        // npy uses python like ' for strings, not json like "
        // and tuple () to array []
        o_str_replace_char_this(header, '\'', '\"');
        o_str_replace_char_this(header, '(', '[');
        o_str_replace_char_this(header, ')', ']');

        // json is lower case not Pascal for booleans
        osize fortran_bool_idx = o_str_find(header, "False");
        if (fortran_bool_idx>=0) {
            header[fortran_bool_idx] = 'f';
        } else {
            fortran_bool_idx = o_str_find(header, "True");
            if (fortran_bool_idx>=0) {
                header[fortran_bool_idx] = 't';
            }
        }
    }

    oobj json = OJson_new_read_string(container, NULL, header).o;
    if (!json) {
        o_log_warn_s(__func__, "failed to create MMat from npy file (header json parsing failed): %s", file);
        goto CLEAN_UP;
    }

    const char *json_descr = OJson_path_string(json, "descr");
    const bool *json_fortran_order = OJson_path_boolean(json, "fortran_order");
    oobj json_shape = OJson_path(json, "shape").o;

    if (!json_descr || !json_fortran_order || !json_shape
        || o_strlen(json_descr) < 3
        || OJson_num(json_shape) <= 0 || OJson_num(json_shape) > 3
        || OJson_type(OJson_at(json_shape, 0).o) != OJson_NUMBER) {
        o_log_warn_s(__func__, "failed to create MMat from npy file (header json missing fields): %s", file);
        goto CLEAN_UP;
    }

    osize rows = (osize) *OJson_number(OJson_at(json_shape, 0).o);
    osize cols = 1;
    osize channels = 1;
    if (OJson_num(json_shape)>=2) {
        cols = (osize) *OJson_number(OJson_at(json_shape, 1).o);
    }
    if (OJson_num(json_shape)==3) {
        channels = (osize) *OJson_number(OJson_at(json_shape, 2).o);
    }

    osize num = rows * cols;
    if (rows<=0 || cols<=0 || channels<=0 || num<=0) {
        o_log_warn_s(__func__, "failed to create MMat from npy file (invalid shape: %i;%i;%i): %s",
            rows, cols, channels, file);
        goto CLEAN_UP;
    }

    bool format_is_little_endian;
    if (json_descr[0] == '>') {
        format_is_little_endian = false;
    } else if (json_descr[0] == '<' || json_descr[0] == '|') {
        format_is_little_endian = true;
    } else {
        o_log_warn_s(__func__, "failed to create MMat from npy file (invalid format descriptor endian: %c): %s",
            json_descr[0], file);
        goto CLEAN_UP;
    }

    ou32 format_primitive_type;
    if (json_descr[1] == 'b' || json_descr[1] == 'u') {
        format_primitive_type = M_FORMAT__PRIMITIVE_TYPE_U;
    } else if (json_descr[1] == 'i') {
        format_primitive_type = M_FORMAT__PRIMITIVE_TYPE_IR;
    } else if (json_descr[1] == 'f') {
        format_primitive_type = M_FORMAT__PRIMITIVE_TYPE_F;
    } else {
        o_log_warn_s(__func__, "failed to create MMat from npy file (invalid format descriptor type: %c): %s",
            json_descr[1], file);
        goto CLEAN_UP;
    }

    char *prim_size_end;
    int format_primitive_size = strtol(json_descr+2, &prim_size_end, 10);
    if (prim_size_end==json_descr+2 || format_primitive_size<=0 || format_primitive_size>M_FORMAT_MAX_SIZE) {
        o_log_warn_s(__func__, "failed to create MMat from npy file (invalid format primitive size: %i): %s",
            format_primitive_size, file);
        goto CLEAN_UP;
    }

    ou32 format = M_FORMAT_X_FULL(channels, format_primitive_type, format_primitive_size);

    osize remaining_size = npy_num - header_len_full;
    osize needed_size = num * m_format_size(format);
    if (remaining_size < needed_size) {
        o_log_warn_s(__func__, "failed to create MMat from npy file (invalid data length: %i/%i bytes): %s",
            remaining_size, needed_size, file);
        goto CLEAN_UP;
    }
    obyte *data = (obyte *) (npy_root + header_len_full);

    res = MMat_new_ex(container, data, npy_root, false, format, ivec2_(cols, rows), ivec2_(0));

    if (*json_fortran_order) {
        res = MMat_transpose(res);
        MMat_owning_make_try(res);
        if (!MMat_valid(res)) {
            o_log_warn_s(__func__, "failed to create MMat from npy file (fortran reoder allocation failed): %s", file);
            res = NULL;
            goto CLEAN_UP;
        }
    } else {
        // we just move the array data into our MMat and declare it as owning
        OArray_move(npy_file_array, res);
        res->owning = true;
    }

    if (o_endian_system_is_binary_little_endian() != format_is_little_endian) {
        o_log_info_s(__func__, "npy file endian mismatch: %s", file);
        // need a swap
        for (osize i=0; i<num; i++) {
            void *restrict at = MMat_at_idx_raw(res, i);
            o_endian_swap(at, format_primitive_size);
        }
    }

    o_move(res, parent);

    CLEAN_UP:
    o_del(container);
    return oobj_opt(res);
}

struct oobj_opt MMat_new_json(oobj parent, oobj json_mat, ou32 format_primitive)
{
    oobj res = NULL;
    oobj container = OObj_new(parent);
    
    ou32 dbl_format = M_FORMAT_X(1, M_FORMAT__PRIMITIVE_F64);
    ou32 res_format = M_FORMAT_X(1, format_primitive);
    
    oobj mat = NULL;

    // Dim 0: single value
    if(OJson_type(json_mat) == OJson_NUMBER) {
        // no need to _try for a single value...
        double value = *OJson_number(json_mat);
        mat = MMat_new(container, &value, 1, 1, dbl_format);
        goto CAST;
    }

    osize rows;
    OJson **rows_data = OJson_list(json_mat, &rows);
    o_data_move(json_mat, container, rows_data);
    
    if(OJson_type(json_mat) != OJson_ARRAY || rows <= 0) {
        o_log_warn_s(__func__, "failed to create MMat from json (invalid type, need number or array)");
        goto CLEAN_UP;
    }
    
    // Dim 1: [0, 1, 2] -> cols=channels=1
    if(OJson_type(rows_data[0]) == OJson_NUMBER) {
        mat = MMat_new_try(container, NULL, 1, rows, dbl_format).o;
        if(!mat) {
            o_log_warn_s(__func__, "failed to create MMat from json (allocation failed)");
            goto CLEAN_UP;
        }
        for(osize i=0; i<rows; i++) {
            double *val = OJson_number(rows_data[i]);
            if(!val) {
                o_log_warn_s(__func__, "failed to create MMat from json (invalid json mat; dim 1)");
                goto CLEAN_UP;
            }
            double *restrict dst = MMat_at_idx_raw(mat, i);
            *dst = *val;
        }
        goto CAST;
    }


    osize cols;
    OJson **cols_0_data = OJson_list(rows_data[0], &cols);
    o_data_move(rows_data[0], container, cols_0_data);
    
    if(OJson_type(rows_data[0]) != OJson_ARRAY || cols <= 0) {
        o_log_warn_s(__func__, "failed to create MMat from json (invalid type (dim 2 test), need number or array)");
        goto CLEAN_UP;
    }
    
    
    // Dim 2: [[0, 1], [2, 3]] -> channels=1
    if(OJson_type(cols_0_data[0]) == OJson_NUMBER) {
        mat = MMat_new_try(container, NULL, cols, rows, dbl_format).o;
        if(!mat) {
            o_log_warn_s(__func__, "failed to create MMat from json (allocation failed)");
            goto CLEAN_UP;
        }
        for(osize r=0; r<rows; r++) {
            oobj row = rows_data[r];

            osize cols_check;
            OJson **cols_data = OJson_list(row, &cols_check);
            o_data_move(row, container, cols_data);

            if(OJson_type(row) != OJson_ARRAY || cols_check != cols) {
                o_log_warn_s(__func__, "failed to create MMat from json (invalid type (dim 2), need number or array)");
                goto CLEAN_UP;
            }
            
            for(osize c=0 ; c<cols; c++) {
                double *val = OJson_number(cols_data[c]);
                if(!val) {
                    o_log_warn_s(__func__, "failed to create MMat from json (invalid json mat; dim 2)");
                    goto CLEAN_UP;
                }
                double *restrict dst = MMat_at_raw(mat, c, r);
                *dst = *val;
            }
        }
        goto CAST;
    }
    
    osize channels;
    OJson **channels_0_data = OJson_list(cols_0_data[0], &channels);
    o_data_move(cols_0_data[0], container, channels_0_data);
    
    if(OJson_type(cols_0_data[0]) != OJson_ARRAY
        || channels <= 0
        || OJson_type(channels_0_data[0]) != OJson_NUMBER) {
        o_log_warn_s(__func__, "failed to create MMat from json (invalid type (dim 3 test), need number or array)");
        goto CLEAN_UP;
    }
    
    // Dim 3: [[[0, 1], [2, 3]], [[3, 4], [5, 6]]]
    {
        dbl_format = M_FORMAT_X(channels, M_FORMAT__PRIMITIVE_F64);
        res_format = M_FORMAT_X(channels, format_primitive);
        
        mat = MMat_new_try(container, NULL, cols, rows, dbl_format).o;
        if(!mat) {
            o_log_warn_s(__func__, "failed to create MMat from json (allocation failed)");
            goto CLEAN_UP;
        }
        for(osize r=0; r<rows; r++) {
            oobj row = rows_data[r];

            osize cols_check;
            OJson **cols_data = OJson_list(row, &cols_check);
            o_data_move(row, container, cols_data);

            if(OJson_type(row) != OJson_ARRAY || cols_check != cols) {
                o_log_warn_s(__func__, "failed to create MMat from json (invalid type (dim 3 rows), need number or array)");
                goto CLEAN_UP;
            }
            
            for(osize c=0 ; c<cols; c++) {
                oobj col = cols_data[c];

                osize channels_check;
                OJson **channels_data = OJson_list(col, &channels_check);
                o_data_move(col, container, channels_data);

                if(OJson_type(col) != OJson_ARRAY || channels_check != channels) {
                    o_log_warn_s(__func__, "failed to create MMat from json (invalid type (dim 3 cols), need number or array)");
                    goto CLEAN_UP;
                }
                double *restrict dst = MMat_at_raw(mat, c, r);
                
                for(osize i=0; i<channels; i++) {
                    double *val = OJson_number(channels_data[i]);
                    if(!val) {
                        o_log_warn_s(__func__, "failed to create MMat from json (invalid json mat; dim 3)");
                        goto CLEAN_UP;
                    }
                    dst[i] = *val;
                }
            }
        }
        goto CAST;
    }
    
    
    CAST:
    if(res_format != dbl_format) {
        mat = MMat_cast_try(mat, res_format).o;
        if(!mat) {
            o_log_warn_s(__func__, "failed to create MMat from json (cast failed)");
            goto CLEAN_UP;
        }
    }
    
    o_move(mat, parent);
    res = mat;
    
    CLEAN_UP:
    o_del(container);
    return oobj_opt(res);
}


//
// write io
//


bool MMat_img(oobj obj, struct o_img *out_img)
{
    MMat_assert(obj);
    MMat *self = obj;

    if (m_format_primitive(self->format) != M_FORMAT__PRIMITIVE_U8) {
        o_log_warn_s(__func__, "invalid format: %x, need M_FORMAT__PRIMITIVE_U8", self->format);
        return false;
    }

    int channels = m_format_channels(self->format);
    if (channels <= 0 || channels >= o_img_ENUM_MAX) {
        o_log_warn_s(__func__, "invalid format channels: %i, need [1:4] channels", channels);
        return false;
    }
    if (!MMat_packed(self)) {
        o_log_warn_s(__func__, "needs a packed MMat, use MMat_clone, etc.");
        return false;
    }
    *out_img = (struct o_img) {
        (obyte *) self->data,
        self->size.x,
        self->size.y,
        channels,
        NULL
    };
    return true;
}


bool MMat_write_file_image(oobj obj, const char *file)
{
    MMat_assert(obj);
    MMat *self = obj;
    oobj container = NULL;
    int channels = m_format_channels(self->format);
    assert(channels > 0);
    oi32 primitive = m_format_primitive(self->format);
    if (channels > 4 || primitive!=M_FORMAT__PRIMITIVE_U8 || !MMat_packed(self)) {
        MMat *cast = MMat_cast_try(self, M_FORMAT_4_U8).o;
        if (!cast) {
            return false;
        }
        container = OObj_new(self);
        o_move(cast, container);
        self = cast;
    }

    struct o_img img;
    bool valid = MMat_img(self, &img);
    assert(valid);
    valid = o_img_write_file(img, file);
    o_del(container);
    return valid;
}


bool MMat_write_file_image_indexed(oobj obj, oobj palette, const char *file)
{
    MMat_assert(obj);
    MMat *self = obj;
    MMat_assert(obj);
    MMat *pal = palette;

    assert(self->format == M_FORMAT_1_U8 || self->format == M_FORMAT_1_UR8);

    oobj container = NULL;
    if (!MMat_packed(self)) {
        MMat *cast = MMat_clone_try(self).o;
        if (!cast) {
            return false;
        }
        container = OObj_new(self);
        o_move(cast, container);
        self = cast;
    }

    int pal_channels = m_format_channels(pal->format);
    assert(pal_channels > 0);
    oi32 pal_primitive = m_format_primitive(pal->format);

    if (pal_channels > 4 || pal_primitive!=M_FORMAT__PRIMITIVE_U8 || !MMat_packed(pal)) {
        MMat *cast = MMat_cast_try(pal, M_FORMAT_4_U8).o;
        if (!cast) {
            o_del(container);
            return false;
        }
        if (!container) {
         container = OObj_new(self);
        }
        o_move(cast, container);
        pal = cast;
    }
    struct o_img img_indexed, img_palette;
    bool valid = MMat_img(self, &img_indexed);
    assert(valid);
    valid = MMat_img(pal, &img_palette);
    assert(valid);
    valid = o_img_indexed_write_file(img_indexed, img_palette, file);
    o_del(container);
    return valid;
}

bool MMat_write_file_csv_ex(oobj obj, const char *file, const char *channel_sep, const char *col_sep, const char *row_sep)
{
    MMat_assert(obj);
    MMat *self = obj;
    
    oobj container = OObj_new(obj);
    oobj array = OArray_new_dyn(container, NULL, 1, 0, 1024*16);
    
    ou32 format = self->format;
    int channels = m_format_channels(format);
    
    double *vals = o_new(container, double, channels);
    
    for(osize r=0; r<self->size.y; r++) {
        for(osize c=0; c<self->size.x; c++) {
            const void *at = MMat_at_raw(self, c, r);
            m_format_cast_to_double(vals, at, format);
            for(int i=0; i<channels; i++) {
                
                OArray_append_stringf(array, "%f", vals[i]);
                if(i<channels-1) {
                    OArray_append_string(array, channel_sep);
                }
            }
            if(c<self->size.x-1) {
                OArray_append_string(array, col_sep);
            }
        }
        if(r<self->size.y-1) {
            OArray_append_string(array, row_sep);
        }
    }
    bool valid = o_file_write(file, true, OArray_data_void(array), 1, OArray_num(array));
    o_del(container);
    return valid;
}

bool MMat_write_file_npy(oobj obj, const char *file)
{
    MMat_assert(obj);
    MMat *self = obj;

    oobj container = OObj_new(obj);
    oobj array = OArray_new_dyn(container, NULL, 1, 0, 1024*16);

    if (!MMat_packed(self)) {
        self = MMat_clone(self);
        o_move(self, container);
    }

    ou32 format = self->format;
    ou32 format_primitive_type = m_format_primitive_type(format);
    ou32 format_primitive_size = m_format_primitive_size(format);
    ou32 channels = m_format_channels(format);

    char descr[8]={0};
    if (o_endian_system_is_binary_little_endian()) {
        descr[0] = format_primitive_size==1? '|' : '<';
    } else {
        descr[0] = '>';
    }
    if (format_primitive_type == M_FORMAT__PRIMITIVE_TYPE_IR) {
        descr[1] = 'i';
    } else if (format_primitive_type == M_FORMAT__PRIMITIVE_TYPE_UR
        || format_primitive_type == M_FORMAT__PRIMITIVE_TYPE_U) {
        descr[1] = 'u';
    } else if (format_primitive_type == M_FORMAT__PRIMITIVE_TYPE_F) {
        descr[1] = 'f';
    } else {
        assert(0 && "invalid primitive type?");
    }
    snprintf(descr+2, sizeof(descr)-2, "%i", format_primitive_size);

    char *shape;
    if (channels>1) {
        shape = o_strf(container, "%i, %i, %i", self->size.y, self->size.x, channels);
    } else if (self->size.x>1) {
        shape = o_strf(container, "%i, %i", self->size.y, self->size.x);
    } else {
        shape = o_strf(container, "%i", self->size.y);
    }

    char *header = o_strf(container, "{\'descr\': \'%s\', \'fortran_order\': False, \'shape\': (%s), }",
        descr, shape);

    // 10 for fixed and 1 for newline
    osize header_len = 11 + o_strlen(header);

    osize header_len_full = header_len;
    if (header_len%64!=0) {
        header_len_full += (64 - header_len%64);
    }

    osize header_len_npy = header_len_full - 10;
    ou16 header_buffer = ou16_endian_to_lil(header_len_npy);

    osize needed_spaces = header_len_full - header_len;

    OArray_raw_append_string_static(array, "\x93NUMPY\x01\x00");
    OArray_raw_append(array, &header_buffer, 2);
    OArray_raw_append_string(array, header);

    for (osize i=0; i<needed_spaces; i++) {
        OArray_raw_append_string_static(array, " ");
    }

    OArray_raw_append_string_static(array, "\n");
    assert(OArray_num(array) == header_len_full);

    osize data_bytes_num = self->size.x*self->size.y*m_format_size(format);
    OArray_raw_append(array, self->data, data_bytes_num);
    OArray_raw_done(array);

    bool valid = o_file_write(file, true, OArray_data_void(array), 1, OArray_num(array));
    o_del(container);
    return valid;
}

oobj MMat_json(oobj obj, oobj parent, const char *name)
{
    if(!MMat_valid(obj)) {
        return OJson_new_null(parent, name);
    }
    MMat *self = obj;
    ou32 format = self->format;
    int channels = m_format_channels(format);
    
    assert(channels>0 && self->size.x>0 && self->size.y>0);
    
    // Dim 0: number <- rows=cols=channels=1
    if(channels == 1
        && self->size.x == 1 
        && self->size.y == 1) {
        const void *restrict at = MMat_at_idx_raw(self, 0);
        double val = m_format_primitive_cast_to_double(at, format);
        return OJson_new_number(parent, name, val);
    }
    
    // Dim 1: [0, 1, 2] <- cols=channels=1
    if(channels == 1
        && self->size.x == 1) {
        oobj rows = OJson_new_array(parent, name);
        OJson_packed_set(rows, true);
        for(osize r=0; r<self->size.y; r++) {
            const void *restrict at = MMat_at_raw(self, 0, r);
            double val = m_format_primitive_cast_to_double(at, format);
            OJson_new_number(rows, NULL, val);
        }
        return rows;
    }
    
    // Dim 2: [[0, 1], [2, 3]] <- channels=1
    if(channels == 1) {
        oobj rows = OJson_new_array(parent, name);
        for(osize r=0; r<self->size.y; r++) {
            oobj cols = OJson_new_array(rows, NULL);
            
            OJson_packed_set(cols, true);
            for(osize c=0; c<self->size.x; c++) {
                const void *restrict at = MMat_at_raw(self, c, r);
                double val = m_format_primitive_cast_to_double(at, format);
                OJson_new_number(cols, NULL, val);
            }
        }
        return rows;
    }
    
    // Dim 3: [[[0, 1], [2, 3]], [[3, 4], [5, 6]]]
    {
        oobj rows = OJson_new_array(parent, name);
        int prim_size = m_format_primitive_size(format);
        for(osize r=0; r<self->size.y; r++) {
            oobj cols = OJson_new_array(rows, NULL);
            OJson_packed_set(cols, true);
            
            for(osize c=0; c<self->size.x; c++) {
                oobj chans = OJson_new_array(cols, NULL);
                OJson_packed_set(chans, true);
                
                const obyte *restrict at = MMat_at_raw(self, c, r);
                
                for(osize i=0; i<channels; i++) {
                    const obyte *at_channel = at + prim_size*i;
                    double val = m_format_primitive_cast_to_double(at_channel, format);
                    OJson_new_number(chans, NULL, val);
                }
            }
        }
        return rows;
    }
}

char *MMat_str(oobj obj)
{
    oobj json = MMat_json(obj, obj, NULL);
    char *dump = OJson_dump(json, NULL);
    o_data_move(json, obj, dump);
    o_del(json);
    return dump;
}

void MMat_print(oobj obj)
{
    char *str = MMat_str(obj);
    printf("%s", str);
    o_free(obj, str);
#ifdef NDEBUG
    fflush(stdout);
#endif
}

void MMat_println(oobj obj)
{
    char *str = MMat_str(obj);
    printf("%s\n", str);
    o_free(obj, str);
#ifdef NDEBUG
    fflush(stdout);
#endif
}
