/**
	@file
	ears.essentia.c
 
	@name
	ears.essentia~
 
	@realname
	ears.essentia~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Extract buffer features via the Essentia library
 
	@description
	Perform descriptor analysis on the incoming buffer via the Essentia library
 
	@discussion
 
	@category
	ears analysis
 
	@keywords
	buffer, essentia, feature, descriptor
 
	@seealso
	ears.info~, ears.vamp~, ears.spectrogram~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"

#include "ears.essentia_commons.h"

using namespace essentia;
using namespace standard;




#define EARS_ESSENTIA_MAX_NUM_FEATURES 1024

typedef struct _buf_essentia {
    t_earsbufobj       e_ob;
    
    long               num_features;
    long               *features;
    long               *temporalmodes;
    t_llll             **algorithm_args;
    t_ears_essentia_extractors_library   extractors_lib;

    long               num_outlets;
    long               *features_numoutputs;
    long               *outlet_featureidx;
    long               *outlet_featureoutputidx;

    
    char               must_recreate_extractors;
    double             curr_sr;
    
    char               summarization;
    char               summarizationweight;
    char               summarizationpositiveonly;

    double             a_envattacktime;
    double             a_envreleasetime;

    
    // Specific parameters for transforms:
    long               CQT_binsPerOctave;
    double             CQT_minPitch;
    long               CQT_numBins;
    double             CQT_threshold;
    long               CQT_minimumKernelSize;
    double             CQT_scale;
    
    double             YIN_minFrequency;
    double             YIN_maxFrequency;
    double             YIN_tolerance;

    double             PEAKS_magnitudeThreshold;
    double             PEAKS_maxFrequency;
    double             PEAKS_minFrequency;
    long               PEAKS_maxPeaks;
    t_symbol           *PEAKS_orderBy;
    
    t_symbol           *onsetDetectionMethod;
    
    long               buffer_output_interpolation_mode;
    
} t_buf_essentia;



// Prototypes
t_buf_essentia*         buf_essentia_new(t_symbol *s, short argc, t_atom *argv);
void			buf_essentia_free(t_buf_essentia *x);
void			buf_essentia_bang(t_buf_essentia *x);
void			buf_essentia_anything(t_buf_essentia *x, t_symbol *msg, long ac, t_atom *av);

void buf_essentia_assist(t_buf_essentia *x, void *b, long m, long a, char *s);
void buf_essentia_inletinfo(t_buf_essentia *x, void *b, long a, char *t);

t_ears_essentia_analysis_params buf_essentia_get_default_params(t_buf_essentia *x);

// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(essentia)


/**********************************************************************/
// Class Definition and Life Cycle




t_max_err buf_essentia_notify(t_buf_essentia *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    if (msg == _sym_attr_modified) {
        x->must_recreate_extractors = true;
//        t_symbol *attrname = (t_symbol *)object_method((t_object *)data, _sym_getname);
//        if (attrname == gensym("wintype"))
    }
    return MAX_ERR_NONE;
}

