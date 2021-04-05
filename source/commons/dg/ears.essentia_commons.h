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
    EARS_FEATURE_UNKNOWN = -1,
    EARS_FEATURE_NONE = 0,

    EARS_FEATURE_SPECTRUM,
    EARS_FEATURE_POWERSPECTRUM,
    
    // Envelopes
    EARS_FEATURE_ENVELOPE,
    EARS_FEATURE_LOGATTACKTIME,
    EARS_FEATURE_ENVMAXTIME,
    EARS_FEATURE_ENVMINTIME,
    EARS_FEATURE_STRONGDECAY,

    // Standard
    EARS_FEATURE_DERIVATIVE,
    EARS_FEATURE_MIN,
    EARS_FEATURE_MAX,
    EARS_FEATURE_WELCH,
    
    EARS_FEATURE_FLUX,

    EARS_FEATURE_ZEROCROSSINGRATE,
    EARS_FEATURE_ENERGY,
    EARS_FEATURE_ENERGYBAND,
    EARS_FEATURE_ENERGYBANDRATIO,
    EARS_FEATURE_MFCC,
    EARS_FEATURE_BFCC,
    EARS_FEATURE_BARKBANDS,
    EARS_FEATURE_ERBBANDS,
    EARS_FEATURE_FREQUENCYBANDS,
    EARS_FEATURE_GFCC,
    EARS_FEATURE_HFC,
    EARS_FEATURE_LPC,
    EARS_FEATURE_MAXMAGFREQ,
    EARS_FEATURE_ROLLOFF,
    EARS_FEATURE_TIMEDOMAINSPECTRALCENTROID,
    EARS_FEATURE_SPECTRALCOMPLEXITY,
    EARS_FEATURE_SPECTRALCONTRAST,
    EARS_FEATURE_STRONGPEAK,
    EARS_FEATURE_TRIANGULARBANDS,
    EARS_FEATURE_TRIANGULARBARKBANDS,
    
    // Rhythm
    EARS_FEATURE_BEATTRACKERDEGARA,
    EARS_FEATURE_BEATTRACKERMULTIFEATURE,
    EARS_FEATURE_BEATSLOUDNESS,
    EARS_FEATURE_DANCEABILITY,
    EARS_FEATURE_LOOPBPMESTIMATOR,
    EARS_FEATURE_ONSETDETECTION,
    
    //.... to do
    
    // Statistics
    EARS_FEATURE_TEMPORALCENTRALMOMENTS,
    EARS_FEATURE_SPECTRALCENTRALMOMENTS,
    EARS_FEATURE_TEMPORALRAWMOMENTS,
    EARS_FEATURE_SPECTRALRAWMOMENTS,
    EARS_FEATURE_TEMPORALCENTROID,
    EARS_FEATURE_SPECTRALCENTROID,
    EARS_FEATURE_TEMPORALCREST,
    EARS_FEATURE_SPECTRALCREST,
    EARS_FEATURE_TEMPORALDECREASE,
    EARS_FEATURE_SPECTRALDECREASE,
    EARS_FEATURE_TEMPORALDISTRIBUTIONSHAPE,
    EARS_FEATURE_SPECTRALDISTRIBUTIONSHAPE,
    EARS_FEATURE_TEMPORALSPREAD,
    EARS_FEATURE_SPECTRALSPREAD,
    EARS_FEATURE_TEMPORALSKEWNESS,
    EARS_FEATURE_SPECTRALSKEWNESS,
    EARS_FEATURE_TEMPORALKURTOSIS,
    EARS_FEATURE_SPECTRALKURTOSIS,
    EARS_FEATURE_SPECTRALENERGY,
    EARS_FEATURE_SPECTRALENTROPY,
    EARS_FEATURE_TEMPORALFLATNESS,
    EARS_FEATURE_SPECTRALFLATNESS,
    EARS_FEATURE_SPECTRALGEOMETRICMEAN,
    EARS_FEATURE_INSTANTPOWER,
    EARS_FEATURE_SPECTRALMEAN,
    EARS_FEATURE_SPECTRALMEDIAN,
    EARS_FEATURE_SPECTRALRMS,
    EARS_FEATURE_TEMPORALVARIANCE,
    EARS_FEATURE_SPECTRALVARIANCE,

    // TONAL
    EARS_FEATURE_CHORDSDETECTION,
    EARS_FEATURE_DISSONANCE,
    EARS_FEATURE_HPCP,
    EARS_FEATURE_HARMONICPEAKS,
    EARS_FEATURE_HIGHRESOLUTIONFEATURES,
    EARS_FEATURE_INHARMONICITY,
    EARS_FEATURE_KEY,
    EARS_FEATURE_KEYEXTRACTOR,
    EARS_FEATURE_ODDTOEVENHARMONICENERGYRATIO,
    EARS_FEATURE_PITCHSALIENCE,
    EARS_FEATURE_SPECTRUMCQ,
    EARS_FEATURE_TRISTIMULUS,
    EARS_FEATURE_TUNINGFREQUENCY,
    
    // FINGERPRINTING
    EARS_FEATURE_CHROMAPRINTER,
    
    // DURATION/SILENCE
    EARS_FEATURE_DURATION,
    EARS_FEATURE_EFFECTIVEDURATION,
    EARS_FEATURE_SILENCERATE,
    
    // LOUDNESS/DYNAMICS
    EARS_FEATURE_DYNAMICCOMPLEXITY,
    EARS_FEATURE_LARM,
    EARS_FEATURE_LEQ,
    EARS_FEATURE_LOUDNESS,
    EARS_FEATURE_LOUDNESSVICKERS,
    EARS_FEATURE_REPLAYGAIN,
 
    
    // PITCH
    EARS_FEATURE_MULTIPITCHKLAPURI,
    EARS_FEATURE_MULTIPITCHMELODIA,
    EARS_FEATURE_PITCHSALIENCEFUNCTION,
    EARS_FEATURE_PITCHYIN,
    EARS_FEATURE_PITCHYINFFT,
    EARS_FEATURE_PITCHYINPROBABILISTIC,

} e_ears_feature;


