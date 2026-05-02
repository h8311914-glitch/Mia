#include "x/XViewShell.h"
#include "o/OObj_builder.h"
#include "o/OArray_raw.h"
#include "o/OStream.h"
#include "o/OMap.h"
#include "o/OPtr.h" 
#include "o/str.h"
#include "o/file.h"
#include "r/RTex_ex.h"
#include "r/RObjText.h"
#include "r/RCam.h"
#include "a/app.h"
#include "a/AViewStage.h"

#include <ctype.h> // toupper

#undef O_LOG_LIB
#define O_LOG_LIB "x"
#include "o/log.h"

#define CURSOR_BLINK_TIME 1.0f

struct shell_api {
    oobj ptr;
    char *command;
    XViewShell_api_fn fn;
    XViewShell_api_autocomplete_fn opt_autocomplete_fn;
    char *help;
};

static void shell_api_help(oobj shell, oobj user_obj, int argc, char **argv)
{
    OObj_assert(shell, XViewShell);
    XViewShell *self = shell;
    
    if (!o_str_equals(o_str_toupper_a(argv[0]), "HELP") || argc <= 1) {
        XViewShell_print(shell, "---HELP---\nUSAGE: HELP <CMD> TO SHOW CMD HELP\nAVAILABLE CMDS:");
        osize num = OMap_num(self->api_dict);
        char *cmds = "";
        int cnt = 0;
        for (osize i=0; i<num; i++) {
            struct shell_api *api = OMap_value_at(self->api_dict, i, struct shell_api);
            assert(api);
            oobj api_obj = OPtr_get(api->ptr).o;
            if (!api_obj) {
                continue;
            }
            if(cnt<4) {
                cmds = o_strf_a("%s  %s", cmds, api->command);
                cnt++;
            } else {
                cmds = o_strf_a("%s\n  %s", cmds, api->command);
                cnt = 0;
            }
        }
        XViewShell_print(shell, cmds);
        return;
    }
    char *cmd = o_str_toupper_a(argv[1]);
    struct shell_api *api = OMap_get(self->api_dict, &cmd, struct shell_api);
    if (api == 0) {
        XViewShell_print(shell, "HELP FAILED, INVALID CMD");
        return;
    }
    oobj api_obj = OPtr_get(api->ptr).o;
    if (!api_obj) {
        o_log_error_s(__func__, "api object got deleted for command: %s", api->command);
        return;
    }
    XViewShell_print(shell, api->help);
}

bool shell_api_help_autocomplete(oobj shell, oobj user_obj, OList *completions, int argc, char **argv)
{
    OObj_assert(shell, XViewShell);
    XViewShell *self = shell;

    osize num = OMap_num(self->api_dict);
    for (osize i=0; i<num; i++) {
        struct shell_api *api = OMap_value_at(self->api_dict, i, struct shell_api);
        assert(api);
        oobj api_obj = OPtr_get(api->ptr).o;
        if (!api_obj) {
            continue;
        }
        OList_push(completions, api->command);
    }
    return true;
}


static void shell_ac_print_all(XViewShell *self)
{
    XViewShell_clear(self);
    osize num = OList_num(self->ac_current_list);
    for (osize i=0; i<num; i++) {
        char *line = o_strf_a(": %s", OList_at(self->ac_current_list, i));
        XViewShell_print(self, line);
    }
}

static void shell_ac_update_current_n_hint(XViewShell *self, const char *input)
{
    oobj container = OObj_new(self);
    
    char *input_u = o_str_toupper(container, input);
    
    OList_clear(self->ac_current_list);
    for (osize i=0; i<OList_num(self->ac_api_list); i++) {
        char *api = OList_at(self->ac_api_list, i);
        char *api_u = o_str_toupper(container, api);
        if (o_str_begins(api_u, input_u)) {
            OList_push(self->ac_current_list, api);
        }
    }
    
    // create hint
    OArray_clear(self->ac_hint_array);
    osize num = OList_num(self->ac_current_list);
    if (num <= 0) {
        goto CLEAN_UP;
    }
    if (num == 1) {
        OArray_append_string(self->ac_hint_array, OList_at(self->ac_current_list, 0));
        goto CLEAN_UP;
    }
    // num > 1
    const char *api_0 = OList_at(self->ac_current_list, 0);
    osize api_0_len = o_strlen(api_0);
    for (osize c=0; c<api_0_len; c++) {
        char ch = api_0[c];
        for (osize i=1; i<num; i++) {
            const char *api = OList_at(self->ac_current_list, i);
            if (toupper(api[c]) != toupper(ch)) {
                goto CLEAN_UP;
            }
        }
        OArray_push(self->ac_hint_array, &ch);
    }
    
    CLEAN_UP:
    o_del(container);
}