int C74_EXPORT main(void)
{
    ears_essentia_init();
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.essentia~",
                         (method)buf_essentia_new,
                         (method)buf_essentia_free,
                         sizeof(t_buf_essentia),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a essentia threshold (depending on the <m>ampunit</m>) or
    // an envelope (also see <m>envampunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(essentia)
    
    
    class_addmethod(c, (method)buf_essentia_notify,        "bachnotify",        A_CANT,        0);

    // @method number @digest Set essentia
    // @description A number in the second inlet sets the essentia parameter (depending on the <m>ampunit</m>).

    llllobj_class_add_out_attr(c, LLLL_OBJ_VANILLA);

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_antimeunit_attr(c);
    earsbufobj_class_add_frequnit_attr(c);
    earsbufobj_class_add_pitchunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    
    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);
    earsbufobj_class_add_numframes_attr(c);
    earsbufobj_class_add_overlap_attr(c);
    earsbufobj_class_add_wintype_attr(c);
    earsbufobj_class_add_winstartfromzero_attr(c);
    earsbufobj_class_add_winnormalized_attr(c);

    CLASS_ATTR_DOUBLE(c, "envattack", 0, t_buf_essentia, a_envattacktime);
    CLASS_ATTR_STYLE_LABEL(c,"envattack",0,"text","Envelope Attack Time");
    CLASS_ATTR_CATEGORY(c, "envattack", 0, "Envelopes");
    // @description Sets the attack time for computing envelopes (the unit depends on the <m>envtimeunit</m> attribute)
    // Floating point values are allowed.

    CLASS_ATTR_DOUBLE(c, "envrelease", 0, t_buf_essentia, a_envreleasetime);
    CLASS_ATTR_STYLE_LABEL(c,"envrelease",0,"text","Envelope Release Time");
    CLASS_ATTR_CATEGORY(c, "envrelease", 0, "Envelopes");
    // @description Sets the attack time for computing envelopes (the unit depends on the <m>envtimeunit</m> attribute)
    // Floating point values are allowed.

    
    CLASS_ATTR_LONG(c, "bufinterp", 0, t_buf_essentia, buffer_output_interpolation_mode);
    CLASS_ATTR_STYLE_LABEL(c,"bufinterp",0,"text","Output Buffer Frame Interpolation Mode");
    CLASS_ATTR_ENUMINDEX(c, "bufinterp", 0, "Don't Resample Lower Neighbor Linear")
    CLASS_ATTR_BASIC(c, "bufinterp", 0);
    CLASS_ATTR_FILTER_CLIP(c, "bufinterp", 0, 2);
    // @description Sets the interpolation mode for output buffers: <br />
    // 0 (don't resample, default): the output buffer has one sample per analysis frame, and its sample rate will be changed accordingly; <br />
    // 1 (lower neighbor): the output buffer is resampled to the same samplerate as the input buffer but the samples are not interpolated; <br />
    // 2 (linear): same as the previous one, and the samples are interpolated linearly.

    
    CLASS_ATTR_CHAR(c, "summary", 0, t_buf_essentia, summarization);
    CLASS_ATTR_STYLE_LABEL(c,"summary",0,"enumindex","Summarization Mode");
    CLASS_ATTR_ENUMINDEX(c, "summary", 0, "First Last Middle Mean Median Mode Min Max");
    CLASS_ATTR_BASIC(c, "summary", 0);
    CLASS_ATTR_FILTER_CLIP(c, "summary", 0, 7);
    CLASS_ATTR_CATEGORY(c, "summary", 0, "Summarization");
    // @description Sets the summarization mode, for features that are requested as static but needs to be computed on a frame-by-frame basis.
    // Available modes are:
    // <b>First</b>: take first frame; <br />
    // <b>Last</b>: last last frame; <br />
    // <b>Middle</b>: take middle frame; <br />
    // <b>Mean</b>: average through frames; <br />
    // <b>Median</b>: median through frames (for use with single-valued essentia); <br />
    // <b>Mode</b>: mode through frames (for use with discrete essentia). <br />

    
    CLASS_ATTR_CHAR(c, "summaryweight", 0, t_buf_essentia, summarizationweight);
    CLASS_ATTR_STYLE_LABEL(c,"summaryweight",0,"enumindex","Summarization Weight");
    CLASS_ATTR_ENUMINDEX(c, "summaryweight", 0, "None RMS Loudness");
    CLASS_ATTR_BASIC(c, "summaryweight", 0);
    CLASS_ATTR_FILTER_CLIP(c, "summaryweight", 0, 3);
    CLASS_ATTR_CATEGORY(c, "summaryweight", 0, "Summarization");
    // @description Sets the summarization weight (only applicable when <m>summary</m> is set to "Mean"): None, RMS, Loudness.

    
    CLASS_ATTR_CHAR(c, "summarypositive", 0, t_buf_essentia, summarizationpositiveonly);
    CLASS_ATTR_STYLE_LABEL(c,"summarypositive",0,"onoff","Summarize Positive Values Only");
    CLASS_ATTR_FILTER_CLIP(c, "summarypositive", 0, 1);
    CLASS_ATTR_CATEGORY(c, "summarypositive", 0, "Summarization");
    // @description Toggles the ability to only summarize the essentia with positive values (i.e. ignore negative or zero essentia),
    // if any positive feature is found (otherwise, the summarization happens with all the values).
    // When a feature is a vector or a matrix, it is enough for a single value in the vector or matrix to be positive in order for
    // the whole vector to be accounted for.

    
    
    
    // CQT attributes
    CLASS_ATTR_LONG(c, "cqtbinsperoctave", 0, t_buf_essentia, CQT_binsPerOctave);
    CLASS_ATTR_STYLE_LABEL(c,"cqtbinsperoctave",0,"text","CQT Bins Per Octave");
    CLASS_ATTR_CATEGORY(c, "cqtbinsperoctave", 0, "Constant-Q Transform");
    // @description Sets the number of bins per octave of the Constant-Q Transform.
    
    CLASS_ATTR_LONG(c, "cqtnumbins", 0, t_buf_essentia, CQT_numBins);
    CLASS_ATTR_STYLE_LABEL(c,"cqtnumbins",0,"text","CQT Total Number of Bins");
    CLASS_ATTR_CATEGORY(c, "cqtnumbins", 0, "Constant-Q Transform");
    // @description Sets the total number of bins of the Constant-Q Transform.
    
    CLASS_ATTR_DOUBLE(c, "cqtminpitch", 0, t_buf_essentia, CQT_minPitch);
    CLASS_ATTR_STYLE_LABEL(c,"cqtminpitch",0,"text","CQT Minimum Pitch");
    CLASS_ATTR_CATEGORY(c, "cqtminpitch", 0, "Constant-Q Transform");
    // @description Sets the minimum pitch (in the <m>pitchunit</m>) of the Constant-Q Transform.
    
    CLASS_ATTR_DOUBLE(c, "cqtthresh", 0, t_buf_essentia, CQT_threshold);
    CLASS_ATTR_STYLE_LABEL(c,"cqtthresh",0,"text","CQT Threshold");
    CLASS_ATTR_CATEGORY(c, "cqtthresh", 0, "Constant-Q Transform");
    // @description Bins whose magnitude is below this quantile are discarded of the Constant-Q Transform.
    
    CLASS_ATTR_DOUBLE(c, "cqtscale", 0, t_buf_essentia, CQT_scale);
    CLASS_ATTR_STYLE_LABEL(c,"cqtscale",0,"text","CQT Filters Scale");
    CLASS_ATTR_CATEGORY(c, "cqtscale", 0, "Constant-Q Transform");
    // @description Filters scale (larger values use longer windows) of the Constant-Q Transform.
    
    CLASS_ATTR_LONG(c, "cqtminkernelsize", 0, t_buf_essentia, CQT_minimumKernelSize);
    CLASS_ATTR_STYLE_LABEL(c,"cqtminkernelsize",0,"text","CQT Minimum Kernel Size");
    CLASS_ATTR_CATEGORY(c, "cqtminkernelsize", 0, "Constant-Q Transform");
    // @description Sets the minimum size allowed for frequency kernels of the Constant-Q Transform.
    
    
    // YIN attributes
    CLASS_ATTR_DOUBLE(c, "yinminfreq", 0, t_buf_essentia, YIN_minFrequency);
    CLASS_ATTR_STYLE_LABEL(c,"yinminfreq",0,"text","YIN Minimum Frequency");
    CLASS_ATTR_CATEGORY(c, "yinminfreq", 0, "YIN");
    // @description Sets the minimum frequency for the YIN analyis (in the <m>frequnit</m> attribute).

    CLASS_ATTR_DOUBLE(c, "yinmaxfreq", 0, t_buf_essentia, YIN_maxFrequency);
    CLASS_ATTR_STYLE_LABEL(c,"yinmaxfreq",0,"text","YIN Maximum Frequency");
    CLASS_ATTR_CATEGORY(c, "yinmaxfreq", 0, "YIN");
    // @description Sets the maximum frequency for the YIN analyis (in the <m>frequnit</m> attribute).

    CLASS_ATTR_DOUBLE(c, "yintolerance", 0, t_buf_essentia, YIN_tolerance);
    CLASS_ATTR_STYLE_LABEL(c,"yintolerance",0,"text","YIN Tolerance");
    CLASS_ATTR_CATEGORY(c, "yintolerance", 0, "YIN");
    // @description Sets the tolerance for the YIN analyis.

    
    // PEAKS attributes
    CLASS_ATTR_DOUBLE(c, "peaksmagthresh", 0, t_buf_essentia, PEAKS_magnitudeThreshold);
    CLASS_ATTR_STYLE_LABEL(c,"peaksmagthresh",0,"text","Peaks Magnitude Threshold");
    CLASS_ATTR_CATEGORY(c, "peaksmagthresh", 0, "Peaks");
    // @description Sets the magnitude threshold for the Peaks analyis (in the <m>ampunit</m> attribute).

    CLASS_ATTR_DOUBLE(c, "peaksminfreq", 0, t_buf_essentia, PEAKS_minFrequency);
    CLASS_ATTR_STYLE_LABEL(c,"peaksminfreq",0,"text","Peaks Minimum Frequency");
    CLASS_ATTR_CATEGORY(c, "peaksminfreq", 0, "Peaks");
    // @description Sets the minimum frequency for the Peaks analyis (in the <m>frequnit</m> attribute).

    CLASS_ATTR_DOUBLE(c, "peaksmaxfreq", 0, t_buf_essentia, PEAKS_maxFrequency);
    CLASS_ATTR_STYLE_LABEL(c,"peaksmaxfreq",0,"text","Peaks Maximum Frequency");
    CLASS_ATTR_CATEGORY(c, "peaksmaxfreq", 0, "Peaks");
    // @description Sets the maximum frequency for the Peaks analyis (in the <m>frequnit</m> attribute).

    CLASS_ATTR_LONG(c, "peaksmaxnum", 0, t_buf_essentia, PEAKS_maxPeaks);
    CLASS_ATTR_STYLE_LABEL(c,"peaksmaxnum",0,"text","Maximum Number Of Peaks");
    CLASS_ATTR_CATEGORY(c, "peaksmaxnum", 0, "Peaks");
    // @description Sets the maximum number of peaks for the Peaks analyis.

    CLASS_ATTR_SYM(c, "peaksorderby", 0, t_buf_essentia, onsetDetectionMethod);
    CLASS_ATTR_STYLE_LABEL(c,"peaksorderby",0,"enum","Order Peaks By");
    CLASS_ATTR_ENUM(c, "peaksorderby", 0, "frequency magnitude");
    CLASS_ATTR_CATEGORY(c, "peaksorderby", 0, "Peaks");
    // @description Sets the ordering method of the peaks in the Peaks analysis.
    
    
    CLASS_ATTR_SYM(c, "onsetdetectionmethod", 0, t_buf_essentia, onsetDetectionMethod);
    CLASS_ATTR_STYLE_LABEL(c,"onsetdetectionmethod",0,"enum","Onset Detection Method");
    CLASS_ATTR_ENUM(c, "onsetdetectionmethod", 0, "hfc complex complex_phase flux melflux rms");
    CLASS_ATTR_CATEGORY(c, "onsetdetectionmethod", 0, "Onset Detection");
    // @description Sets the method used for the onset detection: "hfc" (high frequency content),
    // "complex" (the complex-domain spectral difference function), "complex_phase", (the simplified complex-domain
    // spectral difference function taking into account phase changes, weighted by magnitude), "flux" (spectral flux),
    // "melflux" (similar to spectral flux, but using half-rectified energy changes in Mel-frequency bands),
    // "rms" (the difference function, measuring the half-rectified change of the RMS of the magnitude spectrum).
    // Refer to the Essentia documentation for the "OnsetDetection" algorithm to know more.
    

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}


