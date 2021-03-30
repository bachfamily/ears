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
	Extract buffer features
 
	@description
	Perform descriptor analysis on the incoming buffer via the Essentia library
 
	@discussion
 
	@category
	ears distorsion
 
	@keywords
	buffer, essentia, feature, descriptor
 
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

typedef struct _buf_essentia {
    t_earsbufobj       e_ob;
    
    long               num_features;
    long               *features;
    long               *temporalmodes;
    t_llll             **algorithm_args;
    t_ears_essentia_extractors_library   extractors_lib;
    
    char               must_recreate_extractors;
    double             curr_sr;
    
    double             a_winsize;
    double             a_hopsize;
    t_symbol           *a_wintype;
    double             a_envattacktime;
    double             a_envreleasetime;

    long                buffer_output_interpolation_order;
} t_buf_essentia;



// Prototypes
t_buf_essentia*         buf_essentia_new(t_symbol *s, short argc, t_atom *argv);
void			buf_essentia_free(t_buf_essentia *x);
void			buf_essentia_bang(t_buf_essentia *x);
void			buf_essentia_anything(t_buf_essentia *x, t_symbol *msg, long ac, t_atom *av);

void buf_essentia_assist(t_buf_essentia *x, void *b, long m, long a, char *s);
void buf_essentia_inletinfo(t_buf_essentia *x, void *b, long a, char *t);


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
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(essentia)
    
    
    class_addmethod(c, (method)buf_essentia_notify,        "bachnotify",        A_CANT,        0);

    // @method number @digest Set essentia
    // @description A number in the second inlet sets the essentia parameter (depending on the <m>ampunit</m>).

    llllobj_class_add_out_attr(c, LLLL_OBJ_VANILLA);

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    

    CLASS_ATTR_DOUBLE(c, "winsize", 0, t_buf_essentia, a_winsize);
    CLASS_ATTR_STYLE_LABEL(c,"winsize",0,"text","Windows Size");
    CLASS_ATTR_BASIC(c, "winsize", 0);
    CLASS_ATTR_CATEGORY(c, "winsize", 0, "Analysis");
    // @description Sets the analysis frame size (the unit depends on the <m>timeunit</m> attribute)

    
    CLASS_ATTR_DOUBLE(c, "hopsize", 0, t_buf_essentia, a_hopsize);
    CLASS_ATTR_STYLE_LABEL(c,"hopsize",0,"text","Hop Size");
    CLASS_ATTR_BASIC(c, "hopsize", 0);
    CLASS_ATTR_CATEGORY(c, "hopsize", 0, "Analysis");
    // @description Sets the analysis hop size (the unit depends on the <m>timeunit</m> attribute)
    // Floating point values are allowed.

    CLASS_ATTR_SYM(c, "wintype", 0, t_buf_essentia, a_wintype);
    CLASS_ATTR_STYLE_LABEL(c,"wintype",0,"text","Window Type");
    CLASS_ATTR_ENUM(c,"wintype", 0, "hamming hann hannnsgcq triangular square blackmanharris62 blackmanharris70 blackmanharris74 blackmanharris92");
    CLASS_ATTR_BASIC(c, "wintype", 0);
    CLASS_ATTR_CATEGORY(c, "wintype", 0, "Analysis");
    // @description Sets the window type.
    // Available windows are the ones allowed by the Essentia library:
    // "hamming", "hann", "hannnsgcq", "triangular", "square", "blackmanharris62", "blackmanharris70", "blackmanharris74", "blackmanharris92"

    CLASS_ATTR_DOUBLE(c, "envattack", 0, t_buf_essentia, a_envattacktime);
    CLASS_ATTR_STYLE_LABEL(c,"envattack",0,"text","Envelope Attack Time");
    CLASS_ATTR_CATEGORY(c, "envattack", 0, "Analysis");
    // @description Sets the attack time for computing envelopes (the unit depends on the <m>timeunit</m> attribute)
    // Floating point values are allowed.

    CLASS_ATTR_DOUBLE(c, "envrelease", 0, t_buf_essentia, a_envreleasetime);
    CLASS_ATTR_STYLE_LABEL(c,"envrelease",0,"text","Envelope Release Time");
    CLASS_ATTR_CATEGORY(c, "envrelease", 0, "Analysis");
    // @description Sets the attack time for computing envelopes (the unit depends on the <m>timeunit</m> attribute)
    // Floating point values are allowed.

    
    CLASS_ATTR_LONG(c, "bufinterp", 0, t_buf_essentia, buffer_output_interpolation_order);
    CLASS_ATTR_STYLE_LABEL(c,"bufinterp",0,"text","Output Buffer Interpolation Order");
    CLASS_ATTR_FILTER_CLIP(c, "bufinterp", 0, 1);
    // @description Sets the interpolation order for output buffers.
    
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}