static void shell_ac_update_get_cmds(XViewShell *self)
{
    o_del(self->ac_api_list);
    self->ac_api_list = OList_new(self, NULL, 0);
    osize num = OMap_num(self->api_dict);
    for (osize i=0; i<num; i++) {
        struct shell_api *api = OMap_value_at(self->api_dict, i, struct shell_api);
        assert(api);
        oobj api_obj = OPtr_get(api->ptr).o;
        if (!api_obj) {
            continue;
        }
        char *cmd = o_strf(self->ac_api_list, "%s ", api->command);
        OList_push(self->ac_api_list, cmd);
    }
    self->ac_api_list_done = false;
}


static void shell_ac_update_get_api(XViewShell *self, struct shell_api *api, int argc, char **argv)
{
    if (self->ac_api_list_done) {
        return;
    }
    o_del(self->ac_api_list);
    self->ac_api_list = OList_new(self, NULL, 0);
    if (!api->opt_autocomplete_fn) {
        self->ac_api_list_done = false;
        return;
    }
    oobj api_list = OList_new(self, NULL, 0);
    bool done = api->opt_autocomplete_fn(self, api, api_list, argc, argv);
    // the created api list only has cmd arguments in the strings, without the leading cmd
    for (osize i=0; i<OList_num(api_list); i++) {
        char *api_str = OList_at(api_list, i);
        char *cmd_api_str = o_strf(self->ac_api_list, "%s %s", api->command, api_str);
        OList_push(self->ac_api_list, cmd_api_str);
    }
    o_del(api_list);
    self->ac_api_list_done = done;
}


static void shell_ac_update(XViewShell *self, const char *input)
{
    char **argv = o_str_split_a(input, " ");
    osize argc = o_list_num(argv);
    if (argc<=0) {
        shell_ac_update_get_cmds(self);
        goto UPDATE_CURRENTS;
    }
    char *cmd = o_str_toupper_a(argv[0]);
    struct shell_api *api = OMap_get(self->api_dict, &cmd, struct shell_api);
    if (api == 0) {
        shell_ac_update_get_cmds(self);
        goto UPDATE_CURRENTS;
    }
    oobj api_obj = OPtr_get(api->ptr).o;
    if (!api_obj) {
        shell_ac_update_get_cmds(self);
        goto UPDATE_CURRENTS;
    }
    if (argc==1 && !o_str_ends(input, " " )) {
        shell_ac_update_get_cmds(self);
        goto UPDATE_CURRENTS;
    }
    shell_ac_update_get_api(self, api, argc, argv);

    UPDATE_CURRENTS:
    shell_ac_update_current_n_hint(self, input);
}


//
// public
//

XViewShell *XViewShell_init(oobj obj, oobj parent, oobj stream, int cols, int rows)
{

    AView *super = obj;
    XViewShell *self = obj;
    o_clear(self, sizeof *self, 1);

    AView_init(obj, parent, XViewShell__v_setup, XViewShell__v_update, XViewShell__v_render);
    OObj_id_set(self, XViewShell_ID);
    
    
    self->stream = stream;
    self->cols = cols;
    self->rows = rows;
    
    self->input_array = OArray_new_dyn(self, NULL, sizeof(char), 0, self->cols);
    self->text_array = OArray_new_dyn(self, NULL, sizeof(char), 0, self->rows * self->cols);
    
    
    // autocomplete
    self->ac_hint_array = OArray_new_dyn(self, NULL, sizeof(char), 0, self->cols);
    
    self->ac_current_list = OList_new(self, NULL, 0);
    // ac_api_list is created and deleted at runtime
    
    
    // buffers, both char [rows][cols]
    char *buf = o_new0(self, char, 2*rows*cols);
    self->buffer = buf;
    self->history = buf + (rows*cols);
    
    self->text_ro = RObjText_new_font35_shadow(self, ">", NULL);
    RObjText_layout_ref(self->text_ro)->casing = r_textlayout_casing_UNCHANGED;
    
    
    self->api_dict = OMap_new_string_keys(self, sizeof(struct shell_api), 128);
    
    
    XViewShell_api_add(self, self, "help", shell_api_help, shell_api_help_autocomplete,
        "HELP -> SHOW LIST OF CMDS\n"
        "HELP <COMMAND> -> SHOW HELP OF A CMD\n"
        "HELP VIEWTEX -> SHOW HELP OF CMD VIEWTEX"
        );

    return self;
    
}


