/**
	@file
	ears.spectral.h
	Common utilities header for the buffer spectral operations
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_SPECTRAL_H_
#define _EARS_BUF_SPECTRAL_H_

#include "ears.commons.h"
#include "math/spectral.h"

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
    EARS_SEAM_CARVE_MODE_SOBELHARMO = 3, // experimental, unused
};



void test_kiss_fft();

// also lets you choose the fft normalization type, and works coherently with inverses
void ears_fft_kiss(kiss_fft_cfg cfg, int nfft, const kiss_fft_cpx *fin, kiss_fft_cpx *fout, e_ears_fft_normalization normalization, bool inverse);

long ears_get_window(float *win, const char *type, long numframes);
t_ears_err ears_buffer_apply_window(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_symbol *window_type);

/// These functions compute FFTs directly based on kiss fft.
/// BEWARE: if you link this with the essentia library, that's a recipe for trouble. Don't know why to be honest, but the essentia library MUST NOT be linked to the target if you use these functions
/// I suspect something weird happens with kissfft inside essentia...
t_ears_err ears_buffer_fft(t_object *ob, t_buffer_obj *source1, t_buffer_obj *source2, t_buffer_obj *dest1, t_buffer_obj *dest2, long polar_input, long polar_output, long inverse, long fullspectrum, e_ears_angleunit angleunit, e_ears_fft_normalization fftnormalization);
t_ears_err ears_buffer_stft(t_object *ob, t_buffer_obj *source1, t_buffer_obj *source2, long channel, t_buffer_obj *dest1, t_buffer_obj *dest2, long framesize_samps, double hopsize_samps, const char *wintype, long polar_input, long polar_output, long fullspectrum, e_ears_angleunit angleunit, long left_aligned_windows, e_ears_fft_normalization fftnormalization);
t_ears_err ears_buffer_istft(t_object *ob, long num_input_buffers, t_buffer_obj **source1, t_buffer_obj **source2, t_buffer_obj *dest1, t_buffer_obj *dest2, const char *wintype, long polar_input, long polar_output, long fullspectrum, e_ears_angleunit angleunit, double force_sr, long left_aligned_windows, e_ears_fft_normalization fftnormalization);


t_ears_err ears_buffer_griffinlim(t_object *ob, long num_buffers, t_buffer_obj **orig_amps, t_buffer_obj **reconstructed_phases, t_buffer_obj **phases_mask, t_buffer_obj *dest_signal, const char *analysiswintype, const char *synthesiswintype, long fullspectrum, long left_aligned_windows, e_ears_fft_normalization fftnormalization, long num_iterations, bool randomize_phases);


t_ears_err ears_buffer_paulstretch(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, double stretchfactor, long framesize_samps, char spectral = true, bool precise_output_time = false);
t_ears_err ears_buffer_paulstretch_envelope(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, t_llll *stretchenv, long framesize_samps, char spectral, e_slope_mapping slopemapping, e_ears_timeunit factor_timeunit);

t_ears_err ears_buffer_paulfreeze(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, long onset_samps, long framesize_samps, long jitter_samps, long duration_samps, char spectral);

t_ears_err ears_buffer_spectral_seam_carve(t_object *ob, long num_channels, t_buffer_obj **amplitudes, t_buffer_obj **phases, t_buffer_obj **out_amplitudes, t_buffer_obj **out_phases, t_buffer_obj *energy_map, t_buffer_obj *energy_map_cumul, t_buffer_obj *seam_path, long delta_num_frames, double framesize_samps, double hopsize_samps, long energy_mode, updateprogress_fn update_progress, long phase_handling, double regularization, double forward_energy_contribution, long forward_energy_type, bool forward_energy_embedded_in_matrix,
    // only used for griffin-lim;
   long fullspectrum, long winleftalign, e_ears_fft_normalization fftnormalization, long num_griffin_lim_iter, long griffin_lim_invalidate_width, bool griffin_lim_vertical, bool griffin_lim_randomize, const char *analysiswintype, const char *synthesiswintype, long batch_size, bool interrupt_batch_at_crossings);


// mezza porcata, to avoid linkage of Essentia (which had a different KissFFT version and therefore caused issues with objects
// linked both to spectrla funcionts and essentia) I've kept the same type of processing
void ears_get_peaks(std::vector<double> array, std::vector<double> &peakPosition, std::vector<double> &peakValue,
                    bool _interpolate, long _maxPeaks, double _minPeakDistance, const char *_orderBy, double _threshold);

t_llll *ears_specbuffer_peaks(t_object *ob, t_buffer_obj *mags, t_buffer_obj *phases, bool interpolate, int maxPeaks, double minPeakDistance, t_symbol *orderBy, double threshold, e_ears_timeunit timeunit, e_ears_ampunit ampunit, e_ears_angleunit angleunit, e_ears_ampunit threshampunit, t_ears_err *err);


t_llll *ears_ptrack(t_object *ob, t_llll *peaks,
                    double freq_speed_threshold, // how much the frequency (in the unit below) can change *per second*
                    double graceperiod_before_dying, // grace period in ms for which a partial can be silent (in the threshold_timeunit)
                    double min_partial_length, // grace period in ms for which a partial can be silent (in the threshold_timeunit)
                    double min_partial_avg_amp,
                    e_ears_timeunit in_timeunit, // units for time
                    e_ears_frequnit in_frequnit, // units for frequencies
                    e_ears_ampunit in_ampunit, // units for amplitudes
                    e_ears_timeunit threshold_timeunit, // time unit for threshold
                    e_ears_frequnit threshold_frequnit, // frequency unit for threshold
                    e_ears_ampunit threshold_ampunit, // amplitude unit for threshold
                    t_buffer_obj *inbuf // only used for unit conversion
);

t_llll *ears_ptrack_to_roll(t_object *ob, t_llll *ptrack,
                            e_ears_timeunit in_timeunit, e_ears_frequnit in_frequnit, e_ears_ampunit in_ampunit);

#endif // _EARS_BUF_SPECTRAL_H_