const char *ears_essentia_feature_to_description(e_ears_essentia_feature feature)
{
    switch (feature) {
        case EARS_ESSENTIA_FEATURE_ENVELOPE:
            return "Envelope";
            break;

        case EARS_ESSENTIA_FEATURE_LOGATTACKTIME:
            return "Logarithmic (base-10) attack time";
            break;
            
        case EARS_ESSENTIA_FEATURE_MAXTOTOTAL:
            return "Relative position of the envelope maximum";
            break;

        case EARS_ESSENTIA_FEATURE_MINTOTOTAL:
            return "Relative position of the envelope minimum";
            break;

        case EARS_ESSENTIA_FEATURE_STRONGDECAY:
            return "Strong decay";
            break;

        case EARS_ESSENTIA_FEATURE_TCTOTOTAL:
            return "Relative position of temporal centroid";
            break;
            
        case EARS_ESSENTIA_FEATURE_TEMPORALCENTROID:
            return "Temporal centroid";
            break;
            
        case EARS_ESSENTIA_FEATURE_DURATION:
            return "Duration (seconds)";
            break;

            
        case EARS_ESSENTIA_FEATURE_SPECTRALMOMENTS:
            return "Spectral moments";
            break;

        case EARS_ESSENTIA_FEATURE_SPECTRALCENTROIDTIME:
            return "Spectral centroid via time representation";
            break;

        case EARS_ESSENTIA_FEATURE_SPECTRALCENTROID:
            return "Spectral centroid";
            break;
            
        case EARS_ESSENTIA_FEATURE_SPECTRALSPREAD:
            return "Spectral spread";
            break;
            
        case EARS_ESSENTIA_FEATURE_SPECTRALSKEWNESS:
            return "Spectral skewness";
            break;
            
        case EARS_ESSENTIA_FEATURE_SPECTRALKURTOSIS:
            return "Spectral kurtosis";
            break;
            

            
        case EARS_ESSENTIA_FEATURE_SPECTRALFLATNESS:
            return "Spectral flatness";
            break;

            
            
        case EARS_ESSENTIA_FEATURE_FLUX:
            return "Flux";
            break;

            
        case EARS_ESSENTIA_FEATURE_ZEROCROSSINGRATE:
            return "Zero-crossing rate";
            break;
            
        case EARS_ESSENTIA_FEATURE_ENERGYBAND:
            return "Energy band";
            break;

        case EARS_ESSENTIA_FEATURE_ENERGYBANDRATIO:
            return "Energy band ratio";
            break;
            
        case EARS_ESSENTIA_FEATURE_MFCC:
            return "MFCC";
            break;

        case EARS_ESSENTIA_FEATURE_BFCC:
            return "BFCC";
            break;

        case EARS_ESSENTIA_FEATURE_BARKBANDS:
            return "Bark bands";
            break;

        case EARS_ESSENTIA_FEATURE_ERBBANDS:
            return "ERB bands";
            break;

        case EARS_ESSENTIA_FEATURE_FREQUENCYBANDS:
            return "Frequency bands";
            break;
            
        default:
            return "Unknown feature";
            break;
    }
}

