#ifndef MS_MSFILE_H
#define MS_MSFILE_H

/**
 * @file MSFile.h
 *
 * object.
 *
 * A MiaStudio "file".
 * May be smth like an image or say a real file.
 * But also valid to be a dir, an exe like MiaPaint, or sync to cloud, etc.
 */

#include "o/OObj.h"
#include "m/types/flt.h"


// maximal o_strlen of name, so buffer needs be bigger
#define MSFile_name_MAX_LEN 16

static const vec2 MSFile_ICON_SIZE = {{32, 32}};
static const vec2 MSFile_RENDER_SIZE = {{32, 48}};

enum MSFile_type {
    MSFile_type_EXE,
    MSFile_type_IMG,
    MSFile_type_ENUM_MAX,
};


/**
 * If a file can be executed, this virtual function is called.
 * Or as a test if that file is valid to be loaded / executed.
 * Example would be MiaPaint (obj) either starts NULL or a given image in opt_file
 * @param obj MSFile that will be executed (where this function is called)
 * @param opt_file may be NULL or an MSFile to open with obj
 * @return true if file can be executed
 */
typedef bool (*MSFile__exe_fn)(oobj obj, oobj opt_file);

/** object id */
#define MSFile_ID OObj_ID "MSFile."

typedef struct {
    OObj super;

    // internal file path / name
    // examples: "img01", "bar/img02" "bar/foo/sound"
    // each path (not the whole) name is limited to MSFile_name_MAX_LEN (bar, foo, sound)
    char *path;

    enum MSFile_type type;
    
    // if false, it renders a bit transparent
    // and opt_exe will be a noop
    bool enabled;

    // true if drag and drop to other directories is supported
    bool moveable;

    // optional execute function, used for smth like Mia Paint (which is a file :D)
    MSFile__exe_fn opt_exe;
    // optional test if given MSFile is executable
    MSFile__exe_fn opt_file_executable;

    // OList of MSFile's that support opening this file.
    // on a click on this file, a list of action is shown.
    // exactly this list here...
    // top is last opened with
    // double click is starting the top [0] entry
    // even copy or rename are special MSFile (executables)
    oobj executables;

    // RTex of the icon and its name
    oobj thumb;
} MSFile;


/**
 * Initializes the object.
 * @param obj MSFile object
 * @param parent to inherit from
 * @return obj casted as MSFile
 */
MSFile *MSFile_init(oobj obj, oobj parent, const char *path, enum MSFile_type type,
                    oobj icon, bool moveable, MSFile__exe_fn opt_exe, MSFile__exe_fn opt_file_executable,
                    oobj executables, bool move_executables);

/**
 * Creates a new MSFile object.
 * @param parent to inherit from
 * @return The new object
 */
o_inline MSFile *MSFile_new(oobj parent, const char *path, enum MSFile_type type,
                    oobj icon, bool moveable, MSFile__exe_fn opt_exe, MSFile__exe_fn opt_file_executable,
                    oobj executables, bool move_executables)
{
    OObj_DECL_IMPL_NEW(MSFile, parent, path, type, icon, moveable, opt_exe, opt_file_executable, executables, move_executables);
}


MSFile *MSFile_new_dir(oobj parent, const char *path);
MSFile *MSFile_new_img(oobj parent, const char *path);

//
// object functions:
//


/**
 * @param obj MSFile object
 * @return RTex of the icon + name
 */
OObj_DECL_GET(MSFile, oobj, thumb)

/**
 * @param obj MSFile object
 * @return if false, it renders a bit transparent
 *         and opt_exe will be a noop
 */
OObj_DECL_GETSET(MSFile, bool, enabled)

/**
 * @param obj MSFile object
 * @return internal file path / name
 *         examples: "img01", "bar/img02" "bar/foo/sound"
 *         each path (not the whole) name is limited to MSFile_name_MAX_LEN (bar, foo, sound)
 */
OObj_DECL_GET(MSFile, const char *, path)


/**
 * @param obj MSFile object
 * @return only the file name of the path (at max MSFile_name_MAX_LEN characters)
 */
const char *MSFile_name(oobj obj);

/**
 * @param obj MSFile object
 * @return file type
 */
OObj_DECL_GET(MSFile, enum MSFile_type, type)

/**
 * @param obj MSFile object
 * @return true if moving directories by drag and drop is allowed
 */
OObj_DECL_GET(MSFile, bool, moveable)

/**
 * @param obj MSFile object
 * @return OList of MSFile's that support opening this file
 */
OObj_DECL_GET(MSFile, oobj, executables)


/**
 * Executes that file
 * @param obj MSFile object to execute
 * @param opt_file MSFile object to pass "open with" to obj, or NULL
 * @note noop if not enabled or opt_exe is not available
 */
void MSFile_exe(oobj obj, oobj opt_file);

/**
 * Tests if that file is executable
 * @param obj MSFile object to execute
 * @param file MSFile object to pass "open with" to obj, or NULL
 * @return true if that file is executable
 * @note ignores enabled flag
 */
bool MSFile_file_executable(oobj obj, oobj file);

/**
 * Renders the MSFile onto an RTex
 * @param obj MSFile object
 * @param tex RTex to render to
 * @param x, y: left top position
 * @note render size is fixed, see MSFile_RENDER_SIZE
 */
void MSFile_render(oobj obj, oobj tex, float x, float y);

#endif //MS_MSFILE_H
