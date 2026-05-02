#include "ms/MSFile.h"
#include "o/OObj_builder.h"
#include "o/str.h"
#include "o/log.h"
#include "r/RTex_blit.h"
#include "o/OList.h"

#include "ms/exe.h"
#include "ms/tree.h"


static oobj msfile_thumb_generate(MSFile *self, oobj icon)
{
    const char *name = MSFile_name(self);

    oobj tex = RTex_new(self, NULL, m_2(MSFile_RENDER_SIZE));
    RTex_clear_full(tex, R_TRANSPARENT);
    vec4 rect = vec4_(0, 0, m_2(MSFile_ICON_SIZE));
    vec4 uv = RTex_rect(icon, 0, 0);
    RTex_blend_rect(tex, icon, rect, uv);
    
    char buf[32];
    o_strf_buf(buf, "%s", name);
    o_memmove(buf+8, buf+7, 1, 8);
    buf[8] = '\n';
    
    RTex_text(tex, buf, 0, MSFile_ICON_SIZE.y + 2, RTex_font35_shadow, R_BLACK);
    return tex;
}

//
// public
//

MSFile *MSFile_init(oobj obj, oobj parent, const char *path, enum MSFile_type type,
                    oobj icon, bool moveable, MSFile__exe_fn opt_exe, MSFile__exe_fn opt_file_executable,
                    oobj executables, bool move_executables)
{
    MSFile *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, MSFile_ID);

    self->path = o_str_clone(self, path);
    self->type = type;
    self->opt_exe = opt_exe;
    self->opt_file_executable = opt_file_executable;
    self->moveable = moveable;
    
    OObj_assert(executables, OList);
    self->executables = executables;
    if (move_executables) {
        o_move(self->executables, self);
    }

    self->thumb = msfile_thumb_generate(self, icon);

    return self;
}

MSFile *MSFile_new_dir(oobj parent, const char *path)
{
    return NULL;
}
MSFile *MSFile_new_img(oobj parent, const char *path)
{
    oobj list = OList_new(parent,
        o_list_compound(oobj,
            ms_exe_mia_paint(),
            ms_exe_cp(),
            ms_exe_mv(),
            ms_exe_rm()
        ), -1);
    return MSFile_new(parent, path, MSFile_type_IMG, NULL, true, NULL, NULL, list, true);
}

//
// object functions:
//

const char *MSFile_name(oobj obj)
{
    OObj_assert(obj, MSFile);
    MSFile *self = obj;
    osize len = o_strlen(self->path);
    assert(len>0 && "invalid path");
    osize pos = o_str_find_back_char(self->path, '/');
    if (pos<0) {
        return self->path;
    }
    assert(pos < len -1 && "invalid path, either '/' is missing or name");
    return self->path + pos + 1;
}

void MSFile_exe(oobj obj, oobj opt_file)
{
    OObj_assert(obj, MSFile);
    MSFile *self = obj;
    if(self->opt_exe && self->enabled) {
        self->opt_exe(self, opt_file);
    }
}


bool MSFile_file_executable(oobj obj, oobj file)
{
    OObj_assert(obj, MSFile);
    MSFile *self = obj;
    if(!self->opt_file_executable) {
        return false;
    }
    return self->opt_file_executable(self, file);
}

void MSFile_render(oobj obj, oobj tex, float x, float y)
{
    OObj_assert(obj, MSFile);
    MSFile *self = obj;
    float alpha = self->enabled? 1.0 : 0.5;
    RTex_blend_color(tex, self->thumb, x, y, vec4_(1, 1, 1, alpha), vec4_(0));
}
