#ifndef O_TIMER_H
#define O_TIMER_H

/**
 * @file timer.h
 *
 * Provides functions to get current system times with high precision.
 * Includes a timer object to measure elapsed times in ticks, seconds, and milliseconds.
 */

#include "common.h"

/**
 * @return The high precision clock time as ticks.
 * @sa o_timer_freq to get the number of ticks per second.
 */
ou64 o_timer(void);

/**
 * @return The number of ticks per second.
 * @sa o_timer
 */
ou64 o_timer_freq_s(void);

//
// in ticks
//

/**
 * Helper function to return the difference in time in ticks.
 * @param start_time Time previously obtained with o_timer.
 * @return The elapsed time in ticks, which is new_time-start_time
 * @note start_time should be <= the new_time, else an overflow will occur.
 * @sa o_timer_freq
 */
o_inline ou64 o_timer_diff_ticks(ou64 start_time, ou64 new_time)
{
    return new_time - start_time;
}

/**
 * Helper function to return the elapsed time in ticks.
 * @param start_time Time previously obtained with o_timer.
 * @return The elapsed time in ticks.
 * @note start_time should be <= the current time, else an overflow will occur.
 * @sa o_timer_freq
 */
o_inline ou64 o_timer_elapsed_ticks(ou64 start_time)
{
    return o_timer() - start_time;
}

/**
 * Helper function to reset the timer and return the elapsed time in ticks.
 * @param in_out_start_time Pointer to a previously obtained start time with o_timer. Sets the new start time.
 * @return The elapsed time in ticks.
 * @note start_time should be <= the current time, else an overflow will occur.
 * @sa o_timer_freq
 */
o_inline ou64 o_timer_reset_ticks(ou64 *in_out_start_time)
{
    ou64 time = o_timer();
    ou64 elapsed = time - *in_out_start_time;
    *in_out_start_time = time;
    return elapsed;
}

//
// in seconds
//

/**
 * Helper function to return the difference in time in seconds.
 * @param start_time Time previously obtained with o_timer.
 * @return The elapsed time in seconds
 * @note start_time should be <= the new_time, else an overflow will occur.
 */
o_inline double o_timer_diff_s(ou64 start_time, ou64 new_time)
{
    return (double) o_timer_diff_ticks(start_time, new_time) / (double) o_timer_freq_s();
}

/**
 * Helper function to return the elapsed time in seconds.
 * @param start_time Time previously obtained with o_timer.
 * @return The elapsed time in seconds.
 * @note start_time should be <= the current time, else an overflow will occur.
 */
o_inline double o_timer_elapsed_s(ou64 start_time)
{
    return (double) o_timer_elapsed_ticks(start_time) / (double) o_timer_freq_s();
}

/**
 * Helper function to reset the timer and return the elapsed time in seconds.
 * @param in_out_start_time Pointer to a previously obtained start time with o_timer. Sets the new start time.
 * @return The elapsed time in seconds.
 * @note start_time should be <= the current time, else an overflow will occur.
 */
o_inline double o_timer_reset_s(ou64 *in_out_start_time)
{
    return (double) o_timer_reset_ticks(in_out_start_time) / (double) o_timer_freq_s();
}

//
// in milliseconds
//

/**
 * Helper function to return the difference in time in seconds.
 * @param start_time Time previously obtained with o_timer.
 * @return The elapsed time in ticks, which is new_time-start_time
 * @note start_time should be <= the new_time, else an overflow will occur.
 */
o_inline double o_timer_diff_millis(ou64 start_time, ou64 new_time)
{
    return (double) o_timer_diff_ticks(start_time, new_time) * 1000.0 / (double) o_timer_freq_s();
}

/**
 * Helper function to return the elapsed time in milliseconds.
 * @param start_time Time previously obtained with o_timer.
 * @return The elapsed time in milliseconds.
 * @note start_time should be <= the current time, else an overflow will occur.
 */
o_inline double o_timer_elapsed_millis(ou64 start_time)
{
    return (double) o_timer_elapsed_ticks(start_time) * 1000.0 / (double) o_timer_freq_s();
}

/**
 * Helper function to reset the timer and return the elapsed time in milliseconds.
 * @param in_out_start_time Pointer to a previously obtained start time with o_timer. Sets the new start time.
 * @return The elapsed time in milliseconds.
 * @note start_time should be <= the current time, else an overflow will occur.
 */
o_inline double o_timer_reset_millis(ou64 *in_out_start_time)
{
    return (double) o_timer_reset_ticks(in_out_start_time) * 1000.0 / (double) o_timer_freq_s();
}

//
// in milliseconds as integer (ou64)
//

/**
 * Helper function to return the difference in time in milliseconds.
 * @param start_time Time previously obtained with o_timer.
 * @return The elapsed time in milliseconds as integer (ou64).
 * @note start_time should be <= the new_time, else an overflow will occur.
 */
o_inline ou64 o_timer_diff_millis_int(ou64 start_time, ou64 new_time)
{
    return o_timer_diff_ticks(start_time, new_time) * 1000 / o_timer_freq_s();
}

/**
 * Helper function to return the elapsed time in milliseconds.
 * @param start_time Time previously obtained with o_timer.
 * @return The elapsed time in milliseconds as integer (ou64).
 * @note start_time should be <= the current time, else an overflow will occur.
 */
o_inline ou64 o_timer_elapsed_millis_int(ou64 start_time)
{
    return (double) o_timer_elapsed_ticks(start_time) * 1000 / o_timer_freq_s();
}

/**
 * Helper function to reset the timer and return the elapsed time in milliseconds.
 * @param in_out_start_time Pointer to a previously obtained start time with o_timer. Sets the new start time.
 * @return The elapsed time in milliseconds as integer (ou64).
 * @note start_time should be <= the current time, else an overflow will occur.
 */
o_inline ou64 o_timer_reset_millis_int(ou64 *in_out_start_time)
{
    return (double) o_timer_reset_ticks(in_out_start_time) * 1000 / o_timer_freq_s();
}

#endif // O_TIMER_H
