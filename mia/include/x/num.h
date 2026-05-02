#ifndef X_NUM_H
#define X_NUM_H

/**
 * @file num.h
 *
 * Creates a WNum and sets the num_pressed_event to open a viewnum edit scene
 */

#include "w/WNum.h"



/**
 * Creates a new WNum object.
 * Sets the num_pressed_event to open a viewnum edit scene
 * @param parent to inherit from
 * @param min, max minimal and maximal num (clamped)
 * @param step size between a value, will determine the printed precision (asserts >0)
 * @param opt_label if not NULL set as text in the frame label
 * @return The new object
 */
WNum *x_num(oobj parent, float min, float max, float step, const char *opt_label);


/**
 * Creates a new WNum object.
 * Sets the num_pressed_event to open a viewnum edit scene.
 * Custom replacement list instead of the mixed 6 btns.
 * @param parent to inherit from
 * @param min, max minimal and maximal num (clamped)
 * @param step size between a value, will determine the printed precision (asserts >0)
 * @param opt_label if not NULL set as text in the frame label
 * @param values a list to create replace buttons from
 * @param num_values number of values list items
 * @return The new object
 */
WNum *x_num_ex(oobj parent, float min, float max, float step, const char *opt_label, const float *values, osize num_values);

#endif //X_NUM_H
