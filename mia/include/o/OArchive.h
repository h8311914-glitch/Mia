#ifndef O_OARCHIVE_H
#define O_OARCHIVE_H

/**
 * @file OArchive.h
 *
 * Object
 *
 * Loads and writes to different archives file types:
 * - .tar
 * - .zip (needs MIA_OPTION_ZIP)
 * 
 * tar makes use of microtar: "https://github.com/rxi/microtar"
 * 
 * zip makes use of miniz "https://github.com/richgel999/miniz"
 *     (needs MIA_OPTION_ZIP)
 *
 * Operators:
 * o_num -> OArchive_num
 * o_at -> OArchive_at (returning struct OArchive_entry *)
 */


#include "OObj.h"

/** object id */
#define OArchive_ID OObj_ID "OArchive."


enum OArchive_type {
    OArchive_TAR,
    OArchive_ZIP,  // only valid with MIA_OPTION_ZIP
    OArchive_type_ENUM_MAX
};

/**
 * file type including the dot '.'
 */
static const char *OArchive_type_str[] = {
    ".tar",
    ".zip",
    NULL
};


/**
 * A file entry for mode_read
 * path may contain nested directories like "foo/bar/baz.txt"
 * size is the uncompressed file size
 * size_compressed is the size on disk (same as size for tar, may be smaller or larger for zip)
 */
struct OArchive_entry {
    char *path;
    int idx;
    osize size;
    osize size_compressed;
};


typedef struct {
    OObj super;
    
    // fixed
    enum OArchive_type type;
    bool mode_read;
    
    // to read from or write to (.tar, .zip archives)
    oobj stream;

    struct OArchive_entry *entries;
    osize num;
    
    // used internally
    void *impl;
    oobj impl_stream;
} OArchive;


/**
 * Initializes the object.
 * @param obj OArchive object
 * @param parent to inherit from
 * @param stream OStream to read from or to write to (.tar, .zip) (NULL safe)
 * @param move_stream true to o_move the stream into this object
 * @param type archive type (.tar, .zip) to read or write to
 * @param read true to read the archive (.tar, .zip) from the stream.
 *             false to write to that stream
 * @return obj casted as OArchive
 * @note reading needs a seekable readable OStream, uses OStream_size to get its size
 *       writing needs a writeable OStream
 */
OArchive *OArchive_init(oobj obj, oobj parent, oobj stream, bool move_stream, enum OArchive_type type, bool read);

/**
 * Creates a new OArchive.
 * @param parent to inherit from
 * @param stream OStream to read from or to write to (.tar, .zip) (NULL safe)
 * @param move_stream true to o_move the stream into this object
 * @param type archive type (.tar, .zip) to read or write to
 * @param read true to read the archive (.tar, .zip) from the stream.
 *             false to write to that stream
 * @return The new object
 */
o_inline OArchive *OArchive_new(oobj parent, oobj stream, bool move_stream, enum OArchive_type type, bool read)
{
    OObj_DECL_IMPL_NEW(OArchive, parent, stream, move_stream, type, read);
}


/**
 * Creates a new OArchive by reading from an archive file.
 * OArchive_type is selected from the file ending (.tar, .zip)
 * @param parent to inherit from
 * @param file route to read the archive (.tar, .zip) from
 * @return The new object
 * @note sets mode_write to false
 */
OArchive *OArchive_new_read_file(oobj parent, const char *file);

/**
 * Creates a new OArchive to write to an archive file.
 * OArchive_type is selected from the file ending (.tar, .zip)
 * @param parent to inherit from
 * @param file route to write the archive (.tar, .zip) into
 * @return The new object
 * @note sets mode_write to true
 */
OArchive *OArchive_new_write_file(oobj parent, const char *file);

//
// virtual implementations:
//

/**
 * Object deletor that calls OArchive_close
 * @param obj OArchive object
 */
void OArchive__v_del(oobj obj);

/**
 * virtual operator function
 * @param obj OArchive object
 * @return the number of entries stored in the archive
 * @note only valid in read more, as num is else 0
 */
osize OArchive__v_op_num(oobj obj);

/**
 * virtual operator function
 * @param obj OArchive object
 * @param idx entry index
 * @return struct OArchive_entry *
 * @note only valid in read mode, as the array length is else 0 (but a zero terminated element...)
 */
void *OArchive__v_op_at(oobj obj, osize idx);



//
// object functions:
//

/**
 * @param obj OArchive object
 * @return archive file type (.tar, .zip) (fixed)
 */
OObj_DECL_GET(OArchive, enum OArchive_type, type)

/**
 * @param obj OArchive object
 * @return true if OArchive is created for reading, else for writing (fixed)
 */
OObj_DECL_GET(OArchive, bool, mode_read)


