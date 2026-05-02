#include "o/OArchive.h"

#include "o/OArray.h"
#include "o/OStreamArray.h"
#include "o/str.h"

#define test(expr) o_assume(expr, "test failed")

//#define SAVE_TO_FILE


#ifdef SAVE_TO_FILE
#include "o/file.h"
#endif

static void archive_stream_test(oobj obj, enum OArchive_type type)
{
    const char *file_a = "test.txt";
    const char *file_b = "nested/path/foo.txt";
    const char *test_a = "test";
    const char *test_b = o_str_rep(obj, "Hello World\nFoo Bar Baz\n", 128, NULL);
    
    bool valid;
    oobj array = OArray_new_dyn(obj, NULL, 1, 0, 1024);
    oobj stream = OStreamArray_new(obj, array, false, OStreamArray_SEEKABLE);
    oobj archive = OArchive_new(obj, stream, false, type, false);
    valid = OArchive_write_entry_from_memory(archive, file_a, test_a, -1);
    test(valid);
    valid = OArchive_write_entry_from_memory(archive, file_b, test_b, -1);
    test(valid);
    OArchive_close(archive);

    // read and verify

    // save to test
#ifdef SAVE_TO_FILE
    const char *test_file = o_strf(obj, "#OArchive__test%s", OArchive_type_str[type]);
    o_file_write(test_file, false, OArray_data_void(array), 1, OArray_num(array));
#endif

    OStream_seek(stream, 0, OStream_SET);
    archive = OArchive_new(obj, stream, false, type, true);
    test(OArchive_num(archive) == 2);
    test(o_num(archive) == 2);
    struct OArchive_entry entry_a = OArchive_at(archive, 0);
    struct OArchive_entry entry_b = OArchive_at(archive, 1);
    test(entry_a.idx == 0);
    test(entry_b.idx == 1);
    test(o_str_equals(entry_a.path, file_a));
    test(o_str_equals(entry_b.path, file_b));
    
    test(entry_a.size == o_strlen(test_a));
    test(entry_b.size == o_strlen(test_b));
    
    test(o_str_equals(((struct OArchive_entry*)o_at(archive, 0))->path, file_a));
    test(o_str_equals(((struct OArchive_entry*)o_at(archive, 1))->path, file_b));
    
    oobj mem_a = OArchive_entry_to_memory(archive, 0).o;
    test(mem_a);
    test(o_str_equals(OArray_data_void(mem_a), test_a));

    oobj mem_b = OArchive_entry_to_memory(archive, 1).o;
    test(mem_b);
    test(o_str_equals(OArray_data_void(mem_b), test_b));
}

int OArchive__test(oobj obj)
{
    archive_stream_test(obj, OArchive_TAR);
#ifdef MIA_OPTION_ZIP
    archive_stream_test(obj, OArchive_ZIP);
#endif
    return 0;
}

