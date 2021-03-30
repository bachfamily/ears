/**
	@file
	ears.essentia_commons.h
	Essentia bridge for ears
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_ESSENTIA_COMMONS_H_
#define _EARS_BUF_ESSENTIA_COMMONS_H_

#include "ears.commons.h"
#include "essentia/essentiamath.h"
#include "essentia/pool.h"
#include "essentia/essentia.h"
#include "essentia/algorithm.h"
#include "essentia/algorithmfactory.h"

using namespace essentia;
using namespace standard;

typedef enum {
    EARS_ESSENTIA_FEATURE_UNKNOWN = -1,
    EARS_ESSENTIA_FEATURE_NONE = 0,
    EARS_ESSENTIA_FEATURE_ENVELOPE,
    EARS_ESSENTIA_FEATURE_SPECTRALENVELOPE, // TO DO
    EARS_ESSENTIA_FEATURE_LOGATTACKTIME,
    EARS_ESSENTIA_FEATURE_MAXTOTOTAL,
    EARS_ESSENTIA_FEATURE_MINTOTOTAL,
    EARS_ESSENTIA_FEATURE_STRONGDECAY,
    EARS_ESSENTIA_FEATURE_TCTOTOTAL,
    EARS_ESSENTIA_FEATURE_TEMPORALCENTROID,
    EARS_ESSENTIA_FEATURE_DURATION,
    
    EARS_ESSENTIA_FEATURE_SPECTRALMOMENTS,
    EARS_ESSENTIA_FEATURE_SPECTRALCENTROIDTIME,
    EARS_ESSENTIA_FEATURE_SPECTRALCENTROID,
    EARS_ESSENTIA_FEATURE_SPECTRALSPREAD,
    EARS_ESSENTIA_FEATURE_SPECTRALSKEWNESS,
    EARS_ESSENTIA_FEATURE_SPECTRALKURTOSIS,

    EARS_ESSENTIA_FEATURE_SPECTRALFLATNESS,
    EARS_ESSENTIA_FEATURE_FLUX,

    EARS_ESSENTIA_FEATURE_ZEROCROSSINGRATE,
    EARS_ESSENTIA_FEATURE_ENERGYBAND,
    EARS_ESSENTIA_FEATURE_ENERGYBANDRATIO,
    EARS_ESSENTIA_FEATURE_MFCC,
    EARS_ESSENTIA_FEATURE_BFCC,
    EARS_ESSENTIA_FEATURE_BARKBANDS,
    EARS_ESSENTIA_FEATURE_ERBBANDS,
    EARS_ESSENTIA_FEATURE_FREQUENCYBANDS,

} e_ears_essentia_feature;


typedef enum {
    EARS_ESSENTIA_TEMPORALMODE_WHOLE = 0,
    EARS_ESSENTIA_TEMPORALMODE_TIMESERIES,
    EARS_ESSENTIA_TEMPORALMODE_BUFFER,
} e_ears_essentia_temporalmode;


typedef enum {
    EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO = 0,
    EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE,
    EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM,
} e_ears_essentia_extractor_input_type;

typedef enum {
    EARS_ESSENTIA_EXTRACTOR_OUTPUT_SIGNAL = 0,
    EARS_ESSENTIA_EXTRACTOR_OUTPUT_TIMEVALUES,
    EARS_ESSENTIA_EXTRACTOR_OUTPUT_AMPVALUES,
    EARS_ESSENTIA_EXTRACTOR_OUTPUT_FREQVALUES,
    EARS_ESSENTIA_EXTRACTOR_OUTPUT_VALUES,
} e_ears_essentia_extractor_output_type;

#define EARS_ESSENTIA_EXTRACTOR_MAX_ARGS 10
#define EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUT_LABELS 3

typedef struct _ears_essentia_extractor
{
    Algorithm                   *algorithm;
    e_ears_essentia_feature     feature;
    
    const char  *inputlabel;
    const char  *outputlabel;
    const char  *outputlabels_dummy[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUT_LABELS];
    
//    long      num_args;
//    t_atom    args[EARS_ESSENTIA_EXTRACTOR_MAX_ARGS];

    long      num_fields;

    e_ears_essentia_extractor_input_type  input_type;
    e_ears_essentia_extractor_output_type output_type;
    char  output_dummy_type[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUT_LABELS];  // 'f': float, 'i': int, 'v': vector of floats
    e_ears_essentia_temporalmode          temporalmode;

    e_ears_timeunit     essentia_output_timeunit;
    e_ears_ampunit      essentia_output_ampunit;

    e_ears_timeunit     output_timeunit;
    e_ears_ampunit      output_ampunit;

    t_llll          *result;      //< will be filled with the result
    t_buffer_obj    *result_buf; //< buffer that will be filled with the result
} t_ears_essentia_extractor;


typedef struct _ears_essentia_extractors_library
{
    long num_extractors;
    t_ears_essentia_extractor   *extractors;
    
    // fundamental processors
    Algorithm *alg_FrameCutter;
    Algorithm *alg_Windower;
    Algorithm *alg_Spectrum;
    Algorithm *alg_Envelope;
    Algorithm *alg_Car2pol;
} t_ears_essentia_extractors_library;


typedef struct _ears_essentia_analysis_params
{
    // windowing
    int      framesize_samps; // must be even
    Real     hopsize_samps;
    long     duration_samps;
    const char  *windowType;
    bool        startFromZero;
    bool        lastFrameToEndOfFile;

    // envelope
    Real    envelope_attack_time_samps;
    Real    envelope_release_time_samps;
    bool    envelope_rectify;
} t_ears_essentia_analysis_params;

void ears_essentia_init();
void *ears_essentia_quit(t_symbol *s, short argc, t_atom *argv);

t_ears_err ears_vector_stft(t_object *ob, std::vector<Real> samples, t_buffer_obj *dest1, t_buffer_obj *dest2, long polar, long fullspectrum, t_ears_essentia_analysis_params *params, e_ears_angleunit angleunit);

void ears_essentia_extractors_library_free(t_ears_essentia_extractors_library *lib);
void ears_essentia_extractors_library_build(t_earsbufobj *e_ob, long num_features, long *features, long *temporalmodes, double sr, t_llll **args, t_ears_essentia_extractors_library *lib, t_ears_essentia_analysis_params *params);
void ears_essentia_extractors_library_compute(t_object *x, t_buffer_obj *buf, t_ears_essentia_extractors_library *lib, t_ears_essentia_analysis_params *params, long buffer_output_interpolation_order);

#endif // _EARS_BUF_RUBBERBAND_COMMONS_H_
