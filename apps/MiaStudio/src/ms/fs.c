#include "ms/fs.h"
#include "o/ODict.h"
#include "o/OArray.h"
#include "o/OList.h"
#include "o/OJson.h"
#include "o/str.h"
#include "o/file.h"
#include "o/log.h"
#include "m/types/flt.h"
#include "m/MMat_ex.h"

#include <stdlib.h> // strtoll

#include "o/OTask.h"


// stat fields
#define FS_KEY_TYPE         "?type"
#define FS_KEY_TIME         "?time"
#define FS_KEY_SIZE         "?size"
#define FS_KEY_LOCKED       "?locked"
#define FS_KEY_CLOUD        "?cloud"

// list of files and list of used idx numbers
#define FS_KEY_FILES        "?files"
#define FS_KEY_FILES_IDX    "?files_idx"

// file idx numbers
#define FS_KEY_IDX_THUMB    "?idx_thumb"
#define FS_KEY_IDX_PNG      "?idx_png"


struct fs_thumb_job {
    char thumb_file[ms_fs_file_MAX];
    char img_file[ms_fs_file_MAX];
    oobj container;
    oobj img;
    oobj thumb;
    oobj task;
};


static struct {
    oobj container;
    
    oobj json;
    oobj json_fs;
    oobj empty_array;
    ou16 max_file_idx;

    oobj thumb_job_list;

    bool changed;
    int running_jobs;
} fs_L;

static const char *FS_TYPE_STR[] = {
    "NONE",
    "DIR",
    "EXE",
    "PNG",
};

static const char *FS_CLOUD_STR[] = {
    "AVAILABLE",
    "REQUESTED",
    "MISSING",
};


static ou16 fs_next_idx(void)
{
    if(OArray_num(fs_L.empty_array)>0) {
        ou16 next;
        OArray_pop(fs_L.empty_array, &next);
        return next;
    }
    return fs_L.max_file_idx++;
}

static ou16 fs_touch_file_insert(oobj file_json, const char *file_ending)
{
    ou16 idx = fs_next_idx();
    char real_file[ms_fs_file_MAX];
    o_strf_buf(real_file, "ms_file_%" ou16_PRI "%s", idx, file_ending);
    
    oobj files = OJson_child(file_json, FS_KEY_FILES).o;
    oobj files_idx = OJson_child(file_json, FS_KEY_FILES_IDX).o;
    assert(files && files_idx);
    
    OJson_new_string(files, NULL, real_file);
    OJson_new_number(files, NULL, idx);
    
    return idx;
}

static void fs_invalidate_stat_cache(oobj file_json)
{
    o_free(file_json, o_user(file_json));
    o_user_set(file_json, NULL);
}


static int fs_sort_compare(void *user_data, const void *a, const void *b)
{
    enum ms_fs_sort mode = *(enum ms_fs_sort*) user_data;
    const char *path_a = a;
    const char *path_b = b;
    struct ms_fs_stat stat_a = ms_fs_stat(path_a);
    struct ms_fs_stat stat_b = ms_fs_stat(path_b);
    switch(mode) {
        default:
        case ms_fs_sort_DEFAULT:
            // EXE -> DIR -> REMAINING and sort by name
            if (stat_a.type != stat_b.type) {
                if (stat_a.type == ms_fs_type_EXE) {
                    return -1;
                } else if (stat_b.type == ms_fs_type_EXE) {
                    return +1;
                }
                if (stat_a.type == ms_fs_type_DIR) {
                    return -1;
                } else if (stat_b.type == ms_fs_type_DIR) {
                    return +1;
                }
            }
            // either same type or not EXE nor DIR, just sort by name
            return strcmp(stat_a.name, stat_b.name);
        case ms_fs_sort_NAME_ASCENDING:
            return strcmp(stat_a.name, stat_b.name);
        case ms_fs_sort_NAME_DESCENDING:
            return strcmp(stat_b.name, stat_a.name);
        case ms_fs_sort_TIME_ASCENDING:
            return stat_a.time - stat_b.time;
        case ms_fs_sort_TIME_DESCENDING:
            return stat_b.time - stat_a.time;
        case ms_fs_sort_SIZE_ASCENDING:
            return stat_a.size - stat_b.size;
        case ms_fs_sort_SIZE_DESCENDING:
            return stat_b.size - stat_a.size;
    }
}


