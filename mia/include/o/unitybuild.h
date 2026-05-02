// O_UNITYBUILD_H must NOT have an include guard!




/**
 * @file unitybuild.h
 *
 * Helper which uniqueifies some common used static names.
 * That way they can be used in multiple sources of a unity build module.
 *    So each source may have its own "L." or "update()", etc.
 *
 * One of the few headers, that is not included in o/o.h or mia.h
 *
 * Not all modules make use of this, but some do. 
 * The examples app "ex" as an example.
 */


/**
 * Needs to be set to a new id between each include.
 * Must not be a number, could also be 1_b or smth.
 * Will get part of the object name.
 */
#ifndef O__ID
#define O__ID 0
#endif

#include "o/common.h"


//
// common static names:
//

#undef L
#define L O_NAME_CONCAT(L__, O__ID)

#undef context
#define context O_NAME_CONCAT(context__, O__ID)

#undef setup
#define setup O_NAME_CONCAT(setup__, O__ID)

#undef update
#define update O_NAME_CONCAT(update__, O__ID)

#undef render
#define render O_NAME_CONCAT(render__, O__ID)


