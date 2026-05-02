#ifdef MIA_OPTION_SDL2
#include <SDL2/SDL_events.h>
#define SDL_EVENT_KEY_DOWN SDL_KEYDOWN
#define SDLK_APOSTROPHE SDLK_QUOTE
#define SDL_KMOD_SHIFT KMOD_SHIFT
#define SDL_KMOD_CAPS KMOD_CAPS
#define SDLK_A SDLK_a
#define SDLK_B SDLK_b
#define SDLK_C SDLK_c
#define SDLK_D SDLK_d
#define SDLK_E SDLK_e
#define SDLK_F SDLK_f
#define SDLK_G SDLK_g
#define SDLK_H SDLK_h
#define SDLK_I SDLK_i
#define SDLK_J SDLK_j
#define SDLK_K SDLK_k
#define SDLK_L SDLK_l
#define SDLK_M SDLK_m
#define SDLK_N SDLK_n
#define SDLK_O SDLK_o
#define SDLK_P SDLK_p
#define SDLK_Q SDLK_q
#define SDLK_R SDLK_r
#define SDLK_S SDLK_s
#define SDLK_T SDLK_t
#define SDLK_U SDLK_u
#define SDLK_V SDLK_v
#define SDLK_W SDLK_w
#define SDLK_X SDLK_w
#define SDLK_Y SDLK_y
#define SDLK_Z SDLK_z
#else
#include <SDL3/SDL.h>
#endif

#include "a/input.h"
#include "o/OPtr.h"
#include "o/OStreamArray.h"
#include "o/OArray.h"
#include "o/OList.h"
#include "o/str.h"
#include "m/vec/vec2.h"
#include "a/app.h"
#include <ctype.h>

#undef O_LOG_LIB
#define O_LOG_LIB "a"
#include "o/log.h"


static struct {
    bool digital_current[A_INPUT_ENUM_MAX];
    bool digital_prev[A_INPUT_ENUM_MAX];
    float analogs[A_INPUT_ANALOG_ENUM_MAX];

    bool key_escape_down;
    bool key_back_down;

    // OList of OPtr of OStreamArray
    oobj stream_ptr_array;

    // OStreamArray (optional) cleared each frame
    oobj stream_current;

    void *sdl_gamepad;
    ou32 sdl_gamepad_id;

    ou64 key_timer;
    ou64 gamepad_timer;
} input_L;


static const char *a_input__stream_text(SDL_Keycode keycode, bool shift);


//
// public
//

bool a_input_down(enum a_input_digital field)
{
    return input_L.digital_current[field];
}

bool a_input_pressed(enum a_input_digital field)
{
    return input_L.digital_current[field] && !input_L.digital_prev[field];
}

bool a_input_released(enum a_input_digital field)
{
    return !input_L.digital_current[field] && input_L.digital_prev[field];
}

float a_input_analog(enum a_input_analog analog)
{
    return input_L.analogs[analog];
}

vec2 a_input_analog_vec2(enum a_input_analog analog)
{
    return vec2_(input_L.analogs[analog], input_L.analogs[analog+1]);
}

