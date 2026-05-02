#ifndef MS_FS_H
#define MS_FS_H

/**
 * @file fs.h
 *
 * MiaStudio's virtual file system
 *
 * Like unix files, even directories and executables are "files"
 *
 * Reserved invisible dirs:
 * ".bin" for executables like .bin/paint
 * ".recent" for the 16 last used files (not dir nor exe)
 * Reserved visible dirs:
 * "cloud" for the sync cloud tree (locked)
 */

#include "o/common.h"
#include "m/types/int.h"

#define ms_fs_file_MAX 32

static const ivec2 ms_fs_THUMB_SIZE = {{32, 32}};

enum ms_fs_type {
    ms_fs_type_NONE,
    ms_fs_type_DIR,
    ms_fs_type_EXE,
    ms_fs_type_PNG,
    ms_fs_type_ENUM_MAX
};

enum ms_fs_sort {
    ms_fs_sort_IGNORE,
    ms_fs_sort_DEFAULT,
    ms_fs_sort_NAME_ASCENDING,
    ms_fs_sort_NAME_DESCENDING,
    ms_fs_sort_TIME_ASCENDING,
    ms_fs_sort_TIME_DESCENDING,
    ms_fs_sort_SIZE_ASCENDING,
    ms_fs_sort_SIZE_DESCENDING,
    ms_fs_sort_ENUM_MAX
};

enum ms_fs_cloud {
    ms_fs_cloud_AVAILABLE,
    ms_fs_cloud_REQUESTED,
    ms_fs_cloud_MISSING,
    ms_fs_cloud_ENUM_MAX
};

struct ms_fs_stat {
    const char *name;
    
    enum ms_fs_type type;
    
    // last modified o_time()
    oi64 time;
    
    // file size in bytes
    osize size;
    
    // if true, mv and rm will fail
    bool locked;
    
    // cloud state, always AVAILABLE if not in the cloud tree
    enum ms_fs_cloud cloud;
};

/**
 * Inits the file fs manager
 * @param parent
 */
void ms_fs_init(oobj parent);

/**
 * Runs updates on the file system, called each frame by main
 */
void ms_fs_update(void);


/**
 * @return in the frame (till next ms_fs_update call) in which the fs got some changes.
 *         Like some newly created thumbnails, etc.
 */
bool ms_fs_changed(void);

/**
 * @return 0 if no work is done currently, >0 for active (bg) jobs like creating thumbnails.
 *         Useful to show progress, or say whats ongoing to do.
 */
int ms_fs_running_jobs(void);

/**
 * Lists files in a directory.
 * does not list files starting with a '.' (except of "..")
 * @param parent to allocate in
 * @param path to list
 * @param sort_mode mode to sort
 * @return OList of files (full path) in that directory.
 * @note some files have fixed returned positions:
 *       ".." is always the first
 *       "cloud" + cloud/sync_up" + "cloud/sync_down" also came first
 */
oobj ms_fs_ls(oobj parent, const char *path, enum ms_fs_sort sort_mode);


/**
 * Move or rename a file (full path)
 * @param from_path current file
 * @param to_path result file
 * @return true on success, false if path already exists
 * @note directories must already exist, else fails
 */
bool ms_fs_mv(const char *from_path, const char *to_path);

/**
 * Copy a file (full path)
 * @param from_path current file
 * @param to_path result file (must not exist, but dir of it)
 * @return true on success
 * @note directories must already exist, else fails
 */
bool ms_fs_cp(const char *from_path, const char *to_path);

/**
 * Removes a file (full path)
 * @param path to be removed
 * @param recursive if true, directories are removed even if not empty.
 *                  if false, directories must be empty.
 * @return true on success
 */
bool ms_fs_rm(const char *path, bool recursive);


/**
 * @param path to get stat from
 * @return file stats, type==ms_fs_NONE if failed
 */
struct ms_fs_stat ms_fs_stat(const char *path);


/**
 * Sorts the given list of (full path) files
 * @param files (full path)
 * @param num of files or <0 for files beeing NULL terminated
 * @param sort_mode mode to sort with
 */
void ms_fs_sort(char **files, osize num, enum ms_fs_sort sort_mode);


/**
 * Creates a new (empty) file.
 * must not have a '.' as first name character in the path and sub names.
 * @param path for the new file, the parent directories must already exist
 * @param type the type for the new file
 * @return true on success
 */
bool ms_fs_touch(const char *path, enum ms_fs_type type);

/**
 * Calls an exe file with a NULL terminated argument vector/list
 * @param path exe to run
 * @param argv arguments to pass (NULL terminated list)
 * @return true on success
 */
bool ms_fs_exe(const char *path, const char **argv);

/**
 * Updates time_last_modified to current o_time().
 * @param path to sync
 * @return true on success
 */
bool ms_fs_sync_time(const char *path);

/**
 * Updates type specific stuff such as thumbnails and calls ms_fs_sync_time
 * @param path to sync
 * @return true on success
 */
bool ms_fs_sync(const char *path);

/**
 * @param parent to allocate on
 * @param path to get its dir from
 * @returns dir (full path) of the given file path.
 * @note if path == "", also "" is returned (special case...)
 * 
 */
char *ms_fs_path_dir(oobj parent, const char *path);

/**
 * @param path to get its name from
 * @returns name of the given file path, uses the same data as path
 * @note if path == "", also "" is returned (special case...)
 * 
 */
const char *ms_fs_path_name(const char *path);


/**
 * Lists real files of a given virtual file
 * @param parent to allocate in
 * @param path file to list
 * @return OList of files (full path) in that directory
 */
oobj ms_fs_file_list(oobj parent, const char *path);


/**
 * Returns the real save file for an image file (.png)
 * @param path to get the route from
 * @param opt_buffer optional buffer, needs to be ms_fs_file_MAX big
 * @return NULL or the .png file path for write and read
 * @note call ms_fs_update after writing.
 *       if opr_buffer is NULL, only valid until next call.
 */
const char *ms_fs_file_png(const char *path, char *opt_buffer);

/**
 * Returns the real save file for a generated thumbnail .png
 * @param path to get the route from
 * @param opt_buffer optional buffer, needs to be ms_fs_file_MAX big
 * @return NULL or the .png file path to read
 * @note generated by ms_fs_update.
 *       if opr_buffer is NULL, only valid until next call.
 */
const char *ms_fs_file_thumb(const char *path, char *opt_buffer);

#endif //MS_FS_H