//
// virtual implementations
//

void XViewShell__v_setup(oobj view)
{
    // noop
}

void XViewShell__v_update(oobj view, oobj tex, float dt)
{
    OObj_assert(view, XViewShell);
    XViewShell *self = view;
    
    
    bool enter_typed = false;
    bool up_typed = false;
    bool down_typed = false;
    bool escape = false;
    bool escape_dir = false;
    bool changed = false;
    for (;;) {
        char read;
        osize num = OStream_read_try(self->stream, &read, sizeof(char), 1);
        if (num == 0) {
            break;
        }
        if (escape_dir) {
            escape = escape_dir = false;
            if (read == 'A') {
                up_typed = true;
            } else if (read == 'B') {
                down_typed = true;
            } else if (read == 'C') {
                self->cursor++;
            } else {
                self->cursor--;
            }
            continue;
        }
        if (escape) {
            escape = false;
            if (read == '[') {
                escape_dir = true;
            }
            continue;
        }
        if (read == '\x1B') {
            escape = true;
            continue;
        }
        if (o_str_find_char(self->ignore_filter, read) >= 0) {
            continue;
        }
        if (read == '\b') {
            if (o_num(self->input_array) > 0 && self->cursor > 0
                    && self->cursor <= o_num(self->input_array)) {
                OArray_pop_at(self->input_array, self->cursor - 1, NULL);
                self->cursor--;
                changed = true;
            }
            continue;
        }
        if (read == '\n') {
            enter_typed = true;
            break;
        }
        if (read == '\t') {
            // auto complete
            shell_ac_update(self, OArray_data_void(self->input_array));
            
            osize in_num = OArray_num(self->input_array);
            osize auto_num = OArray_num(self->ac_hint_array);
            
            if(in_num>=auto_num) {
                shell_ac_print_all(self);
            } else {
            
                OArray_clear(self->input_array);
                OArray_append(self->input_array, 
                    OArray_data(self->ac_hint_array, char),
                    auto_num);
                self->cursor = auto_num;
            }
            changed = true;
            continue;
        }

        // cols -3 cause it will get printed with "$ " and is null terminated
        if (o_num(self->input_array) < (self->cols-3) && self->cursor >= 0
                    && self->cursor <= o_num(self->input_array)) {
            OArray_push_at(self->input_array, self->cursor, &read);
            self->cursor++;
            changed = true;
        }
    }

    if (up_typed || down_typed) {
        o_log_debug_s(__func__, "history up down");
        self->history_pos += up_typed? -1 : 1;
        self->history_pos = o_mod(self->history_pos, self->rows);
        OArray_clear(self->input_array);
        OArray_append_string(self->input_array, &self->history[self->history_pos*self->cols]);
        self->cursor = o_num(self->input_array);
        changed = true;
    }
    
    // remove unneeded spaces
    char prev = ' ';
    for(osize i=0; i<OArray_num(self->input_array); i++)
    {
        char cur = *OArray_at(self->input_array, i, char);
        if(prev == ' ' && cur == ' ') {
            OArray_pop_at(self->input_array, i, NULL);
            self->cursor--;
            i--;
            continue;
        }
        prev = cur;
    }

    if (enter_typed) {
        const char *input = OArray_data_void(self->input_array);
        o_log_s(__func__, "terminal input: <%s>", input);
        XViewShell_print(self, o_strf_a("$ %s", input));
        o_strf_buf(&self->history[self->history_next++ * self->cols], "%s", input);
        self->history_next %= self->rows;
        self->history_pos = self->history_next;
        XViewShell_eval(self, input);
        OArray_clear(self->input_array);
        self->cursor = 0;
        changed = true;
    }

    if (changed) {
        shell_ac_update(self, OArray_data_void(self->input_array));
    }


    // Using raw mode to speed it up a little
    //     NEEDS OArray_raw_done AFTER THAT
    OArray_raw_resize(self->text_array, 0);
    for (int i=0; i<self->rows; i++) {
        int r = (i + self->next.y) % self->rows;
        OArray_raw_append_string(self->text_array, &self->buffer[r*self->cols]);
        OArray_raw_append_string_static(self->text_array, "\n");
    }
    OArray_raw_append_string_static(self->text_array, "> ");
    osize input_start = OArray_num(self->text_array);
    const char *input = OArray_data_void(self->input_array);
    OArray_raw_append_string(self->text_array, input);
    
    // autocompletion
    osize auto_complete_start = o_strlen(input);
    
    if(OArray_num(self->ac_hint_array) > auto_complete_start) {
        OArray_raw_append_string(self->text_array,
            OArray_at(self->ac_hint_array, auto_complete_start, char));
    }
    
    
    OArray_raw_append_string_static(self->text_array, " ");
    OArray_raw_done(self->text_array);

    char *text = OArray_data_void(self->text_array);

    // cursor
    self->cursor = o_clamp(self->cursor, 0, OArray_num(self->input_array));
    self->cursor_blink_time -= dt;
    while (self->cursor_blink_time < 0) {
        self->cursor_blink_time += CURSOR_BLINK_TIME;
    }
    if (self->cursor_blink_time < CURSOR_BLINK_TIME/2) {
        // all non-ascii chars are a full rect
        text[input_start + self->cursor] = ('~'+1);
    }
    vec2 text_size = RObjText_text_set(self->text_ro, text, R_GRAY_X(0.8));

    // R_WHITE for input line
    // a bit transparence for auto complete
    oobj boxes_ro = RObjText_boxes_ro(self->text_ro);
    osize num_boxes = o_num(boxes_ro);
    assert(num_boxes>0);
    struct r_box *boxes = o_at(boxes_ro, 0);
    for (osize i=input_start; i<num_boxes; i++) {
        boxes[i].fx = R_WHITE;
        if(i>=input_start+auto_complete_start) {
            boxes[i].fx.a = 0.33;
        }
    }
    if (self->cursor_blink_time < CURSOR_BLINK_TIME/2) {
        // cursor may flip between auto complete and white
        boxes[input_start + self->cursor].fx = R_WHITE;
    }

    // scroll shell output to bottom

    vec2 max_size = RCam_size(AView_cam(view));
    mat4 *pose = RObjText_pose_ref(self->text_ro);
    if (text_size.y > max_size.y) {
        // move the text field up in a way that the text is aligned to text rows
        float diff = text_size.y - max_size.y;
        pose->axis_t.y = -diff-8;
    } else {
        pose->axis_t.y = 0;
    }
}

