#include "a/action.h"
#include "o/OMap.h"
#include "o/OArray.h"
#include "o/str.h"
#include "a/common.h"


#undef O_LOG_LIB
#define O_LOG_LIB "a"
#include "o/log.h"
#include "o/str.h"


enum binding_type {
    BINDING_TYPE_DIGITAL,
    BINDING_TYPE_ANALOG,
    BINDING_TYPE_ENUM_MAX
};

struct action {
    float current;
    float prev;
};

struct action_binding {
    char *action;
    int source;
    float scale;
    enum binding_type type;
};

static struct {
    oobj action_map;
    oobj bindings_array;
    oobj bindings_action_container;
} L;


struct action *action_get(const char *action)
{
    if (!L.action_map) {
        o_log_error_s(__func__, "no actions are installed!");
        return NULL;
    }
    assert(action != NULL);
    struct action *a = OMap_get(L.action_map, &action, struct action);
    if (!a) {
        o_log_error_s(__func__, "action not found: \"%s\"", action);
    }
    return a;
}




// protected
void a_action__update(void)
{
    if (!L.action_map) {
        return;
    }
    osize actions_num = OMap_num(L.action_map);
    for (osize i = 0; i < actions_num; i++) {
        struct action *a = OMap_value_at(L.action_map, i, struct action);
        a->prev = a->current;
        a->current = 0;
    }

    osize bindings_num = OArray_num(L.bindings_array);
    for (osize i = 0; i < bindings_num; i++) {
        struct action_binding *b = OArray_at(L.bindings_array, i, struct action_binding);
        switch (b->type) {
            case BINDING_TYPE_DIGITAL:
                if (a_input_down(b->source)) {
                    a_action_set(b->action, 1.0f * b->scale);
                }
                break;
            case BINDING_TYPE_ANALOG:
                a_action_set(b->action, a_input_analog(b->source) * b->scale);
                break;
            default:
                assert(0);
                break;
        }
    }
}


//
// public
//
void a_action_install(const char *action)
{
    if (!L.action_map) {
        L.action_map = OMap_new_string_keys(a_root(), sizeof(struct action), 64);
        L.bindings_array = OArray_new_dyn(L.action_map, NULL, sizeof(struct action_binding), 0, 32);
        L.bindings_action_container = OObj_new(L.bindings_array);
    }
    struct action *a = OMap_get(L.action_map, &action, struct action);
    if (!a) {
        struct action init = {0};
        OMap_set(L.action_map, &action, &init);
    }
}

const char **a_action_list(oobj parent, osize *opt_out_num)
{
    if (!L.action_map) {
        o_opt_set(opt_out_num, 0);
        // Nevertheless, needs a {NULL}
        return o_new0(parent, char*, 1);
    }
    osize num = OMap_num(L.action_map);
    o_opt_set(opt_out_num, num);
    const char **list = o_new(parent, char*, num+1);
    for (osize i = 0; i < num; ++i) {
        const char **key = OMap_key_at(L.action_map, i, char *);
        list[i] = *key;
    }
    list[num] = NULL;
    return list;
}

void a_action_clear(const char *action)
{
    struct action *a = action_get(action);
    if (a) {
        a->prev = 0;
    }
}

void a_action_set(const char *action, float value)
{
    struct action *a = action_get(action);
    if (a) {
        a->current += value;
    }
}

bool a_action_down(const char *action)
{
    struct action *a = action_get(action);
    if (a) {
        return a->current != 0;
    }
    return false;
}

bool a_action_pressed(const char *action)
{
    struct action *a = action_get(action);
    if (a) {
        return a->current != 0 && a->prev == 0;
    }
    return false;
}

bool a_action_released(const char *action)
{
    struct action *a = action_get(action);
    if (a) {
        return a->current == 0 && a->prev != 0;
    }
    return false;
}

float a_action_analog(const char *action)
{
    struct action *a = action_get(action);
    if (a) {
        return a->current;
    }
    return 0;
}

//
// bindings
//


static bool bind_available(const char *action)
{
    osize num = OArray_num(L.bindings_array);
    for (osize i=0; i<num; i++) {
        struct action_binding *b = OArray_at(L.bindings_array, i, struct action_binding);
        if (o_str_equals(b->action, action)) {
            return true;
        }
    }
    return false;
}