const char *ears_essentia_feature_to_description(e_ears_feature feature)
{
    switch (feature) {

        case EARS_FEATURE_FRAMETIME:
            return "Analysis Frame Center Time";
            break;
            

        case EARS_FEATURE_SPECTRUM:
            return "Magnitude Spectrum";
            break;

        case EARS_FEATURE_POWERSPECTRUM:
            return "Power Spectrum";
            break;

/// ENVELOPES
            
        case EARS_FEATURE_ENVELOPE:
            return "Envelope";
            break;
            
        case EARS_FEATURE_LOGATTACKTIME:
            return "Logarithmic (base-10) attack time";
            break;
            
        case EARS_FEATURE_ENVMAXTIME:
            return "Time of the envelope maximum";
            break;

        case EARS_FEATURE_ENVMINTIME:
            return "Time of the envelope minimum";
            break;

        case EARS_FEATURE_STRONGDECAY:
            return "Strong decay";
            break;

            
// Standard
        case EARS_FEATURE_DERIVATIVE:
            return "Derivative";
            break;

        case EARS_FEATURE_MIN:
            return "Minimum";
            break;
        
        case EARS_FEATURE_MAX:
            return "Maximum";
            break;

        case EARS_FEATURE_WELCH:
            return "Welch";
            break;
            
            
            

            
            
            
        case EARS_FEATURE_FLUX:
            return "Flux";
            break;

            
        case EARS_FEATURE_ZEROCROSSINGRATE:
            return "Zero-crossing rate";
            break;
            
        case EARS_FEATURE_ENERGYBAND:
            return "Energy band";
            break;

        case EARS_FEATURE_ENERGYBANDRATIO:
            return "Energy band ratio";
            break;
            
        case EARS_FEATURE_MFCC:
            return "MFCC";
            break;

        case EARS_FEATURE_BFCC:
            return "BFCC";
            break;

        case EARS_FEATURE_BARKBANDS:
            return "Bark bands";
            break;

        case EARS_FEATURE_ERBBANDS:
            return "ERB bands";
            break;

        case EARS_FEATURE_FREQUENCYBANDS:
            return "Frequency bands";
            break;

        case EARS_FEATURE_GFCC:
            return "GFCC";
            break;
            

        case EARS_FEATURE_HFC:
            return "HFC";
            break;

        case EARS_FEATURE_LPC:
            return "LPC";
            break;

        case EARS_FEATURE_MAXMAGFREQ:
            return "Maximum magnitude frequency";
            break;

        case EARS_FEATURE_ROLLOFF:
            return "Roll-off frequency";
            break;

        case EARS_FEATURE_TIMEDOMAINSPECTRALCENTROID:
            return "Time-domain spectral centroid";
            break;

        case EARS_FEATURE_SPECTRALCOMPLEXITY:
            return "Spectral complexity";
            break;

        case EARS_FEATURE_SPECTRALCONTRAST:
            return "Spectral contrast";
            break;

            
        case EARS_FEATURE_STRONGPEAK:
            return "Strong peak";
            break;
            
        case EARS_FEATURE_TRIANGULARBANDS:
            return "Triangular bands";
            break;

        case EARS_FEATURE_TRIANGULARBARKBANDS:
            return "Triangular bark bands";
            break;

        case EARS_FEATURE_BEATTRACKERDEGARA:
            return "Degara beat tracker";
            break;

        case EARS_FEATURE_BEATTRACKERMULTIFEATURE:
            return "Multi-feature beat tracker";
            break;

        case EARS_FEATURE_BEATSLOUDNESS:
            return "Beats loudness";
            break;
            
        case EARS_FEATURE_DANCEABILITY:
            return "Danceability";
            break;

        case EARS_FEATURE_LOOPBPMESTIMATOR:
            return "Loop BPM estimator";
            break;

        case EARS_FEATURE_ONSETDETECTION:
            return "Onset detection function";
            break;

        case EARS_FEATURE_ONSETDETECTIONGLOBAL:
            return "Onset detection global";
            break;

        case EARS_FEATURE_ONSETRATE:
            return "Onset rate";
            break;

        case EARS_FEATURE_ONSETS:
            return "Onsets";
            break;
            
        case EARS_FEATURE_PERCIVALBPMESTIMATOR:
            return "Percival BPM estimator";
            break;

        case EARS_FEATURE_RHYTHMDESCRIPTORS:
            return "Rhythm descriptors";
            break;

        case EARS_FEATURE_RHYTHMEXTRACTOR:
            return "Rhythm extractor";
            break;

        case EARS_FEATURE_RHYTHMEXTRACTOR2013:
            return "Rhythm extractor 2013";
            break;
            
        case EARS_FEATURE_SINGLEBEATLOUDNESS:
            return "Single beat loudness";
            break;

        case EARS_FEATURE_SUPERFLUXEXTRACTOR:
            return "SuperFlux extractor";
            break;

            
            
        case EARS_FEATURE_SPECTRALCENTRALMOMENTS:
            return "Spectral central moments";
            break;

        case EARS_FEATURE_TEMPORALCENTRALMOMENTS:
            return "Temporal central moments";
            break;
            
        case EARS_FEATURE_SPECTRALRAWMOMENTS:
            return "Spectral raw moments";
            break;
            
        case EARS_FEATURE_TEMPORALRAWMOMENTS:
            return "Temporal raw moments";
            break;
            
        case EARS_FEATURE_SPECTRALCENTROID:
            return "Spectral centroid";
            break;
            
        case EARS_FEATURE_TEMPORALCENTROID:
            return "Temporal centroid";
            break;
            
        case EARS_FEATURE_SPECTRALCREST:
            return "Spectral crest";
            break;
            
        case EARS_FEATURE_TEMPORALCREST:
            return "Temporal crest";
            break;
            
        case EARS_FEATURE_SPECTRALDECREASE:
            return "Spectral decrease";
            break;
            
        case EARS_FEATURE_TEMPORALDECREASE:
            return "Temporal decrease";
            break;
            
        case EARS_FEATURE_TEMPORALDISTRIBUTIONSHAPE:
            return "Temporal distribution shape";
            break;

        case EARS_FEATURE_SPECTRALDISTRIBUTIONSHAPE:
            return "Spectral distribution shape";
            break;

        case EARS_FEATURE_SPECTRALSPREAD:
            return "Spectral spread";
            break;
            
        case EARS_FEATURE_TEMPORALSPREAD:
            return "Temporal spread";
            break;

        case EARS_FEATURE_SPECTRALSKEWNESS:
            return "Spectral skewness";
            break;
            
        case EARS_FEATURE_TEMPORALSKEWNESS:
            return "Temporal skewness";
            break;

        case EARS_FEATURE_SPECTRALKURTOSIS:
            return "Spectral kurtosis";
            break;
            
        case EARS_FEATURE_TEMPORALKURTOSIS:
            return "Temporal kurtosis";
            break;


        case EARS_FEATURE_SPECTRALENERGY:
            return "Spectral energy";
            break;

        case EARS_FEATURE_SPECTRALENTROPY:
            return "Spectral entropy";
            break;

        case EARS_FEATURE_TEMPORALFLATNESS:
            return "Temporal flatness";
            break;

        case EARS_FEATURE_SPECTRALFLATNESS:
            return "Spectral flatness";
            break;

        case EARS_FEATURE_SPECTRALGEOMETRICMEAN:
            return "Geometric mean";
            break;

        case EARS_FEATURE_INSTANTPOWER:
            return "Instant power";
            break;

        case EARS_FEATURE_SPECTRALMEAN:
            return "Spectral mean";
            break;

        case EARS_FEATURE_SPECTRALMEDIAN:
            return "Spectral median";
            break;

        case EARS_FEATURE_SPECTRALRMS:
            return "Spectral root mean square";
            break;

        case EARS_FEATURE_TEMPORALVARIANCE:
            return "Temporal variance";
            break;

        case EARS_FEATURE_SPECTRALVARIANCE:
            return "Spectral variance";
            break;

            // Tonal
            
        case EARS_FEATURE_CHORDSDETECTION:
            return "Chords detection";
            break;

        case EARS_FEATURE_DISSONANCE:
            return "Dissonance";
            break;

        case EARS_FEATURE_HPCP:
            return "HPCP";
            break;

        case EARS_FEATURE_HARMONICPEAKS:
            return "Harmonic peaks";
            break;

        case EARS_FEATURE_HIGHRESOLUTIONFEATURES:
            return "High resolution features";
            break;

        case EARS_FEATURE_INHARMONICITY:
            return "Inharmonicity";
            break;

        case EARS_FEATURE_KEY:
            return "Key";
            break;
            
        case EARS_FEATURE_KEYEXTRACTOR:
            return "Key extractor";
            break;

        case EARS_FEATURE_ODDTOEVENHARMONICENERGYRATIO:
            return "Odd-to-even harmonic energy ratio";
            break;

        case EARS_FEATURE_PITCHSALIENCE:
            return "Pitch salience";
            break;

        case EARS_FEATURE_SPECTRUMCQ:
            return "Constant-Q Spectrum";
            break;

        case EARS_FEATURE_TRISTIMULUS:
            return "Tristimulus";
            break;

        case EARS_FEATURE_TUNINGFREQUENCY:
            return "Tuning frequency";
            break;

// FINGERPRINTING
//        case EARS_FEATURE_CHROMAPRINTER:
//            return "Chromaprinter";
//            break;

            
// AUDIO QUALITY
        case EARS_FEATURE_SNR:
            return "Signal-to-noise ratio";
            break;

// DURATION/SILENCE
        case EARS_FEATURE_DURATION:
            return "Duration";
            break;

        case EARS_FEATURE_EFFECTIVEDURATION:
            return "Effective duration";
            break;

        case EARS_FEATURE_SILENCERATE:
            return "Silence rate";
            break;
            
// LOUDNESS/DYNAMICS
        case EARS_FEATURE_DYNAMICCOMPLEXITY:
            return "Dynamic complexity";
            break;

        case EARS_FEATURE_LARM:
            return "LARM long-term loudness";
            break;

        case EARS_FEATURE_LEQ:
            return "Equivalent sound level";
            break;

        case EARS_FEATURE_LOUDNESS:
            return "Loudness";
            break;
            
        case EARS_FEATURE_LOUDNESSVICKERS:
            return "Vickers's loudness";
            break;

        case EARS_FEATURE_REPLAYGAIN:
            return "Replay Gain loudness value";
            break;
            
// Pitch
            
        case EARS_FEATURE_MULTIPITCHKLAPURI:
            return "Multi-pitch Klapuri";
            break;

        case EARS_FEATURE_MULTIPITCHMELODIA:
            return "Multi-pitch Melodia";
            break;

        case EARS_FEATURE_PITCHSALIENCEFUNCTION:
            return "Pitch salience function";
            break;

        case EARS_FEATURE_PITCHMELODIA:
            return "Pitch Melodia";
            break;

        case EARS_FEATURE_PREDOMINANTPITCHMELODIA:
            return "Predominant Pitch Melodia";
            break;

        case EARS_FEATURE_PITCHYIN:
            return "Pitch Yin";
            break;

        case EARS_FEATURE_PITCHYINFFT:
            return "Pitch Yin FFT";
            break;

        case EARS_FEATURE_PITCHYINPROBABILISTIC:
            return "Pitch Yin probabilistic";
            break;

        case EARS_FEATURE_PITCHYINPROBABILITIES:
            return "Pitch Yin probabilities";
            break;
        
        case EARS_FEATURE_VIBRATO:
            return "Vibrato";
            break;

            
        default:
            return "Unknown feature";
            break;
    }
}

