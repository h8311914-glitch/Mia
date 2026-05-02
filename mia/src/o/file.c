#ifdef MIA_OPTION_SDL2
#  include <SDL2/SDL_rwops.h>
#  include <SDL2/SDL_system.h>
#else
#  include <SDL3/SDL_iostream.h>
#  include <SDL3/SDL_dialog.h>
#  include <SDL3/SDL_filesystem.h>
#  include <SDL3/SDL_timer.h>
#endif

#include "o/file.h"
#include "o/str.h"
#include "o/OStreamSdl.h"
#include "o/OObjRoot.h"  // for internal work
#include "o/OArray.h"
#include "o/OList.h"
#include "o/OPtr.h"
#include "o/OEvent.h"
#include "o/ODelcallback.h"

// list dir and check file type stuff
#include "file_dirent.h"
#include <stdlib.h>
#include <sys/stat.h>

// mkdirs
#ifdef MIA_PLATFORM_WINDOWS
#include <direct.h>
#else
#include <unistd.h>
#endif


#undef O_LOG_LIB
#define O_LOG_LIB "o"

#include "o/log.h"


static struct {
    char format_mia_tmp[16];
    char format_mia_save[16];


#ifdef MIA_PLATFORM_EMSCRIPTEN
    oobj sync_load_root;
    oobj sync_load_event_list;
    bool sync_load;
#endif

} file_L;

static _Thread_local bool file_L_root_quiet = false;

typedef void (*o_file_event_delegate_fn)(o_file_event_fn fn, void *user_data, const char *file,
                                         char *opt_user_file, bool success);


// protected
void o_file__init(void)
{
    bool got_mia_tmp_dir;
    bool got_mia_save_dir;

    // create mia_save dir (and mount in emscripten)
#ifdef MIA_PLATFORM_EMSCRIPTEN
    // protected
    void o_file_route_save__idbfs_mount(void);
    o_file_route_save__idbfs_mount();
    got_mia_tmp_dir = true;
    got_mia_save_dir = true;

#else
    file_L_root_quiet = true;
    got_mia_tmp_dir = o_file_mkdirs("mia_tmp");
    got_mia_save_dir = o_file_mkdirs("mia_save");
    file_L_root_quiet = false;
#endif


    if (got_mia_tmp_dir) {
        o_str_copy(file_L.format_mia_tmp, "mia_tmp/%s");
        o_str_copy(file_L.format_mia_save, "mia_save/%s");
    } else {
        o_str_copy(file_L.format_mia_tmp, "mia_tmp.%s");
        o_str_copy(file_L.format_mia_save, "mia_save.%s");
    }
}

void o_file_route_resolve(char *out_resolved_path, const char *path)
{
    if (!path || path[0] == '\0') {
        out_resolved_path[0] = '\0';
    }
    enum o_file_route_type type = o_file_route_type(path);
    int written = 0;
    switch (type) {
        case o_file_route_TMP:
            written = snprintf(out_resolved_path, O_FILE_PATH_MAX, file_L.format_mia_tmp, path + 1);
            assert(written<O_FILE_PATH_MAX && out_resolved_path[written]=='\0');
            return;
        case o_file_route_RES:
            written = snprintf(out_resolved_path, O_FILE_PATH_MAX, "mia_res/%s", path + 1);
            assert(written<O_FILE_PATH_MAX && out_resolved_path[written]=='\0');
            return;
        case o_file_route_SAVE:
            written = snprintf(out_resolved_path, O_FILE_PATH_MAX, file_L.format_mia_save, path + 1);
            assert(written<O_FILE_PATH_MAX && out_resolved_path[written]=='\0');
            return;
        case o_file_route_NATIVE:
#ifdef MIA_OPTION_FILE_WARN_NATIVE
            if (!file_L_root_quiet) {
                o_log_warn_s(__func__, "Native route used! Have a look at \"o/file.h\"! Route: %s", path);
            }
#endif
            written = snprintf(out_resolved_path, O_FILE_PATH_MAX, "%s", path);
            assert(written<O_FILE_PATH_MAX && out_resolved_path[written]=='\0');
            return;
        default:
            out_resolved_path[0] = '\0';
            return;
    }

    // to turn off warnings that its not used in release mode
    o_noop(&written);
}

