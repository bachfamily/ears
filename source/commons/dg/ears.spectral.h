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

#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef TWOPI
#define TWOPI        6.28318530717958647692
#endif
#ifndef PIOVERTWO
#define PIOVERTWO    1.57079632679489661923
#endif



enum {
    EARS_SEAM_CARVE_MODE_MAGNITUDE = 0,
    EARS_SEAM_CARVE_MODE_GRADIENT_MAGNITUDE = 1,
    EARS_SEAM_CARVE_MODE_SOBEL = 2,
};

void test_kiss_fft();

long ears_get_window(float *win, const char *type, long numframes);
t_ears_err ears_buffer_apply_window(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_symbol *window_type);

/// These functions compute FFTs directly based on kiss fft.
/// BEWARE: if you link this with the essentia library, that's a recipe for trouble. Don't know why to be honest, but the essentia library MUST NOT be linked to the target if you use these functions
/// I suspect something weird happens with kissfft inside essentia...
t_ears_err ears_buffer_fft(t_object *ob, t_buffer_obj *source1, t_buffer_obj *source2, t_buffer_obj *dest1, t_buffer_obj *dest2, long polar_input, long polar_output, long inverse, long fullspectrum, e_ears_angleunit angleunit, long unitary);
t_ears_err ears_buffer_stft(t_object *ob, t_buffer_obj *source1, t_buffer_obj *source2, long channel, t_buffer_obj *dest1, t_buffer_obj *dest2, long framesize_samps, double hopsize_samps, const char *wintype, long polar_input, long polar_output, long fullspectrum, e_ears_angleunit angleunit, long left_aligned_windows, long unitary);
t_ears_err ears_buffer_istft(t_object *ob, long num_input_buffers, t_buffer_obj **source1, t_buffer_obj **source2, t_buffer_obj *dest1, t_buffer_obj *dest2, const char *wintype, long polar_input, long polar_output, long fullspectrum, e_ears_angleunit angleunit, double force_sr, long left_aligned_windows, long unitary);


t_ears_err ears_buffer_griffinlim(t_object *ob, long num_buffers, t_buffer_obj **orig_amps, t_buffer_obj **reconstructed_phases, t_buffer_obj **phases_mask, t_buffer_obj *dest_signal, const char *analysiswintype, const char *synthesiswintype, long fullspectrum, long left_aligned_windows, long unitary, long num_iterations, bool randomize_phases);


t_ears_err ears_buffer_paulstretch(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double stretchfactor, long framesize_samps, char spectral = true, bool precise_output_time = false);
t_ears_err ears_buffer_paulstretch_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *stretchenv, long framesize_samps, char spectral, e_slope_mapping slopemapping, e_ears_timeunit factor_timeunit);

t_ears_err ears_buffer_paulfreeze(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long onset_samps, long framesize_samps, long jitter_samps, long duration_samps, char spectral);

t_ears_err ears_buffer_spectral_seam_carve(t_object *ob, long num_channels, t_buffer_obj **amplitudes, t_buffer_obj **phases, t_buffer_obj **out_amplitudes, t_buffer_obj **out_phases, t_buffer_obj *energy_map, t_buffer_obj *seam_path, long delta_num_frames, double framesize_samps, double hopsize_samps, long energy_mode, updateprogress_fn update_progress, long phase_handling, double weighting_amount, double weighting_numframes_stdev, long fullspectrum, long winleftalign, long unitary, long num_griffin_lim_iter, long griffin_lim_invalidate_width, bool griffin_lim_vertical, bool griffin_lim_randomize, const char *analysiswintype, const char *synthesiswintype);

//t_ears_err ears_buffer_spectral_seam_carve_orig(t_object *ob, long num_channels, t_buffer_obj **amplitudes, t_buffer_obj **phases, t_buffer_obj **out_amplitudes, t_buffer_obj **out_phases, t_buffer_obj *energy_map, t_buffer_obj *seam_path, long delta_num_frames, double framesize_samps, double hopsize_samps, long energy_mode, updateprogress_fn update_progress, double compensate_phases, double weighting_amount, double weighting_numframes_stdev);


#endif // _EARS_BUF_SPECTRAL_H_