e_ears_feature ears_essentia_feature_from_symbol(t_symbol *sym, long *temporalmode, t_ears_err *err)
{
    if (!sym || !sym->s_name)
        return EARS_FEATURE_NONE;
    
    
    // convert to lower case
    char buf[2048];
    long len = strlen(sym->s_name);
    for (long i = 0; i <= len; i++) { // including trailing 0
        buf[i] = tolower(sym->s_name[i]);
    }
    t_symbol *s = gensym(buf);
    
    long tm = EARS_ANALYSIS_TEMPORALMODE_WHOLE;
    if (ears_symbol_ends_with(s, "...", false)) {
        snprintf_zero(buf, 2048, "%s", s->s_name);
        buf[strlen(s->s_name)-3] = 0;
        s = gensym(buf);
        tm = EARS_ANALYSIS_TEMPORALMODE_TIMESERIES;
    } else if (ears_symbol_ends_with(s, ":::", false)) {
        snprintf_zero(buf, 2048, "%s", s->s_name);
        buf[strlen(s->s_name)-3] = 0;
        s = gensym(buf);
        tm = EARS_ANALYSIS_TEMPORALMODE_LABELLEDTIMESERIES;
    } else if (ears_symbol_ends_with(s, "~", false)) {
        snprintf_zero(buf, 2048, "%s", s->s_name);
        buf[strlen(s->s_name)-1] = 0;
        s = gensym(buf);
        tm = EARS_ANALYSIS_TEMPORALMODE_BUFFER;
    } else {
        tm = EARS_ANALYSIS_TEMPORALMODE_WHOLE;
    }
    
    if (err)
        *err = EARS_ERR_NONE;

    *temporalmode = tm;

    if (s == gensym("frametime"))
        return EARS_FEATURE_FRAMETIME;

    
    if (s == gensym("spectrum"))
        return EARS_FEATURE_SPECTRUM;
    if (s == gensym("powerspectrum"))
        return EARS_FEATURE_POWERSPECTRUM;
    if (s == gensym("envelope"))
        return EARS_FEATURE_ENVELOPE;
    if (s == gensym("logattacktime"))
        return EARS_FEATURE_LOGATTACKTIME;
    if (s == gensym("envmaxtime"))
        return EARS_FEATURE_ENVMAXTIME;
    if (s == gensym("envmintime"))
        return EARS_FEATURE_ENVMINTIME;
    if (s == gensym("strongdecay"))
        return EARS_FEATURE_STRONGDECAY;
    if (s == gensym("temporalcentroid"))
        return EARS_FEATURE_TEMPORALCENTROID;
    if (s == gensym("duration"))
        return EARS_FEATURE_DURATION;

    
    // Standard
    if (s == gensym("derivative")) {
        if (tm != EARS_ANALYSIS_TEMPORALMODE_BUFFER)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_DERIVATIVE;
    }
    if (s == gensym("min"))
        return EARS_FEATURE_MIN;
    if (s == gensym("max"))
        return EARS_FEATURE_MAX;
    if (s == gensym("welch"))
        return EARS_FEATURE_WELCH;
    

    if (s == gensym("flatness") || s == gensym("spectralflatness"))
        return EARS_FEATURE_SPECTRALFLATNESS;
    if (s == gensym("flux"))
        return EARS_FEATURE_FLUX;

    
    if (s == gensym("zerocrossingrate"))
        return EARS_FEATURE_ZEROCROSSINGRATE;
    if (s == gensym("energyband"))
        return EARS_FEATURE_ENERGYBAND;
    if (s == gensym("energybandratio"))
        return EARS_FEATURE_ENERGYBANDRATIO;
    if (s == gensym("mfcc"))
        return EARS_FEATURE_MFCC;
    if (s == gensym("bfcc"))
        return EARS_FEATURE_BFCC;
    if (s == gensym("barkbands"))
        return EARS_FEATURE_BARKBANDS;
    if (s == gensym("erbbands"))
        return EARS_FEATURE_ERBBANDS;
    if (s == gensym("frequencybands"))
        return EARS_FEATURE_FREQUENCYBANDS;
    if (s == gensym("gfcc"))
        return EARS_FEATURE_GFCC;
    if (s == gensym("lpc"))
        return EARS_FEATURE_LPC;
    if (s == gensym("hfc"))
        return EARS_FEATURE_HFC;
    if (s == gensym("maxmagfreq"))
        return EARS_FEATURE_MAXMAGFREQ;
    if (s == gensym("rolloff"))
        return EARS_FEATURE_ROLLOFF;
    if (s == gensym("timedomainspectralcentroid") || s == gensym("spectralcentroidtime"))
        return EARS_FEATURE_TIMEDOMAINSPECTRALCENTROID;
    if (s == gensym("spectralcomplexity"))
        return EARS_FEATURE_SPECTRALCOMPLEXITY;
    if (s == gensym("spectralcontrast"))
        return EARS_FEATURE_SPECTRALCONTRAST;
    if (s == gensym("strongpeak"))
        return EARS_FEATURE_STRONGPEAK;
    if (s == gensym("triangularbands"))
        return EARS_FEATURE_TRIANGULARBANDS;
    if (s == gensym("triangularbarkbands"))
        return EARS_FEATURE_TRIANGULARBARKBANDS;

    
    if (s == gensym("beattrackerdegara"))
        return EARS_FEATURE_BEATTRACKERDEGARA;
    if (s == gensym("beattrackermultifeature"))
        return EARS_FEATURE_BEATTRACKERMULTIFEATURE;
    if (s == gensym("beatsloudness"))
        return EARS_FEATURE_BEATSLOUDNESS;
    if (s == gensym("danceability"))
        return EARS_FEATURE_DANCEABILITY;
    if (s == gensym("loopbpmestimator"))
        return EARS_FEATURE_LOOPBPMESTIMATOR;
    if (s == gensym("onsetdetection"))
        return EARS_FEATURE_ONSETDETECTION;
    if (s == gensym("onsetdetectionglobal"))
        return EARS_FEATURE_ONSETDETECTIONGLOBAL;
    if (s == gensym("onsetrate"))
        return EARS_FEATURE_ONSETRATE;
    if (s == gensym("onsets"))
        return EARS_FEATURE_ONSETS;
    if (s == gensym("percivalbpmestimator"))
        return EARS_FEATURE_PERCIVALBPMESTIMATOR;
    if (s == gensym("rhythmdescriptors"))
        return EARS_FEATURE_RHYTHMDESCRIPTORS;
    if (s == gensym("rhythmextractor"))
        return EARS_FEATURE_RHYTHMEXTRACTOR;
    if (s == gensym("rhythmextractor2013"))
        return EARS_FEATURE_RHYTHMEXTRACTOR2013;
    if (s == gensym("singlebeatloudness"))
        return EARS_FEATURE_SINGLEBEATLOUDNESS;
    if (s == gensym("superfluxextractor"))
        return EARS_FEATURE_SUPERFLUXEXTRACTOR;
    
    
    
    // Statistics
    
    if (s == gensym("spectralcentralmoments"))
        return EARS_FEATURE_SPECTRALCENTRALMOMENTS;
    if (s == gensym("temporalcentralmoments"))
        return EARS_FEATURE_TEMPORALCENTRALMOMENTS;
    if (s == gensym("spectralrawmoments"))
        return EARS_FEATURE_SPECTRALRAWMOMENTS;
    if (s == gensym("temporalrawmoments"))
        return EARS_FEATURE_TEMPORALRAWMOMENTS;
    if (s == gensym("temporalcentroid"))
        return EARS_FEATURE_TEMPORALCENTROID;
    if (s == gensym("spectralcentroid"))
        return EARS_FEATURE_SPECTRALCENTROID;
    if (s == gensym("temporalcrest"))
        return EARS_FEATURE_TEMPORALCREST;
    if (s == gensym("spectralcrest"))
        return EARS_FEATURE_SPECTRALCREST;
    if (s == gensym("temporaldecrease"))
        return EARS_FEATURE_TEMPORALDECREASE;
    if (s == gensym("spectraldecrease"))
        return EARS_FEATURE_SPECTRALDECREASE;
    if (s == gensym("temporaldistributionshape"))
        return EARS_FEATURE_TEMPORALDISTRIBUTIONSHAPE;
    if (s == gensym("spectraldistributionshape"))
        return EARS_FEATURE_SPECTRALDISTRIBUTIONSHAPE;
    if (s == gensym("temporalspread"))
        return EARS_FEATURE_TEMPORALSPREAD;
    if (s == gensym("spectralspread"))
        return EARS_FEATURE_SPECTRALSPREAD;
    if (s == gensym("temporalskewness"))
        return EARS_FEATURE_TEMPORALSKEWNESS;
    if (s == gensym("spectralskewness"))
        return EARS_FEATURE_SPECTRALSKEWNESS;
    if (s == gensym("temporalkurtosis"))
        return EARS_FEATURE_TEMPORALKURTOSIS;
    if (s == gensym("spectralkurtosis"))
        return EARS_FEATURE_SPECTRALKURTOSIS;
    if (s == gensym("spectralenergy"))
        return EARS_FEATURE_SPECTRALENERGY;
    if (s == gensym("spectralentropy"))
        return EARS_FEATURE_SPECTRALENTROPY;
    if (s == gensym("temporalflatness"))
        return EARS_FEATURE_TEMPORALFLATNESS;
    if (s == gensym("spectralflatness"))
        return EARS_FEATURE_SPECTRALFLATNESS;
    if (s == gensym("spectralgeometricmean"))
        return EARS_FEATURE_SPECTRALGEOMETRICMEAN;
    if (s == gensym("spectralmean"))
        return EARS_FEATURE_SPECTRALMEAN;
    if (s == gensym("spectralmedian"))
        return EARS_FEATURE_SPECTRALMEDIAN;
    if (s == gensym("spectralrms"))
        return EARS_FEATURE_SPECTRALRMS;
    if (s == gensym("temporalvariance"))
        return EARS_FEATURE_TEMPORALVARIANCE;
    if (s == gensym("spectralvariance"))
        return EARS_FEATURE_SPECTRALVARIANCE;
    if (s == gensym("instantpower"))
        return EARS_FEATURE_INSTANTPOWER;

    
    if (s == gensym("chordsdetection"))
        return EARS_FEATURE_CHORDSDETECTION;
    if (s == gensym("dissonance"))
        return EARS_FEATURE_DISSONANCE;
    if (s == gensym("hpcp"))
        return EARS_FEATURE_HPCP;
    if (s == gensym("harmonicpeaks"))
        return EARS_FEATURE_HARMONICPEAKS;
    if (s == gensym("highresolutionfeatures"))
        return EARS_FEATURE_HIGHRESOLUTIONFEATURES;
    if (s == gensym("inharmonicity"))
        return EARS_FEATURE_INHARMONICITY;
    if (s == gensym("key"))
        return EARS_FEATURE_KEY;
    if (s == gensym("keyextractor"))
        return EARS_FEATURE_KEYEXTRACTOR;
    if (s == gensym("oddtoevenharmonicenergyratio"))
        return EARS_FEATURE_ODDTOEVENHARMONICENERGYRATIO;
    if (s == gensym("pitchsalience"))
        return EARS_FEATURE_PITCHSALIENCE;
    if (s == gensym("spectrumcq"))
        return EARS_FEATURE_SPECTRUMCQ;
    if (s == gensym("tristimulus"))
        return EARS_FEATURE_TRISTIMULUS;
    if (s == gensym("tuningfrequency"))
        return EARS_FEATURE_TUNINGFREQUENCY;
    
    // FINGERPRINTING
//    if (s == gensym("chromaprinter"))
//        return EARS_FEATURE_CHROMAPRINTER;

    // AUDIO QUALITY
    if (s == gensym("snr"))
        return EARS_FEATURE_SNR;

    
    // DURATION/SILENCE
    if (s == gensym("duration"))
        return EARS_FEATURE_DURATION;
    if (s == gensym("effectiveduration"))
        return EARS_FEATURE_EFFECTIVEDURATION;
    if (s == gensym("silencerate"))
        return EARS_FEATURE_SILENCERATE;
    
    // LOUDNESS/DYNAMICS
    if (s == gensym("dynamiccomplecity"))
        return EARS_FEATURE_DYNAMICCOMPLEXITY;
    if (s == gensym("larm"))
        return EARS_FEATURE_LARM;
    if (s == gensym("leq"))
        return EARS_FEATURE_LEQ;
    if (s == gensym("loudness"))
        return EARS_FEATURE_LOUDNESS;
    if (s == gensym("loudnessvickers"))
        return EARS_FEATURE_LOUDNESSVICKERS;
    if (s == gensym("replaygain"))
        return EARS_FEATURE_REPLAYGAIN;

    // PITCH
    if (s == gensym("multipitchklapuri"))
        return EARS_FEATURE_MULTIPITCHKLAPURI;
    if (s == gensym("multipitchmelodia"))
        return EARS_FEATURE_MULTIPITCHMELODIA;
    if (s == gensym("pitchsaliencefunction"))
        return EARS_FEATURE_PITCHSALIENCEFUNCTION;
    if (s == gensym("pitchmelodia"))
        return EARS_FEATURE_PITCHMELODIA;
    if (s == gensym("predominantpitchmelodia"))
        return EARS_FEATURE_PREDOMINANTPITCHMELODIA;
    if (s == gensym("pitchyin"))
        return EARS_FEATURE_PITCHYIN;
    if (s == gensym("pitchyinfft"))
        return EARS_FEATURE_PITCHYINFFT;
    if (s == gensym("pitchyinprobabilistic"))
        return EARS_FEATURE_PITCHYINPROBABILISTIC;
    if (s == gensym("pitchyinprobabilities"))
        return EARS_FEATURE_PITCHYINPROBABILITIES;
    if (s == gensym("vibrato"))
        return EARS_FEATURE_VIBRATO;
    
    return EARS_FEATURE_UNKNOWN;
}

