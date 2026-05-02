#include "o/OArchive.h"
#include "o/OObj_builder.h"
#include "o/file.h"
#include "o/str.h"
#include "o/OArray.h"
#include "o/OStreamArray.h"
#include "o/OStreamMem.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


#include "OArchive_microtar.h"


//
// .tar
//


static int tar_file_write(mtar_t *tar, const void *data, unsigned size) {
    OObj_assert(tar->stream, OArchive);
    OArchive *self = tar->stream;
    unsigned res = OStream_write(self->stream, data, 1, size);
    return (res == size) ? MTAR_ESUCCESS : MTAR_EWRITEFAIL;
}

static int tar_file_read(mtar_t *tar, void *data, unsigned size) {
    OObj_assert(tar->stream, OArchive);
    OArchive *self = tar->stream;
    unsigned res = OStream_read_try(self->stream, data, 1, size);
    return (res == size) ? MTAR_ESUCCESS : MTAR_EREADFAIL;
}

static int tar_file_seek(mtar_t *tar, unsigned offset) {
    OObj_assert(tar->stream, OArchive);
    OArchive *self = tar->stream;
    osize res = OStream_seek(self->stream, offset, SEEK_SET);
    return res >= 0? MTAR_ESUCCESS : MTAR_ESEEKFAIL;
}

static int tar_file_close(mtar_t *tar) {
    // do not close the stream or smth...
    return MTAR_ESUCCESS;
}


static void tar_init(OArchive *self)
{
    assert(!self->impl);
    self->impl = o_new0(self, mtar_t, 1);
    mtar_t *tar = self->impl;

    tar->write = tar_file_write;
    tar->read = tar_file_read;
    tar->seek = tar_file_seek;
    tar->close = tar_file_close;
    tar->stream = self;

    if (!self->mode_read) {
        //
        // mode write
        //
        return;
    }

    //
    // mode read
    //

    oobj array = OArray_new_dyn(self, NULL, sizeof (struct OArchive_entry), 0, 8);

    mtar_header_t h;
    while (mtar_read_header(tar, &h) == MTAR_ESUCCESS) {
        struct OArchive_entry entry = {0};
        entry.path = o_str_clone(self, h.name);
        entry.idx = OArray_num(array);
        entry.size = h.size;
        entry.size_compressed = h.size; // uncompressed
        OArray_push(array, &entry);

        if (mtar_next(tar) != MTAR_ESUCCESS) {
            o_log_error_s("OArchive_init", "failed to read microtar header (next)");
            break;
        }
    }

    self->num = OArray_num(array);
    self->entries = OArray_data_void(array);
    OArray_move(array, self);
    o_del(array);
}

static void tar_close(OArchive *self)
{
    bool ok;
    if (self->mode_read) {
        ok = mtar_close(self->impl) == MTAR_ESUCCESS;
    } else {
        bool finalized = mtar_finalize(self->impl) == MTAR_ESUCCESS;
        ok = mtar_close(self->impl) == MTAR_ESUCCESS;
        ok = ok && finalized;
    }
    o_free(self, self->impl);
    self->impl = NULL;
    if (!ok) {
        o_log_error_s("OArchive_close", "failed to close microtar");
    }
}

static bool tar_entry_to_stream(OArchive *self, int entry_idx, oobj stream)
{
    mtar_header_t header = {0};
    if(mtar_find(self->impl, self->entries[entry_idx].path, &header) != MTAR_ESUCCESS) {
        return false;
    }
    osize file_size = header.size;
    void *data = o_alloc(self, 1, file_size);
    bool valid = mtar_read_data(self->impl, data, file_size) == MTAR_ESUCCESS;
    valid = valid && OStream_write(stream, data, 1, file_size) == file_size;
    o_free(self, data);
    return valid;
}

static bool tar_entry_from_stream(OArchive *self, const char *entry_name, oobj stream)
{
    osize file_size = OStream_size(stream);

    if (mtar_write_file_header(self->impl, entry_name, file_size) != MTAR_ESUCCESS) {
        return false;
    }
    void *data = o_alloc(self, 1, file_size);
    osize read = OStream_read(stream, data, 1, file_size);
    bool valid = read == file_size;
    valid = valid && mtar_write_data(self->impl, data, file_size) == MTAR_ESUCCESS;
    o_free(self, data);
    return valid;
}

//
// .zip
//

#ifdef MIA_OPTION_ZIP
#  include "OArchive_miniz.h"

static size_t zip_read_func(void *pOpaque, mz_uint64 file_ofs, void *pBuf, size_t n)
{
    // we need to seek to file_ofs in reading a zip
    OObj_assert(pOpaque, OArchive);
    OArchive *self = pOpaque;
    OStream_seek(self->stream, file_ofs, OStream_SET);
    osize read_num = OStream_read_try(self->stream, pBuf, 1, n);
    return read_num;
}

static size_t zip_write_func(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n)
{
    // file_ofs should always be += n
    OObj_assert(pOpaque, OArchive);
    OArchive *self = pOpaque;
    osize written = OStream_write(self->stream, pBuf, 1, n);
    return written;
}