char a_input_digital_symbol(enum a_input_digital field)
{
    SDL_Scancode scancode = 0;
    switch (field) {
        case A_INPUT_1:
            scancode = SDL_SCANCODE_1;
            break;
        case A_INPUT_2:
            scancode = SDL_SCANCODE_2;
            break;
        case A_INPUT_3:
            scancode = SDL_SCANCODE_3;
            break;
        case A_INPUT_4:
            scancode = SDL_SCANCODE_4;
            break;
        case A_INPUT_5:
            scancode = SDL_SCANCODE_5;
            break;
        case A_INPUT_6:
            scancode = SDL_SCANCODE_6;
            break;
        case A_INPUT_7:
            scancode = SDL_SCANCODE_7;
            break;
        case A_INPUT_8:
            scancode = SDL_SCANCODE_8;
            break;
        case A_INPUT_9:
            scancode = SDL_SCANCODE_9;
            break;
        case A_INPUT_0:
            scancode = SDL_SCANCODE_0;
            break;

        case A_INPUT_Q:
            scancode = SDL_SCANCODE_Q;
            break;
        case A_INPUT_W:
            scancode = SDL_SCANCODE_W;
            break;
        case A_INPUT_E:
            scancode = SDL_SCANCODE_E;
            break;
        case A_INPUT_R:
            scancode = SDL_SCANCODE_R;
            break;
        case A_INPUT_T:
            scancode = SDL_SCANCODE_T;
            break;
        case A_INPUT_Y:
            scancode = SDL_SCANCODE_Y;
            break;
        case A_INPUT_U:
            scancode = SDL_SCANCODE_U;
            break;
        case A_INPUT_I:
            scancode = SDL_SCANCODE_I;
            break;
        case A_INPUT_O:
            scancode = SDL_SCANCODE_O;
            break;
        case A_INPUT_P:
            scancode = SDL_SCANCODE_P;
            break;

        case A_INPUT_A:
            scancode = SDL_SCANCODE_A;
            break;
        case A_INPUT_S:
            scancode = SDL_SCANCODE_S;
            break;
        case A_INPUT_D:
            scancode = SDL_SCANCODE_D;
            break;
        case A_INPUT_F:
            scancode = SDL_SCANCODE_F;
            break;
        case A_INPUT_G:
            scancode = SDL_SCANCODE_G;
            break;
        case A_INPUT_H:
            scancode = SDL_SCANCODE_H;
            break;
        case A_INPUT_J:
            scancode = SDL_SCANCODE_J;
            break;
        case A_INPUT_K:
            scancode = SDL_SCANCODE_K;
            break;
        case A_INPUT_L:
            scancode = SDL_SCANCODE_L;
            break;

        case A_INPUT_Z:
            scancode = SDL_SCANCODE_Z;
            break;
        case A_INPUT_X:
            scancode = SDL_SCANCODE_X;
            break;
        case A_INPUT_C:
            scancode = SDL_SCANCODE_C;
            break;
        case A_INPUT_V:
            scancode = SDL_SCANCODE_V;
            break;
        case A_INPUT_B:
            scancode = SDL_SCANCODE_B;
            break;
        case A_INPUT_N:
            scancode = SDL_SCANCODE_N;
            break;
        case A_INPUT_M:
            scancode = SDL_SCANCODE_M;
            break;

        default:
            return 0;
    }
    SDL_Keycode key;

#ifdef MIA_OPTION_SDL2
    key = SDL_GetKeyFromScancode(scancode);
#else
    key = SDL_GetKeyFromScancode(scancode, SDL_KMOD_NONE, false);
#endif

    return key;
}

oobj a_input_key_stream(oobj parent)
{
    o_thread_assert_main();
    if(!input_L.stream_ptr_array) {
        input_L.stream_ptr_array = OList_new(a_root(), NULL, 0);
    }

    o_log_info_s(__func__, "new key stream");

    oobj stream_array = OArray_new_dyn(parent, NULL, sizeof(char), 0, 256);
    oobj stream = OStreamArray_new(parent, stream_array, true, OStreamArray_FIFO);

    oobj ptr = OPtr_new(input_L.stream_ptr_array, stream);
    OList_push(input_L.stream_ptr_array, ptr);
    return stream;
}

const char *a_input_key_string()
{
    o_thread_assert_main();
    if(!input_L.stream_current) {
        input_L.stream_current = a_input_key_stream(a_root());
    }
    oobj array = OStreamArray_array(input_L.stream_current);
    return OArray_data(array, char);
}

ou64 a_input_key_timer(void)
{
    return input_L.key_timer;
}

ou64 a_input_gamepad_timer(void)
{
    return input_L.gamepad_timer;
}

//
// protected
//

