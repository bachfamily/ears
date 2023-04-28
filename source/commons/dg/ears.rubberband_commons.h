/**
	@file
	ears.rubberband_commons.h
	Rubberband bridge for ears
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_RUBBERBAND_COMMONS_H_
#define _EARS_BUF_RUBBERBAND_COMMONS_H_

// Rubberband options via define
#define HAVE_VDSP
#define NO_THREADING
// #define USE_PTHREADS

#include "ears.commons.h"
#include "RubberBandStretcher.h"

RubberBand::RubberBandStretcher::Options buf_rubberband_get_default_options();

t_ears_err ears_buffer_rubberband(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *timestretch_factor, t_llll *pitchshift_factor, RubberBand::RubberBandStretcher::Options options, long blocksize_samps, e_slope_mapping slopemapping, bool precise_output_time);

#endif // _EARS_BUF_RUBBERBAND_COMMONS_H_