void XViewShell__v_render(oobj view, oobj tex, float dt)
{
    OObj_assert(view, XViewShell);
    XViewShell *self = view;

    RTex_clear_ex(tex, vec4_(0, 0.1, 0, 0.75), true);
    RTex_ro(tex, self->text_ro);
}

//
// object functions:
//

void XViewShell_print(oobj obj, const char *str)
{
    OObj_assert(obj, XViewShell);
    XViewShell *self = obj;
    
    if (!str) {
        return;
    }
    
    bool newline = false;
    for (;*str; str++) {
        newline = *str=='\n';
        if (newline) {
            self->buffer[self->next.y*self->cols + self->next.x] = '\0';
            self->next.x = 0;
            self->next.y++;
            self->next.y %= self->rows;
            continue;
        }
        self->buffer[self->next.y*self->cols + self->next.x++] = *str;
        if (self->next.x >= self->cols-1) {
            self->next.x = 0;
            self->next.y++;
            self->next.y %= self->rows;
        }
    }
    self->buffer[self->next.y*self->cols + self->next.x] = '\0';
    if (self->next.x > 0 || newline) {
        self->next.x = 0;
        self->next.y++;
        self->next.y %= self->rows;
    }
}

void XViewShell_clear(oobj obj)
{
    XViewShell_print(obj, o_str_rep_a("\n", XViewShell_rows(obj), NULL));
}

