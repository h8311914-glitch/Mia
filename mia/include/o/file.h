#ifndef O_FILE_H
#define O_FILE_H

/**
 * @file file.h
 *
 * Open files as streams or into memory.
 *
 * All path's in mia are using routes (except for o_file_route_* stuff)
 * Routes:
 *  - "#image.png"
 *      *Temporary files*
 *      Resolved to smth like "mia_tmp.image.png" on most platforms
 *      **May or may not be available in the next app start**
 *  - "$logo16.png" or "$ex/tea_cup.png"
 *      *Resources*
 *      Static resourced placed under "$" like "$logo16.png" or "$ex/tea_cup.png"
 *      These are the ones created in the "res/" directory (or "apps/res/").
 *      **Read only**
 *  - "&config.json"
 *      *Saving records*
 *      Configs or other stuff saved for the next app load like "mia_save/config.png" or "mia_save.config.json"
 *      **Should be available in the next app start**
 *  - "file_in_wd.txt" || "/home/unix_user/..." || "C:/Users/windows_user/..."
 *      *Escape*
 *      For system paths or wd, use with caution
 *      Usage will log a warning. To turn off, see cmake option MIA_USE_FILE_WARN_NATIVE
 *
 *  @note don't use anything that begins prefixed with "mia_".
 *        Such as mia_res/ or mia_save/ or mia_tmp.*.* or the like.
 *        Those are used on some platforms for the routes
 *
 * @sa OEventFile to up or download files (file picker)
 *
 */


#include "OStream.h"
#include "OArray.h"

/** excluding the null terminator */
#define O_FILE_RECORD_MAX_FILE_LENGTH 128

/** maximal size of a path, including the null terminator */
#define O_FILE_PATH_MAX 4096

enum o_file_info_type {
    o_file_info_type_NONE,
    o_file_info_type_FILE,
    o_file_info_type_DIR,
    o_file_info_type_OTHER,
    o_file_info_type_ENUM_MAX
};

struct o_file_info {
    enum o_file_info_type type;

    // in bytes
    ou64 size;

    // times in seconds since unix epoch, like o_time()
    // on MIA_PLATFORM_EMSCRIPTEN this is somewhat useless!
    // as the file dir lives in ram recreated on boot...
    oi64 time_modify;
    oi64 time_access;
};

/**
 * Different valid routes, see o_file_route_type()
 */
enum o_file_route_type {
    o_file_route_INVALID,
    o_file_route_TMP,
    o_file_route_RES,
    o_file_route_SAVE,
    o_file_route_NATIVE,
    o_file_route_ENUM_MAX
};

/**
 * Optional finished event function, >MAY< be called.
 * @param file is the given file
 * @param opt_user_file may be NULL or the user selected file
 * @param success true if cloned some bytes
 */
typedef void (*o_file_event_fn)(void *user_data, const char *file, const char *opt_user_file, bool success);


/**
 * Returns the routing type of the given path
 */
o_inline enum o_file_route_type o_file_route_type(const char *path)
{
    if (!path || !path[0]) {
        return o_file_route_INVALID;
    }
    switch (path[0]) {
        case '#':
            return o_file_route_TMP;
        case '$':
            return o_file_route_RES;
        case '&':
            return o_file_route_SAVE;
        default:
            break;
    }
    return o_file_route_NATIVE;
}

/**
 * This function resolves a path into the real platform dependent path
 * @param out_resolved_path platform dependent full path according to the route of path
 *                          (buffer with at least O_FILE_PATH_MAX size)
 * @param path smth like: "image.png" or "$logo16.png" or "&config.json"
 */
void o_file_route_resolve(char *out_resolved_path, const char *path);