typedef enum {
    EARS_ESSENTIA_TEMPORALMODE_WHOLE = 0,
    EARS_ESSENTIA_TEMPORALMODE_TIMESERIES,
    EARS_ESSENTIA_TEMPORALMODE_BUFFER,
} e_ears_essentia_temporalmode;

typedef enum {
    EARS_ESSENTIA_SUMMARIZATION_FIRST = 0,
    EARS_ESSENTIA_SUMMARIZATION_LAST,
    EARS_ESSENTIA_SUMMARIZATION_MIDDLE,
    EARS_ESSENTIA_SUMMARIZATION_MEAN,
    EARS_ESSENTIA_SUMMARIZATION_MODE,
} e_ears_essentia_summarization;


typedef enum {
    EARS_ESSENTIA_SUMMARIZATIONWEIGHT_NONE = 0,
    EARS_ESSENTIA_SUMMARIZATIONWEIGHT_RMS,
    EARS_ESSENTIA_SUMMARIZATIONWEIGHT_LOUDNESS,
} e_ears_essentia_summarizationweight;


typedef enum {
    EARS_ESSENTIA_EXTRACTOR_INPUT_AUDIO = 0,
    EARS_ESSENTIA_EXTRACTOR_INPUT_FRAME,
    EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUM,
    EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPE,
    EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRUMCENTRALMOMENTS,
    EARS_ESSENTIA_EXTRACTOR_INPUT_ENVELOPECENTRALMOMENTS,
    EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKS,
    EARS_ESSENTIA_EXTRACTOR_INPUT_SPECTRALPEAKSANDFZERO,
    EARS_ESSENTIA_EXTRACTOR_INPUT_LOUDNESS,
    EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILE,
    EARS_ESSENTIA_EXTRACTOR_INPUT_PITCHCLASSPROFILEBATCH,
} e_ears_essentia_extractor_input_type;

typedef enum {
    EARS_ESSENTIA_EXTRACTOR_OUTPUT_SIGNAL = 0,
    EARS_ESSENTIA_EXTRACTOR_OUTPUT_FLOAT,
    EARS_ESSENTIA_EXTRACTOR_OUTPUT_INT,
} e_ears_essentia_extractor_output_type;


typedef enum {
    EARS_ESSENTIA_BUFFERINTERPMODE_DONT = 0,
    EARS_ESSENTIA_BUFFERINTERPMODE_LOWEST = 1,
    EARS_ESSENTIA_BUFFERINTERPMODE_LINEAR = 2,
} e_ears_essentia_bufferinterpmode;

#define EARS_ESSENTIA_EXTRACTOR_MAX_ARGS 10
#define EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS 5
#define EARS_ESSENTIA_EXTRACTOR_MAX_INPUTS 3