oobj fs_file_idx_array(oobj parent, const char *path)
{
    oobj array = OArray_new_dyn(parent, NULL, sizeof(ou16), 0, 8);
    oobj file = OJson_path(fs_L.json_fs, path).o;
    if(!file) {
        return array;
    }
    oobj file_indices = OJson_child(file, FS_KEY_FILES_IDX).o;
    if(!file_indices) {
        return array;
    }
    OJson **json_it = OJson_list(file_indices, NULL);

    for(OJson **it = json_it; *it; it++) {
        double *idx = OJson_number(*it);
        if (!idx) {
            o_log_error("invalid file idx in " FS_KEY_FILES_IDX " : %s", path);
            continue;
        }
        ou16 add = *idx;
        OArray_push(array, &add);
    }
    o_free(file_indices, json_it);

    return array;
}

ou16 fs_key_idx(const char *path, const char *key)
{
    oobj file = OJson_path(fs_L.json_fs, path).o;
    if(!file) {
        o_log_warn("path does not exist %s", path);
        return 0;
    }
    fs_invalidate_stat_cache(file);
    
    oobj idx = OJson_child(file, key).o;
    if(!idx) {
        o_log_error("invalid file, missing %s : %s", key, path);
        return 0;
    }
    
    double *num = OJson_number(idx);
    if(!num) {
        o_log_error("invalid file, missing %s : %s", key, path);
        return 0;
    }
    return (ou16) *num;
}

static bool fs_thumb_job_run(struct fs_thumb_job *job)
{
    assert(job->container);

    if (!job->img) {
        job->img = MMat_new_file_image(job->container, job->img_file).o;
        job->thumb = MMat_new_0(job->container, m_2(ms_fs_THUMB_SIZE), M_FORMAT_RGBA_U8);
        job->task = MMat_resample_resize_smooth_rect_into_task(job->img, job->thumb, vec4_(0));
        return false;
    }
    if (!OTask_finished(job->task)) {
        OTask_run(job->task);
        return false;
    }
    MMat_write_file_image(job->thumb, job->thumb_file);
    o_del(job->container);
    return true;
}

//
// public
//

void ms_fs_init(oobj parent)
{
    if (fs_L.container) {
        o_log_error("init already called");
        return;
    }
    fs_L.container = OObj_new(parent);

    fs_L.empty_array = OArray_new(fs_L.container, NULL, sizeof(ou16), 0);
    
    fs_L.thumb_job_list = OArray_new_dyn(fs_L.container, NULL, sizeof(struct fs_thumb_job), 0, 32);
    
}


void ms_fs_update(void)
{
    fs_L.changed = false;

    if (OArray_num(fs_L.thumb_job_list) > 0) {
        struct fs_thumb_job *job = OArray_at(fs_L.thumb_job_list, 0, struct fs_thumb_job);
        if (fs_thumb_job_run(job)) {
            OArray_pop_front(fs_L.thumb_job_list, NULL);
            fs_L.changed = true;
        }
    }

    // calc all running jobs
    fs_L.running_jobs = OArray_num(fs_L.thumb_job_list);
}

bool ms_fs_changed(void)
{
    return fs_L.changed;
}
int ms_fs_running_jobs(void)
{
    return fs_L.running_jobs;
}


oobj ms_fs_ls(oobj parent, const char *path, enum ms_fs_sort sort_mode)
{
    oobj list = OList_new(parent, NULL, 0);
    oobj file = OJson_path(fs_L.json_fs, path).o;
    if(!file) {
        return list;
    }
    const char *type = OJson_path_string(file, FS_KEY_TYPE);
    if(!type || !o_str_equals(type, FS_TYPE_STR[ms_fs_type_DIR])) {
        o_log_warn("failed, invalid " FS_KEY_TYPE);
        return list;
    }
    osize sort_start = 0;
    
    // needs an o_free
    OJson **json_it = OJson_list(file, NULL);
    
    if(path[0]!='\0') {
        // add prev dir file ".."
        char *add = o_strf(list, "%s/..", path);
        OList_push(list, add);
        sort_start++;
    }
    
    for(OJson **it = json_it; *it; it++) {
        const char *name = OObj_name(*it);
        assert(name);
        if(name[0]=='?') {
            // ignore file meta
            continue;
        }
        
        char *add;
        if(path[0]=='\0') {
            add = o_strf(list, "%s", name);
        } else {
            add = o_strf(list, "%s/%s", path, name);
        }
        OList_push(list, add);
    }
    
    osize sort_num = OList_num(list) - sort_start;
    
    char **sort_data = (char **) OList_at_ref(list, sort_start);
    ms_fs_sort(sort_data, sort_num, sort_mode);
    
    // free OJson_list
    o_free(file, json_it);
    
    return list;
}