void buf_essentia_assist(t_buf_essentia *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else {
            const char *feat_desc = ears_essentia_feature_to_description((e_ears_feature)x->features[a-1]);
            sprintf(s, "list/llll: Parameters for feature No. %ld (%s)", a, feat_desc);
            // @in 1 @loop 1 @type llll @digest Parameters for each of the features (one inlet per feature).
        }
    } else {
        long featidx = x->outlet_featureidx[a];
        long outidx = x->outlet_featureoutputidx[a];
        const char *feat_desc = ears_essentia_feature_to_description((e_ears_feature)x->features[featidx]);
        if (featidx < x->num_features) {
            char *type = NULL;
            const char *unit = NULL;
            llllobj_get_llll_outlet_type_as_string((t_object *) x, LLLL_OBJ_VANILLA, a, &type);

            const char *feat_out_desc = "";
            if (featidx < x->extractors_lib.num_extractors) {
                long map = x->extractors_lib.extractors[featidx].output_map[outidx];
                if (map >= 0)
                    feat_out_desc = x->extractors_lib.extractors[featidx].output_desc[map];
                if (x->extractors_lib.extractors[featidx].essentia_output_pitchunit[map] != EARS_PITCHUNIT_UNKNOWN) {
                    unit = ears_pitchunit_to_abbrev(x->extractors_lib.extractors[featidx].local_pitchunit);
                } else if (x->extractors_lib.extractors[featidx].essentia_output_frequnit[map] != EARS_FREQUNIT_UNKNOWN) {
                    unit = ears_frequnit_to_abbrev(x->extractors_lib.extractors[featidx].local_frequnit);
                } else if (x->extractors_lib.extractors[featidx].essentia_output_ampunit[map] != EARS_AMPUNIT_UNKNOWN) {
                    unit = ears_ampunit_to_abbrev(x->extractors_lib.extractors[featidx].local_ampunit);
                } else if (x->extractors_lib.extractors[featidx].essentia_output_timeunit[map] != EARS_TIMEUNIT_UNKNOWN) {
                    unit = ears_timeunit_to_abbrev(x->extractors_lib.extractors[featidx].local_timeunit);
                }
            }

            if (x->temporalmodes[featidx] == EARS_ANALYSIS_TEMPORALMODE_WHOLE)
                sprintf(s, "llll (%s): %s - %s%s%s (global)", type, feat_desc, feat_out_desc, unit ? " " : "", unit ? unit : "");
            else if (x->temporalmodes[featidx] == EARS_ANALYSIS_TEMPORALMODE_TIMESERIES)
                sprintf(s, "llll (%s): %s - %s%s%s (time series)", type, feat_desc, feat_out_desc, unit ? " " : "", unit ? unit : "");
            else if (x->temporalmodes[featidx] == EARS_ANALYSIS_TEMPORALMODE_LABELLEDTIMESERIES)
                sprintf(s, "llll (%s): %s - %s%s%s (time-tagged time series)", type, feat_desc, feat_out_desc, unit ? " " : "", unit ? unit : "");
            else if (x->temporalmodes[featidx] == EARS_ANALYSIS_TEMPORALMODE_BUFFER)
                sprintf(s, "buffer: %s - %s%s%s", feat_desc, feat_out_desc, unit ? " " : "", unit ? unit : "");
        } else {
            sprintf(s, "Unused outlet");
        }
        // @out 0 @loop 1 @type llll/buffer @digest Feature for the corresponding key
        // @description Feature for one of the introduced key; some features require multiple outlets (see outlet assistance).
    }
}

void buf_essentia_inletinfo(t_buf_essentia *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}

