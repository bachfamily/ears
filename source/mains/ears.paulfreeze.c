/**
	@file
	ears.paulfreeze.c
 
	@name
	ears.paulfreeze~
 
	@realname
	ears.paulfreeze~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Paulstretch freeze
 
	@description
    Implements a freeze via the public domain 'Paulstretch' algorithm
 
	@discussion
    The original algorithm is by Nasca Octavian PAUL, Targu Mures, Romania, http://www.paulnasca.com/

	@category
	ears time and pitch
 
	@keywords
	buffer, stretch, freeze, eternal, expand
 
	@seealso
	ears.paulstretch~
	
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


typedef struct _buf_paulfreeze {
    t_earsbufobj       e_ob;
    
    t_llll*            e_duration;
    t_llll*            e_onset;
    char               e_spectral;
} t_buf_paulfreeze;



// Prototypes
t_buf_paulfreeze*         buf_paulfreeze_new(t_symbol *s, short argc, t_atom *argv);
void			buf_paulfreeze_free(t_buf_paulfreeze *x);
void			buf_paulfreeze_bang(t_buf_paulfreeze *x);
void			buf_paulfreeze_anything(t_buf_paulfreeze *x, t_symbol *msg, long ac, t_atom *av);

void buf_paulfreeze_assist(t_buf_paulfreeze *x, void *b, long m, long a, char *s);
void buf_paulfreeze_inletinfo(t_buf_paulfreeze *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(paulfreeze)

/**********************************************************************/
// Class Definition and Life Cycle




int C74_EXPORT main(void)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.paulfreeze~",
                         (method)buf_paulfreeze_new,
                         (method)buf_paulfreeze_free,
                         sizeof(t_buf_paulfreeze),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(paulfreeze)

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_antimeunit_attr(c);

    earsbufobj_class_add_framesize_attr(c);

    CLASS_ATTR_CHAR(c, "spectral", 0, t_buf_paulfreeze, e_spectral);
    CLASS_ATTR_STYLE_LABEL(c,"spectral",0,"onoff","Frequency Domain");
    // @description If toggled, the algorithm works in the frequency domain, by overlap-adding FFT windows (default);
    // if untoggled, it works in the time domain, by overlap-adding time grains

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_paulfreeze_assist(t_buf_paulfreeze *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else if (a == 1)
            sprintf(s, "float/list/llll: Freeze Duration");
        // @in 1 @type float/list/llll @digest Freeze duration
        // @description Sets the freeze duration (in the <m>timeunit</m>).
        else
            sprintf(s, "float/list/llll: Freeze Onset");
        // @in 2 @type float/list/llll @digest Freeze onset
        // @description Sets the freeze onset (in the <m>timeunit</m>).
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_paulfreeze_inletinfo(t_buf_paulfreeze *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_paulfreeze *buf_paulfreeze_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_paulfreeze *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_paulfreeze*)object_alloc_debug(s_tag_class);
    if (x) {
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name duration @optional 1 @type int/float
        // @digest Freeze duration
        // @description Sets the freeze duration (in the <m>timeunit</m>).

        // @arg 2 @name onset @optional 1 @type int/float/llll
        // @digest Freeze onset
        // @description Sets the freeze onset (in the <m>timeunit</m>).

        x->e_duration = llll_from_text_buf("1000");
        x->e_onset = llll_from_text_buf("0");
        x->e_spectral = true;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        x->e_ob.a_framesize = 8192; // 8192 samples as default

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_clear(x->e_duration);
            llll_appendhatom_clone(x->e_duration, &args->l_head->l_hatom);
            if (args->l_head->l_next) {
                llll_clear(x->e_onset);
                llll_appendhatom_clone(x->e_onset, &args->l_head->l_hatom);
            }
        }
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E44", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_paulfreeze_free(t_buf_paulfreeze *x)
{
    llll_free(x->e_onset);
    llll_free(x->e_duration);
    earsbufobj_free((t_earsbufobj *)x);
}

void buf_paulfreeze_bang(t_buf_paulfreeze *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);

    t_llllelem *el = x->e_onset->l_head;
    t_llllelem *del = x->e_duration->l_head;
    if (!del) {
        object_error((t_object *)x, "Duration not defined!");
    }
    for (long count = 0; count < num_buffers && del; count++, el = el && el->l_next ? el->l_next : el, del = del && del->l_next ? del->l_next : del) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        long onset_samps = 0, jitter_samps = 0;
        long framesize_samps = earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_ob.a_framesize, in, false, true);
        long duration_samps = earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&del->l_hatom), in);
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            t_llll *ll = hatom_getllll(&el->l_hatom);
            if (ll && ll->l_size >= 1) {
                onset_samps = earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&ll->l_head->l_hatom), in);
                if (ll->l_size >= 2) {
                    if (hatom_gettype(&ll->l_head->l_next->l_hatom) == H_SYM)
                        jitter_samps = MAX(0, ears_buffer_get_size_samps((t_object *)x, in) - framesize_samps);
                    else {
                        jitter_samps = earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&ll->l_head->l_next->l_hatom), in);
                        if (jitter_samps < 0)
                            jitter_samps += ears_buffer_get_size_samps((t_object *)x, in);
                    }
                }
            }
        } else if (is_hatom_number(&el->l_hatom)) {
            onset_samps = earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&el->l_hatom), in);
            jitter_samps = 0;
        }

        ears_buffer_paulfreeze((t_object *)x, in, out, onset_samps, framesize_samps, jitter_samps, duration_samps, x->e_spectral);
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_paulfreeze_anything(t_buf_paulfreeze *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long num_bufs = llll_get_num_symbols_root(parsed);

            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_bufs, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_paulfreeze_bang(x);
         
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->e_duration);
            x->e_duration = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else if (inlet == 2) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->e_onset);
            x->e_onset = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
        
    }
    llll_free(parsed);
}