/**
 * Closes the aechive file (if not yet) and frees the internal data.
 * @param obj OArchive object
 * @note called by the deletor OArchive__v_del
 */
void OArchive_close(oobj obj);

/**
 * @param obj OArchive object
 * @return true if opened / active and not closed yet
 */
o_inline bool OArchive_active(oobj obj)
{
    OObj_assert(obj, OArchive);
    OArchive *self = obj;
    return self->impl != NULL;
}


//
// read stuff
//

/**
 * @param obj OArchive object
 * @return list of entries (zero terminated)
 * @note only valid in read mode, as the array length is else 0 (but a zero terminated element...)
 */
OObj_DECL_GET(OArchive, const struct OArchive_entry *, entries)

/**
 * @param obj OArchive object
 * @return entries num
 * @note only valid in read more, as num is else 0
 */
OObj_DECL_GET(OArchive, osize, num)

/**
 * Returns the entry info of a given entry index
 * @param obj OArchive object
 * @param idx element index
 * @return struct OArchive_entry for entry info
 * @note asserts idx bounds.
 *       or use the o_at operator.
 *       Only valid in read mode as num is else 0
 */
o_inline struct OArchive_entry OArchive_at(oobj obj, osize idx)
{
    OObj_assert(obj, OArchive);
    OArchive *self = obj;
    assert(idx>=0 && idx < self->num);
    return self->entries[idx];
}


/**
 * @param obj OArchive object
 * @param entry_path to search
 * @return entry idx or -1 on failure
 */
osize OArchive_find(oobj obj, const char *entry_path);


/**
 * @param obj OArchive object
 * @param entry_idx read from the archive
 * @param stream OStream to write into
 * @return true on success
 * @note asserts mode_read==true
 */
bool OArchive_entry_to_stream(oobj obj, int entry_idx, oobj stream);

/**
 * @param obj OArchive object
 * @param entry_idx read from the archive
 * @param file route to write into
 * @return true on success
 * @note asserts mode_read==true
 */
bool OArchive_entry_to_file(oobj obj, int entry_idx, const char *file);

/**
 * @param obj OArchive object
 * @param entry_idx read from the archive
 * @return OArray of the entry data (allocated on obj) or NULL on failure
 * @note asserts mode_read==true
 */
struct oobj_opt OArchive_entry_to_memory(oobj obj, int entry_idx);



/**
 * @param obj OArchive object
 * @param entry_path to search and read from the archive
 * @param stream OStream to write into
 * @return true on success
 * @note asserts mode_read==true
 */
o_inline bool OArchive_find_entry_to_stream(oobj obj, const char *entry_path, oobj stream)
{
    osize idx = OArchive_find(obj, entry_path);
    if(idx<0) {
        return false;
    }
    return OArchive_entry_to_stream(obj, idx, stream);
}

/**
 * @param obj OArchive object
 * @param entry_path to search and read from the archive
 * @param file route to write into
 * @return true on success
 * @note asserts mode_read==true
 */
o_inline bool OArchive_find_entry_to_file(oobj obj, const char *entry_path, const char *file)
{
    osize idx = OArchive_find(obj, entry_path);
    if(idx<0) {
        return false;
    }
    return OArchive_entry_to_file(obj, idx, file);
}

/**
 * @param obj OArchive object
 * @param entry_path to search and read from the archive
 * @return OArray of the entry data (allocated on obj) or NULL on failure
 * @note asserts mode_read==true
 */
o_inline struct oobj_opt OArchive_find_entry_to_memory(oobj obj, const char *entry_path)
{
    osize idx = OArchive_find(obj, entry_path);
    if(idx<0) {
        return oobj_opt(NULL);
    }
    return OArchive_entry_to_memory(obj, idx);
}

//
// write stuff
//

/**
 * @param obj OArchive object
 * @param entry_path to write into the archive
 * @param stream OStream to read from
 * @return true on success
 * @note asserts mode_read==false
 */
bool OArchive_write_entry_from_stream(oobj obj, const char *entry_path, oobj stream);

/**
 * @param obj OArchive object
 * @param entry_path to write into the archive
 * @param file route to read from
 * @return true on success
 * @note asserts mode_read==false
 */
bool OArchive_write_entry_from_file(oobj obj, const char *entry_path, const char *file);

/**
 * @param obj OArchive object
 * @param entry_path to write into the archive
 * @param memory buffer to create from
 * @param memory_size size of the memory block, if <0: o_strlen(memory) is used instead
 * @return true on success
 * @note asserts mode_read==false
 *       uses OStreamMem internally and calls OArchive_entry_from_stream
 */
bool OArchive_write_entry_from_memory(oobj obj, const char *entry_path, const void *memory, osize memory_size);




#endif //O_OARCHIVE_H