#ifdef MIA_PLATFORM_EMSCRIPTEN
// protected called if sync is true from a_app
void o_file__route_sync_loaded()
{
    if (!file_L.sync_load && file_L.sync_load_root && OList_num(file_L.sync_load_event_list) > 0) {
        while (OList_num(file_L.sync_load_event_list) > 0) {
            oobj ptr = OList_pop(file_L.sync_load_event_list);
            oobj event = OPtr_get(ptr).o;
            if (event) {
                OEvent_post(event);
            }
            o_del(ptr);
        }
    }
    file_L.sync_load = true;
}
#endif

void o_file_route_save_sync_import(oobj opt_loaded_event)
{
#ifdef MIA_PLATFORM_EMSCRIPTEN
    if (!file_L.sync_load_root && opt_loaded_event) {
        // new unregistered root
        file_L.sync_load_root = OObjRoot_new(NULL, o_allocator_heap_new(), false);
        file_L.sync_load_event_list = OList_new(file_L.sync_load_root, NULL, 0);
    }
    if (opt_loaded_event) {
        oobj ptr = OPtr_new(file_L.sync_load_root, opt_loaded_event);
        OList_push(file_L.sync_load_event_list, ptr);
    }
    void o_file_route_save__idbfs_import(void);
    o_file_route_save__idbfs_import();
    file_L.sync_load = false;

#else
    if (opt_loaded_event) {
        // noop, post immediatly
        OEvent_post(opt_loaded_event);
    }
#endif
}

void o_file_route_save_sync_export(void)
{
#ifdef MIA_PLATFORM_EMSCRIPTEN
    void o_file_route_save__idbfs_export(void);
    o_file_route_save__idbfs_export();
#endif // else noop
}

void o_file_route_save(const char *path)
{
    enum o_file_route_type type = o_file_route_type(path);
    if (type == o_file_route_SAVE) {
        o_file_route_save_sync_export();
    } else if (type == o_file_route_RES) {
        o_log_wtf_s(__func__, "You should not write to the res(ources)! Route: %s", path);
    }
}

bool o_file_route_quiet(void)
{
    return file_L_root_quiet;
}


bool o_file_route_quiet_set(bool set)
{
    file_L_root_quiet = set;
    return set;
}

static void o_file_open__stream_del(oobj obj)
{
    o_log_trace_s("o_file_open", "save stream deleted, syncing");
    o_file_route_save_sync_export();
}

struct oobj_opt o_file_open(oobj parent, const char *file, const char *mode)
{
    if(!file) {
        return oobj_opt(NULL);
    }
    char routed[O_FILE_PATH_MAX];
    o_file_route_resolve(routed, file);

    bool read_only = o_str_equals(mode, "r") || o_str_equals(mode, "rb");

#ifdef MIA_OPTION_SDL2
    SDL_RWops *sdl_stream = SDL_RWFromFile(routed, mode);
#else
    SDL_IOStream *sdl_stream = SDL_IOFromFile(routed, mode);
#endif

    if (!sdl_stream) {
        return oobj_opt(NULL);
    }
    OStreamSdl *stream = OStreamSdl_new(parent, sdl_stream);

    char buf[64];
    o_strf_buf(buf, "Stream:%s", file);
    OObj_name_set(stream, buf);

    if (!read_only) {
        enum o_file_route_type type = o_file_route_type(file);
        if (type == o_file_route_SAVE) {
            ODelcallback_new(stream, o_file_open__stream_del);
        } else if (type == o_file_route_RES) {
            o_log_wtf_s(__func__, "You should not write to the res(ources)! Route: %s", file);
        }
    }

    return oobj_opt(stream);
}