static size_t zip_entry_read_func(void *pOpaque, mz_uint64 file_ofs, void *pBuf, size_t n)
{
    // file_ofs should always be += n
    OObj_assert(pOpaque, OArchive);
    OArchive *self = pOpaque;
    osize read_num = OStream_read_try(self->impl_stream, pBuf, 1, n);
    return read_num;
}

static size_t zip_entry_write_func(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n)
{
    // file_ofs should always be += n
    OObj_assert(pOpaque, OArchive);
    OArchive *self = pOpaque;
    osize written = OStream_write(self->impl_stream, pBuf, 1, n);
    return written;
}

static void zip_init(OArchive *self)
{
    assert(!self->impl);
    self->impl = o_new0(self, mz_zip_archive, 1);
    mz_zip_archive *archive = self->impl;


    if (!self->mode_read) {
        //
        // mode write:
        //

        // setup custom user write func (before init is called...)
        archive->m_pWrite = zip_write_func;
        archive->m_pIO_opaque = self;

        bool valid = mz_zip_writer_init(self->impl, 0);
        if (!valid) {
            o_log_error_s("OArchive_init", "failed to init miniz writer");
            o_free(self, self->impl);
            self->impl = NULL;
        }

        return;
    }

    //
    // mode read:
    //

    // setup custom user read func (before init is called...)
    archive->m_pRead = zip_read_func;
    archive->m_pIO_opaque = self;

    osize file_size = OStream_size(self->stream);

    bool valid = mz_zip_reader_init(self->impl, file_size, MZ_ZIP_FLAG_ASCII_FILENAME);
    if (!valid) {
        o_log_error_s("OArchive_init", "failed to init miniz reader");
        o_free(self, self->impl);
        self->impl = NULL;
        return;
    }

    int num = (int) mz_zip_reader_get_num_files(self->impl);

    // read in entries (write has num==0)
    self->entries = o_new0(self, *self->entries, num+1);

    self->num = 0;
    for (int i = 0; i < num; i++) {
        if (mz_zip_reader_is_file_a_directory(self->impl, i)) {
            continue;
        }

        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(self->impl, i, &file_stat)) {
            continue;
        }
        if (!file_stat.m_is_supported) {
            continue;
        }

        osize idx = self->num++;
        struct OArchive_entry *entry = &self->entries[idx];

        entry->path = o_str_clone(self, file_stat.m_filename);
        entry->idx = idx;
        entry->size = file_stat.m_uncomp_size;
        entry->size_compressed = file_stat.m_comp_size;
    }

    if (self->num == 0) {
        OArchive_close(self);
    }
}

static void zip_close(OArchive *self)
{
    bool ok;
    if (self->mode_read) {
        ok = mz_zip_reader_end(self->impl);
    } else {
        bool finalized = mz_zip_writer_finalize_archive(self->impl);
        ok = mz_zip_writer_end(self->impl);
        ok = ok && finalized;
    }
    o_free(self, self->impl);
    self->impl = NULL;
    if (!ok) {
        o_log_error_s("OArchive_close", "failed to close miniz");
    }
}

static bool zip_entry_to_stream(OArchive *self, int entry_idx, oobj stream)
{
    self->impl_stream = stream;
    bool valid = mz_zip_reader_extract_to_callback(self->impl, entry_idx, zip_entry_write_func, self, 0);
    self->impl_stream = NULL;
    return valid;
}

static bool zip_entry_from_stream(OArchive *self, const char *entry_name, oobj stream)
{
    self->impl_stream = stream;
    osize file_size = OStream_size(stream);
    // using an empty comment
    // level_and_flags < 0 -> default
    // empty other locale stuff
    bool valid = mz_zip_writer_add_read_buf_callback(self->impl, entry_name, zip_entry_read_func, self, file_size,
                                                     NULL, "", 0, -1, "", 0, "", 0);
    self->impl_stream = NULL;
    return valid;
}

#endif //MIA_OPTION_ZIP





static enum OArchive_type archive_file_type(oobj parent, const char *file)
{
    enum OArchive_type type = OArchive_type_ENUM_MAX;
    char *lower = o_str_tolower(parent, file);
    for (int i=0; i<OArchive_type_ENUM_MAX; i++) {
        if (o_str_ends(lower, OArchive_type_str[i])) {
            type = i;
            break;
        }
    }
    if (type == OArchive_type_ENUM_MAX) {
        o_log_wtf_s(__func__, "invalid file ending: %s", file);
        assert(0);
    }
    o_free(parent, lower);
    return type;
}


//
// public
//

OArchive *OArchive_init(oobj obj, oobj parent, oobj stream, bool move_stream, enum OArchive_type type, bool read)
{
    OArchive *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, OArchive_ID);

#ifndef MIA_OPTION_ZIP
    assert(type != OArchive_ZIP && "needs MIA_OPTION_ZIP")
