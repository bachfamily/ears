/**
	@file
	ears.features.c
 
	@name
	ears.features~
 
	@realname
	ears.features~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Extract buffer features
 
	@description
	Perform descriptor analysis on the incoming buffer via the Essentia library
 
	@discussion
 
	@category
	ears distorsion
 
	@keywords
	buffer, features, feature, descriptor
 
	@seealso
	ears.spectrogram~
	
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

typedef struct _buf_features {
    t_earsbufobj       e_ob;
    
    long               num_features;
    long               *features;
    long               *temporalmodes;
    t_llll             **algorithm_args;
    t_ears_essentia_extractors_library   extractors_lib;
    
    char               must_recreate_extractors;
    double             curr_sr;
    
    char               summarization;
    
    double             a_envattacktime;
    double             a_envreleasetime;

    long                buffer_output_interpolation_mode;
} t_buf_features;



// Prototypes
t_buf_features*         buf_features_new(t_symbol *s, short argc, t_atom *argv);
void			buf_features_free(t_buf_features *x);
void			buf_features_bang(t_buf_features *x);
void			buf_features_anything(t_buf_features *x, t_symbol *msg, long ac, t_atom *av);

void buf_features_assist(t_buf_features *x, void *b, long m, long a, char *s);
void buf_features_inletinfo(t_buf_features *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(features)


/**********************************************************************/
// Class Definition and Life Cycle




t_max_err buf_features_notify(t_buf_features *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.features~",
                         (method)buf_features_new,
                         (method)buf_features_free,
                         sizeof(t_buf_features),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a features threshold (depending on the <m>ampunit</m>) or
    // an envelope (also see <m>envampunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(features)
    
    
    class_addmethod(c, (method)buf_features_notify,        "bachnotify",        A_CANT,        0);

    // @method number @digest Set features
    // @description A number in the second inlet sets the features parameter (depending on the <m>ampunit</m>).

    llllobj_class_add_out_attr(c, LLLL_OBJ_VANILLA);

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_antimeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    
    earsbufobj_class_add_winsize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);
    earsbufobj_class_add_numframes_attr(c);
    earsbufobj_class_add_overlap_attr(c);
    earsbufobj_class_add_wintype_attr(c);
    earsbufobj_class_add_winstartfromzero_attr(c);


    CLASS_ATTR_DOUBLE(c, "envattack", 0, t_buf_features, a_envattacktime);
    CLASS_ATTR_STYLE_LABEL(c,"envattack",0,"text","Envelope Attack Time");
    CLASS_ATTR_CATEGORY(c, "envattack", 0, "Envelopes");
    // @description Sets the attack time for computing envelopes (the unit depends on the <m>envtimeunit</m> attribute)
    // Floating point values are allowed.

    CLASS_ATTR_DOUBLE(c, "envrelease", 0, t_buf_features, a_envreleasetime);
    CLASS_ATTR_STYLE_LABEL(c,"envrelease",0,"text","Envelope Release Time");
    CLASS_ATTR_CATEGORY(c, "envrelease", 0, "Envelopes");
    // @description Sets the attack time for computing envelopes (the unit depends on the <m>envtimeunit</m> attribute)
    // Floating point values are allowed.

    
    CLASS_ATTR_LONG(c, "bufinterp", 0, t_buf_features, buffer_output_interpolation_mode);
    CLASS_ATTR_STYLE_LABEL(c,"bufinterp",0,"text","Output Buffer Frame Interpolation Mode");
    CLASS_ATTR_ENUMINDEX(c, "bufinterp", 0, "Don't Resample Lower Neighbor Linear")
    CLASS_ATTR_BASIC(c, "bufinterp", 0);
    CLASS_ATTR_FILTER_CLIP(c, "bufinterp", 0, 2);
    // @description Sets the interpolation mode for output buffers: <br />
    // 0 (don't resample, default): the output buffer has one sample per analysis frame, and its sample rate will be changed accordingly; <br />
    // 1 (lower neighbor): the output buffer is resampled to the same samplerate as the input buffer but the samples are not interpolated; <br />
    // 2 (linear): same as the previous one, and the samples are interpolated linearly.

    
    CLASS_ATTR_LONG(c, "summary", 0, t_buf_features, summarization);
    CLASS_ATTR_STYLE_LABEL(c,"summary",0,"text","Output Buffer Frame Interpolation Mode");
    CLASS_ATTR_ENUMINDEX(c, "summary", 0, "First Last Middle Mean Loudness-weighted Mean")
    CLASS_ATTR_BASIC(c, "summary", 0);
    CLASS_ATTR_FILTER_CLIP(c, "summary", 0, 6);
    // @description Sets the summarization mode, for features that are requested as static but needs to be computed on a frame-by-frame basis.
    // Available modes are:
    // <b>First</b>: take first frame; <br />
    // <b>Last</b>: last last frame; <br />
    // <b>Middle</b>: take middle frame; <br />
    // <b>Mean</b>: average through frames; <br />
    // <b>Loudness-weighted Mean</b>: average through frames, weighting by frame loudness (default)<br />


    
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}