struct oobj_opt o_file_read(oobj parent, const char *file, bool ascii, osize element_size)
{
    if(!file) {
        return oobj_opt(NULL);
    }
    oobj data = NULL;
    struct oobj_opt stream = o_file_open(parent, file, ascii ? "r" : "rb");
    if (!stream.o) {
        o_log_debug_s(__func__, "failed to open file: %s", file);
        goto CLEAN_UP;
    }
    osize size = OStream_size(stream.o);
    if (size <= 0) {
        o_log_debug_s(__func__, "failed to read file size: %s", file);
        goto CLEAN_UP;
    }
    osize num = size / element_size;

    if (size % element_size != 0) {
        o_log_debug_s(__func__,
                      "warning: skipped some bytes: size % element_size = %i",
                      size % element_size);
    }

    data = OArray_new(parent, NULL, element_size, num);
    osize read = OStream_read(stream.o, OArray_data_void(data), element_size, num);
    if (read != num) {
        o_log_debug_s(__func__,
                      "warning: read fewer bytes than expacted: %i/%i",
                      read, num);
        OArray_resize(data, read);
    }


    char buf[64];
    o_strf_buf(buf, "Data:%s", file);
    OObj_name_set(data, buf);

CLEAN_UP:
    o_del(stream.o);
    return oobj_opt(data);
}

osize o_file_write(const char *file, bool ascii, const void *data, osize element_size, osize num)
{
    if(!file) {
        return 0;
    }
    // new unregistered root
    oobj root = OObjRoot_new(NULL, o_allocator_heap_new(), false);
    osize written = 0;
    struct oobj_opt stream = o_file_open(root, file, ascii ? "w" : "wb");
    if (!stream.o) {
        o_log_debug_s(__func__, "failed to open file: %s", file);
        goto CLEAN_UP;
    }
    written = OStream_write(stream.o, data, element_size, num);

CLEAN_UP:
    o_del(root);
    return written;
}

osize o_file_append(const char *file, bool ascii, const void *data, osize element_size, osize num)
{
    if(!file) {
        return 0;
    }
    // new unregistered root
    oobj root = OObjRoot_new(NULL, o_allocator_heap_new(), false);
    osize written = 0;
    struct oobj_opt stream = o_file_open(root, file, ascii ? "a" : "ab");
    if (!stream.o) {
        o_log_debug_s(__func__, "failed to open file: %s", file);
        goto CLEAN_UP;
    }
    written = OStream_write(stream.o, data, element_size, num);

CLEAN_UP:
    o_del(root);
    return written;
}


osize o_file_clone(const char *dst, const char *src)
{
    if(!dst || !src) {
        return 0;
    }
    // new unregistered root
    oobj root = OObjRoot_new(NULL, o_allocator_heap_new(), false);
    struct oobj_opt stream_dst = o_file_open(root, dst, "wb");
    struct oobj_opt stream_src = o_file_open(root, src, "rb");

    osize cloned_bytes = 0;

    ou8 buffer[1024];
    while (OStream_valid(stream_dst.o) && OStream_valid(stream_src.o)) {
        osize read = OStream_read_try(stream_src.o, buffer, 1, sizeof buffer);
        OStream_write(stream_dst.o, buffer, 1, read);
        cloned_bytes += read;
    }
    o_del(root);

    return cloned_bytes;
}

bool o_file_remove(const char *file)
{
    if (!file || file[0] == '\0' || o_file_route_type(file) == o_file_route_RES) {
        return false;
    }

    char routed[O_FILE_PATH_MAX];
    o_file_route_resolve(routed, file);

    bool file_removed;
#ifdef MIA_OPTION_SDL2
    file_removed = remove(routed) == 0;
#else
    file_removed = SDL_RemovePath(routed);
#endif

    if (!file_removed) {
        o_log_warn_s(__func__, "failed to remove file: %s", file);
    }
    return file_removed;
}

struct o_file_info o_file_info(const char *file)
{
    struct o_file_info info = {0};
    if(!file) {
        return info;
    }