e_ears_errorcodes check_temporal_mode(t_buf_essentia *x, e_ears_feature feat, e_ears_analysis_temporalmode temporalmode)
{
    e_ears_errorcodes temporalmode_err = EARS_ERR_NONE;
    e_ears_essentia_framemode framemode = ears_essentia_feature_to_framemode((t_object *)x, feat);
    
    if (feat == EARS_FEATURE_FRAMETIME && temporalmode == EARS_ANALYSIS_TEMPORALMODE_LABELLEDTIMESERIES)
        temporalmode_err = EARS_ERR_INVALID_MODE;

    switch (framemode) {
        case EARS_ESSENTIA_FRAMEMODE_GLOBALONLY:
            if (temporalmode != EARS_ANALYSIS_TEMPORALMODE_WHOLE)
                temporalmode_err = EARS_ERR_INVALID_MODE;
            break;
        case EARS_ESSENTIA_FRAMEMODE_FRAMEWISEONLY:
        case EARS_ESSENTIA_FRAMEMODE_GLOBALRETURNINGFRAMESONLY:
            if (temporalmode == EARS_ANALYSIS_TEMPORALMODE_WHOLE)
                temporalmode_err = EARS_ERR_INVALID_MODE;
            break;
        case EARS_ESSENTIA_FRAMEMODE_GLOBALRETURNINGFRAMESONLYNOBUFFERS:
            if (temporalmode != EARS_ANALYSIS_TEMPORALMODE_TIMESERIES && temporalmode != EARS_ANALYSIS_TEMPORALMODE_LABELLEDTIMESERIES)
                temporalmode_err = EARS_ERR_INVALID_MODE;
            break;
        case EARS_ESSENTIA_FRAMEMODE_GLOBALNOBUFFERS:
        case EARS_ESSENTIA_FRAMEMODE_FRAMEWISENOBUFFERS:
            if (temporalmode == EARS_ANALYSIS_TEMPORALMODE_BUFFER)
                temporalmode_err = EARS_ERR_INVALID_MODE;
            break;
        default:
            break;
    }
    return temporalmode_err;
}

const char *ears_temporalmode_to_desc(e_ears_analysis_temporalmode temporalmode)
{
    switch (temporalmode) {
        case EARS_ANALYSIS_TEMPORALMODE_WHOLE:
            return "global";
            break;
            
        case EARS_ANALYSIS_TEMPORALMODE_TIMESERIES:
            return "time series";
            break;

        case EARS_ANALYSIS_TEMPORALMODE_LABELLEDTIMESERIES:
            return "time-tagged time series";
            break;

        case EARS_ANALYSIS_TEMPORALMODE_BUFFER:
            return "buffer";
            break;
            
        default:
            return "unknown";
            break;
    }
}

t_ears_err buf_essentia_set_essentia(t_buf_essentia *x, t_llll *args)
{
    t_ears_err err = EARS_ERR_NONE;
    
    if (!args) {
        object_error((t_object *)x, "Wrong arguments!");
        return EARS_ERR_GENERIC;
    }
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    
    long new_num_features = args->l_size;
    
    for (long i = 0; i < x->num_features; i++)
        llll_free(x->algorithm_args[i]);
    x->num_features = new_num_features;
    x->features = (long *)bach_resizeptr(x->features, new_num_features * sizeof(long));
    x->features_numoutputs = (long *)bach_resizeptr(x->features_numoutputs, new_num_features * sizeof(long));
    x->temporalmodes = (long *)bach_resizeptr(x->temporalmodes, new_num_features * sizeof(long));
    x->algorithm_args = (t_llll **)bach_resizeptr(x->algorithm_args, new_num_features * sizeof(t_llll *));
    for (long i = 0; i < new_num_features; i++)
        x->algorithm_args[i] = llll_get();
    
    // finding number of outlets
    t_ears_err temporalmode_err = EARS_ERR_NONE;
    long temporalmode;
    long i = 0;
    long tot_num_outlets = 0;
    for (t_llllelem *el = args->l_head; el; el = el->l_next, i++) {
        e_ears_feature feat = EARS_FEATURE_UNKNOWN;
        if (hatom_gettype(&el->l_hatom) == H_SYM)
            feat = ears_essentia_feature_from_symbol(hatom_getsym(&el->l_hatom), &temporalmode, &temporalmode_err);
        else if (hatom_gettype(&el->l_hatom) == H_LLLL && hatom_getllll(&el->l_hatom)->l_head)
            feat = ears_essentia_feature_from_symbol(hatom_getsym(&hatom_getllll(&el->l_hatom)->l_head->l_hatom), &temporalmode, &temporalmode_err);
        x->features_numoutputs[i] = ears_essentia_feature_to_numouts(feat);
        tot_num_outlets += x->features_numoutputs[i];
    }

    x->outlet_featureidx = (long *)bach_resizeptr(x->outlet_featureidx, tot_num_outlets * sizeof(long));
    x->outlet_featureoutputidx = (long *)bach_resizeptr(x->outlet_featureoutputidx, tot_num_outlets * sizeof(long));

    x->num_outlets = tot_num_outlets;
    
    i = 0;
    long o_offset = 0;
    for (t_llllelem *el = args->l_head; el; el = el->l_next, i++) {
        e_ears_feature feat = EARS_FEATURE_UNKNOWN;
        if (hatom_gettype(&el->l_hatom) == H_SYM)
            feat = ears_essentia_feature_from_symbol(hatom_getsym(&el->l_hatom), &temporalmode, &temporalmode_err);
        else if (hatom_gettype(&el->l_hatom) == H_LLLL && hatom_getllll(&el->l_hatom)->l_head)
            feat = ears_essentia_feature_from_symbol(hatom_getsym(&hatom_getllll(&el->l_hatom)->l_head->l_hatom), &temporalmode, &temporalmode_err);
        long this_num_outputs = ears_essentia_feature_to_numouts(feat);
        for (long o = 0; o < this_num_outputs; o++) {
            x->outlet_featureidx[o_offset+o] = i;
            x->outlet_featureoutputidx[o_offset+o] = o;
        }
        o_offset += this_num_outputs;
    }
    

    i = 0;
    temporalmode_err = EARS_ERR_NONE;
    for (t_llllelem *el = args->l_head; el; el = el->l_next, i++) {
        x->features[i] = EARS_FEATURE_UNKNOWN;
        x->temporalmodes[i] = EARS_ANALYSIS_TEMPORALMODE_WHOLE;
        if (hatom_gettype(&el->l_hatom) == H_SYM) {
            e_ears_feature feat = ears_essentia_feature_from_symbol(hatom_getsym(&el->l_hatom), &x->temporalmodes[i], &temporalmode_err);
            x->features[i] = feat;
            if (temporalmode_err == EARS_ERR_NONE)
                temporalmode_err = check_temporal_mode(x, feat, (e_ears_analysis_temporalmode)x->temporalmodes[i]);
            if (feat == EARS_FEATURE_UNKNOWN) {
                object_error((t_object *)x, "Unknown feature at index %d", i+1);
                err = EARS_ERR_GENERIC;
            } else if (temporalmode_err == EARS_ERR_INVALID_MODE) {
                object_error((t_object *)x, "Unsupported temporal mode '%s' for feature '%s'", ears_temporalmode_to_desc((e_ears_analysis_temporalmode)x->temporalmodes[i]), ears_essentia_feature_to_description(feat));
                err = EARS_ERR_GENERIC;
            }
        } else if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            t_llll *subll = hatom_getllll(&el->l_hatom);
            if (subll && subll->l_head && hatom_gettype(&subll->l_head->l_hatom) == H_SYM) {
                e_ears_feature feat = ears_essentia_feature_from_symbol(hatom_getsym(&subll->l_head->l_hatom), &x->temporalmodes[i], &temporalmode_err);
                x->features[i] = feat;

                if (temporalmode_err == EARS_ERR_NONE)
                    temporalmode_err = check_temporal_mode(x, feat, (e_ears_analysis_temporalmode)x->temporalmodes[i]);
                
                if (feat == EARS_FEATURE_UNKNOWN) {
                    object_error((t_object *)x, "Unknown feature at index %d", i+1);
                    err = EARS_ERR_GENERIC;
                } else if (temporalmode_err == EARS_ERR_INVALID_MODE) {
                    object_error((t_object *)x, "Unsupported temporal mode '%s' for feature '%s'", ears_temporalmode_to_desc((e_ears_analysis_temporalmode)x->temporalmodes[i]), ears_essentia_feature_to_description(feat));
                    err = EARS_ERR_GENERIC;
                }
                llll_free(x->algorithm_args[i]);
                x->algorithm_args[i] = llll_clone(subll);
                llll_behead(x->algorithm_args[i]);
            } else {
                object_error((t_object *)x, "Unknown feature at index %d", i+1);
                err = EARS_ERR_GENERIC;
            }
        } else {
            object_error((t_object *)x, "Unknown feature at index %d", i+1);
            err = EARS_ERR_GENERIC;
        }
    }
    x->must_recreate_extractors = true;
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    return err;
}