const char *ears_features_feature_to_description(e_ears_feature feature)
{
    switch (feature) {
            
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
            
            
            
        case EARS_FEATURE_DURATION:
            return "Duration";
            break;

            


            
        case EARS_FEATURE_SPECTRALFLATNESS:
            return "Spectral flatness";
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

        default:
            return "Unknown feature";
            break;
    }
}

e_ears_feature ears_features_feature_from_symbol(t_symbol *s, long *temporalmode, t_ears_err *err)
{
    long tm = EARS_ESSENTIA_TEMPORALMODE_WHOLE;
    if (ears_symbol_ends_with(s, "...", false)) {
        char buf[2048];
        snprintf_zero(buf, 2048, "%s", s->s_name);
        buf[strlen(s->s_name)-3] = 0;
        s = gensym(buf);
        tm = EARS_ESSENTIA_TEMPORALMODE_TIMESERIES;
    } else if (ears_symbol_ends_with(s, "~", false)) {
        char buf[2048];
        snprintf_zero(buf, 2048, "%s", s->s_name);
        buf[strlen(s->s_name)-1] = 0;
        s = gensym(buf);
        tm = EARS_ESSENTIA_TEMPORALMODE_BUFFER;
    } else {
        tm = EARS_ESSENTIA_TEMPORALMODE_WHOLE;
    }
    
    if (err)
        *err = EARS_ERR_NONE;

    *temporalmode = tm;

    if (s == gensym("spectrum")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_BUFFER)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_SPECTRUM;
    }

    if (s == gensym("powerspectrum")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_BUFFER)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_POWERSPECTRUM;
    }

    
    if (s == gensym("envelope")) {
        if (tm == EARS_ESSENTIA_TEMPORALMODE_WHOLE)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_ENVELOPE;
    }
    if (s == gensym("logattacktime")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_WHOLE)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_LOGATTACKTIME;
    }

    if (s == gensym("envmaxtime")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_WHOLE)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_ENVMAXTIME;
    }
    if (s == gensym("envmintime")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_WHOLE)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_ENVMINTIME;
    }
    if (s == gensym("strongdecay")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_WHOLE)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_STRONGDECAY;
    }
    if (s == gensym("temporalcentroid")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_WHOLE)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_TEMPORALCENTROID;
    }
    if (s == gensym("duration")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_WHOLE)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_DURATION;
    }

    
    // Standard
    if (s == gensym("derivative")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_BUFFER)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_DERIVATIVE;
    }
    if (s == gensym("min")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_WHOLE)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_MIN;
    }
    if (s == gensym("max")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_WHOLE)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_MAX;
    }
    if (s == gensym("welch")) {
        return EARS_FEATURE_WELCH;
    }
    

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

    
    if (s == gensym("beattrackerdegara")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_WHOLE)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_BEATTRACKERDEGARA;
    }
    if (s == gensym("beattrackermultifeature")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_WHOLE)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_BEATTRACKERMULTIFEATURE;
    }
    if (s == gensym("beatsloudness")) {
        if (tm != EARS_ESSENTIA_TEMPORALMODE_WHOLE)
            *err = EARS_ERR_INVALID_MODE;
        return EARS_FEATURE_BEATSLOUDNESS;
    }

    if (s == gensym("danceability"))
        return EARS_FEATURE_DANCEABILITY;

    if (s == gensym("loopbmpestimator"))
        return EARS_FEATURE_LOOPBPMESTIMATOR;
    
    if (s == gensym("onsetdetection"))
        return EARS_FEATURE_ONSETDETECTION;

    
    
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
    


    return EARS_FEATURE_UNKNOWN;
}


