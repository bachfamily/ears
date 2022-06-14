/**
	@file
	ears.peaks.c
 
	@name
	ears.peaks~
 
	@realname
	ears.peaks~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Find peaks of a spectrogram
 
	@description
	Find the peaks of an incoming buffer in spectrogram matrix form (as the ones obtained via <o>ears.stft</o> or <o>ears.cqt</o>).
 
	@discussion
    The incoming buffer must have as many channels as bins.
 
	@category
	ears spectral
 
	@keywords
	buffer, peaks, spectrum
 
	@seealso
	ears.stft~, ears.cqt~, ears.trans~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include "ears.spectral.h"
#include "ears.essentia_commons.h"

typedef struct _buf_peaks {
    t_earsbufobj       e_ob;

    long        e_interpolate;
    long        e_maxPeaks;
    double      e_minPeakDistance;
    t_symbol    *e_orderBy;
    double      e_threshold;
} t_buf_peaks;



// Prototypes
t_buf_peaks*     buf_peaks_new(t_symbol *s, short argc, t_atom *argv);
void			buf_peaks_free(t_buf_peaks *x);
void			buf_peaks_bang(t_buf_peaks *x);
void			buf_peaks_anything(t_buf_peaks *x, t_symbol *msg, long ac, t_atom *av);

void buf_peaks_assist(t_buf_peaks *x, void *b, long m, long a, char *s);
void buf_peaks_inletinfo(t_buf_peaks *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_peaks_class = NULL;

EARSBUFOBJ_ADD_IO_METHODS(peaks)

/**********************************************************************/
// Class Definition and Life Cycle

void C74_EXPORT ext_main(void* moduleRef)
{
    ears_essentia_init();
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.peaks~",
                         (method)buf_peaks_new,
                         (method)buf_peaks_free,
                         sizeof(t_buf_peaks),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(peaks)
    
    llllobj_class_add_out_attr(c, LLLL_OBJ_VANILLA);

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_angleunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);

    CLASS_ATTR_LONG(c, "interpolate", 0, t_buf_peaks, e_interpolate);
    CLASS_ATTR_STYLE_LABEL(c,"interpolate",0,"onoff","Interpolate");
    CLASS_ATTR_BASIC(c, "interpolate", 0);
    // @description Toggles the ability to perform cubic interpolation to find 'true' peaks.
    
    
    CLASS_ATTR_LONG(c, "maxpeaks", 0, t_buf_peaks, e_maxPeaks);
    CLASS_ATTR_STYLE_LABEL(c,"maxpeaks",0,"text","Maximum Number of Peaks");
    CLASS_ATTR_BASIC(c, "maxpeaks", 0);
    // @description Sets the maximum number of peaks, 0 means: all.

    CLASS_ATTR_DOUBLE(c, "minpeakdistance", 0, t_buf_peaks, e_minPeakDistance);
    CLASS_ATTR_STYLE_LABEL(c,"minpeakdistance",0,"text","Minimum Distance Between Peaks");
    CLASS_ATTR_BASIC(c, "minpeakdistance", 0);
    // @description Sets the minimum distance between the peaks

    CLASS_ATTR_SYM(c, "orderby", 0, t_buf_peaks, e_orderBy);
    CLASS_ATTR_STYLE_LABEL(c,"orderby",0,"enum","Order By");
    CLASS_ATTR_ENUM(c,"orderby", 0, "position amplitude");
    CLASS_ATTR_BASIC(c, "orderby", 0);
    // @description Sets the ordering type (either "position" or "amplitude").

    CLASS_ATTR_DOUBLE(c, "thresh", 0, t_buf_peaks, e_threshold);
    CLASS_ATTR_STYLE_LABEL(c,"thresh",0,"text","Peak Threshold");
    CLASS_ATTR_BASIC(c, "thresh", 0);
    // @description Sets a threshold, so that peaks with amplitude below this threshold are discarded.
    

    class_register(CLASS_BOX, c);
    s_peaks_class = c;
    return 0;
}

void buf_peaks_assist(t_buf_peaks *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            // @in 0 @type symbol @digest Buffer containing spectrogram magnitudes
            // @description Source spectrogram magnitudes
            sprintf(s, "symbol/llll: Magnitude Spectrogram Buffer");
        else
            // @in 1 @type symbol @digest Optional buffer containing spectrogram phases
            // @description Source spectrogram phases
            sprintf(s, "symbol/llll: Optional Phase Spectrogram Buffer");
    } else {
        char *type = NULL;
        llllobj_get_llll_outlet_type_as_string((t_object *) x, LLLL_OBJ_VANILLA, a, &type);
        // @out 0 @type llll @digest Detected peaks
        // @description Outputs peaks organized as follows: one llll per frame
        // with the onset as first element, then one llll per peak;
        // each peak being in the form: <b><m>position</m> <m>amplitude</m> <m>optional:phase</m></b>,
        // where <m>phase</m> is only present if a phase spectrogram has been provided.
        sprintf(s, "llll (%s): Peaks", type);
    }
}

void buf_peaks_inletinfo(t_buf_peaks *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_peaks *buf_peaks_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_peaks *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_peaks*)object_alloc_debug(s_peaks_class);
    if (x) {
        x->e_interpolate = 1;
        x->e_maxPeaks = 0; // 0 = all
        x->e_minPeakDistance = 0.;
        x->e_orderBy = gensym("amplitude");
        x->e_threshold = -100000;

        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE); // EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "ee", "4", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_peaks_free(t_buf_peaks *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_peaks_bang(t_buf_peaks *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);

    if (num_buffers > 0) { // must be just 1 buffer
        t_buffer_obj *in1 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
        t_buffer_obj *in2 = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, 0);
        t_ears_err err = EARS_ERR_NONE;
        
        t_llll *out = ears_specbuffer_peaks((t_object *)x, in1, in2, x->e_interpolate, x->e_maxPeaks > 0 ? x->e_maxPeaks : INT_MAX, x->e_minPeakDistance, x->e_orderBy, x->e_threshold, (e_ears_timeunit)x->e_ob.l_timeunit, (e_ears_angleunit)x->e_ob.l_angleunit, &err);
        
        earsbufobj_outlet_llll((t_earsbufobj *)x, 0, out);
        llll_free(out);
    }
}



void buf_peaks_anything(t_buf_peaks *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
            
            if (inlet == 0) {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
                
                buf_peaks_bang(x);
            } else {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 1, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 1);
            }
        }
    }
    llll_free(parsed);
}