#endif

    self->type = type;
    self->mode_read = read;

    self->stream = stream;
    if (move_stream) {
        // NULL safe for stream
        o_move(stream, self);
    }

    // vfuncs
    self->super.v_del = OArchive__v_del;
    self->super.v_op_num = OArchive__v_op_num;
    self->super.v_op_at = OArchive__v_op_at;

    switch (type) {
        case OArchive_TAR:
            tar_init(self);
            break;
        case OArchive_ZIP:
#ifdef MIA_OPTION_ZIP
            zip_init(self);
#endif
            break;
        default:
            assert(0 && "invalid type");
    }

    return self;
}

OArchive *OArchive_new_read_file(oobj parent, const char *file)
{
    enum OArchive_type type = archive_file_type(parent, file);
    oobj stream = o_file_open(parent, file, "rb").o;
    return OArchive_new(parent, stream, true, type, true);
}


OArchive *OArchive_new_write_file(oobj parent, const char *file)
{
    enum OArchive_type type = archive_file_type(parent, file);
    oobj stream = o_file_open(parent, file, "wb").o;
    return OArchive_new(parent, stream, true, type, false);
}

//
// virtual implementations:
//

void OArchive__v_del(oobj obj)
{
    OObj_assert(obj, OArchive);
    OArchive *self = obj;

    OArchive_close(self);

    OObj__v_del(self);
}

osize OArchive__v_op_num(oobj obj)
{
    OObj_assert(obj, OArchive);
    OArchive *self = obj;
    return self->num;
}

void *OArchive__v_op_at(oobj obj, osize idx)
{
    OObj_assert(obj, OArchive);
    OArchive *self = obj;
    assert(idx>=0 && idx < self->num);
    return &self->entries[idx];
}


//
// object functions
//


void OArchive_close(oobj obj)
{
    OObj_assert(obj, OArchive);
    OArchive *self = obj;
    if (!OArchive_active(self)) {
        return;
    }
    switch (self->type) {
        case OArchive_TAR:
            tar_close(self);
            return;
        case OArchive_ZIP:
#ifdef MIA_OPTION_ZIP
            zip_close(self);
#endif
            return;
        default:
            assert(0 && "invalid type");
    }
}

//
// read stuff
//


osize OArchive_find(oobj obj, const char *entry_path)
{
    OObj_assert(obj, OArchive);
    OArchive *self = obj;
    for(osize i=0; i<self->num; i++) {
        if(o_str_equals(entry_path, self->entries[i].path)) {
            return i;
        }
    }
    return -1;
}



bool OArchive_entry_to_stream(oobj obj, int entry_idx, oobj stream)
{
    OObj_assert(obj, OArchive);
    OArchive *self = obj;
    assert(self->mode_read==true);
    assert(entry_idx>=0 && entry_idx < self->num);
    if (!OArchive_active(self)) {
        return false;
    }
    switch (self->type) {
        case OArchive_TAR:
            return tar_entry_to_stream(self, entry_idx, stream);
        case OArchive_ZIP:
#ifdef MIA_OPTION_ZIP
            return zip_entry_to_stream(self, entry_idx, stream);
#endif
            return false;
        default:
            assert(0 && "invalid type");
    }
    return false;
}


bool OArchive_entry_to_file(oobj obj, int entry_idx, const char *file)
{
    oobj stream = o_file_open(obj, file, "wb").o;
    if (!stream) {
        return false;
    }
    bool valid = OArchive_entry_to_stream(obj, entry_idx, stream);
    o_del(stream);
    return valid;
}


struct oobj_opt OArchive_entry_to_memory(oobj obj, int entry_idx)
{
    oobj array = OArray_new_dyn(obj, NULL, 1, 0, 1024);
    oobj stream = OStreamArray_new(array, array, false, OStreamArray_SEEKABLE);
    bool valid = OArchive_entry_to_stream(obj, entry_idx, stream);
    o_del(stream);
    if (valid) {
        return oobj_opt(array);
    }
    o_del(array);
    return oobj_opt(NULL);
}


//
// write stuff
//

bool OArchive_write_entry_from_stream(oobj obj, const char *entry_name, oobj stream)
{
    OObj_assert(obj, OArchive);
    OArchive *self = obj;
    assert(self->mode_read==false);
    if (!OArchive_active(self)) {
        return false;
    }
    switch (self->type) {
        case OArchive_TAR:
            return tar_entry_from_stream(self, entry_name, stream);
        case OArchive_ZIP:
#ifdef MIA_OPTION_ZIP
            return zip_entry_from_stream(self, entry_name, stream);
#endif
            return false;
        default:
            assert(0 && "invalid type");
    }
    return false;
}


bool OArchive_write_entry_from_file(oobj obj, const char *entry_name, const char *file)
{
    oobj stream = o_file_open(obj, file, "rb").o;
    if (!stream) {
        return false;
    }
    bool valid = OArchive_write_entry_from_stream(obj, entry_name, stream);
    o_del(stream);
    return valid;
}

bool OArchive_write_entry_from_memory(oobj obj, const char *entry_name, const void *memory, osize memory_size)
{
    if (!memory) {
        return false;
    }
    oobj stream = OStreamMem_new(obj, (void*) memory, memory_size);
    bool valid = OArchive_write_entry_from_stream(obj, entry_name, stream);
    o_del(stream);
    return valid;
}
