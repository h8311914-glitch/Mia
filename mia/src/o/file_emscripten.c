#ifdef MIA_PLATFORM_EMSCRIPTEN

#include "o/file.h"
#include "o/common.h"
#include "o/str.h"
#include "o/timer.h"
#include <emscripten.h>

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"

#define SYNC_TIMEOUT 0.5

static struct {
    bool mounted;
    ou64 sync_timer;
    bool import_request;
    bool import_done;
    bool export_request;
} fe_L;




static void o_file_route__idbfs_import_request(void)
{
    fe_L.import_request = false;
    fe_L.import_done = false;
    fe_L.sync_timer = o_timer();

    o_log_trace_s(__func__, "import ...");

    // true = import file system from idbfs
    EM_ASM(
        o__route_save_idbfs_synced = false;
        FS.syncfs(true, function(err)
        {
            assert(!err);
            o__route_save_idbfs_synced = true;
        });
    );
}

// protected for the web app interface to be called by ccall
void o_file_route_save__idbfs_synced(void)
{
    o_log_trace_s(__func__, "import synced");
    fe_L.import_done = true;
}

static void o_file_route__idbfs_import_check(void)
{
    if (!fe_L.import_done) {
        EM_ASM(
            if (o__route_save_idbfs_synced) { ccall('o_file_route_save__idbfs_synced', 'v'); }
        );
    }
}


static void o_file_route__idbfs_export_request(void)
{
    fe_L.export_request = false;
    fe_L.sync_timer = o_timer();

    o_log_trace_s(__func__, "export ...");

    // false = export file system from idbfs
    EM_ASM(
        o__route_save_idbfs_synced = false;
        FS.syncfs(false, function(err)
        {
            assert(!err);
        });
    );
}
//
// protected
//

void o_file_route_save__idbfs_mount(void)
{
    o_log_debug_s(__func__, "route_save mount");

    if (fe_L.mounted) {
        return;
    }
    EM_ASM(
        FS.mkdir('/mia_tmp');
        FS.mkdir('/mia_save');
        FS.mount(IDBFS, {}, '/mia_save');
        o__route_save_idbfs_synced = false;
    );
    fe_L.mounted = true;
    o_log_trace_s(__func__, "route_save mounted");

    o_file_route__idbfs_import_request();
}

void o_file_route_save__idbfs_import(void)
{
    fe_L.import_request = true;
}

void o_file_route_save__idbfs_export(void)
{
    fe_L.export_request = true;
}

// called each frame, returns true if imported
bool o_file_route__idbfs_update(void)
{
    bool sync_valid = o_timer_elapsed_s(fe_L.sync_timer) >= SYNC_TIMEOUT;

    if (fe_L.import_request && fe_L.import_done && sync_valid) {
        o_file_route__idbfs_import_request();
    }
    o_file_route__idbfs_import_check();

    if (fe_L.export_request && sync_valid) {
        o_file_route__idbfs_export_request();
    }

    return fe_L.import_done;
}


#endif
typedef int avoid_empty_translation_unit;