void a_action_bind_digital(const char *action, enum a_input_digital bind, float scale)
{
    o_thread_assert_main();
    assert(action != NULL);
    a_action_install(action);
    if (bind_available(action)) {
        o_log_warn_s(__func__, "action \"%s\" already bound", action);
        return;
    }
    struct action_binding b = {
        o_str_clone(L.bindings_action_container, action),
        bind,
        scale,
        BINDING_TYPE_DIGITAL
    };
    OArray_push(L.bindings_array, &b);
}

void a_action_bind_analog(const char *action, enum a_input_analog bind, float scale)
{
    o_thread_assert_main();
    assert(action != NULL);
    a_action_install(action);
    if (bind_available(action)) {
        o_log_warn_s(__func__, "action \"%s\" already bound", action);
        return;
    }
    struct action_binding b = {
        o_str_clone(L.bindings_action_container, action),
        bind,
        scale,
        BINDING_TYPE_ANALOG
    };
    OArray_push(L.bindings_array, &b);
}

void a_action_bind_remove_from(const char *action)
{
    o_thread_assert_main();
    assert(action != NULL);
    if (!L.action_map) {
        o_log_error_s(__func__, "no actions are installed!");
        return;
    }
    int cnt = 0;
    for (osize i=0; i<OArray_num(L.bindings_array); i++) {
        struct action_binding *b = OArray_at(L.bindings_array, i, struct action_binding);
        if (o_str_equals(b->action, action)) {
            OArray_pop_at(L.bindings_array, i, NULL);
            i--;
            cnt++;
        }
    }
    if (cnt == 0) {
        o_log_warn_s(__func__, "action \"%s\" was not bound", action);
    }
}

void a_action_bind_remove_all(void)
{
    o_thread_assert_main();
    if (!L.action_map) {
        // remove in silent cause nothing to remove
        return;
    }
    o_del(L.bindings_action_container);
    L.bindings_action_container = OObj_new(L.bindings_array);
    OArray_clear(L.bindings_array);
}


void a_action_bind_game(void)
{
    a_action_bind_digital(A_ACTION_GAME_X, A_INPUT_LEFT, -1.0f);
    a_action_bind_digital(A_ACTION_GAME_X, A_INPUT_A, -1.0f);
    a_action_bind_digital(A_ACTION_GAME_X, A_INPUT_GAMEPAD_DPAD_LEFT, -1.0f);
    a_action_bind_digital(A_ACTION_GAME_X, A_INPUT_RIGHT, +1.0f);
    a_action_bind_digital(A_ACTION_GAME_X, A_INPUT_D, +1.0f);
    a_action_bind_digital(A_ACTION_GAME_X, A_INPUT_GAMEPAD_DPAD_RIGHT, +1.0f);
    a_action_bind_analog(A_ACTION_GAME_X, A_INPUT_ANALOG_GAMEPAD_STICK_LX, 1.0f);

    a_action_bind_digital(A_ACTION_GAME_Y, A_INPUT_DOWN, -1.0f);
    a_action_bind_digital(A_ACTION_GAME_Y, A_INPUT_S, -1.0f);
    a_action_bind_digital(A_ACTION_GAME_Y, A_INPUT_GAMEPAD_DPAD_DOWN, -1.0f);
    a_action_bind_digital(A_ACTION_GAME_Y, A_INPUT_UP, +1.0f);
    a_action_bind_digital(A_ACTION_GAME_Y, A_INPUT_W, +1.0f);
    a_action_bind_digital(A_ACTION_GAME_Y, A_INPUT_GAMEPAD_DPAD_UP, +1.0f);
    a_action_bind_analog(A_ACTION_GAME_Y, A_INPUT_ANALOG_GAMEPAD_STICK_LY, 1.0f);

    a_action_bind_digital(A_ACTION_GAME_ACTION, A_INPUT_SPACE, 1.0);
    a_action_bind_digital(A_ACTION_GAME_ACTION, A_INPUT_GAMEPAD_SOUTH, 1.0);

    a_action_bind_digital(A_ACTION_GAME_START, A_INPUT_ENTER, 1.0);
    a_action_bind_digital(A_ACTION_GAME_START, A_INPUT_GAMEPAD_START, 1.0);

    a_action_bind_digital(A_ACTION_GAME_ESCAPE, A_INPUT_ESCAPE, 1.0);
    a_action_bind_digital(A_ACTION_GAME_ESCAPE, A_INPUT_GAMEPAD_NORTH, 1.0);
}