bool ms_fs_mv(const char *from_path, const char *to_path)
{
    oobj from_json = OJson_path(fs_L.json_fs, from_path).o;
    struct ms_fs_stat from_stat = ms_fs_stat(from_path);
    struct ms_fs_stat to_stat = ms_fs_stat(to_path);
    if(!from_json
        || from_stat.type == ms_fs_type_NONE 
        || from_stat.locked
        || to_path[0] == '\0') {
        return false;
    }
    if(o_str_equals(from_path, to_path)) {
        // move to itself is a success noop
        return true;
    }
    if(to_stat.type != ms_fs_type_NONE) {
        // to exists alreads
        return false;
    } 
    
    oobj container = OObj_new(fs_L.container);
    bool success = false;
    char *from_dir = ms_fs_path_dir(container, from_path);
    char *to_dir = ms_fs_path_dir(container, to_path);
    const char *to_name = ms_fs_path_name(to_path);
    
    oobj from_dir_json = OJson_path(fs_L.json_fs, from_dir).o;
    oobj to_dir_json = OJson_path(fs_L.json_fs, to_dir).o;
    
    if(!from_dir_json) {
        o_log_error("invalid from dir?");
        goto CLEAN_UP;
    }
    if(!to_dir_json) {
        o_log_warn("to dir does not exist!");
        goto CLEAN_UP;
    }
    
    if(!o_str_equals(from_dir, to_dir)) {
        // move
        o_move(from_json, to_dir_json);
    }
    
    // rename
    OObj_name_set(from_json, to_name);
    fs_invalidate_stat_cache(from_json);
    success = true;
  
    CLEAN_UP:
    o_del(container);
    return success;
}



bool ms_fs_cp(const char *from_path, const char *to_path)
{
    struct ms_fs_stat from_stat = ms_fs_stat(from_path);
    if (from_stat.type==ms_fs_type_NONE) {
        o_log_error("invalid from_path");
        return false;
    }
    if(from_stat.locked) {
        o_log_error("from file locked");
        return false;
    }

    if (!ms_fs_touch(to_path, from_stat.type)) {
        o_log_error("failed to touch to file");
        return false;
    }

    oobj from_file = OJson_path(fs_L.json_fs, from_path).o;
    if(!from_file) {
        o_log_error("failed to get from file");
        return false;
    }
    oobj to_file = OJson_path(fs_L.json_fs, to_path).o;
    if(!to_file) {
        o_log_error("failed to get to file");
        return false;
    }

    // clone real files
    // o_file_clone is NULL safe...
    bool success = true;
    char from_buf[ms_fs_file_MAX];
    char to_buf[ms_fs_file_MAX];
    switch(from_stat.type) {
    case ms_fs_type_PNG:
        success = o_file_clone(ms_fs_file_png(to_path, to_buf), ms_fs_file_png(from_path, from_buf));
        break;
    default:
        break;
    }
    
    // creates thumbnails, etc.
    ms_fs_sync(to_path);

    return success;
}



bool ms_fs_rm(const char *path, bool recursive)
{
    struct ms_fs_stat stat = ms_fs_stat(path);
    if(stat.type==ms_fs_type_NONE) {
        o_log_error("invalid path");
        return false;
    }
    if(stat.locked) {
        o_log_error("file locked");
        return false;
    }
    
    oobj container = OObj_new(fs_L.container);

    // remove all files from the object (some types do not have any and return NULL)
    oobj files = ms_fs_file_list(container, path);
    for (osize i=0; i<o_num(files); i++) {
        o_file_remove(o_at(files, i));
    }

    // get all used file indices and add those to the empty array
    oobj file_indices = fs_file_idx_array(container, path);
    OArray_append(fs_L.empty_array, OArray_data_void(file_indices), OArray_num(file_indices));


    bool remove = true;
    
    if(stat.type==ms_fs_type_DIR) {
        oobj ls = ms_fs_ls(container, path, ms_fs_sort_IGNORE);
        if(recursive) {
            for(osize i=0; i<o_num(ls); i++) {
                bool ls_valid = ms_fs_rm(o_at(ls, i), true);
                if(!ls_valid) {
                    remove = false;
                }
            }
        } else {
            remove = o_num(ls) == 0;
        }
    }
    
    if(remove) {
        oobj file = OJson_path(fs_L.json_fs, path).o;
        if(!file) {
            o_log_warn("failed to rm: %s", path);
            remove = false;
        } else {
            // no extra stuff
            o_del(file);
        }
    }
    
    o_del(container);
    return remove;
}



