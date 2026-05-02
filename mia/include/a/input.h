#ifndef A_INPUT_HEADER
#define A_INPUT_HEADER

/**
 * @file input.h
 *
 * inputs like mouse wheel, keys, gamepad.
 *      Note: for pointer events, see pointer.h
 */

#include "o/common.h"
#include "m/types/flt.h"


#define A_INPUT_ESCAPED_UP       "\x1B[A"
#define A_INPUT_ESCAPED_DOWN     "\x1B[B"
#define A_INPUT_ESCAPED_RIGHT    "\x1B[C"
#define A_INPUT_ESCAPED_LEFT     "\x1B[D"

/**
 * Digital buttons.
 * Keyboard buttons as "scancodes" to get the physical layout independent position.
 * So on a french AZERTY our A_INPUT_W maps to 'Z'
 */
enum a_input_digital {
    A_INPUT_ENTER,
    A_INPUT_SPACE,
    A_INPUT_ESCAPE,
    A_INPUT_UP,
    A_INPUT_LEFT,
    A_INPUT_DOWN,
    A_INPUT_RIGHT,

    A_INPUT_1,
    A_INPUT_2,
    A_INPUT_3,
    A_INPUT_4,
    A_INPUT_5,
    A_INPUT_6,
    A_INPUT_7,
    A_INPUT_8,
    A_INPUT_9,
    A_INPUT_0,

    A_INPUT_Q,
    A_INPUT_W,
    A_INPUT_E,
    A_INPUT_R,
    A_INPUT_T,
    A_INPUT_Y,
    A_INPUT_U,
    A_INPUT_I,
    A_INPUT_O,
    A_INPUT_P,

    A_INPUT_A,
    A_INPUT_S,
    A_INPUT_D,
    A_INPUT_F,
    A_INPUT_G,
    A_INPUT_H,
    A_INPUT_J,
    A_INPUT_K,
    A_INPUT_L,

    A_INPUT_Z,
    A_INPUT_X,
    A_INPUT_C,
    A_INPUT_V,
    A_INPUT_B,
    A_INPUT_N,
    A_INPUT_M,

    A_INPUT_GAMEPAD_START,
    A_INPUT_GAMEPAD_SHOULDER_L,
    A_INPUT_GAMEPAD_SHOULDER_R,
    A_INPUT_GAMEPAD_STICK_L,
    A_INPUT_GAMEPAD_STICK_R,
    A_INPUT_GAMEPAD_DPAD_UP,
    A_INPUT_GAMEPAD_DPAD_LEFT,
    A_INPUT_GAMEPAD_DPAD_DOWN,
    A_INPUT_GAMEPAD_DPAD_RIGHT,
    A_INPUT_GAMEPAD_NORTH,
    A_INPUT_GAMEPAD_EAST,
    A_INPUT_GAMEPAD_SOUTH,
    A_INPUT_GAMEPAD_WEST,

    A_INPUT_ENUM_MAX
};

enum a_input_analog {
    // mouse wheel
    A_INPUT_ANALOG_WHEEL_X,
    A_INPUT_ANALOG_WHEEL_Y,

    A_INPUT_ANALOG_GAMEPAD_STICK_LX,
    A_INPUT_ANALOG_GAMEPAD_STICK_LY,
    A_INPUT_ANALOG_GAMEPAD_STICK_RX,
    A_INPUT_ANALOG_GAMEPAD_STICK_RY,

    A_INPUT_ANALOG_ENUM_MAX,
};


/**
 * @param field: key or button of enum a_input_digital
 * @return true if currently pressed down
 * @note internally uses "scancodes" to get the physical layout independent position.
 *       So on a french AZERTY our A_INPUT_W maps to 'Z'
 */
bool a_input_down(enum a_input_digital field);

/**
 * @param field: key or button of enum a_input_digital
* @return true if got pressed in this frame
 * @note internally uses "scancodes" to get the physical layout independent position.
 *       So on a french AZERTY our A_INPUT_W maps to 'Z'
 */
bool a_input_pressed(enum a_input_digital field);

/**
 * @param field: key or button of enum a_input_digital
* @return true if got released in this frame
 * @note internally uses "scancodes" to get the physical layout independent position.
 *       So on a french AZERTY our A_INPUT_W maps to 'Z'
 */
bool a_input_released(enum a_input_digital field);


/**
 * @param analog: Analog source of enum a_input_analog
 * @return float is often (but not always) normalized
 *         A_INPUT_ANALOG_WHEEL_*:          integer for each step and fraction for inbetween
 *         A_INPUT_ANALOG_GAMEPAD_STICK_*:  range [-1.0 : +1.0]
 * @sa a_input_analog_vec2
 */
float a_input_analog(enum a_input_analog analog);

/**
 * @param analog: Analog source of enum a_input_analog of two fields together, (uses the next for .y)
 *                A_INPUT_ANALOG_WHEEL_X for XY
 *                A_INPUT_ANALOG_GAMEPAD_STICK_LX for LEFT XY
 *                A_INPUT_ANALOG_GAMEPAD_STICK_RX for RIGHT XY
 * @return float is often (but not always) normalized
 *         A_INPUT_ANALOG_WHEEL_*:          integer for each step and fraction for inbetween
 *         A_INPUT_ANALOG_GAMEPAD_STICK_*:  range [-1.0 : +1.0]
 */
vec2 a_input_analog_vec2(enum a_input_analog analog);

/**
 * The digital fields are using scancodes not keycodes.
 * So on a french AZERTY our A_INPUT_W maps to 'z'.
 * This function returns the key symbol on A_INPUT_W (which would return 'z' on AZERTY).
 * @param field digital scancode to get the symbol from
 * @return single char or 0 if not printable in a char (A_INPUT_UP as an example...)
 */
char a_input_digital_symbol(enum a_input_digital field);


/**
 * @param parent to allocate on
 * @return resulting FIFO OStreamArray to read on (writing pushs back new keys...)
 * @note In contrast to a_input_down|pressed|released this function uses keycodes.
 *       So this time the french AZERTY 'Z' really prints a 'Z'
 *       Stream may contain "\b" for backspace or escape codes like "\x1B[A" for cursor Up
 *       escaped codes may be of:
 *       "\x1B[A" Cursor Up
 *       "\x1B[B" Cursor Down
 *       "\x1B[C" Cursor Right
 *       "\x1B[D" Cursor Left
 *       not all special characters work! only a handful and the alphanumerics
 * @note this function call asserts main thread.
 *       deletion of the OStreamArray must also be done in the main thread (releases internal data).
 *       During its lifetime, the OStream may be used in another thread.
 *       Internally, an OPtr is created on the OStream, which is not thread safe...
 */
oobj a_input_key_stream(oobj parent);

/**
 * @return The input key string of the last frame. (never returns NULL)
 * @note Uses an internal a_input_key_stream which is cleared each frame start.
 *       Installed on first usage.
 *       asserts main thread.
 */
const char *a_input_key_string();

/**
 * @return the a_timer() value of the frame that had the last event.
 *         Compare with o_timer_diff_s()
 */
ou64 a_input_key_timer(void);


/**
 * @return True if a gamepad is currently available
 */
bool a_input_gamepad_available(void);

/**
 * @return the a_timer() value of the frame that had the last event.
 *         Compare with o_timer_diff_s()
 */
ou64 a_input_gamepad_timer(void);


#endif //A_INPUT_HEADER
