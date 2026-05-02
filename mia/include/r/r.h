#ifndef R_R_H
#define R_R_H

/**
 * @file r.h
 *
 * Includes most of the "r" library.
 * Based on "m" library. (so "o", "m")
 * Sequence of libraries is nevertheless "o", "m", "s", "r", ...
 *
 * The "r" library deals with all sorts of rendering stuff.
 *
 * All (most...) stuff in this library need an active gl context (created by the "a" module)
 */


//
// library functions, may use objects...
//

#include "box.h"
#include "common.h"
#include "program.h"
#include "proj.h"
#include "quad.h"
#include "tex.h"
#include "textlayout.h"


//
// some useful objects
//

#include "RBuffer.h"
#include "RCam.h"
#include "RProgram.h"
#include "RShader.h"
#include "RTex.h"
#include "RTex_ex.h"

//
// buffers 'n shaders
//

#include "RBufferBox.h"
#include "RBufferQuad.h"
#include "RShaderBox.h"
#include "RShaderBoxIndexed.h"
#include "RShaderQuad.h"
#include "RShaderQuadChannelmerge.h"
#include "RShaderQuadDab.h"
#include "RShaderQuadKernel.h"
#include "RShaderQuadMerge.h"
#include "RShaderQuadOp.h"


//
// render objects
//

#include "RObj.h"
#include "RObjBox.h"
#include "RObjGroup.h"
#include "RObjQuad.h"
#include "RObjText.h"

#endif //R_R_H