e_ears_essentia_feature ears_essentia_feature_from_symbol(t_symbol *s, long *temporalmode)
{
    if (ears_symbol_ends_with(s, "...", false)) {
        char buf[2048];
        snprintf_zero(buf, 2048, "%s", s->s_name);
        buf[strlen(s->s_name)-3] = 0;
        s = gensym(buf);
        *temporalmode = EARS_ESSENTIA_TEMPORALMODE_TIMESERIES;
    } else if (ears_symbol_ends_with(s, "~", false)) {
        char buf[2048];
        snprintf_zero(buf, 2048, "%s", s->s_name);
        buf[strlen(s->s_name)-1] = 0;
        s = gensym(buf);
        *temporalmode = EARS_ESSENTIA_TEMPORALMODE_BUFFER;
    } else {
        *temporalmode = EARS_ESSENTIA_TEMPORALMODE_WHOLE;
    }
    
    if (s == gensym("envelope"))
        return EARS_ESSENTIA_FEATURE_ENVELOPE;
    if (s == gensym("logattacktime"))
        return EARS_ESSENTIA_FEATURE_LOGATTACKTIME;
    if (s == gensym("maxtototal"))
        return EARS_ESSENTIA_FEATURE_MAXTOTOTAL;
    if (s == gensym("mintototal"))
        return EARS_ESSENTIA_FEATURE_MINTOTOTAL;
    if (s == gensym("strongdecay"))
        return EARS_ESSENTIA_FEATURE_STRONGDECAY;
    if (s == gensym("tctototal") || s == gensym("relativetemporalcentroid"))
        return EARS_ESSENTIA_FEATURE_TCTOTOTAL;
    if (s == gensym("temporalcentroid"))
        return EARS_ESSENTIA_FEATURE_TEMPORALCENTROID;
    if (s == gensym("duration"))
        return EARS_ESSENTIA_FEATURE_DURATION;

    
    if (s == gensym("spectralmoments"))
        return EARS_ESSENTIA_FEATURE_SPECTRALMOMENTS;
    if (s == gensym("spectralcentroidtime"))
        return EARS_ESSENTIA_FEATURE_SPECTRALCENTROIDTIME;
    if (s == gensym("centroid") || s == gensym("spectralcentroid"))
        return EARS_ESSENTIA_FEATURE_SPECTRALCENTROID;
    if (s == gensym("spread") || s == gensym("spectralspread"))
        return EARS_ESSENTIA_FEATURE_SPECTRALSPREAD;
    if (s == gensym("skewness") || s == gensym("spectralskewness"))
        return EARS_ESSENTIA_FEATURE_SPECTRALSKEWNESS;
    if (s == gensym("kurtosis") || s == gensym("spectralkurtosis"))
        return EARS_ESSENTIA_FEATURE_SPECTRALKURTOSIS;
    

    if (s == gensym("flatness") || s == gensym("spectralflatness"))
        return EARS_ESSENTIA_FEATURE_SPECTRALFLATNESS;
    if (s == gensym("flux"))
        return EARS_ESSENTIA_FEATURE_FLUX;

    
    if (s == gensym("zerocrossingrate"))
        return EARS_ESSENTIA_FEATURE_ZEROCROSSINGRATE;
    if (s == gensym("energyband"))
        return EARS_ESSENTIA_FEATURE_ENERGYBAND;
    if (s == gensym("energybandratio"))
        return EARS_ESSENTIA_FEATURE_ENERGYBANDRATIO;
    if (s == gensym("mfcc"))
        return EARS_ESSENTIA_FEATURE_MFCC;
    if (s == gensym("bfcc"))
        return EARS_ESSENTIA_FEATURE_BFCC;
    if (s == gensym("barkbands"))
        return EARS_ESSENTIA_FEATURE_BARKBANDS;
    if (s == gensym("erbbands"))
        return EARS_ESSENTIA_FEATURE_ERBBANDS;
    if (s == gensym("frequencybands"))
        return EARS_ESSENTIA_FEATURE_FREQUENCYBANDS;
    return EARS_ESSENTIA_FEATURE_UNKNOWN;
}


void buf_essentia_assist(t_buf_essentia *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        if (a < x->num_features) {  // @out 0 @type symbol/list @digest Output buffer names
            if (x->temporalmodes[a] == EARS_ESSENTIA_TEMPORALMODE_WHOLE)
                sprintf(s, "llll: %s (static)", ears_essentia_feature_to_description((e_ears_essentia_feature)x->features[a]));
            else if (x->temporalmodes[a] == EARS_ESSENTIA_TEMPORALMODE_TIMESERIES)
                sprintf(s, "llll: %s (time series)", ears_essentia_feature_to_description((e_ears_essentia_feature)x->features[a]));
            else if (x->temporalmodes[a] == EARS_ESSENTIA_TEMPORALMODE_BUFFER)
                sprintf(s, "buffer: %s", ears_essentia_feature_to_description((e_ears_essentia_feature)x->features[a]));
        } else {
            sprintf(s, "Unused outlet");
        }
        // @out 0 @loop 1 @type llll/buffer @digest Feature for the corresponding key
    }
}

