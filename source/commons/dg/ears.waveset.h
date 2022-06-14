/**
	@file
	ears.waveset.h
	Functions dealing with Wishart-style wavesets
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_WAVESET_H_
#define _EARS_BUF_WAVESET_H_


#include "ears.h"
#include "ext.h"
#include "ext_obex.h"
#include "ext_buffer.h"
#include "ext_strings.h"

#include "llll_commons_ext.h"
#include "ears.conversions.h" // llllstuff is included in here
#include "lexpr.h"
#include "bach_math_utilities.h"
#include "ears.object.h" // already included in previous one
#include "ears.utils.h"
#include "notation.h"
#include "bach_threads.h"

#include <vector>
#include "ext_globalsymbol.h"


// Waveset stuff
t_ears_err ears_buffer_waveset_getnum(t_object *ob, t_buffer_obj *source, long channel, long span, long *num);
t_ears_err ears_buffer_waveset_split(t_object *ob, t_buffer_obj *source, t_buffer_obj **dest, long channel, long dest_size, long span, bool normalize);
t_ears_err ears_buffer_waveset_repeat(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long howmany, long span, bool normalize);
t_ears_err ears_buffer_waveset_subs(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_buffer_obj *waveform, long span, long resampling_filter_size);
t_ears_err ears_buffer_waveset_shuffle(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long span, long group);
t_ears_err ears_buffer_waveset_decimate(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long howmany, long offset, long span, bool normalize);
t_ears_err ears_buffer_waveset_interp(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long span, long normalize, long numinterp, long resamplingfiltersize, e_ears_resamplingmode resamplingmode, bool equalpowerinterp);
t_ears_err ears_buffer_waveset_average(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long span, long normalize, long groupsize, long resamplingfiltersize, e_ears_resamplingmode resamplingmode, bool equalpowerinterp, bool keep_waveset_length);


#endif // _EARS_BUF_COMMONS_H_
