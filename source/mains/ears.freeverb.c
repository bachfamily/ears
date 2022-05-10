/**
	@file
	ears.freeverb.c
 
	@name
	ears.freeverb~
 
	@realname
	ears.freeverb~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Freeverb reverberation
 
	@description
    Implements the public domain 'Freeverb' reverberation algorithm
 
	@discussion
    The original code for the reverberation was written by Jezar at Dreampoint
 
	@category
	ears reverb
 
	@keywords
	buffer, reverb, freeverb, reverberation
 
	@seealso
	ears.biquad~, ears.onepole~, ears.paulstretch~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"
#include "ears.freeverb_commons.h"


typedef struct _buf_freeverb {
    t_earsbufobj       e_ob;
    
    t_llll             *e_wet;
    t_llll             *e_dry;
    double             e_room_size;
    double             e_damp;
    double             e_width;
    char               e_mode;
    
    double             e_tail;
    
    revmodel           *e_model;
    long               e_current_num_channels;
} t_buf_freeverb;



// Prototypes
t_buf_freeverb*         buf_freeverb_new(t_symbol *s, short argc, t_atom *argv);
void			buf_freeverb_free(t_buf_freeverb *x);
void			buf_freeverb_bang(t_buf_freeverb *x);
void			buf_freeverb_anything(t_buf_freeverb *x, t_symbol *msg, long ac, t_atom *av);

void buf_freeverb_assist(t_buf_freeverb *x, void *b, long m, long a, char *s);
void buf_freeverb_inletinfo(t_buf_freeverb *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(freeverb)

/**********************************************************************/
// Class Definition and Life Cycle



DEFINE_LLLL_ATTR_DEFAULT_GETTER(t_buf_freeverb, e_wet, buf_freeverb_getattr_wet);
DEFINE_LLLL_ATTR_DEFAULT_GETTER(t_buf_freeverb, e_dry, buf_freeverb_getattr_dry);
DEFINE_LLLL_ATTR_DEFAULT_SETTER(t_buf_freeverb, e_wet, buf_freeverb_setattr_wet);
DEFINE_LLLL_ATTR_DEFAULT_SETTER(t_buf_freeverb, e_dry, buf_freeverb_setattr_dry);

t_max_err buf_freeverb_setattr_roomsize(t_buf_freeverb *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (is_atom_number(argv)) {
            x->e_room_size = atom_getfloat(argv);
            x->e_model->setroomsize(x->e_room_size);
        }
    }
    return MAX_ERR_NONE;
}

t_max_err buf_freeverb_setattr_damp(t_buf_freeverb *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (is_atom_number(argv)) {
            x->e_damp = atom_getfloat(argv);
            x->e_model->setdamp(x->e_damp);
        }
    }
    return MAX_ERR_NONE;
}

t_max_err buf_freeverb_setattr_width(t_buf_freeverb *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (is_atom_number(argv)) {
            x->e_width = atom_getfloat(argv);
            x->e_model->setwidth(x->e_width);
        }
    }
    return MAX_ERR_NONE;
}

t_max_err buf_freeverb_setattr_mode(t_buf_freeverb *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (is_atom_number(argv)) {
            x->e_mode = atom_getlong(argv);
            x->e_model->setmode(x->e_mode);
        }
    }
    return MAX_ERR_NONE;
}


