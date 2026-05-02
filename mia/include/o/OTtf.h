#ifdef MIA_OPTION_TTF
#ifndef O_OTTF_H
#define O_OTTF_H

/**
 * @file OTtf.h
 *
 * Object
 *
 * Loads a .ttf file to be rendered / drawn into a struct o_img.
 * I have to admit that this ttf object is somewhat small in functionality, may be enhanced later.
 *
 * @note Needs MIA_OPTION_TTF -> MIA_USE_TTF
 */

#include "OObj.h"
#include "img.h"


/** object id */
#define OTtf_ID OObj_ID "OTtf."


typedef struct {
    OObj super;

    // TTF_FONT*
    void *sdl_ttf;
} OTtf;


/**
 * Initializes the object.
 * @param obj OTtf object
 * @param parent to inherit from
 * @param file .ttf file to load from (routed path)
 * @param size in pixels for the height of the basic text
 *             Resulting img will be larger
 * @return obj casted as OTtf
 */
OTtf *OTtf_init(oobj obj, oobj parent, const char *file, float size);

/**
 * Creates a new OTtf object
 * @param parent to inherit from
 * @param file .ttf file to load from (routed path)
 * @param size in pixels for the height
 * @return The new object
 */
o_inline OTtf *OTtf_new(oobj parent, const char *file, float size)
{
    OObj_DECL_IMPL_NEW(OTtf, parent, file, size);
}


//
// virtual implementations:
//

/**
 * Object deletor that closes the loaded ttf_font
 * @param obj OTtf object
 */
void OTtf__v_del(oobj obj);


//
// object functions:
//

/**
 * Draws / Renders the given text to a new o_img image buffer
 * @param obj OTtf object
 * @param text to be drawn (utf-8) (in white)
 * @return a new o_img (allocated on obj) on which text is rendered.
 *         May return an empty o_img {0} on failure (like for file not found)
 */
struct o_img OTtf_draw(oobj obj, const char *text);

#endif //O_OTTF_H
#endif //MIA_OPTION_TTF