void XViewShell_eval(oobj obj, const char *input)
{
    OObj_assert(obj, XViewShell);
    XViewShell *self = obj;
    
    char **argv = o_str_split_a(input, " ");
    osize argc = o_list_num(argv);
    if (argc<=0) {
        // silent
        return;
    }
    char *cmd = o_str_toupper_a(argv[0]);
    struct shell_api *api = OMap_get(self->api_dict, &cmd, struct shell_api);
    if (api == 0) {
        shell_api_help(self, self, argc, argv);
        return;
    }
    oobj api_obj = OPtr_get(api->ptr).o;
    if (!api_obj) {
        o_log_error_s(__func__, "api object got deleted for command: %s", api->command);
        return;
    }

    bool no_stage = a_stage_try().o == NULL;
    if (no_stage) {
        OObj_assert(a_app_main_view(), AViewStage);
        a_app_stage_set(a_app_main_view());
    }

    api->fn(self, api_obj, argc, argv);

    if (no_stage) {
        a_app_stage_set(NULL);
    }
}

void XViewShell_api_add(oobj obj, oobj user_obj, const char *command, 
        XViewShell_api_fn fn, 
        XViewShell_api_autocomplete_fn opt_autocomplete_fn,
        const char *help)
{
    OObj_assert(obj, XViewShell);
    XViewShell *self = obj;
    OObj_assert(user_obj, OObj);
    
    char *cmd_lower = o_str_toupper(self->api_dict, command);
    struct shell_api api = {
        OPtr_new(self->api_dict, user_obj),
        cmd_lower,
        fn,
        opt_autocomplete_fn,
        o_str_clone(self->api_dict, help)
    };
    OMap_set(self->api_dict, &cmd_lower, &api);
}



static void helper_add_routes(OList *completions)
{
    OList_push(completions, "$");
    OList_push(completions, "&");
    OList_push(completions, "#");
}
static void helper_add_file_paths(OList *completions, OList *files, const char *parent_dir)
{
    osize files_num = OList_num(files);
    for (osize i=0; i<files_num; i++) {
            char *file = OList_at(files, i);
            if(o_str_equals(file, ".") || o_str_equals(file, "..")) {
            continue;
        }
            file = o_strf(completions, "%s%s", parent_dir, file);
            OList_push(completions, file);
        }
}

bool XViewShell_helper_api_ls_autocomplete(oobj shell, oobj user_obj, OList *completions, int argc, char **argv)
{
    if (argc <= 1) {
        helper_add_routes(completions);
        return false;
    }

    char *arg_dir = o_str_join(completions, argv+1, -1, NULL);
    if (!arg_dir || !*arg_dir) {
        helper_add_routes(completions);
        return false;
    }
    oobj files = o_file_list(completions, arg_dir, 0, NULL);
    osize files_num = OList_num(files);
    if (files_num > 0) {
        if (o_strlen(arg_dir)>1 && !o_str_ends(arg_dir, "/")) {
            arg_dir = o_strf(completions, "%s/", arg_dir);
        }
        
        helper_add_file_paths(completions, files, arg_dir);
        return false;
    }
    o_del(files);


    // no files found, just list parent dir
    char *parent_dir = "";
    osize end_slash = o_str_find_back_char(arg_dir, '/');
    if (end_slash > 0) {
        parent_dir = o_str_clone_sub(completions, arg_dir, 0, end_slash+1);
    } else if (arg_dir[0] == '$' || arg_dir[0] == '&' || arg_dir[0] == '#') {
        parent_dir = o_new(completions, char, 2);
        parent_dir[0] = arg_dir[0];
        parent_dir[1] = '\0';
    }


    files = o_file_list(completions, parent_dir, 0, NULL);
    files_num = OList_num(files);
    if (files_num <= 0) {
        // nothing found
        return false;
    }

    helper_add_file_paths(completions, files, parent_dir);
    return false;
}