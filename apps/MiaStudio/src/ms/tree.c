#include "ms/tree.h"
#include "o/ODict.h"
#include "o/OArray.h"
#include "o/OJson.h"
#include "o/str.h"
#include "o/log.h"
#include "ms/MSFile.h"

#define TREE_MAX_RAW_LEN 16
#define TREE_MAX_FILE_LEN 32

static struct {
    oobj container;
    char img_path[TREE_MAX_FILE_LEN];
    char thumb_path[TREE_MAX_FILE_LEN];
    oobj json;
    oobj json_tree;
    oobj empty_array;
    ou16 max_file_idx;
} tree_L;


static void ms_tree_new_raw(char *raw_buffer)
{
    ou16 raw_val;
    if (OArray_num(tree_L.empty_array) <= 0) {
        raw_val = ++tree_L.max_file_idx;
    } else {
        OArray_pop(tree_L.empty_array, &raw_val);
    }
    snprintf(raw_buffer, TREE_MAX_RAW_LEN, "&tree_%"ou16_PRI, raw_val);
}


//
// public
//

void ms_tree_init(oobj parent)
{
    if (tree_L.container) {
        o_log_error("init already called");
        return;
    }
    tree_L.container = OObj_new(parent);

    tree_L.empty_array = OArray_new(tree_L.container, NULL, sizeof(ou16), 0);
}


const char *ms_tree_raw_path(oobj file)
{
    return NULL;
}

const char *ms_tree_img_path(oobj file)
{
    assert(MSFile_type(file) == MSFile_type_IMG);
    const char *raw = ms_tree_raw_path(file);
    o_strf_buf(tree_L.img_path, "%s.png", raw);
    return tree_L.img_path;
}

const char *ms_tree_img_thumb_path(oobj file)
{
    assert(MSFile_type(file) == MSFile_type_IMG);
    const char *raw = ms_tree_raw_path(file);
    o_strf_buf(tree_L.img_path, "%s.thumb.png", raw);
    return tree_L.thumb_path;
}

void ms_tree_img_thumb_update(oobj file)
{
    const char *path = ms_tree_img_path(file);
    const char *thumb_path = ms_tree_img_thumb_path(file);

}


MSFile **ms_tree_ls(oobj parent, const char *file_dir, osize *opt_out_num_files)
{
    oobj container = OObj_new(parent);
    if (o_str_ends(file_dir, "/..")) {
        // get parent
        char *dir = o_str_clone(container, file_dir);
        // replace "/.." to "\0"
        dir[o_str_find_back_char(dir, '/')] = '\0';
        // check if the parent is either root or a sub dir
        osize parent_slash = o_str_find_back_char(dir, '/');
        dir[o_max(0, parent_slash)] = '\0';
        file_dir = dir;
    }

    oobj json_dir = OJson_path(tree_L.json_tree, file_dir).o;
    if (!json_dir) {
        o_log_error("failed to load dir: %s", file_dir);
        o_del(container);
        return o_new0(parent, MSFile *, 1);
    }


    osize num_children;
    OJson **it = OJson_list(json_dir, &num_children);
    MSFile **files = o_new0(parent, MSFile *, num_children+2);
    osize num_files = 0;

    if (file_dir[0] != '\0') {
        const char *back_path = o_strf(container, "%s/..", file_dir);
        files[num_files++] = MSFile_new_dir(parent, back_path);
    }

    for (osize i=0; i<num_children; i++) {
        OJson *at = it[i];
        const char *name = OObj_name(at);
        if (!name || name[0] == '?') {
            continue;
        }
        const char *type = OJson_path_string(at, "?type");
        if (!type) {
            o_log_error("invalid item, missing type: %s", name);
            continue;
        }

        char *path = o_strf(container, "%s/%s", file_dir, name);

        MSFile *file = NULL;
        if (o_str_equals(type, "DIR")) {
            file = MSFile_new_dir(parent, path);
        } else if (o_str_equals(type, "FILE")) {
            file = MSFile_new_img(parent, path);
        }

        if (file) {
            files[num_files++] = file;
        }
    }
    o_opt_set(opt_out_num_files, num_files);

    o_del(container);
    return files;
}
