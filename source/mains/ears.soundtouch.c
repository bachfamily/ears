/**
	@file
	ears.soundtouch.c
 
	@name
	ears.soundtouch~
 
	@realname
	ears.soundtouch~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	SoundTouch pitch and time processing
 
	@description
    Implements the SoundTouch library algorithms for pitch shifting and time stretching.
 
	@discussion
    The original code for the SoundTouch library is by Olli Parviainen (https://codeberg.org/soundtouch/soundtouch)
 
	@category
	ears time and pitch
 
	@keywords
	buffer, pitch, stretch, shift, pitchshift, timestretch, soundtouch, sound, touch
 
	@seealso
	ears.rubberband~, ears.paulstretch~, ears.resample~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include "ears.soundtouch_commons.h"


typedef struct _buf_soundtouch {
    t_earsbufobj       e_ob;
    
    double             e_stretch_factor;
    double             e_pitch_shift;

    char               e_speech;
    char               e_no_antialias;
    char               e_quick;
} t_buf_soundtouch;


// Prototypes
t_buf_soundtouch*   buf_soundtouch_new(t_symbol *s, short argc, t_atom *argv);
void			    buf_soundtouch_free(t_buf_soundtouch *x);
void			    buf_soundtouch_bang(t_buf_soundtouch *x);
void			    buf_soundtouch_anything(t_buf_soundtouch *x, t_symbol *msg, long ac, t_atom *av);

void buf_soundtouch_assist(t_buf_soundtouch *x, void *b, long m, long a, char *s);
void buf_soundtouch_inletinfo(t_buf_soundtouch *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(soundtouch)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.soundtouch~",
                         (method)buf_soundtouch_new,
                         (method)buf_soundtouch_free,
                         sizeof(t_buf_soundtouch),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(soundtouch)

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_pitchunit_attr(c);

    earsbufobj_class_add_poly_attr(c);

    CLASS_ATTR_CHAR(c, "speech", 0, t_buf_soundtouch, e_speech);
    CLASS_ATTR_STYLE_LABEL(c,"speech",0,"onoff","Speech");
    CLASS_ATTR_DEFAULT(c, "speech", 0, "0");
    // @description Tune algorithm for speech processing (default is for music).

    CLASS_ATTR_CHAR(c, "quick", 0, t_buf_soundtouch, e_quick);
    CLASS_ATTR_STYLE_LABEL(c,"quick",0,"onoff","Quick");
    CLASS_ATTR_DEFAULT(c, "quick", 0, "0");
    // @description Toggle the usage of quicker tempo change algorithm (gain speed, lose quality).

    CLASS_ATTR_CHAR(c, "naa", 0, t_buf_soundtouch, e_no_antialias);
    CLASS_ATTR_STYLE_LABEL(c,"naa",0,"onoff","Disable Antialiasing");
    CLASS_ATTR_DEFAULT(c, "naa", 0, "0");
    // @description Don't use anti-alias filtering (gain speed, lose quality).

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_soundtouch_assist(t_buf_soundtouch *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else if (a == 1)
            // @in 1 @type float @digest Stretch factor or envelope
            // @description Sets the stretch factor.
            sprintf(s, "float: Stretch Factor");
        else if (a == 2)
            // @in 2 @type float @digest Pitch shift amount
            // @description Sets the pitch shift amount (unit defined via the <m>pitchunit</m> attribute).
            sprintf(s, "float: Pitch Shift Amount");
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_soundtouch_inletinfo(t_buf_soundtouch *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_soundtouch *buf_soundtouch_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_soundtouch *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_soundtouch*)object_alloc_debug(s_tag_class);
    if (x) {
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        x->e_speech = 0;
        x->e_quick = 0;
        x->e_no_antialias = 0;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 1 @name stretch_factor @type float
        // @digest Stretch factor
        // @description Sets the stretch factor.
        
        x->e_stretch_factor = 1.;

        // @arg 2 @name pitch_shift_amount @type float
        // @digest Pitch shift amount
        // @description Sets the pitch shift amount (unit defined via the <m>pitchunit</m> attribute).
        
        x->e_pitch_shift = 0.;

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
       
        if (args && args->l_head) {
            x->e_stretch_factor = hatom_getdouble(&args->l_head->l_hatom);
            if (args->l_head->l_next) {
                x->e_pitch_shift = hatom_getdouble(&args->l_head->l_next->l_hatom);
            }
        }

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E44", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_soundtouch_free(t_buf_soundtouch *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_soundtouch_bang(t_buf_soundtouch *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        ears_buffer_soundtouch((t_object *)x, in, out, x->e_stretch_factor, earsbufobj_pitch_to_cents((t_earsbufobj *)x, x->e_pitch_shift)/100., x->e_quick, x->e_no_antialias, x->e_speech);

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_soundtouch_anything(t_buf_soundtouch *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_soundtouch_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            x->e_stretch_factor = hatom_getdouble(&parsed->l_head->l_hatom);
            if (x->e_stretch_factor <= 0) {
                object_error((t_object *)x, "Stretch factor must be positive. Defaulting to 1.");
                x->e_stretch_factor = 1.;
            }
            earsbufobj_mutex_unlock((t_earsbufobj *)x);

        } else if (inlet == 2) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            x->e_pitch_shift = hatom_getdouble(&parsed->l_head->l_hatom);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