// protected
void a_input__init(void)
{
#ifndef MIA_OPTION_SDL2
    // this is needed for emscripten to enable real keycodes like on other platforms
    // otherwise keycodes (which represent the symbol) are sometimes scancodes (key layout position)
    // so this hint makes from a qwertz keyboard qwertz instead of qwerty
    SDL_SetHint(SDL_HINT_KEYCODE_OPTIONS, "none");
#endif
}

// protected
void a_input__update(void)
{
    if (input_L.stream_current) {
        OArray_clear(OStreamArray_array(input_L.stream_current));
    }

    input_L.analogs[A_INPUT_ANALOG_WHEEL_X] = 0;
    input_L.analogs[A_INPUT_ANALOG_WHEEL_Y] = 0;

    o_memcpy(input_L.digital_prev, input_L.digital_current, sizeof input_L.digital_prev, 1);

#ifdef MIA_OPTION_GAMEPAD
    if (input_L.sdl_gamepad) {
        SDL_Gamepad *gp = input_L.sdl_gamepad;

        input_L.digital_current[A_INPUT_GAMEPAD_START] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_START);
        input_L.digital_current[A_INPUT_GAMEPAD_SHOULDER_L] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
        input_L.digital_current[A_INPUT_GAMEPAD_SHOULDER_R] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
        input_L.digital_current[A_INPUT_GAMEPAD_STICK_L] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_LEFT_STICK);
        input_L.digital_current[A_INPUT_GAMEPAD_STICK_R] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_RIGHT_STICK);
        input_L.digital_current[A_INPUT_GAMEPAD_DPAD_UP] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_DPAD_UP);
        input_L.digital_current[A_INPUT_GAMEPAD_DPAD_LEFT] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
        input_L.digital_current[A_INPUT_GAMEPAD_DPAD_DOWN] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
        input_L.digital_current[A_INPUT_GAMEPAD_DPAD_RIGHT] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
        input_L.digital_current[A_INPUT_GAMEPAD_NORTH] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_NORTH);
        input_L.digital_current[A_INPUT_GAMEPAD_EAST] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_EAST);
        input_L.digital_current[A_INPUT_GAMEPAD_SOUTH] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_SOUTH);
        input_L.digital_current[A_INPUT_GAMEPAD_WEST] = SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_WEST);

        input_L.analogs[A_INPUT_ANALOG_GAMEPAD_STICK_LX] = +(SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_LEFTX)/32768.0f);
        input_L.analogs[A_INPUT_ANALOG_GAMEPAD_STICK_LY] = -(SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_LEFTY)/32768.0f);
        input_L.analogs[A_INPUT_ANALOG_GAMEPAD_STICK_RX] = +(SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_RIGHTX)/32768.0f);
        input_L.analogs[A_INPUT_ANALOG_GAMEPAD_STICK_LY] = -(SDL_GetGamepadAxis(gp, SDL_GAMEPAD_AXIS_RIGHTY)/32768.0f);

        input_L.gamepad_timer = a_timer();
    }
#endif //MIA_OPTION_GAMEPAD
}

