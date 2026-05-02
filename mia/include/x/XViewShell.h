#ifndef X_XVIEWSHELL_H
#define X_XVIEWSHELL_H

/**
 * @file XViewShell.h
 *
 * object.
 *
 * AView, which shows a custom text based shell to use.
 * x_terminal has an instance of it.
 *
 * Subclass of the AView object
 */

#include "o/OList.h"
#include "m/types/int.h"
#include "a/AView.h"


/** object id */
#define XViewShell_ID AView_ID "XViewShell."

/**
 * Virtual shell api function for custom commands
 * As with C main, the command name is at argv[0] (so argc is always >=1 )
 * And argv id NULL terminated.
 * Call XViewShell_print to print a line on the terminal shell.
 */
typedef void (*XViewShell_api_fn)(oobj shell, oobj user_obj, int argc, char **argv);

/**
 * Optional autocomplete helper function for custom commands.
 * Called like x_terminal_api_fn with the current input.
 * @param completions: OList of possible strings.
 *                     Is cleared before calling.
 *                     Push autocomplete arguments here.
 * @return true: The function is not called again as it got all options (e. g. list of env keys) 
 *         false: its called again on each character input (e. g. nested ls).
 * @note do not do any actions if this fn is called
 */
typedef bool (*XViewShell_api_autocomplete_fn)(oobj shell, oobj user_obj, OList *completions, int argc, char **argv);



typedef struct {
    AView super;
    
    // OStream to read from as shell input
    oobj stream;
    
    // (const) text columns for the shell
    int cols;
    // (const) maximal visible text rows, typically less are rendered
    int rows;
    
    // characters of the input line
    oobj input_array;
    // printed text on the shell
    oobj text_array;
    
    // autocomplete
    oobj ac_hint_array;
    oobj ac_current_list;
    oobj ac_api_list;
    bool ac_api_list_done;
    
    // cursor psotion on the input line
    int cursor;
    float cursor_blink_time;
    
    // input character set to ignore
    // currently unused / NULL ?
    char *ignore_filter;
    
    // of [rows][cols]
    char *buffer;
    ivec2 next;
    
    // history [rows][cols]
    char *history;
    int history_next;
    int history_pos;
    
    // text render object
    oobj text_ro;
    
    oobj api_dict;
    
} XViewShell;


/**
 * Initializes the object.
 * Creates an AView that renders a text based shell.
 * @param obj XViewShell object
 * @param parent to inherit from
 * @param stream OStream to read character input from.
 *               For example: set to XViewKeys_stream
 *               Must live as long as the ViewShell!
 * @param cols columns of shell characters
 * @param rows maximal available shell rows, typcially less should are visible
 * @return obj casted as XViewShell
 */
XViewShell *XViewShell_init(oobj obj, oobj parent, oobj stream, int cols, int rows);

/**
 * Creates a new XViewShell object
 * Creates an AView that renders a text based shell.
 * @param parent to inherit from
 * @param stream OStream to read character input from.
 *               For example: set to XViewKeys_stream
 *               Must live as long as the ViewShell!
 * @param cols columns of shell characters
 * @param rows maximal available shell rows, typcially less should are visible
 * @return The new object
 */
o_inline XViewShell *XViewShell_new(oobj parent, oobj stream, int cols, int rows)
{
    OObj_DECL_IMPL_NEW(XViewShell, parent, stream, cols, rows);
}


//
// virtual implementations
//

void XViewShell__v_setup(oobj view);

void XViewShell__v_update(oobj view, oobj tex, float dt);

void XViewShell__v_render(oobj view, oobj tex, float dt);


//
// object functions:
//

/**
 * @param obj XViewShell object
 * @return OStream to read the input from.
 *         For example: set to XViewKeys_stream
 *         Must live as long as the ViewShell!
 */
OObj_DECL_GETSET(XViewShell, oobj, stream)

/**
 * @param obj XViewShell object
 * @return shell text character columns
 */
OObj_DECL_GET(XViewShell, int, cols)

/**
 * @param obj XViewShell object
 * @return maximal shell text character rows.
 *         Typically less are rendered
 */
OObj_DECL_GET(XViewShell, int, rows)

/**
 * Prints a new line to the shell terminal.
 * If the line exceeds cols (-1),
 *     the remaining is printed to the next line.
 * '\n' also work to print multiple lines.
 * The shell is not extrem big and currently does not support scrolling.
 * So keep your results minimal.
 * @param obj XViewShell object
 * @param str to be printed (NULL safe -> noop)
 */
void XViewShell_print(oobj obj, const char *str);

/**
 * Clears the output shell text
 * @param obj XViewShell object
 * @note uses o_arena to allocate string of rows*'\n' to be printed
 */
void XViewShell_clear(oobj obj);


/**
 * Evaluates the given input string.
 * Splits program and arguments.
 * Calls that api.
 * @param obj XViewShell object
 * @param input like "env foo bar"
 */
void XViewShell_eval(oobj obj, const char *input);

/**
 * Adds a new api command to the terminal shell.
 * @param obj XViewShell object
 * @param user_obj Use this to pass user data. If deleted the api stops working.
 *            Internally uses OPtr for that.
 * @param command casing is irrelevant
 * @param fn function that is called if command is typed
 * @param opt_autocomplete_fn if not NULL, called to get a list of possible arguments for auto completion
 * @param help string that is printed to the terminal from the help command.
 *        Like "help myapi" will print that help string
 */
void XViewShell_api_add(oobj obj, oobj user_obj, const char *command, 
        XViewShell_api_fn fn, 
        XViewShell_api_autocomplete_fn opt_autocomplete_fn,
        const char *help);


/**
 * This is a helper function to be used as XViewShell_api_autocomplete_fn function.
 * Depending on argv[1:] it lists that dir via o_list to generate the completions for that dir.
 * returns false, cause it always needs to reavulate.
 * @note does NOT make use of shell, nor user_obj, nor argv[0]
 */
bool XViewShell_helper_api_ls_autocomplete(oobj shell, oobj user_obj, OList *completions, int argc, char **argv);




#endif //X_XVIEWSHELL_H