void buf_essentia_inletinfo(t_buf_essentia *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


void buf_essentia_set_features(t_buf_essentia *x, t_llll *args)
{
    if (!args) {
        object_error((t_object *)x, "Wrong arguments!");
        return;
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
    for (t_llllelem *el = args->l_head; el; el = el->l_next, i++) {
        x->features[i] = EARS_ESSENTIA_FEATURE_UNKNOWN;
        x->temporalmodes[i] = EARS_ESSENTIA_TEMPORALMODE_WHOLE;
        if (hatom_gettype(&el->l_hatom) == H_SYM) {
            e_ears_essentia_feature feat = ears_essentia_feature_from_symbol(hatom_getsym(&el->l_hatom), &x->temporalmodes[i]);
            x->features[i] = feat;
            if (feat == EARS_ESSENTIA_FEATURE_UNKNOWN)
                object_error((t_object *)x, "Unknown feature at index %d", i+1);
        } else if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            t_llll *subll = hatom_getllll(&el->l_hatom);
            if (subll && subll->l_head && hatom_gettype(&subll->l_head->l_hatom) == H_SYM) {
                e_ears_essentia_feature feat = ears_essentia_feature_from_symbol(hatom_getsym(&subll->l_head->l_hatom), &x->temporalmodes[i]);
                x->features[i] = feat;
                if (feat == EARS_ESSENTIA_FEATURE_UNKNOWN)
                    object_error((t_object *)x, "Unknown feature at index %d", i+1);
                llll_free(x->algorithm_args[i]);
                x->algorithm_args[i] = llll_clone(subll);
                llll_behead(x->algorithm_args[i]);
            } else {
                object_error((t_object *)x, "Unknown feature at index %d", i+1);
            }
        } else {
            object_error((t_object *)x, "Unknown feature at index %d", i+1);
        }
    }
    x->must_recreate_extractors = true;
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
     
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
        x->temporalmodes = (long *)bach_newptrclear(1 * sizeof(long));
        x->algorithm_args = (t_llll **)bach_newptrclear(1 * sizeof(t_llll *));
        x->algorithm_args[0] = llll_get();

        // default analysis parameters
        x->a_winsize = 46.4399093;
        x->a_hopsize = 23.21995465;
        x->a_wintype = gensym("hann");
        x->a_envattacktime = 10;
        x->a_envreleasetime = 100;  //< Beware: this is different from essentia's default
                                    // But I think that essentia's default was WAY too long.
        
        x->buffer_output_interpolation_order = 1;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name features @optional 0 @type list
        // @digest Features to be computed
        // @description A list of symbols, each associated with a feature to be computed
        
        t_llll *args = llll_parse(true_ac, argv);
        buf_essentia_set_features(x, args);
        
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


void buf_essentia_free(t_buf_essentia *x)
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


t_ears_essentia_analysis_params buf_essentia_get_params(t_buf_essentia *x, t_buffer_obj *buf)
{
    t_ears_essentia_analysis_params params;
    params.framesize_samps = earsbufobj_input_to_samps((t_earsbufobj *)x, x->a_winsize, buf);
    params.framesize_samps = 2 * (params.framesize_samps / 2);     // ensure this is even
    params.hopsize_samps = (Real)earsbufobj_input_to_fsamps((t_earsbufobj *)x, x->a_hopsize, buf);
    params.duration_samps = ears_buffer_get_size_samps((t_object *)x, buf);
    if (params.hopsize_samps <= 0) {
        params.hopsize_samps = 1024;
        object_error((t_object *)x, "Negative hop size ignored; a default value of 1024 samples will be used instead.");
    } else if (params.hopsize_samps < 1) {
        object_warn((t_object *)x, "Hop size is smaller than one sample. The number of output frames may differ from what was expected.");
    }
    params.windowType = x->a_wintype->s_name;
    params.lastFrameToEndOfFile = 0;
    params.startFromZero = 0;
    
    params.envelope_rectify = 1;
    params.envelope_attack_time_samps = (Real)earsbufobj_input_to_fsamps((t_earsbufobj *)x, x->a_envattacktime, buf);
    params.envelope_release_time_samps = (Real)earsbufobj_input_to_fsamps((t_earsbufobj *)x, x->a_envreleasetime, buf);
    return params;
}

void buf_essentia_bang(t_buf_essentia *x)
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

        t_ears_essentia_analysis_params params = buf_essentia_get_params(x, in);
        
        if (x->must_recreate_extractors) {
            if (x->extractors_lib.num_extractors > 0)
                ears_essentia_extractors_library_free(&x->extractors_lib);
            ears_essentia_extractors_library_build((t_earsbufobj *)x, x->num_features, x->features, x->temporalmodes, sr, x->algorithm_args, &x->extractors_lib, &params);
            x->must_recreate_extractors = false;
        }
    
        // set output buffer for writing
        for (long i = 0; i < MIN(x->extractors_lib.num_extractors, x->num_features); i++)
            x->extractors_lib.extractors[i].result_buf = res_buf[i];
        
        // compute descriptors
        ears_essentia_extractors_library_compute((t_object *)x, in, &x->extractors_lib, &params, x->buffer_output_interpolation_order);
        
        // get the lll result, if any
        for (long i = 0; i < MIN(x->extractors_lib.num_extractors, x->num_features); i++)
            if (x->extractors_lib.extractors[i].result)
                llll_chain_clone(res[i], x->extractors_lib.extractors[i].result);
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


void buf_essentia_anything(t_buf_essentia *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_essentia_bang(x);
            
        } else if (inlet == 1) {
            buf_essentia_set_features(x, parsed);
        }
    }
    llll_free(parsed);
}


