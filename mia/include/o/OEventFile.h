#ifndef O_OEVENTFILE_H
#define O_OEVENTFILE_H

/**
 * @file OEventFile.h
 *
 * Object (derives OEvent)
 *
 * Opens a file dialog for up or downloading.
 */


#include "OEvent.h"


/** object id */
#define OEventFile_ID OEvent_ID "File."

struct OEventFile_response {
    bool error;

    char *file;
    char *opt_user_file;
};

typedef struct {
    OEvent super;
    void *impl;

    // valid if OEvent_ready()
    struct OEventFile_response response;
} OEventFile;

/**
* Creates a new OEventFile object.
 * This function can be used to init a derived OJoin (for thread safe stuff as an example)
 * @param object_size size to allocate (asserts >= sizeof(OJoin)
 * @param parent to inherit from
 * @param opt_on_ready event on_ready function, or NULL to ignore
 * @param opt_context optional OContext applied before calling on_ready will be cloned, like o_context()
 *                    If apps need a_view, or a_cam, or a_pointer, or want to create an AScene's,
 *                        a valid app state is needed, pass o_context() to capture the current.
 * @return obj casted as OEventFile
 * @note OEvent_post is called internally on a fetch result, DO NOT CALL IT
 *       Call OEventFile_run to start fetching
 */
OEventFile *OEventFile_new_super(osize object_size, oobj parent, OObj__event_fn opt_on_ready, oobj opt_context);

/**
 * Creates a new OEventFile object
 * @param parent to inherit from
 * @param opt_on_ready event on_ready function, or NULL to ignore
 * @param opt_context optional OContext applied before calling on_ready will be cloned, like o_context()
 *                    If apps need a_view, or a_cam, or a_pointer, or want to create an AScene's,
 *                        a valid app state is needed, pass o_context() to capture the current.
 * @return The new object
 * @note OEvent_post is called internally on a fetch result, DO NOT CALL IT
 *       Call OEventFile_run to start fetching
 */
o_inline OEventFile *OEventFile_new(oobj parent, OObj__event_fn opt_on_ready, oobj opt_context)
{
    return OEventFile_new_super(sizeof(OEventFile), parent, opt_on_ready, opt_context);
}

//
// object functions:
//

/**
 * Starts downloading (dialog)
 * @param obj OEventFile object
 * @param file to download
 * @return The new object
 * @note OEvent_post is called internally on a result, DO NOT CALL IT
 */
void OEventFile_download(oobj obj, const char *file);

/**
 * Starts uploading (dialog)
 * @param obj OEventFile object
 * @param file to upload into. Also may set a file filter.
 *             For images: To enable both png and jpg as a source, choose ".image" file extension
 *             List of supported extensions:
 *             - .png
 *             - .jpg ; .jpeg
 *             - .image -> .png ; .jpg ; .jpeg
 *             - .gif
 *             - .json
 *             - .tar
 * @return The new object
 * @note OEvent_post is called internally on a result, DO NOT CALL IT
 */
void OEventFile_upload(oobj obj, const char *file);

/**
 * @param obj OEventFile object
 * @return the file response, only valid if OEvent_ready is true (which is true in the optional callback)
 */
OObj_DECL_GET(OEventFile, struct OEventFile_response, response)



#endif //O_OEVENTFILE_H