// protected
void a_input__handle_event_key(SDL_Event *event)
{
    input_L.key_timer = a_timer();

    // Difference between keycodes and scancodes:
    // keycode is what the symbol pressed is
    // scancodes is the position according to qwerty
    // we use scancodes here to get wasd feeling on say french AZERTY
    // and keycodes for our a_input_key_stream
#ifdef MIA_OPTION_SDL2
    SDL_Scancode scancode = event->key.keysym.scancode;
    SDL_Keycode keycode = event->key.keysym.sym;
#else
    SDL_Scancode scancode = event->key.scancode;
    SDL_Keycode keycode = event->key.key;
#endif
    bool down = event->type == SDL_EVENT_KEY_DOWN;
    switch (scancode) {
        case SDL_SCANCODE_RETURN:
            input_L.digital_current[A_INPUT_ENTER] = down;
            break;
        case SDL_SCANCODE_SPACE:
            input_L.digital_current[A_INPUT_SPACE] = down;
            break;
        case SDL_SCANCODE_ESCAPE:
            input_L.key_escape_down = down;
            input_L.digital_current[A_INPUT_ESCAPE] = input_L.key_escape_down || input_L.key_back_down;
            break;
        case SDL_SCANCODE_AC_BACK:
            input_L.key_back_down = down;
            input_L.digital_current[A_INPUT_ESCAPE] = input_L.key_escape_down || input_L.key_back_down;
            break;
        case SDL_SCANCODE_UP:
            input_L.digital_current[A_INPUT_UP] = down;
            break;
        case SDL_SCANCODE_LEFT:
            input_L.digital_current[A_INPUT_LEFT] = down;
            break;
        case SDL_SCANCODE_DOWN:
            input_L.digital_current[A_INPUT_DOWN] = down;
            break;
        case SDL_SCANCODE_RIGHT:
            input_L.digital_current[A_INPUT_RIGHT] = down;
            break;

        case SDL_SCANCODE_1:
            input_L.digital_current[A_INPUT_1] = down;
            break;
        case SDL_SCANCODE_2:
            input_L.digital_current[A_INPUT_2] = down;
            break;
        case SDL_SCANCODE_3:
            input_L.digital_current[A_INPUT_3] = down;
            break;
        case SDL_SCANCODE_4:
            input_L.digital_current[A_INPUT_4] = down;
            break;
        case SDL_SCANCODE_5:
            input_L.digital_current[A_INPUT_5] = down;
            break;
        case SDL_SCANCODE_6:
            input_L.digital_current[A_INPUT_6] = down;
            break;
        case SDL_SCANCODE_7:
            input_L.digital_current[A_INPUT_7] = down;
            break;
        case SDL_SCANCODE_8:
            input_L.digital_current[A_INPUT_8] = down;
            break;
        case SDL_SCANCODE_9:
            input_L.digital_current[A_INPUT_9] = down;
            break;
        case SDL_SCANCODE_0:
            input_L.digital_current[A_INPUT_0] = down;
            break;

        case SDL_SCANCODE_Q:
            input_L.digital_current[A_INPUT_Q] = down;
            break;
        case SDL_SCANCODE_W:
            input_L.digital_current[A_INPUT_W] = down;
            break;
        case SDL_SCANCODE_E:
            input_L.digital_current[A_INPUT_E] = down;
            break;
        case SDL_SCANCODE_R:
            input_L.digital_current[A_INPUT_R] = down;
            break;
        case SDL_SCANCODE_T:
            input_L.digital_current[A_INPUT_T] = down;
            break;
        case SDL_SCANCODE_Y:
            input_L.digital_current[A_INPUT_Y] = down;
            break;
        case SDL_SCANCODE_U:
            input_L.digital_current[A_INPUT_U] = down;
            break;
        case SDL_SCANCODE_I:
            input_L.digital_current[A_INPUT_I] = down;
            break;
        case SDL_SCANCODE_O:
            input_L.digital_current[A_INPUT_O] = down;
            break;
        case SDL_SCANCODE_P:
            input_L.digital_current[A_INPUT_P] = down;
            break;

        case SDL_SCANCODE_A:
            input_L.digital_current[A_INPUT_A] = down;
            break;
        case SDL_SCANCODE_S:
            input_L.digital_current[A_INPUT_S] = down;
            break;
        case SDL_SCANCODE_D:
            input_L.digital_current[A_INPUT_D] = down;
            break;
        case SDL_SCANCODE_F:
            input_L.digital_current[A_INPUT_F] = down;
            break;
        case SDL_SCANCODE_G:
            input_L.digital_current[A_INPUT_G] = down;
            break;
        case SDL_SCANCODE_H:
            input_L.digital_current[A_INPUT_H] = down;
            break;
        case SDL_SCANCODE_J:
            input_L.digital_current[A_INPUT_J] = down;
            break;
        case SDL_SCANCODE_K:
            input_L.digital_current[A_INPUT_K] = down;
            break;
        case SDL_SCANCODE_L:
            input_L.digital_current[A_INPUT_L] = down;
            break;

        case SDL_SCANCODE_Z:
            input_L.digital_current[A_INPUT_Z] = down;
            break;
        case SDL_SCANCODE_X:
            input_L.digital_current[A_INPUT_X] = down;
            break;
        case SDL_SCANCODE_C:
            input_L.digital_current[A_INPUT_C] = down;
            break;
        case SDL_SCANCODE_V:
            input_L.digital_current[A_INPUT_V] = down;
            break;
        case SDL_SCANCODE_B:
            input_L.digital_current[A_INPUT_B] = down;
            break;
        case SDL_SCANCODE_N:
            input_L.digital_current[A_INPUT_N] = down;
            break;
        case SDL_SCANCODE_M:
            input_L.digital_current[A_INPUT_M] = down;
            break;

        default:
            break;
    }

    if(down && input_L.stream_ptr_array && OList_num(input_L.stream_ptr_array)>0) {
        bool shift = SDL_GetModState() & SDL_KMOD_SHIFT;
        bool caps = SDL_GetModState() & SDL_KMOD_CAPS;
        if(caps) {
            shift = !shift;
        }
        const char *text = a_input__stream_text(keycode, shift);

        for(osize i=0; i<OList_num(input_L.stream_ptr_array); i++) {
            oobj ptr = OList_at(input_L.stream_ptr_array, i);
            struct oobj_opt stream = OPtr_get(ptr);
            if(stream.o) {
                OStream_print(stream.o, text);
            } else {
                o_log_info_s("a_input_key_stream", "stream got deleted");
                o_del(ptr);
                OList_pop_at(input_L.stream_ptr_array, i);
                i--;
            }
        }
    }
}

