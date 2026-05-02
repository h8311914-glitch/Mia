#ifndef A_ACTION_H
#define A_ACTION_H

/**
 * @file action.h
 *
 * In contrast to input, actions may map multiple sources, such as input sources.
 * Accessible via key strings.
 * Actions are internally always of type float.
 *
 */

#include "o/common.h"
#include "m/types/flt.h"
#include "a/input.h"

#define A_ACTION_GAME_X "game_x"
#define A_ACTION_GAME_Y "game_y"
#define A_ACTION_GAME_ACTION "game_action"
#define A_ACTION_GAME_START "game_start"
#define A_ACTION_GAME_ESCAPE "game_escape"

/**
 * @param action new action to be installed, needed before calling *_set_* or *_down or *_analog, etc.
 */
void a_action_install(const char *action);


/**
 * @param parent to allocate on
 * @param opt_out_num set the the list size, if not NULL
 * @return a (heap) list of key strings (NULL terminated), use o_free to delete it
 * @note that list gets invalid if a_action_install is called
 */
const char **a_action_list(oobj parent, osize *opt_out_num);

/**
 * @param action key string to clear (set to false / 0)
 */
void a_action_clear(const char *action);

/**
 * @param action key string to set down for that frame.
 * @param value to add to the action (use 1 for digital actions)
 */
void a_action_set(const char *action, float value);

/**
 * @param action: key string, if not found an error is logged and false is returned
 * @return true if currently pressed down
 */
bool a_action_down(const char *action);

/**
 * @param action: key string, if not found an error is logged and false is returned
 * @return true if got pressed in this frame
 */
bool a_action_pressed(const char *action);

/**
 * @param action: key string, if not found an error is logged and false is returned
 * @return true if got released in this frame
 */
bool a_action_released(const char *action);


/**
 * @param action: key string, if not found an error is logged and false is returned
 * @return value of that action (summed up) in this frame (so may be beyond [-1:+1])
 */
float a_action_analog(const char *action);

/**
 * @param action_x, action_y: key strings, if not found an error is logged and false is returned
 * @return value of that action (summed up) in this frame (so may be beyond [-1:+1])
 */
o_inline vec2 a_action_analog_vec2(const char *action_x, const char *action_y)
{
    return vec2_(a_action_analog(action_x), a_action_analog(action_y));
}


//
// binding
//

/**
 * @param action to be set automatically
 * @param bind digital input to set automatically (will ignore duplicates)
 * @param scale to apply if down (1.0) (as an example X would be +1 for D but -1 for A (WASD KEYS)
 * @note calls a_action_install(action).
 *       asserts main thread.
 */
void a_action_bind_digital(const char *action, enum a_input_digital bind, float scale);

/**
 * @param action to be set automatically
 * @param bind analog input to set automatically (will ignore duplicates)
 * @param scale to apply on the analog input
 * @note calls a_action_install(action).
 *       asserts main thread.
 */
void a_action_bind_analog(const char *action, enum a_input_analog bind, float scale);

/**
 * @param action from which bindings are removed
 * @note asserts main thread.
 */
void a_action_bind_remove_from(const char *action);

/**
 * removes >all< bindings
 * @note asserts main thread.
 */
void a_action_bind_remove_all(void);


/**
 * predefined game actions.
 * A_ACTION_GAME_X:
 *      A_INPUT_LEFT|RIGHT + A_INPUT|D + A_INPUT_ANALOG_GAMEPAD_STICK_LX + A_INPUT_GAMEPAD_DPAD_LEFT|RIGHT
 * A_ACTION_GAME_Y:
 *      A_INPUT_DOWN|UP + A_INPUT_W|S + A_INPUT_ANALOG_GAMEPAD_STICK_LY + A_INPUT_GAMEPAD_DPAD_DOWN|UP
 * A_ACTION_GAME_ACTION:
 *      A_INPUT_SPACE + A_INPUT_GAMEPAD_SOUTH
 * A_ACTION_GAME_START:
 *      A_INPUT_ENTER + A_INPUT_GAMEPAD_START
 * A_ACTION_GAME_ESCAPE:
 *      A_INPUT_ESCAPE + A_INPUT_GAMEPAD_NORTH
* @note asserts main thread.
 */
void a_action_bind_game(void);


#endif //A_ACTION_H