    char routed[O_FILE_PATH_MAX];
    file_L_root_quiet = true;
    o_file_route_resolve(routed, file);
    file_L_root_quiet = false;

#ifndef MIA_OPTION_SDL2
    SDL_PathInfo sdl_info;
    bool valid = SDL_GetPathInfo(routed, &sdl_info);
    if (!valid) {
        return info;
    }
    switch (sdl_info.type) {
        default:
        case SDL_PATHTYPE_NONE:
            info.type = o_file_info_type_NONE;
            break;
        case SDL_PATHTYPE_FILE:
            info.type = o_file_info_type_FILE;
            break;
        case SDL_PATHTYPE_DIRECTORY:
            info.type = o_file_info_type_DIR;
            break;
    }
    info.size = sdl_info.size;
    info.time_modify = SDL_NS_TO_SECONDS(sdl_info.modify_time);
    info.time_access = SDL_NS_TO_SECONDS(sdl_info.access_time);
    return info;
#else

    struct stat sb;
    if (stat(routed, &sb) == 0) {
        if (S_ISDIR(sb.st_mode)) {
            info.type = o_file_info_type_DIR;
        } else if (S_ISREG(sb.st_mode)) {
            info.type = o_file_info_type_FILE;
        } else {
            info.type = o_file_info_type_OTHER;  // everything else
        }
        info.size = sb.st_size;
        info.time_modify = sb.st_mtime;
        info.time_access = sb.st_atime;
    }
    return info;
#endif
}


osize o_file_size(const char *file, bool use_info)
{
    if(!file) {
        return -1;
    }
    if(use_info) {
        struct o_file_info info = o_file_info(file);
        return info.type == o_file_info_type_FILE? info.size : -1;
    }
    
    // new unregistered root
    oobj root = OObjRoot_new(NULL, o_allocator_heap_new(), false);
    
    osize size = -1;
    oobj stream = o_file_open(root, file, "rb").o;
    if(!stream) {
        goto CLEAN_UP;
    }
    
    osize offset = OStream_seek(stream, 0, OStream_END);
    if(offset>0) {
        size = offset;
    }
    OStream_close(stream);
    
    CLEAN_UP:
    o_del(root);
    return size;
}



bool o_file_mkdirs(const char *path)
{
    if(!path) {
        return false;
    }
    char routed[O_FILE_PATH_MAX];
    file_L_root_quiet = true;
    o_file_route_resolve(routed, path);
    file_L_root_quiet = false;

#ifndef MIA_OPTION_SDL2
    return SDL_CreateDirectory(routed);
#else

#  ifdef MIA_PLATFORM_WINDOWS
#    define MKDIR(path) _mkdir(path)
#  else
#    define MKDIR(path) mkdir(path, 0755)
#  endif

    osize len = o_strlen(routed);
    if (len == 0) {
        return false;
    }

    // remove ending slash
    if (routed[len - 1] == '/') {
        routed[len - 1] = 0;
    }
    for (char *p = routed + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (!o_file_is_dir(routed)) {
                // silently mkdir in the path, could contain system dirs
                MKDIR(routed);
            }
            *p = '/';
        }
    }
    if (!o_file_is_dir(routed)) {
        if (MKDIR(routed) != 0) {
            return false;
        }
    }
    return true;

#undef MKDIR
#endif
}

#ifdef MIA_PLATFORM_DESKTOP
static const char *file_home_user_path(void)
{
#ifdef MIA_PLATFORM_WINDOWS
    static char home_cat[512];
    char *home = getenv("USERPROFILE");
    if (home) {
        o_strf_buf(home_cat, "%s", home);
        home = home_cat;
    } else {
        home = getenv("HOMEDRIVE");
        if (home) {
            char *home_path = getenv("HOMEPATH");
            if (home_path) {
                size_t length = strlen(home) + strlen(home_path) + 1;
                o_strf_buf(home_cat, "%s%s", home, home_path);
                home = home_cat;
            }
        }
    }
    if (home) {
        // use new style and not old...
        o_str_replace_char_this(home, '\\', '/');
    }
    return home;
#else
    return getenv("HOME");
#endif
}
#endif