typedef struct _ears_essentia_extractor
{
    Algorithm                   *algorithm;
    e_ears_feature     feature;
    
    long num_inputs;
    const char  *essentia_input_label[EARS_ESSENTIA_EXTRACTOR_MAX_INPUTS];
    e_ears_essentia_extractor_input_type  input_type;

    // Essentia output
    long        essentia_num_outputs;
    const char  *essentia_output_label[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
    char        essentia_output_type[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS]; // essentia type
    
    // Actual extractor output
    long        num_outputs;
    const char  *output_desc[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS]; // actual description used
    char        output_type[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS]; // actual type used when outputting
    int         output_map[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
    
    e_ears_essentia_temporalmode          temporalmode;

    e_ears_timeunit     essentia_output_timeunit[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
    e_ears_ampunit      essentia_output_ampunit[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
    e_ears_frequnit     essentia_output_frequnit[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];
    e_ears_pitchunit     essentia_output_pitchunit[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];

    e_ears_timeunit     output_timeunit;
    e_ears_ampunit      output_ampunit;
    e_ears_frequnit     output_frequnit;
    e_ears_pitchunit     output_pitchunit;

    bool                        has_spec_metadata;
    t_ears_spectralbuf_metadata specdata;

    t_llll          *result[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];      //< will be filled with the result
    t_buffer_obj    *result_buf[EARS_ESSENTIA_EXTRACTOR_MAX_OUTPUTS];  //< buffer that will be filled with the result
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
    Algorithm *alg_SpectrumCentralMoments;
    Algorithm *alg_EnvelopeCentralMoments;
    Algorithm *alg_Loudness;
    Algorithm *alg_EqualLoudness;
    Algorithm *alg_RMS;
    Algorithm *alg_SpectralPeaks;
    Algorithm *alg_HPCP;
    Algorithm *alg_YinFFT;
} t_ears_essentia_extractors_library;


typedef struct _ears_essentia_analysis_params
{
    // windowing
    int      framesize_samps; // must be even
    Real     hopsize_samps;
    e_ears_timeunit sizes_unit;
    long     duration_samps;
    
    const char  *windowType;
    bool        startFromZero;
    bool        lastFrameToEndOfFile;

    // envelope
    Real    envelope_attack_time_samps;
    Real    envelope_release_time_samps;
    bool    envelope_rectify;
    
    // CQT
    int     binsPerOctave;
    Real    minFrequency;
    int     numberBins;
    Real    threshold;
    int     minimumKernelSize;
    Real    scale;
    
    // Onset detection
    const char *onsetDetectionMethod;
    
    
    // Spectral Peaks
    
    Real    PEAKS_magnitudeThreshold;
    Real    PEAKS_maxFrequency;
    Real    PEAKS_minFrequency;
    int     PEAKS_maxPeaks;
    const char *PEAKS_orderBy;
    
    // HPCP
    bool    HPCP_bandPreset;
    Real    HPCP_bandSplitFrequency;
    int     HPCP_harmonics;
    Real    HPCP_maxFrequency;
    bool    HPCP_maxShifted;
    Real    HPCP_minFrequency;
    bool    HPCP_nonLinear;
    const char *HPCP_normalized;
    Real    HPCP_referenceFrequency;
    int     HPCP_size;
    const char *HPCP_weightType;
    Real    HPCP_windowSize;
    
    // Yin
    Real YIN_minFrequency;
    Real YIN_maxFrequency;
    Real YIN_tolerance;

    // Summarization mode
    e_ears_essentia_summarization summarization;
    e_ears_essentia_summarizationweight summarizationweight;
    
    // Griffin Lim
    int     numGriffinLimIterations;
    
    //
    bool verbose;
    
} t_ears_essentia_analysis_params;

void ears_essentia_init();
void *ears_essentia_quit(t_symbol *s, short argc, t_atom *argv);

// convert params
t_ears_essentia_analysis_params earsbufobj_get_essentia_analysis_params(t_earsbufobj *e_ob, t_buffer_obj *buf);

// Spectral processing
t_ears_err ears_vector_stft(t_object *ob, std::vector<Real> samples, double sr, t_buffer_obj *dest1, t_buffer_obj *dest2, long polar, long fullspectrum, t_ears_essentia_analysis_params *params, e_ears_angleunit angleunit);

t_ears_err ears_specbuffer_istft(t_object *ob, long num_input_buffers, t_buffer_obj **source1, t_buffer_obj **source2, t_buffer_obj *dest, long polar, long fullspectrum, t_ears_essentia_analysis_params *params, e_ears_angleunit angleunit, double force_sr);

t_ears_err ears_vector_cqt(t_object *ob, std::vector<Real> samples, double sr, t_buffer_obj *dest1, t_buffer_obj *dest2, long polar, t_ears_essentia_analysis_params *params, e_ears_angleunit angleunit);

t_llll *ears_specbuffer_peaks(t_object *ob, t_buffer_obj *mags, t_buffer_obj *phases, bool interpolate, int maxPeaks, double minPeakDistance, t_symbol *orderBy, double threshold, e_ears_timeunit timeunit, e_ears_angleunit angleunit, t_ears_err *err);

// Features
void ears_essentia_extractors_library_free(t_ears_essentia_extractors_library *lib);
t_ears_err ears_essentia_extractors_library_build(t_earsbufobj *e_ob, long num_features, long *features, long *temporalmodes, double sr, t_llll **args, t_ears_essentia_extractors_library *lib, t_ears_essentia_analysis_params *params);
t_ears_err ears_essentia_extractors_library_compute(t_earsbufobj *e_ob, t_buffer_obj *buf, t_ears_essentia_extractors_library *lib, t_ears_essentia_analysis_params *params, long buffer_output_interpolation_mode);

#endif // _EARS_BUF_RUBBERBAND_COMMONS_H_
