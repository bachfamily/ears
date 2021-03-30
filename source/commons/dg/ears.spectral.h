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

t_ears_err ears_buffer_paulstretch(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double stretchfactor, long winsize_samps, char spectral = true);
t_ears_err ears_buffer_paulstretch_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *stretchenv, long winsize_samps, char spectral, e_slope_mapping slopemapping);


#endif // _EARS_BUF_SPECTRAL_H_