const char *o_file_home(void)
{
#ifndef MIA_PLATFORM_DESKTOP
    return ".";
#else
    static char home[O_FILE_PATH_MAX] = {0};
    if (home[0] == 0) {
        o_strf_buf(home, "%s", file_home_user_path());
    }
    return home;
#endif
}

static int string_compare_fn(void *array, const void *a, const void *b)
{
    const char **str_a = (const char **) a;
    const char **str_b = (const char **) b;
    return strcmp(*str_a, *str_b);
}


struct o_file_list_context {
    oobj parent;
    oobj items;
    enum o_file_info_type type_filter;
    char **opt_file_filter;
};

#ifndef MIA_OPTION_SDL2
#  define FILE_LIST_NEXT SDL_ENUM_CONTINUE
static SDL_EnumerationResult file_list_callback(void *userdata, const char *dirname, const char *fname)
#else
#  define FILE_LIST_NEXT 1
static int file_list_callback(void *userdata, const char *dirname, const char *fname)
#endif
{
    struct o_file_list_context *C = userdata;
    if (o_str_equals(fname, ".") || o_str_equals(fname, "..")) {
        return FILE_LIST_NEXT;
    }
    bool valid = true;
    if (C->opt_file_filter) {
        valid = false;
        char *lower = o_str_tolower(C->parent, fname);
        char **it = C->opt_file_filter;
        while (*it) {
            assert(o_str_islower(*it) && "file_filter strings must be lower case");
            if (o_str_ends(lower, *it++)) {
                valid = true;
                break;
            }
        }
        o_free(C->parent, lower);
    }
    if (!valid) {
        return FILE_LIST_NEXT;
    }
    char *path = o_strf(C->parent, "%s/%s", dirname, fname);
    struct o_file_info info = o_file_info(path);
    if (C->type_filter == o_file_info_type_NONE) {
        valid = info.type != o_file_info_type_NONE;
    } else {
        valid = info.type == C->type_filter;
    }
    o_free(C->parent, path);
    if (!valid) {
        return FILE_LIST_NEXT;
    }
    // create a clone of the string and push that pointer to the list of files
    char *push = o_str_clone(C->items, fname);
    OList_push(C->items, push);
    return FILE_LIST_NEXT;
}

oobj o_file_list(oobj parent, const char *directory, enum o_file_info_type type_filter, char **opt_file_filter)
{
    directory = o_or(directory, ".");

    char routed[O_FILE_PATH_MAX];
    file_L_root_quiet = true;
    o_file_route_resolve(routed, directory);
    file_L_root_quiet = false;

    oobj items = OList_new_ex(parent, NULL, 0, 256, OArray_REALLOC_DEFAULT);

    // always add ".." prev dir if not files only
    if (type_filter != o_file_info_type_FILE) {
        char *push = o_str_clone(items, "..");
        OList_push(items, push);
    }
    struct o_file_list_context C = {parent, items, type_filter, opt_file_filter};
#ifndef MIA_OPTION_SDL2
    bool valid = SDL_EnumerateDirectory(routed, file_list_callback, &C);
    if (!valid) {
        o_log_debug_s(__func__, "o_file_list failed for: \"%s\"", directory);
        OList_clear(items);
        return items;
    }

#else
    DIR *dir;
    struct dirent *entry;
    dir = opendir(routed);
    if (dir == NULL) {
        o_log_debug_s(__func__, "o_file_list failed for: \"%s\"", directory);
        OList_clear(items);
        return items;
    }
    while ((entry = readdir(dir)) != NULL) {
        const char *entry_name = entry->d_name;
        file_list_callback(&C, routed, entry_name);
    }
    closedir(dir);
#endif

    // sort list of dir files
    OArray_sort(OList_array(items), string_compare_fn);
    return items;
}
