#ifndef S_SFILTERCONV_H
#define S_SFILTERCONV_H

/**
 * @file SFilterConv.h
 *
 * Object (derives SFilter)
 *
 * Filter that applies a convolution to the sound block.
 * Uses a cache internally, which saves the previous block samples (size depends on kernel size).
 *
 * To use a SFilter, simply use that STrack(Buf;Mix) ad its parent
 */


#include "SFilter.h"

/** object id */
#define SFilterConv_ID SFilter_ID "Conv."


typedef struct {
    SFilter super;

    float *kernel;
    int kernel_size;

    float *cache;
    int cache_channels;
    int cache_pos;

} SFilterConv;


/**
 * Initializes the object.
 * kernel is setup as identity convolution, see SFilterFade_kernel_set_eye
 * @param obj SFilterConv object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this filter gets auto deleted
 * @return obj casted as SFilter
 */
SFilterConv *SFilterConv_init(oobj obj, oobj parent, oobj opt_parent2, int kernel_size);

/**
 * Creates a new SFilterConv object
 * kernel is setup as identity convolution, see SFilterFade_kernel_set_eye
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this filter gets auto deleted
 * @return The new object
 */
o_inline SFilterConv *SFilterConv_new(oobj parent, oobj opt_parent2, int kernel_size)
{
    OObj_DECL_IMPL_NEW(SFilterConv, parent, opt_parent2, kernel_size);
}


//
// virtual implementations:
//

/**
 * Virtual function to filter from the track.
 * Convolutes the kernel on the block
 * @param obj SFilterConv object
 * @param data frames (=samples*channels) to fill in data pulled by the track
 * @patam frames number of frames to filter
 * @param spec audio spec passed in data
 */
void SFilterConv__v_apply(oobj obj, float *restrict data, oi64 frames, struct s_spec spec);

//
// object functions
//

/**
 * Clears the cache to silence
 * @param obj SFilterConv object
 */
void SFilterConv_reset(oobj obj);


/**
 * @param obj SFilterConv object
 * @return the kernel array (of kernel_size length)
 * @sa m/utils/kernel to setup the kernel
 */
OObj_DECL_GET(SFilterConv, float *, kernel)


/**
 * @param obj SFilterConv object
 * @return the kernel array size
 */
OObj_DECL_GET(SFilterConv, int, kernel_size)


#endif //S_SFILTERCONV_H
