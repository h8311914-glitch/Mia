#include "r/program.h"
#include "r/RProgram.h"
#include "o/ODelcallback.h"
#include "o/OArray.h"
#include "o/str.h"
#include "o/file.h"
#include <ctype.h>

static struct {
    oobj o;
} program_L;

static oobj L_o(void)
{
    if(!program_L.o) {
        program_L.o = OObj_new(r_root());
        OObj_name_set(program_L.o, "r_program");
    }
    return program_L.o;
}



char *r_program_merge_code(oobj parent, char **shader_codes, const osize *opt_shader_codes_num)
{
    oobj container = OObj_new(parent);
    oobj code_array = OArray_new_dyn(container, NULL, sizeof(char), 0, 16384);

    osize num = 0;
    if (opt_shader_codes_num) {
        num = *opt_shader_codes_num;
    } else {
        while (shader_codes[num]) {
            num++;
        }
    }

    for (int i = 0; i<num; i++) {
        const char *code = shader_codes[i];

        while (*code) {
            osize idx = o_str_find(code, "#include");

            osize len = idx >= 0? idx : o_strlen(code);
            OArray_append(code_array, code, len);
            if (idx < 0) {
                break;
            }

            code += idx + o_strlen("#include");
            while (isspace(*code)) {
                code++;
            }
            o_assume(*code == '"', "invalid #include, missing \'\"\'");
            code++;
            osize end = o_str_find_char(code, '"');
            o_assume(end >= 0, "invalid #include, missing \'\"\'");
            o_assume(end > 0, "empty include file?");
            const char *file = o_str_clone_sub(container, code, 0, end);
            code += end + 1;

            oobj include = o_file_read(container, file, true, 1).o;
            o_assume(include, "include file not found");

            OArray_append(code_array, o_at(include, 0), o_num(include));
            o_del(include);
        }
    }

    char *merged = o_at(code_array, 0);
    OArray_move(code_array, parent);
    o_del(container);
    return merged;
}

#define r_program_DECL(file, num_draw_buffer)\
oobj r_program_ ## file (void)\
{\
    static RProgram *lazy = NULL;            \
    if(!lazy) {                              \
        lazy = RProgram_new_file(L_o(), "$r/" #file ".glsl", NULL, (num_draw_buffer)); \
        ODelcallback_new_assert(lazy, "r_program", "deleted!");\
    }                                         \
    return lazy;\
}


r_program_DECL(Box, 1)
r_program_DECL(Box_rgba, 1)
r_program_DECL(Box_hsva, 1)
r_program_DECL(BoxIndexed, 1)
r_program_DECL(Quad, 1)
r_program_DECL(Quad_addscaled, 1)
r_program_DECL(Quad_rgba, 1)
r_program_DECL(Quad_rgba_hsva, 1)
r_program_DECL(Quad_hsva_from_rgba, 1)
r_program_DECL(Quad_hsva_into_rgba, 1)
r_program_DECL(Quad_mixer, 1)
r_program_DECL(QuadChannelmerge, 1)
r_program_DECL(QuadDab, 2)
r_program_DECL(QuadKernel_contour, 1)
r_program_DECL(QuadKernel_conv, 1)
r_program_DECL(QuadKernel_contour_inv, 1)
r_program_DECL(QuadMerge, 1)
r_program_DECL(QuadMerge_additive, 1)
r_program_DECL(QuadMerge_subtractive, 1)
r_program_DECL(QuadOp_min, 1)
