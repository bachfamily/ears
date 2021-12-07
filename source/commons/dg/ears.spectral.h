/**
	@file
	ears.spectral.h
	Common utilities header for the buffer spectral operations
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_SPECTRAL_H_
#define _EARS_BUF_SPECTRAL_H_

#include "ears.commons.h"
#include "spectral.h"

t_ears_err ears_buffer_fft(t_object *ob, t_buffer_obj *source1, t_buffer_obj *source2, t_buffer_obj *dest1, t_buffer_obj *dest2, long polar, long inverse, long fullspectrum, e_ears_angleunit angleunit);

t_ears_err ears_buffer_paulstretch(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double stretchfactor, long framesize_samps, char spectral = true);
t_ears_err ears_buffer_paulstretch_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *stretchenv, long framesize_samps, char spectral, e_slope_mapping slopemapping, e_ears_timeunit factor_timeunit);

t_ears_err ears_buffer_paulfreeze(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long onset_samps, long framesize_samps, long jitter_samps, long duration_samps, char spectral);


#endif // _EARS_BUF_SPECTRAL_H_