t_buf_essentia *buf_essentia_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_essentia *x;
//    long true_ac = attr_args_offset(argc, argv);

    // Since we can insert attribute-argument manually inside lllls in the object box, we need to parse the true_ac manually
    long true_ac = argc;
    long i = argc - 1;
    while (i >= 0) {
        if (atom_gettype(argv+i) == A_SYM) {
            t_symbol *sym = atom_getsym(argv+i);
            if (sym && sym->s_name[0] == '@')
                true_ac = i;
            else if (sym && (strchr(sym->s_name, ']') || strchr(sym->s_name, '[')))
                break;
        }
        i--;
    }
    
    
    x = (t_buf_essentia*)object_alloc_debug(s_tag_class);
    if (x) {
        x->num_features = 1;
        x->features = (long *)bach_newptrclear(1 * sizeof(long));
        x->features_numoutputs = (long *)bach_newptrclear(1 * sizeof(long));
        x->outlet_featureoutputidx = (long *)bach_newptrclear(1 * sizeof(long));
        x->outlet_featureidx = (long *)bach_newptrclear(1 * sizeof(long));
        x->temporalmodes = (long *)bach_newptrclear(1 * sizeof(long));
        x->algorithm_args = (t_llll **)bach_newptrclear(1 * sizeof(t_llll *));
        x->algorithm_args[0] = llll_get();
        
        x->summarization = EARS_ANALYSIS_SUMMARIZATION_MEAN;
        x->summarizationweight = EARS_ANALYSIS_SUMMARIZATIONWEIGHT_RMS;
        x->summarizationpositiveonly = false;
        
        //CQT defaults
        x->CQT_binsPerOctave = 12;
        x->CQT_minPitch = 2400;
        x->CQT_minimumKernelSize = 4;
        x->CQT_numBins = 84;
        x->CQT_threshold = 0.01;
        x->CQT_scale = 1;
        
        // YIN defaults
        x->YIN_minFrequency = 20;
        x->YIN_maxFrequency = 22050;
        x->YIN_tolerance = 1;
        
        // PEAKS defaults
        x->PEAKS_magnitudeThreshold = 0;
        x->PEAKS_maxFrequency = 5000;
        x->PEAKS_minFrequency = 0;
        x->PEAKS_maxPeaks = 100;
        x->PEAKS_orderBy = gensym("frequency");
        
        // Onset detection method
        x->onsetDetectionMethod = gensym("complex");

        // default analysis parameters
        x->a_envattacktime = 10;
        x->a_envreleasetime = 100;  //< Beware: this is different from essentia's default
                                    // But I think that essentia's default was WAY too long.
        
        x->buffer_output_interpolation_mode = 0;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name features @optional 0 @type list
        // @digest Features to be computed
        // @description A list of symbols, each associated with a feature to be computed
        
        t_llll *args = llll_parse(true_ac, argv);
        if (buf_essentia_set_essentia(x, args) != EARS_ERR_NONE) {
            llll_free(args);
            object_free_debug(x); // unlike freeobject(), this works even if the argument is NULL
            return NULL;
        }

        long numoutlets = x->num_outlets;
        
        x->e_ob.l_envtimeunit = EARS_TIMEUNIT_MS; // this is used for the envelope analysis

        // choosing default for analysis parameters
        double framesize = 2048;
        double hopsize = 1024;
        e_ears_timeunit antimeunit = EARS_TIMEUNIT_SAMPS;
        
        bool warn_for_feature_default = false;
        for (long i = 0; i < x->num_features; i++) {
            double this_framesize, this_hopsize;
            e_ears_timeunit this_antimeunit;
            ears_essentia_feature_to_default_framesizes_and_hopsize((t_object *)x, (e_ears_feature)x->features[i], &this_framesize, &this_hopsize, &this_antimeunit);
            if (i == 0) {
                framesize = this_framesize; hopsize = this_hopsize; antimeunit = this_antimeunit;
            } else if (this_framesize != framesize || this_hopsize != hopsize || this_antimeunit != antimeunit) {
                warn_for_feature_default = true;
                framesize = 2048;
                hopsize = 1024;
                antimeunit = EARS_TIMEUNIT_SAMPS;
                break;
            }
        }
        
        x->e_ob.a_framesize = framesize;
        x->e_ob.a_hopsize = hopsize;
        x->e_ob.l_antimeunit = antimeunit;

        for (long c = true_ac; c < argc; c++) {
            t_symbol *thissym = atom_getsym(argv+c);
            if (thissym == gensym("@numframes") || thissym == gensym("@framesize")) {
                warn_for_feature_default = false; // users have set the frame size manually, no need to warn
                break;
            }
        }
        
        if (warn_for_feature_default) {
            object_warn((t_object *)x, "Different essentia have different defaults for analysis parameters.");
            object_warn((t_object *)x, "A global default of 2048/1024 samples is used, consider using separate ears.essentia~ objects if results are not satisfactory.");
        }

        // processing attributes
        attr_args_process(x, argc-true_ac, argv+true_ac);
        
        if (numoutlets >= LLLL_MAX_OUTLETS) {
            object_error((t_object *)x, "Too many outlets!");
            llll_free(args);
            return NULL;
        }

        // creating dummy extractors, just to have descriptions for the outlets
        {
            t_ears_essentia_analysis_params params = buf_essentia_get_default_params(x);
            if (x->extractors_lib.num_extractors > 0)
                ears_essentia_extractors_library_free(&x->extractors_lib);
            ears_essentia_extractors_library_build((t_earsbufobj *)x, x->num_features, x->features, x->temporalmodes, 44100, x->algorithm_args, &x->extractors_lib, &params, true);
            x->must_recreate_extractors = true; //still will have to re-create them
        }
        
        

        char outtypes[LLLL_MAX_OUTLETS];
        for (long o = 0; o < numoutlets; o++) {
            long feature_idx = x->outlet_featureidx[o];
            outtypes[numoutlets-o-1] = (x->temporalmodes[feature_idx] == EARS_ANALYSIS_TEMPORALMODE_BUFFER ? 'E' : '4');
        }
        outtypes[numoutlets] = 0;
        
        char intypes[LLLL_MAX_INLETS];
        intypes[0] = 'E';
        for (long i = 0; i < x->num_features && i < LLLL_MAX_INLETS - 2; i++) {
            intypes[i+1] = '4';
        }
        intypes[CLAMP(x->num_features + 1, 0, LLLL_MAX_INLETS-1)] = 0;
        earsbufobj_setup((t_earsbufobj *)x, intypes, outtypes, NULL);
        llll_free(args);
    }
    return x;
}


void buf_essentia_free(t_buf_essentia *x)
{
    ears_essentia_extractors_library_free(&x->extractors_lib);
    bach_freeptr(x->features);
    bach_freeptr(x->features_numoutputs);
    bach_freeptr(x->outlet_featureoutputidx);
    bach_freeptr(x->outlet_featureidx);
    bach_freeptr(x->temporalmodes);
    for (long i = 0; i < x->num_features; i++)
        llll_free(x->algorithm_args[i]);
    bach_freeptr(x->algorithm_args);
 
    earsbufobj_free((t_earsbufobj *)x);
}




void setExtractorDefaultOptions(essentia::Pool &options) {
    options.set("outputFrames", false);
    options.set("outputFormat", "json");
    options.set("requireMbid", false);
    options.set("indent", 4);
    
    options.set("highlevel.inputFormat", "json");
}


t_ears_essentia_analysis_params buf_essentia_get_default_params(t_buf_essentia *x)
{
    t_ears_essentia_analysis_params params;
    // windowing
    params.framesize_samps = 2048;
    params.hopsize_samps = 1024;
    params.duration_samps = 44100;
    params.windowType = "hann";
    params.windowNormalized = 1;
    params.startFromZero = 0;
    params.lastFrameToEndOfFile = 0;

    params.envelope_attack_time_samps = 441;
    params.envelope_release_time_samps = 4410;
    params.envelope_rectify = 1;
    
    // CQT
    params.CQT_binsPerOctave = 12;
    params.CQT_minFrequency = ears_cents_to_hz(2400, EARS_MIDDLE_A_TUNING);
    params.CQT_numberBins = 84;
    params.CQT_threshold = 0.01;
    params.CQT_minimumKernelSize = 4;
    params.CQT_scale = 1;
    
    params.onsetDetectionMethod = "complex";
    
    params.HPCP_bandPreset = true;
    params.HPCP_bandSplitFrequency = 500;
    params.HPCP_harmonics = 0;
    params.HPCP_maxFrequency = 5000;
    params.HPCP_maxShifted = false;
    params.HPCP_minFrequency = 40;
    params.HPCP_nonLinear = false;
    params.HPCP_normalized = "unitMax";
    params.HPCP_referenceFrequency = EARS_MIDDLE_A_TUNING;
    params.HPCP_size = 12;
    params.HPCP_weightType = "squaredCosine";
    params.HPCP_windowSize = 1;
    
    params.PEAKS_magnitudeThreshold = 0;
    params.PEAKS_maxFrequency = 5000;
    params.PEAKS_minFrequency = 0;
    params.PEAKS_maxPeaks = 100;
    params.PEAKS_orderBy = "frequency";
    
    params.YIN_minFrequency = 20;
    params.YIN_maxFrequency = 22050;
    params.YIN_tolerance = 1;

    
    params.summarization = EARS_ANALYSIS_SUMMARIZATION_MEAN;
    params.summarizationweight = EARS_ANALYSIS_SUMMARIZATIONWEIGHT_RMS;
    params.summarizationpositiveonly = false;

    params.numGriffinLimIterations = 10;
    params.verbose = false;
    return params;
}