struct ms_fs_stat ms_fs_stat(const char *path)
{
    struct ms_fs_stat stat = {0};
    oobj file = OJson_path(fs_L.json_fs, path).o;
    if(!file) {
        return stat;
    }
    
    // check cached
    struct ms_fs_stat *cache = o_user(file);
    if(cache) {
        return *cache;
    }
    
    stat.name = OObj_name(file);
    
    const char *type = OJson_path_string(file, FS_KEY_TYPE);
    for(enum ms_fs_type t = ms_fs_type_DIR; t < ms_fs_type_ENUM_MAX; t++) {
        if(o_str_equals(type, FS_TYPE_STR[t])) {
            stat.type = t;
            break;
        }
    }
    if(stat.type == ms_fs_type_NONE) {
        o_log_error("invalid file, invalid " FS_KEY_TYPE);
        return stat;
    }
    
    const char *time = OJson_path_string(file, FS_KEY_TIME);
    if(!time) {
        o_log_error("invalid file, missing " FS_KEY_TIME);
        return stat;
    }
    if(sscanf(time, "%" oi64_SCN, &stat.time) != 1) {
        o_log_error("invalid file, invalid " FS_KEY_TIME);
        return stat;
    }
    
    double *size = OJson_path_number(file, FS_KEY_SIZE);
    if(!size || *size < 0) {
        o_log_error("invalid file, invalid " FS_KEY_SIZE);
        return stat;
    }
    stat.size = (osize) size;
    
    bool *locked = OJson_path_boolean(file, FS_KEY_LOCKED);
    if(!locked) {
        o_log_error("invalid file, missing " FS_KEY_LOCKED);
        return stat;
    }
    stat.locked = *locked;
    
    if(o_str_begins(path, "cloud/")) {
        const char *cloud = OJson_path_string(file, FS_KEY_CLOUD);
        stat.cloud = ms_fs_cloud_ENUM_MAX;
        for(enum ms_fs_cloud c = 0; c < ms_fs_cloud_ENUM_MAX; c++) {
            if(o_str_equals(cloud, FS_CLOUD_STR[c])) {
                stat.cloud = c;
                break;
            }
        }
        if(stat.cloud == ms_fs_cloud_ENUM_MAX) {
            o_log_error("invalid file, invalid " FS_KEY_CLOUD);
            return stat;
        }
    } else {
        stat.cloud = ms_fs_cloud_AVAILABLE;
    }
    
    // create cache
    cache = o_user_new0(file, *cache, 1);
    *cache = stat;
    
    return stat;
}


void ms_fs_sort(char **files, osize num, enum ms_fs_sort sort_mode)
{
    if(sort_mode == ms_fs_sort_IGNORE) {
        return;
    }
    if(num<0) {
        num = o_list_num(files);
    }
    o_qsort(files, sizeof *files, num, &sort_mode, fs_sort_compare);
}


bool ms_fs_touch(const char *path, enum ms_fs_type type)
{
    assert(path && path[0] != '\0');
    assert(type != ms_fs_type_NONE && type>=0 && type < ms_fs_type_ENUM_MAX);
    
    struct ms_fs_stat stat = ms_fs_stat(path);
    if (stat.type!=ms_fs_type_NONE) {
        o_log_error("File already exists: %s", path);
        return false;
    }
    
    oobj container = OObj_new(fs_L.container);
    bool success = false;
    
    char *dir_path = ms_fs_path_dir(container, path);
    oobj dir = OJson_path(fs_L.json_fs, dir_path).o;
    if(!dir) {
        o_log_error("Directoey does not exist: %s", path);
        goto CLEAN_UP;
    }
    
    const char *name = ms_fs_path_name(path);
    if(name[0] == '\0') {
        o_log_error("File has an empty name? %s", path);
        goto CLEAN_UP;
    }
    if(name[0] == '?') {
        o_log_error("File has an invalid name (?*) %s", path);
        goto CLEAN_UP;
    }
    
    // creating the file json entry
    oobj file = OJson_new_object(dir, name);
    
    // creating stat fields
    OJson_new_string(file, FS_KEY_TYPE, FS_TYPE_STR[type]);
    
    char *time = o_strf(container, "%"oi64_PRI, o_time());
    OJson_new_string(file, FS_KEY_TIME, time);
    
    OJson_new_number(file, FS_KEY_SIZE, 0);
    
    OJson_new_boolean(file, FS_KEY_LOCKED, false);
    
    if(o_str_begins(path, "cloud/")) {
        OJson_new_string(file, FS_KEY_CLOUD, FS_CLOUD_STR[ms_fs_cloud_MISSING]);
    }
    
    
    OJson_new_array(file, FS_KEY_FILES);
    OJson_new_array(file, FS_KEY_FILES_IDX);
    
    // creating real file path, according to type
    switch (type) {
    case ms_fs_type_PNG:
        OJson_new_number(file, FS_KEY_IDX_PNG, fs_touch_file_insert(file, ".png"));
        OJson_new_number(file, FS_KEY_IDX_THUMB, fs_touch_file_insert(file, ".png"));
        break;
    default:
        break;
    }
   
    success = true;
    
    CLEAN_UP:
    o_del(container);
    return success;
}


