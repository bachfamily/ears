/**
	@file
	ears.h
	Basic stuff for the whole ears library
 
	by Daniele Ghisi
 */

#ifndef _EARS_H_
#define _EARS_H_

#ifdef __APPLE__
#define EARS_MP3_SUPPORT
#endif
#define EARS_WAVPACK_SUPPORT

// #define EARS_ALLOCATIONVERBOSE



/** Resampling mode
 @ingroup misc */
typedef enum {
    EARS_RESAMPLINGMODE_SINC = 0,
    EARS_RESAMPLINGMODE_NEARESTNEIGHBOR,
    EARS_RESAMPLINGMODE_SAMPLEANDHOLD,
    EARS_RESAMPLINGMODE_LINEAR,
    EARS_RESAMPLINGMODE_QUADRATIC,
    EARS_RESAMPLINGMODE_CUBIC,
} e_ears_resamplingmode;




#endif // _EARS_H_