/**
 * Imports the record/save files.
 * On MIA_PLATFORM_EMSCRIPTEN:  syncs from the indexedDB
 * Else:                        NoOp
 * @param opt_loaded_event optional OEvent that is called if the save dir was loaded.
 *                         On MIA_PLATFORM_EMSCRIPTEN: post if synced from indexedDB
 *                         Else: post immediately
 * @note Typical apps do not need to call this, as mia does import on startup automatically.
 *       Use case would be 2 webapps loaded in parallel, app a saves smth and app b occasionally (re)imports
 * @warning Actual session data may be lost (in MIA_PLATFORM_EMSCRIPTEN)!
 *          Imagine the webapp serves on two browser tabs, A and B.
 *          The app has an "incr" btn that reads a save file ascii number, increments it and saves back (synced).
 *          And a "sync" btn.
 *          User may do:
 *          - Tab A click 10x on incr -> cnt=10
 *          - Tab A reload browser page -> cnt=10
 *          - Tab B opens -> cnt=10
 *          - Tab A clicks 10x on incr -> cnt=20
 *          - Tab B clicks 1x on incr -> cnt=11 (as Tab B was still on 10)
 *          - Tab A clicks sync -> cnt=11 (data lost)
 *          So a better approach is (in app user code):
 *          On the sync btn:
 *          - first clone the save file to a tmp file
 *          - now call this function to import indexedDB files
 *          - merge or use more up to date manually
 *          - export again
 */
void o_file_route_save_sync_import(oobj opt_loaded_event);

/**
 * Syncs the record/save files.
 * Should be called after writing / appending a save file so sync and save.
 * On MIA_PLATFORM_EMSCRIPTEN:  syncs to the indexedDB
 * Else:                        NoOp
 * @note no need to be threadsafe, emscripten is single thread in this framework
 */
void o_file_route_save_sync_export(void);

/**
 * Calls o_file_route_save_sync if the path was in fact a save path.
 * (Only needed for emscripten)
 * @param path route path (not resolved)
 */
void o_file_route_save(const char *path);


/**
 * Set warnings for native routes, etc. on or off (threadlocal)
 */
bool o_file_route_quiet(void);

/**
 * Set warnings for native routes, etc. on or off (threadlocal)
 */
bool o_file_route_quiet_set(bool set);

/**
 * Create a block in which the warnings on file routes are set to quiet (o_log_set_quiet)
 * Use continue to leave the block.
 * @threadsafe
 * @note May be nested, but needs to be in another line (uses __LINE__ internally).
 *       DO NOT:
 *          return in the block!
 *          break in the block! (this block in a for loop...)
 */
#define o_file_route_quiet_block \
for(bool O_NAME_CONCAT(o_file_route_quiet_block__run_, __LINE__) = (o_file_route_quiet_set(true), true); \
O_NAME_CONCAT(o_file_route_quiet_block__run_, __LINE__); \
O_NAME_CONCAT(o_file_route_quiet_block__run_, __LINE__) = (o_file_route_quiet_set(false), false))

/**
 * Opens a file as OStream.
 *
 * The text below is a copy of SDL
 *
 * The `mode` string is treated roughly the same as in a call to the C
 * library's fopen(), even if SDL doesn't happen to use fopen() behind the
 * scenes.
 *
 * Available `mode` strings:
 *
 * - "r": Open a file for reading. The file must exist.
 * - "w": Create an empty file for writing. If a file with the same name
 *   already exists its content is erased and the name is treated as a new
 *   empty file.
 * - "a": Append to a file. Writing operations append data at the end of the
 *   file. The file is created if it does not exist.
 * - "r+": Open a file for update both reading and writing. The file must
 *   exist.
 * - "w+": Create an empty file for both reading and writing. If a file with
 *   the same name already exists its content is erased and the file is
 *   treated as a new empty file.
 * - "a+": Open a file for reading and appending. All writing operations are
 *   performed at the end of the file, protecting the previous content to be
 *   overwritten. You can reposition (fseek, rewind) the internal pointer to
 *   anywhere in the file for reading, but writing operations will move it
 *   back to the end of file. The file is created if it does not exist.
 *
 * **NOTE**: In order to open a file as a binary file, a "b" character has to
 * be included in the `mode` string. This additional "b" character can either
 * be appended at the end of the string (thus making the following compound
 * modes: "rb", "wb", "ab", "r+b", "w+b", "a+b") or be inserted between the
 * letter and the "+" sign for the mixed modes ("rb+", "wb+", "ab+").
 * Additional characters may follow the sequence, although they should have no
 * effect. For example, "t" is sometimes appended to make explicit the file is
 * a text file.
 *
 * This function supports Unicode filenames, but they must be encoded in UTF-8
 * format, regardless of the underlying operating system.
 *
 * @param parent OStream will be a resource of parent.
 * @param file a UTF-8 string representing the filename to open (route path) (NULL safe)
 * @param mode an ASCII string representing the mode to be used for opening.
 *             the file.
 * @return an OStream object to read and write with the file. NULL on error
 * @note installs an ODelcallback on the returned stream if file route was for saving (to sync)
 */