bool ms_fs_exe(const char *path, const char **argv);


bool ms_fs_sync_time(const char *path)
{
    oobj file = OJson_path(fs_L.json_fs, path).o;
    if(!file) {
        return false;
    }
    fs_invalidate_stat_cache(file);
    
    oobj time = OJson_child(file, FS_KEY_TIME).o;
    if(!time) {
        o_log_error("invalid file, missing " FS_KEY_TIME " : %s", path);
        return false;
    }
    oi64 ct = o_time();
    char buf[32];
    o_strf_buf(buf, "%" oi64_PRI, ct);
    OJson_string_set(time, buf);
    return true;
}


bool ms_fs_sync(const char *path)
{
    struct ms_fs_stat stat = ms_fs_stat(path);
    if (stat.type==ms_fs_type_NONE) {
        o_log_error("File does not exist: %s", path);
        return false;
    }

    if (stat.type==ms_fs_type_PNG) {
        struct fs_thumb_job job = {0};
        job.container = OObj_new(fs_L.thumb_job_list);
        bool valid;
        valid = ms_fs_file_png(path, job.img_file);
        valid = valid && ms_fs_file_png(path, job.thumb_file);
        assert(valid);
        OArray_push(fs_L.thumb_job_list, &job);
    }

    return false;
}

char *ms_fs_path_dir(oobj parent, const char *path)
{
    char *dir = o_str_clone(parent, path);
    osize slash = o_str_find_back_char(dir, '/');
    dir[o_max(0, slash)] = '\0';
    return dir;
}



const char *ms_fs_path_name(const char *path)
{
    osize slash = o_str_find_back_char(path, '/');
    return path + o_max(0, slash+1);
}


oobj ms_fs_file_list(oobj parent, const char *path)
{
    oobj list = OList_new(parent, NULL, 0);
    oobj file = OJson_path(fs_L.json_fs, path).o;
    if(!file) {
        return list;
    }
    oobj files = OJson_child(file, FS_KEY_FILES).o;
    if(!files) {
        return list;
    }
    OJson **json_it = OJson_list(files, NULL);
    
    for(OJson **it = json_it; *it; it++) {
        const char *f = OJson_string(*it);
        if (!f) {
            o_log_error("invalid file in " FS_KEY_FILES " : %s", path);
            continue;
        }
        char *add = o_str_clone(list, f);
        OList_push(list, add);
    }
    o_free(files, json_it);

    return list;
}


const char *ms_fs_file_png(const char *path, char *opt_buffer)
{
    o_thread_assert_main();
    static char file[ms_fs_file_MAX];
    char *res = o_or(opt_buffer, file);
    
    ou16 idx = fs_key_idx(path, FS_KEY_IDX_PNG);
    if(idx==0) {
        res[0] = '\0';
        return NULL;
    }
    
    snprintf(res, ms_fs_file_MAX, "ms_file_%" ou16_PRI ".png", idx);
    return res;
}


const char *ms_fs_file_thumb(const char *path, char *opt_buffer)
{
    o_thread_assert_main();
    static char file[ms_fs_file_MAX];
    char *res = o_or(opt_buffer, file);
    
    ou16 idx = fs_key_idx(path, FS_KEY_IDX_THUMB);
    if(idx==0) {
        res[0] = '\0';
        return NULL;
    }
    
    snprintf(res, ms_fs_file_MAX, "ms_file_%" ou16_PRI ".png", idx);
    return res;
}

