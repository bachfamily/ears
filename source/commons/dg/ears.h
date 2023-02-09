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

// Change these defines here to tweak behavior
#ifndef WIN_VERSION
#define EARS_MP3_READ_SUPPORT // via mpg123
#endif

#define EARS_MP3_WRITE_SUPPORT // via lame

#define EARS_WAVPACK_SUPPORT // via Wavpack library

// #define EARS_ALLOCATIONVERBOSE ///< If defined, posts information about buffer allocation in the max windows

// #define EARS_EXPR_USE_LEXPR ///< If not defined, it'll use the standard Max atoms instead (which is quicker!, so there's no reason to define it, except for having access to some specific bach.expr functions which are not in expr (e.g. bessel)




// Do not change anything below this line

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