struct oobj_opt o_file_open(oobj parent, const char *file, const char *mode);


/**
 * Reads in the full given file.
 * @param parent OArray will be a resource of parent.
 * @param file filename to open (route path) (NULL safe).
 * @param ascii If true, the file will be read in ascii mode, else binary mode
 * @param element_size For the OArray, (sizeof(...))
 * @return A new OArray object, containing the full file, or NULL on error
 */
struct oobj_opt o_file_read(oobj parent, const char *file, bool ascii, osize element_size);


/**
 * Writes into the given file (NULL safe)
 * @param file filename to save in (route path).
 * @param ascii If true, the file will be written in ascii mode, else binary mode
 * @param data data to write
 * @param element_size sizeof(...)
 * @param num of elements to write
 * @return number of written elements (!=num on failure)
 */
osize o_file_write(const char *file, bool ascii, const void *data, osize element_size, osize num);



/**
 * Appends to the given file (NULL safe)
 * @param file filename to save in (route path).
 * @param ascii If true, the file will be written in ascii mode, else binary mode
 * @param data data to write
 * @param element_size sizeof(...)
 * @param num of elements to write
 * @return number of written elements (!=num on failure)
 */
osize o_file_append(const char *file, bool ascii, const void *data, osize element_size, osize num);

/**
 * Clones file src into file dst
 * @param dst file to be cloned into (route path) (NULL safe).
 * @param src file to be cloned from (route path) (NULL safe).
 * @return bytes written | cloned
 */
osize o_file_clone(const char *dst, const char *src);
/**
 * @param file path (route path) (NULL safe)
 * @return true if that file got removed successfully
 * @note returns false for res routes ($)
 */
bool o_file_remove(const char *file);

/**
 * @param file path (route path) (NULL safe)
 * @return file info, type = o_file_info_type_NONE (all is 0) if failed or it does not exist
 */
struct o_file_info o_file_info(const char *file);

/**
 * @param file path (route path) (NULL safe)
 * @return true if that file is a directory
 */
o_inline bool o_file_is_dir(const char *file)
{
    return o_file_info(file).type == o_file_info_type_DIR;
}

/**
 * @param file path (route path) (NULL safe)
 * @return true if that file is a regular file
 */
o_inline bool o_file_is_regular(const char *file)
{
    return o_file_info(file).type == o_file_info_type_FILE;
}


/**
 * @param file path (route path) (NULL safe)
 * @param use_info true: returns o_file_info().size
 *                 false: uses o_file_open() 
 *                        and calls OStream_seek() 
 *                            to retrieve the size
 * @return file size in bytes or -1 for an error
 */
osize o_file_size(const char *file, bool use_info);


/**
 * @param path directory path to create (route path) (NULL safe)
 * @return true if path was created successfully or already existed
 */
bool o_file_mkdirs(const char *path);


/**
 * Home dir of the user
 * @return a static const null terminated string to the apps home dir
 * @note only works for MIA_PLATFORM_DESKTOP, returns "." on other platforms (may not be valid at all)
 */
const char *o_file_home(void);

/**
 * @param parent to allocate on
 * @param directory to list the files in, NULL safe (-> ".") (route path)
 * @param type_filter different modes to only list dirs or files, etc, use o_file_info_type_NONE / 0 to ignore.
 * @param opt_file_filter a null terminated list of strings of file endings (case is ignored)
 * @return OList of file strings (char *) (empty on access error)
 */
oobj o_file_list(oobj parent, const char *directory, enum o_file_info_type type_filter, char **opt_file_filter);


#endif //O_FILE_H
