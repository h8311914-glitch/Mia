#ifndef S_S_H
#define S_S_H

/**
 * @file s.h
 *
 * Includes most of the "s" library.
 * Based on "m" library. (so "o", "m")
 *
 * The "s" library deals with all sorts of sound and music stuff.
 *
 */


//
// library functions, may use objects...
//

#include "common.h"
#include "ogg.h"
#include "wav.h"


//
// some useful objects
//

#include "SBuf.h"
#include "SFilter.h"
#include "SFilterConv.h"
#include "SFilterEcho.h"
#include "SFilterFade.h"
#include "SFilterNormalize.h"
#include "SResampler.h"
#include "STrack.h"
#include "STrackBuf.h"
#include "STrackGen.h"
#include "STrackMix.h"
#include "STrackMono.h"
#include "STrackPan.h"
#include "STrackResample.h"


#endif //S_S_H