void buf_features_assist(t_buf_features *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        if (a < x->num_features) {  // @out 0 @type symbol/list @digest Output buffer names
            if (x->temporalmodes[a] == EARS_ESSENTIA_TEMPORALMODE_WHOLE)
                sprintf(s, "llll: %s (static)", ears_features_feature_to_description((e_ears_feature)x->features[a]));
            else if (x->temporalmodes[a] == EARS_ESSENTIA_TEMPORALMODE_TIMESERIES)
                sprintf(s, "llll: %s (time series)", ears_features_feature_to_description((e_ears_feature)x->features[a]));
            else if (x->temporalmodes[a] == EARS_ESSENTIA_TEMPORALMODE_BUFFER)
                sprintf(s, "buffer: %s", ears_features_feature_to_description((e_ears_feature)x->features[a]));
        } else {
            sprintf(s, "Unused outlet");
        }
        // @out 0 @loop 1 @type llll/buffer @digest Feature for the corresponding key
    }
}

void buf_features_inletinfo(t_buf_features *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_ears_err buf_features_set_features(t_buf_features *x, t_llll *args)
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
    x->temporalmodes = (long *)bach_resizeptr(x->temporalmodes, new_num_features * sizeof(long));
    x->algorithm_args = (t_llll **)bach_resizeptr(x->algorithm_args, new_num_features * sizeof(t_llll *));
    for (long i = 0; i < new_num_features; i++)
        x->algorithm_args[i] = llll_get();

    long i = 0;
    t_ears_err temporalmode_err = EARS_ERR_NONE;
    for (t_llllelem *el = args->l_head; el; el = el->l_next, i++) {
        x->features[i] = EARS_FEATURE_UNKNOWN;
        x->temporalmodes[i] = EARS_ESSENTIA_TEMPORALMODE_WHOLE;
        if (hatom_gettype(&el->l_hatom) == H_SYM) {
            e_ears_feature feat = ears_features_feature_from_symbol(hatom_getsym(&el->l_hatom), &x->temporalmodes[i], &temporalmode_err);
            x->features[i] = feat;
            if (feat == EARS_FEATURE_UNKNOWN) {
                object_error((t_object *)x, "Unknown feature at index %d", i+1);
                err = EARS_ERR_GENERIC;
            } else if (temporalmode_err == EARS_ERR_INVALID_MODE) {
                object_error((t_object *)x, "Unsupported temporal mode for feature '%s'", ears_features_feature_to_description(feat));
                err = EARS_ERR_GENERIC;
            }
        } else if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            t_llll *subll = hatom_getllll(&el->l_hatom);
            if (subll && subll->l_head && hatom_gettype(&subll->l_head->l_hatom) == H_SYM) {
                e_ears_feature feat = ears_features_feature_from_symbol(hatom_getsym(&subll->l_head->l_hatom), &x->temporalmodes[i], &temporalmode_err);
                x->features[i] = feat;
                if (feat == EARS_FEATURE_UNKNOWN) {
                    object_error((t_object *)x, "Unknown feature at index %d", i+1);
                    err = EARS_ERR_GENERIC;
                } else if (temporalmode_err == EARS_ERR_INVALID_MODE) {
                    object_error((t_object *)x, "Unsupported temporal mode for feature '%s'", ears_features_feature_to_description(feat));
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

t_buf_features *buf_features_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_features *x;
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
    
    
    x = (t_buf_features*)object_alloc_debug(s_tag_class);
    if (x) {
        x->num_features = 1;
        x->features = (long *)bach_newptrclear(1 * sizeof(long));
        x->temporalmodes = (long *)bach_newptrclear(1 * sizeof(long));
        x->algorithm_args = (t_llll **)bach_newptrclear(1 * sizeof(t_llll *));
        x->algorithm_args[0] = llll_get();
        
        x->summarization = EARS_ESSENTIA_SUMMARIZATION_LOUDNESSWEIGHTEDMEAN;

        // default analysis parameters
        x->a_envattacktime = 10;
        x->a_envreleasetime = 100;  //< Beware: this is different from features's default
                                    // But I think that features's default was WAY too long.
        
        x->buffer_output_interpolation_mode = 0;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name features @optional 0 @type list
        // @digest Features to be computed
        // @description A list of symbols, each associated with a feature to be computed
        
        t_llll *args = llll_parse(true_ac, argv);
        if (buf_features_set_features(x, args) != EARS_ERR_NONE) {
            llll_free(args);
            return NULL;
        }
        
        x->e_ob.l_envtimeunit = EARS_TIMEUNIT_MS; // this is used for the envelope analysis
        
        attr_args_process(x, argc-true_ac, argv+true_ac);
        
        char outtypes[LLLL_MAX_OUTLETS];
        for (long i = 0; i < x->num_features; i++)
            outtypes[x->num_features-i-1] = (x->temporalmodes[i] == EARS_ESSENTIA_TEMPORALMODE_BUFFER ? 'E' : '4');
        outtypes[x->num_features] = 0;
        earsbufobj_setup((t_earsbufobj *)x, "E", outtypes, NULL);
        llll_free(args);
    }
    return x;
}


void buf_features_free(t_buf_features *x)
{
    ears_essentia_extractors_library_free(&x->extractors_lib);
    bach_freeptr(x->features);
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


t_ears_essentia_analysis_params buf_features_get_params(t_buf_features *x, t_buffer_obj *buf)
{
    t_ears_essentia_analysis_params params = earsbufobj_get_essentia_analysis_params((t_earsbufobj *)x, buf);

    params.envelope_rectify = 1;
    params.envelope_attack_time_samps = (Real)earsbufobj_time_to_fsamps((t_earsbufobj *)x, x->a_envattacktime, buf, true, false);
    params.envelope_release_time_samps = (Real)earsbufobj_time_to_fsamps((t_earsbufobj *)x, x->a_envreleasetime, buf, true, false);
    
    params.summarization = (e_ears_essentia_summarization) x->summarization;
    return params;
}

void buf_features_bang(t_buf_features *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    for (long i = 0; i < x->num_features; i++) {
        if (x->temporalmodes[i] == EARS_ESSENTIA_TEMPORALMODE_BUFFER)
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, earsbufobj_outlet_to_bufoutlet((t_earsbufobj *)x, i), num_buffers, true);
    }
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);

    Pool options;
    setExtractorDefaultOptions(options);
    
    t_llll **res = (t_llll **)bach_newptr(x->num_features * sizeof(t_llll *));
    t_buffer_obj **res_buf = (t_buffer_obj **)bach_newptr(x->num_features * sizeof(t_buffer_obj *));
    for (long i = 0; i < x->num_features; i++)
        res[i] = llll_get();

    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        for (long i = 0; i < x->num_features; i++) {
            if (x->temporalmodes[i] == EARS_ESSENTIA_TEMPORALMODE_BUFFER)
                res_buf[i] = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, earsbufobj_outlet_to_bufoutlet((t_earsbufobj *)x, i), count);
            else
                res_buf[i] = NULL;
        }

        double sr = ears_buffer_get_sr((t_object *)x, in);
        if (sr != x->curr_sr) {
            x->must_recreate_extractors = true;
            x->curr_sr = sr;
        }

        t_ears_essentia_analysis_params params = buf_features_get_params(x, in);
        
        if (x->must_recreate_extractors) {
            if (x->extractors_lib.num_extractors > 0)
                ears_essentia_extractors_library_free(&x->extractors_lib);
            ears_essentia_extractors_library_build((t_earsbufobj *)x, x->num_features, x->features, x->temporalmodes, sr, x->algorithm_args, &x->extractors_lib, &params);
            x->must_recreate_extractors = false;
        }
    
        // set output buffer for writing
        for (long i = 0; i < MIN(x->extractors_lib.num_extractors, x->num_features); i++) {
            x->extractors_lib.extractors[i].result_buf[0] = res_buf[i];
            // TO DO: interface for multi-outlets
        }
        
        // compute descriptors
        ears_essentia_extractors_library_compute((t_earsbufobj *)x, in, &x->extractors_lib, &params, x->buffer_output_interpolation_mode);
        
        // get the lll result, if any
        for (long i = 0; i < MIN(x->extractors_lib.num_extractors, x->num_features); i++) {
            if (x->extractors_lib.extractors[i].result[0])
                llll_chain_clone(res[i], x->extractors_lib.extractors[i].result[0]);
            // TO DO: interface for multi-outlets
        }
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    for (long i = 0; i < x->num_features; i++) {
        if (x->temporalmodes[i] == EARS_ESSENTIA_TEMPORALMODE_BUFFER)
            earsbufobj_outlet_buffer((t_earsbufobj *)x, i);
        else
            earsbufobj_outlet_llll((t_earsbufobj *)x, i, res[i]);
    }
    
    for (long i = 0; i < x->num_features; i++)
        llll_free(res[i]);
    bach_freeptr(res);
    bach_freeptr(res_buf);
}


void buf_features_anything(t_buf_features *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_features_bang(x);
            
//        } else if (inlet == 1) {
//            buf_features_set_features(x, parsed);
        }
    }
    llll_free(parsed);
}