// protected
void a_input__handle_event_wheel(SDL_Event *event)
{
    // a event->direction exists with "flipped", but at least on my mac it's exactly wrong..?
    // (so just ignored)
    vec2 add = vec2_(0);
#ifdef MIA_OPTION_SDL2
#ifdef MIA_PLATFORM_CXXDROID
    add = vec2_(+event->wheel.x, +event->wheel.y);
#else
    add = vec2_(+event->wheel.preciseX, +event->wheel.preciseY);
#endif
#else
    // sdl3
    add = vec2_(+event->wheel.x, +event->wheel.y);
#endif

    input_L.analogs[A_INPUT_ANALOG_WHEEL_X] += add.x;
    input_L.analogs[A_INPUT_ANALOG_WHEEL_Y] += add.y;
}

// protected
void a_input__handle_event_gamepad(SDL_Event *event)
{
#ifdef MIA_OPTION_GAMEPAD
    if (event->type == SDL_EVENT_GAMEPAD_REMOVED || event->type == SDL_EVENT_GAMEPAD_REMAPPED) {
        if (input_L.sdl_gamepad && event->gdevice.which == input_L.sdl_gamepad_id) {
            o_log_s("event_gamepad", "Closing Gamepad");
            SDL_CloseGamepad(input_L.sdl_gamepad);
            input_L.sdl_gamepad = NULL;
            input_L.sdl_gamepad_id = 0;
            for (int i=A_INPUT_ANALOG_GAMEPAD_STICK_LX; i<=A_INPUT_ANALOG_GAMEPAD_STICK_RY; i++) {
                input_L.analogs[i] = 0.0f;
            }
            for (int i=A_INPUT_GAMEPAD_START; i<=A_INPUT_GAMEPAD_WEST; i++) {
                input_L.digital_current[i] = input_L.digital_prev[i] = false;
            }
            return;
        }
    }

    if (input_L.sdl_gamepad) {
        return;
    }

    o_log_debug_s("event_gamepad", "checking available joysticks");
    SDL_JoystickID *joys = SDL_GetJoysticks(NULL);
    if (!joys) {
        return;
    }
    for (;*joys; joys++) {
        if (!SDL_IsGamepad(*joys)) {
            continue;
        }

        char *name = o_str_tolower(a_tmp(), SDL_GetGamepadNameForID(*joys));
        bool name_valid = true;
        const char *blacklist[] = {
            "uinput-fortsense",     // android 12 virtual gamecontroller, sebi bug
            "uinput-fpc",           // finger print controller? according to a bug in libgdx
            "uinput-goodixfp",      // reddit guy stating that...
            "uinput-goodix",        // ...
            NULL
        };

        for (const char **it=blacklist; *it; it++) {
            if (o_str_equals(name, *it)) {
                name_valid = false;
                break;
            }
        }

        if (!name_valid) {
            o_log_s("event_gamepad", "Gamepad on blacklist: %s", name);
            continue;
        }

        SDL_Gamepad *gp = SDL_OpenGamepad(*joys);
        if (!gp) {
            continue;
        }

        input_L.sdl_gamepad = gp;
        input_L.sdl_gamepad_id = *joys;
        o_log_s("event_gamepad", "Opened: %s", name);
        break;
    }

#endif
}