t_ears_essentia_analysis_params buf_essentia_get_params(t_buf_essentia *x, t_buffer_obj *buf)
{
    t_ears_essentia_analysis_params params = earsbufobj_get_essentia_analysis_params((t_earsbufobj *)x, buf);

    params.envelope_rectify = 1;
    params.envelope_attack_time_samps = (Real)earsbufobj_time_to_fsamps((t_earsbufobj *)x, x->a_envattacktime, buf, true, false);
    params.envelope_release_time_samps = (Real)earsbufobj_time_to_fsamps((t_earsbufobj *)x, x->a_envreleasetime, buf, true, false);
    
    params.summarization = (e_ears_analysis_summarization) x->summarization;
    params.summarizationweight = (e_ears_analysis_summarizationweight) x->summarizationweight;
    params.summarizationpositiveonly = x->summarizationpositiveonly;

    params.CQT_binsPerOctave = x->CQT_binsPerOctave;
    params.CQT_minFrequency = earsbufobj_pitch_to_hz((t_earsbufobj *)x, x->CQT_minPitch);
    params.CQT_numberBins = x->CQT_binsPerOctave;
    params.CQT_threshold = x->CQT_threshold;
    params.CQT_minimumKernelSize = x->CQT_minimumKernelSize;
    params.CQT_scale = x->CQT_scale;

    params.onsetDetectionMethod = x->onsetDetectionMethod->s_name; // "complex"; // to do: expose this

    // TO DO: Expose HPCP stuff
    params.HPCP_bandPreset = true;
    params.HPCP_bandSplitFrequency = 500;
    params.HPCP_harmonics = 0;
    params.HPCP_maxFrequency = 5000;
    params.HPCP_maxShifted = false;
    params.HPCP_minFrequency = 40;
    params.HPCP_nonLinear = false;
    params.HPCP_normalized = "unitMax";
    params.HPCP_referenceFrequency = EARS_MIDDLE_A_TUNING;
    params.HPCP_size = 12;
    params.HPCP_weightType = "squaredCosine";
    params.HPCP_windowSize = 1;

    params.PEAKS_magnitudeThreshold = earsbufobj_amplitude_to_linear((t_earsbufobj *)x, x->PEAKS_magnitudeThreshold);
    params.PEAKS_maxFrequency = earsbufobj_freq_to_hz((t_earsbufobj *)x, x->PEAKS_maxFrequency);
    params.PEAKS_minFrequency = earsbufobj_freq_to_hz((t_earsbufobj *)x, x->PEAKS_minFrequency);
    params.PEAKS_maxPeaks = x->PEAKS_maxPeaks;
    params.PEAKS_orderBy = x->PEAKS_orderBy->s_name;

    params.YIN_minFrequency = earsbufobj_freq_to_hz((t_earsbufobj *)x, x->YIN_minFrequency);
    params.YIN_maxFrequency = earsbufobj_freq_to_hz((t_earsbufobj *)x, x->YIN_maxFrequency);
    params.YIN_tolerance = x->YIN_tolerance;
    params.verbose = true;
    return params;
}

void buf_essentia_bang(t_buf_essentia *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    for (long i = 0; i < x->num_features; i++) {
        if (x->temporalmodes[i] == EARS_ANALYSIS_TEMPORALMODE_BUFFER)
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, earsbufobj_outlet_to_bufstore((t_earsbufobj *)x, i), num_buffers, true);
    }
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);

    Pool options;
    setExtractorDefaultOptions(options);
    
    t_llll **res = (t_llll **)bach_newptr(x->num_outlets * sizeof(t_llll *));
    t_buffer_obj **res_buf = (t_buffer_obj **)bach_newptr(x->num_outlets * sizeof(t_buffer_obj *));
    for (long i = 0; i < x->num_outlets; i++)
        res[i] = llll_get();

    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        long o = 0;
        for (long i = 0; i < x->num_features; i++) {
            if (x->temporalmodes[i] == EARS_ANALYSIS_TEMPORALMODE_BUFFER) {
                for (long t = 0; t < x->features_numoutputs[i]; t++) {
                    res_buf[o] = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, earsbufobj_outlet_to_bufstore((t_earsbufobj *)x, o), count);
                    o++;
                }
            } else {
                for (long t = 0; t < x->features_numoutputs[i]; t++)
                    res_buf[o++] = NULL;
            }
        }

        double sr = ears_buffer_get_sr((t_object *)x, in);
        if (sr != x->curr_sr) {
            x->must_recreate_extractors = true;
            x->curr_sr = sr;
        }

        t_ears_essentia_analysis_params params = buf_essentia_get_params(x, in);
        
//        if (x->must_recreate_extractors) { // potentially we may need to do this all the time, as parameters may also depend on the buffers
            if (x->extractors_lib.num_extractors > 0)
                ears_essentia_extractors_library_free(&x->extractors_lib);
            ears_essentia_extractors_library_build((t_earsbufobj *)x, x->num_features, x->features, x->temporalmodes, sr, x->algorithm_args, &x->extractors_lib, &params);
//            x->must_recreate_extractors = false;
//        }
    
        // set output buffer for writing
        for (long o = 0; o < x->num_outlets; o++) {
            long feat_idx = x->outlet_featureidx[o];
            long feat_idxout = x->outlet_featureoutputidx[o];
            long map = x->extractors_lib.extractors[feat_idx].output_map[feat_idxout];
            if (map >= 0)
                x->extractors_lib.extractors[feat_idx].result_buf[map] = res_buf[o];
        }
        
        // compute descriptors
        ears_essentia_extractors_library_compute((t_earsbufobj *)x, in, &x->extractors_lib, &params, x->buffer_output_interpolation_mode);
        
        // get the llll result, if any
        for (o = 0; o < x->num_outlets; o++) {
            long feat_idx = x->outlet_featureidx[o];
            long feat_outidx = x->outlet_featureoutputidx[o];
            long map = x->extractors_lib.extractors[feat_idx].output_map[feat_outidx];
            if (map >= 0 && x->extractors_lib.extractors[feat_idx].result[map]) {
                if (x->extractors_lib.extractors[feat_idx].result[map]->l_size == 1 &&
                    x->extractors_lib.extractors[feat_idx].result[map]->l_depth == 1 &&
                    !x->extractors_lib.extractors[feat_idx].keep_singleton_as_lists[map])
                    llll_chain_clone(res[o], x->extractors_lib.extractors[feat_idx].result[map]);
                else
                    llll_appendllll_clone(res[o], x->extractors_lib.extractors[feat_idx].result[map]);
            }
        }
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    for (long o = x->num_outlets - 1; o >= 0; o--) {
        long feat_idx = x->outlet_featureidx[o];
        if (x->temporalmodes[feat_idx] == EARS_ANALYSIS_TEMPORALMODE_BUFFER)
            earsbufobj_outlet_buffer((t_earsbufobj *)x, o);
        else
            earsbufobj_outlet_llll((t_earsbufobj *)x, o, res[o]);
    }
    
    for (long i = 0; i < x->num_features; i++)
        llll_free(res[i]);
    bach_freeptr(res);
    bach_freeptr(res_buf);
}


void buf_essentia_anything(t_buf_essentia *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long num_bufs = llll_get_num_symbols_root(parsed);

            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_essentia_bang(x);
            
//        } else if (inlet == 1) {
//            buf_essentia_set_essentia(x, parsed);
        } else {
            // new arguments for specific feature

            earsbufobj_mutex_lock((t_earsbufobj *)x);
            
            long featidx = inlet - 1;
            if (x->algorithm_args[featidx]) {
                if (parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
                    t_symbol *argname = hatom_getsym(&parsed->l_head->l_hatom);
                    
                    if (argname && argname->s_name) {
                        
                        bool found = false;
                        for (t_llllelem *el = x->algorithm_args[featidx]->l_head; el; el = el->l_next) {
                            if (hatom_gettype(&el->l_hatom) == H_SYM) {
                                t_symbol *this_sym = hatom_getsym(&el->l_hatom);
                                if (this_sym && this_sym->s_name && this_sym->s_name[0] == '@' &&
                                    (strcmp(this_sym->s_name+1, argname->s_name) == 0 || strcmp(this_sym->s_name, argname->s_name) == 0)) {
                                    found = true;
                                    // finding next attribute
                                    t_llllelem *temp = el->l_next;
                                    while (temp) {
                                        t_llllelem *next = temp->l_next;
                                        if (hatom_gettype(&el->l_hatom) == H_SYM) {
                                            t_symbol *this_other_sym = hatom_getsym(&temp->l_hatom);
                                            if (this_other_sym && this_other_sym->s_name && this_other_sym->s_name[0] == '@') {
                                                break;
                                            }
                                        }
                                        llll_destroyelem(temp);
                                        temp = next;
                                    }
                                    for (t_llllelem *insertel = parsed->l_tail; insertel && insertel != parsed->l_head; insertel = insertel->l_prev) {
                                        llll_inserthatom_after_clone(&insertel->l_hatom, el);
                                    }
                                }
                            }
                        }
                        
                        if (!found) {
                            char temp[2000];
                            snprintf_zero(temp, 2000, "@%s", argname->s_name);
                            llll_appendsym(x->algorithm_args[featidx], gensym(temp));
                            for (t_llllelem *insertel = parsed->l_head->l_next; insertel; insertel = insertel->l_next) {
                                llll_appendhatom_clone(x->algorithm_args[featidx], &insertel->l_hatom);
                            }
                        }
                        
                    }
                }
            }
            
            earsbufobj_mutex_unlock((t_earsbufobj *)x);

        }
    }
    llll_free(parsed);
}




