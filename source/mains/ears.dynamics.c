/**
	@file
	ears.dynamics.c
 
	@name
	ears.dynamics~
 
	@realname
	ears.dynamics~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Dynamics processing
 
	@description
    Implements a simple compressor
 
	@discussion
    The implementation is inspired by the tutorial provided in
    https://github.com/p-hlp/CTAGDRC

	@category
	ears dynamics
 
	@keywords
	buffer, compress, expand, dynamics, limit, limiter
 
	@seealso
	ears.gain~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"


typedef struct _buf_dynamics {
    t_earsbufobj       e_ob;

    double             e_threshold;
    double             e_ratio;
    double             e_knee;
    double             e_makeup;

    double             e_attack_time;
    double             e_release_time;
} t_buf_dynamics;



// Prototypes
t_buf_dynamics*         buf_dynamics_new(t_symbol *s, short argc, t_atom *argv);
void			buf_dynamics_free(t_buf_dynamics *x);
void			buf_dynamics_bang(t_buf_dynamics *x);
void			buf_dynamics_anything(t_buf_dynamics *x, t_symbol *msg, long ac, t_atom *av);

void buf_dynamics_assist(t_buf_dynamics *x, void *b, long m, long a, char *s);
void buf_dynamics_inletinfo(t_buf_dynamics *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(dynamics)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.dynamics~",
                         (method)buf_dynamics_new,
                         (method)buf_dynamics_free,
                         sizeof(t_buf_dynamics),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(dynamics)

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);

    CLASS_ATTR_DOUBLE(c, "attack", 0, t_buf_dynamics, e_attack_time);
    CLASS_ATTR_STYLE_LABEL(c,"attack",0,"text","Attack Time");
    CLASS_ATTR_CATEGORY(c, "attack", 0, "Ballistics");
    // @description Sets the attack time (in the unit set by the <m>timeunit</m> attribute).

    CLASS_ATTR_DOUBLE(c, "release", 0, t_buf_dynamics, e_release_time);
    CLASS_ATTR_STYLE_LABEL(c,"release",0,"text","Release Time");
    CLASS_ATTR_CATEGORY(c, "release", 0, "Ballistics");
    // @description Sets the release time (in the unit set by the <m>timeunit</m> attribute).

    CLASS_ATTR_DOUBLE(c, "thresh", 0, t_buf_dynamics, e_threshold);
    CLASS_ATTR_STYLE_LABEL(c,"thresh",0,"text","Threshold");
    CLASS_ATTR_CATEGORY(c, "thresh", 0, "Gain Computation");
    // @description Sets the threshold amplitude (in the unit set by the <m>ampunit</m> attribute).

    CLASS_ATTR_DOUBLE(c, "knee", 0, t_buf_dynamics, e_knee);
    CLASS_ATTR_STYLE_LABEL(c,"knee",0,"text","Knee Width");
    CLASS_ATTR_CATEGORY(c, "knee", 0, "Gain Computation");
    // @description Sets the knee width (in the unit set by the <m>ampunit</m> attribute).

    CLASS_ATTR_DOUBLE(c, "ratio", 0, t_buf_dynamics, e_ratio);
    CLASS_ATTR_STYLE_LABEL(c,"ratio",0,"text","Ratio");
    CLASS_ATTR_CATEGORY(c, "ratio", 0, "Gain Computation");
    // @description Sets the compression ratio.

    CLASS_ATTR_DOUBLE(c, "makeup", 0, t_buf_dynamics, e_makeup);
    CLASS_ATTR_STYLE_LABEL(c,"makeup",0,"text","Make-Up Gain");
    CLASS_ATTR_CATEGORY(c, "makeup", 0, "Gain Computation");
    // @description Sets the make-up gain (in the unit set by the <m>ampunit</m> attribute).


    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_dynamics_assist(t_buf_dynamics *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "symbol/list/llll: Incoming Buffer Names");
        // @in 1 @type symbol/list/llll @digest Sidechain buffer names
        // @description If you want to use the compressor in sidechain mode, set the sidechain buffers in the second inlet,
        // or leave empty to use the compressor on the first-inlet buffers.
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_dynamics_inletinfo(t_buf_dynamics *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_dynamics *buf_dynamics_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_dynamics *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_dynamics*)object_alloc_debug(s_tag_class);
    if (x) {
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        x->e_makeup = 0;
        x->e_ratio = 1.5;
        x->e_knee = 0.7;
        x->e_threshold = -20.;
        x->e_release_time = 50;
        x->e_attack_time = 10;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        x->e_ob.l_ampunit = EARS_AMPUNIT_DECIBEL; // defaults to decibel
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "EE", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_dynamics_free(t_buf_dynamics *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}

void buf_dynamics_bang(t_buf_dynamics *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    long num_buffers_sidechain = ((t_earsbufobj *)x)->l_instore[1].num_stored_bufs;

    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);

    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *sidechain = (num_buffers_sidechain > count) ? earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, count) : in;
        
        ears_buffer_compress((t_object *)x, in, sidechain, out,
                             earsbufobj_amplitude_to_db((t_earsbufobj *)x, x->e_threshold),
                             x->e_ratio,
                             earsbufobj_amplitude_to_db((t_earsbufobj *)x, x->e_knee),
                             earsbufobj_time_to_fsamps((t_earsbufobj *)x, x->e_attack_time, in),
                             earsbufobj_time_to_fsamps((t_earsbufobj *)x, x->e_release_time, in),
                             earsbufobj_amplitude_to_db((t_earsbufobj *)x, x->e_makeup)
                             );
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_dynamics_anything(t_buf_dynamics *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_dynamics_bang(x);
         
        } else if (inlet == 1) {
            long num_bufs = llll_get_num_symbols_root(parsed);
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 1, num_bufs, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 1);
        }
        
    }
    llll_free(parsed);
}