int C74_EXPORT main(void)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.freeverb~",
                         (method)buf_freeverb_new,
                         (method)buf_freeverb_free,
                         sizeof(t_buf_freeverb),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(freeverb)

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);

    earsbufobj_class_add_polyout_attr(c);

    CLASS_ATTR_LLLL(c, "wet", 0, t_buf_freeverb, e_wet, buf_freeverb_getattr_wet, buf_freeverb_setattr_wet);
    CLASS_ATTR_STYLE_LABEL(c,"wet",0,"text","Wet");
    CLASS_ATTR_BASIC(c, "wet", 0);
    // @description Sets the amount of wet output, either as a single number or as an envelope

    CLASS_ATTR_LLLL(c, "dry", 0, t_buf_freeverb, e_dry, buf_freeverb_getattr_dry, buf_freeverb_setattr_dry);
    CLASS_ATTR_STYLE_LABEL(c,"dry",0,"text","Dry");
    CLASS_ATTR_BASIC(c, "dry", 0);
    // @description Sets the amount of wet output, either as a single number or as an envelope

    CLASS_ATTR_DOUBLE(c, "roomsize", 0, t_buf_freeverb, e_room_size);
    CLASS_ATTR_STYLE_LABEL(c,"roomsize",0,"text","Room Size");
    CLASS_ATTR_ACCESSORS(c, "roomsize", NULL, buf_freeverb_setattr_roomsize);
    CLASS_ATTR_BASIC(c, "roomsize", 0);
    // @description Sets the room size.

    CLASS_ATTR_DOUBLE(c, "damp", 0, t_buf_freeverb, e_damp);
    CLASS_ATTR_STYLE_LABEL(c,"damp",0,"text","Damp");
    CLASS_ATTR_ACCESSORS(c, "damp", NULL, buf_freeverb_setattr_damp);
    // @description Sets the damping coefficient.

    CLASS_ATTR_DOUBLE(c, "width", 0, t_buf_freeverb, e_width);
    CLASS_ATTR_STYLE_LABEL(c,"width",0,"text","Width");
    CLASS_ATTR_ACCESSORS(c, "width", NULL, buf_freeverb_setattr_width);
    // @description Sets the width.
    
    CLASS_ATTR_DOUBLE(c, "tail", 0, t_buf_freeverb, e_tail);
    CLASS_ATTR_STYLE_LABEL(c,"tail",0,"text","Reverb Tail Duration");
    CLASS_ATTR_BASIC(c, "tail", 0);
    // @description Sets the duration of the reverb tail (in the unit given by the <m>timeunit</m> attribute):
    // a positive number provides a fixed length of the audio tail; 0 means: no tail; a negative number (default is -1) means that
    // the tail will be automatically computed.

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_freeverb_assist(t_buf_freeverb *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else if (a == 1)
            sprintf(s, "number/llll: Dry amount or envelope"); // @in 0 @type number/llll @digest Dry amount or envelope
        else
            sprintf(s, "number/llll: Wet amount or envelope"); // @in 0 @type number/llll @digest Wet amount or envelope
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_freeverb_inletinfo(t_buf_freeverb *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_freeverb *buf_freeverb_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_freeverb *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_freeverb*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_wet = llll_get();
        llll_appenddouble(x->e_wet, initialwet);
        x->e_dry = llll_get();
        llll_appenddouble(x->e_dry, initialdry);
        x->e_room_size = initialroom;
        x->e_damp = initialdamp;
        x->e_mode = initialmode;
        x->e_width = initialwidth;
        x->e_tail = -1;
        
        x->e_model = new revmodel();
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "E44", "E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_freeverb_free(t_buf_freeverb *x)
{
    delete x->e_model;
    earsbufobj_free((t_earsbufobj *)x);
}

void buf_freeverb_bang(t_buf_freeverb *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    t_llllelem *dry_el = x->e_dry->l_head;
    t_llllelem *wet_el = x->e_wet->l_head;
    for (long count = 0; count < num_buffers; count++,
         dry_el = dry_el && dry_el->l_next ? dry_el->l_next : dry_el,
         wet_el = dry_el && wet_el->l_next ? wet_el->l_next : wet_el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        // to do: set wet set dry
        
        long numchans = ears_buffer_get_numchannels((t_object *)x, in);
        if (numchans >= 1) {
            x->e_model->setnumchannels(numchans);
        }
        float sr = ears_buffer_get_sr((t_object *)x, in);
        if (sr > 0)
            x->e_model->setsr(sr);
        if (hatom_gettype(&dry_el->l_hatom) == H_LLLL || hatom_gettype(&wet_el->l_hatom) == H_LLLL) {
            t_llll *dry_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, dry_el, in);
            t_llll *wet_env = earsbufobj_llllelem_to_env_samples((t_earsbufobj *)x, wet_el, in);
            double dry_default = is_hatom_number(&dry_el->l_hatom) ? hatom_getdouble(&dry_el->l_hatom) : 1.;
            double wet_default = is_hatom_number(&wet_el->l_hatom) ? hatom_getdouble(&wet_el->l_hatom) : 0.;
            ears_buffer_freeverb_envelope((t_object *)x, in, out, x->e_model, earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_tail, in),
                                          dry_env, wet_env, dry_default, wet_default, earsbufobj_get_slope_mapping((t_earsbufobj *)x));
            llll_free(dry_env);
            llll_free(wet_env);
        } else {
            x->e_model->setwet(hatom_getdouble(&wet_el->l_hatom));
            x->e_model->setdry(hatom_getdouble(&dry_el->l_hatom));
            ears_buffer_freeverb((t_object *)x, in, out, x->e_model, earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_tail, in));
        }
        
        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_freeverb_anything(t_buf_freeverb *x, t_symbol *msg, long ac, t_atom *av)
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
            
            buf_freeverb_bang(x);
            
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->e_dry);
            x->e_dry = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
            
        } else if (inlet == 2) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->e_wet);
            x->e_wet = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
    llll_free(parsed);
}


