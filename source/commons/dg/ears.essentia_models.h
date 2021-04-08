/**
	@file
	ears.essentia_models.h
	Bridge for Essentia models intoe ears
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_ESSENTIA_MODELS_H_
#define _EARS_BUF_ESSENTIA_MODELS_H_

#include "ears.essentia_commons.h"

using namespace essentia;
using namespace standard;


// SINUSOIDS
t_ears_err ears_model_sine_analysis(t_object *ob, std::vector<Real> samples, double sr,
                                    t_llll **frequencies_ll, t_llll **magnitudes_ll, t_llll **phases_ll,
                                    t_ears_essentia_analysis_params *params,
                                    e_ears_angleunit out_angleunit,
                                    e_ears_ampunit out_ampunit,
                                    e_ears_frequnit out_frequnit,
                                    double freqDevOffset,
                                    double freqDevSlope,
                                    double magnitudeThreshold,
                                    double minFrequency,
                                    double maxFrequency,
                                    long maxPeaks,
                                    long maxnSines,
                                    const char *orderBy
                                    );

t_ears_err ears_model_sine_synthesis(t_object *ob, double sr,
                                    t_llll *frequencies_ll, t_llll *magnitudes_ll, t_llll *phases_ll,
                                    t_buffer_obj *outbuf,
                                    t_ears_essentia_analysis_params *params,
                                    e_ears_angleunit in_angleunit,
                                    e_ears_ampunit in_ampunit,
                                    e_ears_frequnit in_frequnit,
                                    long channelidx
                                    );

// SPR

t_ears_err ears_model_SPR_analysis(t_object *ob, std::vector<Real> samples, double sr,
                                   t_llll **frequencies_ll, t_llll **magnitudes_ll, t_llll **phases_ll,
                                   std::vector<std::vector<essentia::Real>> &framewiseResiduals,
                                   long channelidx,
                                   t_ears_essentia_analysis_params *params,
                                   e_ears_angleunit out_angleunit,
                                   e_ears_ampunit out_ampunit,
                                   e_ears_frequnit out_frequnit,
                                   double freqDevOffset,
                                   double freqDevSlope,
                                   double magnitudeThreshold,
                                   double minFrequency,
                                   double maxFrequency,
                                   long maxPeaks,
                                   long maxnSines,
                                   const char *orderBy
                                   );

t_ears_err ears_model_SPR_synthesis(t_object *ob, double sr,
                                    t_llll *frequencies_ll, t_llll *magnitudes_ll, t_llll *phases_ll,
                                    t_buffer_obj *residual,
                                    t_buffer_obj *outbuf,
                                    t_ears_essentia_analysis_params *params,
                                    e_ears_angleunit in_angleunit,
                                    e_ears_ampunit in_ampunit,
                                    e_ears_frequnit in_frequnit,
                                    long channelidx
                                    );
#endif // _EARS_BUF_RUBBERBAND_MODELS_H_