//
// private
//

// forwarded
const char *a_input__stream_text(SDL_Keycode keycode, bool shift)
{
    static char char_result[2] = {0};
    if (keycode >= SDLK_A && keycode <= SDLK_Z) {
        if (shift) {
            char_result[0] = (char) toupper((char) keycode);
            return char_result;
        } else {
            char_result[0] = (char) keycode;
            return char_result;
        }
    } else if (keycode >= SDLK_0 && keycode <= SDLK_9) {
        if (shift) {
            int idx = keycode - '0';
            const char *specials_on_qwertz = "=!\"$$%&/()";
            char_result[0] = specials_on_qwertz[idx];
            return char_result;
        } else {
            char_result[0] = (char) keycode;
            return char_result;
        }
    } else {
        switch (keycode) {
            case SDLK_SPACE: return " ";
            case SDLK_TAB: return "\t";
            case SDLK_RETURN: return "\n";
            case SDLK_BACKSPACE: return "\b";
            case SDLK_LEFT: return A_INPUT_ESCAPED_LEFT;
            case SDLK_RIGHT: return A_INPUT_ESCAPED_RIGHT;
            case SDLK_UP: return A_INPUT_ESCAPED_UP;
            case SDLK_DOWN: return A_INPUT_ESCAPED_DOWN;
            case SDLK_HASH: return "#";
            case SDLK_DOLLAR: return "$";
            case SDLK_PERCENT: return "%";
            case SDLK_AMPERSAND: return "&";
            case SDLK_MINUS: return "-";
            case SDLK_EQUALS: return "=";
            case SDLK_LEFTBRACKET: return "[";
            case SDLK_RIGHTBRACKET: return "]";
            case SDLK_BACKSLASH: return "\\";
            case SDLK_SEMICOLON: return ";";
            case SDLK_APOSTROPHE: return "'";
            case SDLK_COMMA: return ",";
            case SDLK_PERIOD: return ".";
            case SDLK_SLASH: return "/";
            case SDLK_KP_DIVIDE: return "/";
            case SDLK_KP_MULTIPLY: return "*";
            case SDLK_KP_MINUS: return "-";
            case SDLK_KP_PLUS: return "+";
            case SDLK_KP_ENTER: return "\n";
            default: break;
        }
    }
    return NULL; // Not a printable character or sequence
}

bool a_input_gamepad_available(void)
{
    return input_L.sdl_gamepad != NULL;
}

// protected
void *a_input__sdl_gamepad(void)
{
    return input_L.sdl_gamepad;
}