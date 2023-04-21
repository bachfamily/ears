/**
	@file
	ears.assemble.c
 
	@name
	ears.assemble~
 
	@realname
	ears.assemble~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Mix buffers one by one
 
	@description
	Assembles an output buffer from a sequence of incoming buffers
 
	@discussion
    This module is related to <o>ears.mix~</o> (and, with the tail-onset
    <m>mode</m>, to the <o>ears.join~</o> object), but should be preferred to them,
    for instance in combination with <o>ears.read~</o> with <m>iter</m> 1 attribute set,
    in order to optimize memory allocation. Indeed, <o>ears.mix~</o> needs all the buffers
    to be allocated before mixing, while <o>ears.assemble~</o> will do this with one buffer at a time.
 
 
	@category
	ears basic
 
	@keywords
	buffer, assemble, mix, superpose, merge
 
	@seealso
	ears.mix~, ears.join, ears.read~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"


enum {
    EARS_ASSEMBLE_ONSETMODE_STANDARD = 0,
    EARS_ASSEMBLE_ONSETMODE_INTERONSETINTERVAL = 1,
    EARS_ASSEMBLE_ONSETMODE_TAILONSETINTERVAL = 2
};

typedef struct _buf_assemble {
    t_earsbufobj       e_ob;
    
    char               normalization_mode;
    char               interp_offsets;
    double             last_offset_samps;
    char               onset_mode;

    char               assembly_line_status; // 0 = new, 1 = first being constructed, 2 = under construction, 3 = done
    
    long               allocated_samps;
    long               curr_length_samps;
    double             default_ioi;
    
    t_llll              *gains;
    t_llll              *offsets;
} t_buf_assemble;



// Prototypes
t_buf_assemble*     buf_assemble_new(t_symbol *s, short argc, t_atom *argv);
void			buf_assemble_free(t_buf_assemble *x);
void			buf_assemble_bang(t_buf_assemble *x);
void			buf_assemble_anything(t_buf_assemble *x, t_symbol *msg, long ac, t_atom *av);

void buf_assemble_assist(t_buf_assemble *x, void *b, long m, long a, char *s);
void buf_assemble_inletinfo(t_buf_assemble *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(assemble)

/**********************************************************************/
// Class Definition and Life Cycle

void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.assemble~",
                         (method)buf_assemble_new,
                         (method)buf_assemble_free,
                         sizeof(t_buf_assemble),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the furst inlet with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute). <br />
    // A number, list or llll in the second inlet is interpreted to contain the gain
    // for each one of the incoming buffer (in the current <m>ampunit</m>). <br />
    // A number, list or llll in the third inlet is interpreted to contain the temporal offset
    // for each one of the incoming buffer (in the current <m>timeunit</m>). Non-integer sample offsets are only accounted for
    // if the <m>interp</m> attribute is on, otherwise they are rounded to the nearest integer sample. <br />
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(assemble)

    // @method number @digest Set Gain or Offset
    // See <m>list</m> method, for second or third inlet.

    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_envampunit_attr(c);
    earsbufobj_class_add_envtimeunit_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);

    earsbufobj_class_add_resamplingpolicy_attr(c);
    earsbufobj_class_add_resamplingfiltersize_attr(c);
    earsbufobj_class_add_resamplingmode_attr(c);

    CLASS_ATTR_CHAR(c, "normalize", 0, t_buf_assemble, normalization_mode);
    CLASS_ATTR_STYLE_LABEL(c,"normalize",0,"enumindex","Normalize Output");
    CLASS_ATTR_ENUMINDEX(c,"normalize", 0, "Never Always Overload Protection Only");
    CLASS_ATTR_BASIC(c, "normalize", 0);
    // @description Sets the normalization mode for the output buffer:
    // 0 = Never, does not normalize; 1 = Always, always normalizes to 1.; 2 = Overload Protection Only, only
    // normalizes to 1. if some samples exceed in modulo 1.

    CLASS_ATTR_CHAR(c, "mode", 0, t_buf_assemble, onset_mode);
    CLASS_ATTR_STYLE_LABEL(c,"mode",0,"enumindex","Onset Mode");
    CLASS_ATTR_ENUMINDEX(c,"mode", 0, "Onsets Inter-Onset Intervals Tail-Onset Intervals");
    CLASS_ATTR_BASIC(c, "mode", 0);
    // @description Sets the type of information that comes through the right inlet: <br />
    // 0: Onsets; <br />
    // 1: Inter-Onset Intervals; <br />
    // 2: Tail-Onset Intervals (use it to have assemble work like <o>ears.join~</o>).

    
    CLASS_ATTR_CHAR(c, "interp", 0, t_buf_assemble, interp_offsets);
    CLASS_ATTR_STYLE_LABEL(c,"interp",0,"onoff","Interpolate Non-Integer Offset");
    // @description Toggles the ability to perform band-limited interpolation via resampling for non-integer offsets.

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_assemble_assist(t_buf_assemble *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0) // @in 0 @type bang @digest Output Assembled Result
            sprintf(s, "bang: Output Assembled Result");
        else if (a == 1) // @in 1 @type list @digest Name of the buffer to be assembled
            sprintf(s, "list: Buffer Name");
        else if (a == 2) // @in 2 @type number @digest Gain
            sprintf(s, "list/float: Gain");
        else if (a == 3) // @in 3 @type number @digest Offset or inter-offset intervals
                        // @description The usage of offsets or inter-offset intervals depends on the <m>delta</m> attribute
            sprintf(s, "list/float: Offset");

    } else {
        sprintf(s, "Output Buffer Name"); // @out 0 @type symbol/list @digest Output buffer names
                                           // @description Name of the output buffer
    }
}

void buf_assemble_inletinfo(t_buf_assemble *x, void *b, long a, char *t)
{
    if (a != 0 && a != 1)
        *t = 1;
}


t_buf_assemble *buf_assemble_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_assemble *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_assemble*)object_alloc_debug(s_tag_class);
    if (x) {
/*        x->xfade_type = EARS_FADE_SINE;
        x->xfade_left = x->xfade_right = 0;
        x->xfade_amount_mode = EARS_assemble_XFADE_AMOUNT_MS;
*/
        x->onset_mode = EARS_ASSEMBLE_ONSETMODE_STANDARD;
        x->gains = llll_get();
        x->offsets = llll_get();
        x->normalization_mode = EARS_NORMALIZE_OVERLOAD_PROTECTION_ONLY;
        x->interp_offsets = 0;
        
        earsbufobj_init((t_earsbufobj *)x, 0);
        

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        // @arg 1 @name default_interval @optional 1 @type number
        // @digest Default time interval
        // @description Sets a default time interval: either an inter-onset interval
        // or a tail-onset interval (depending on the <m>mode</m> attribute)
        
        if (args && args->l_head && is_hatom_number(&args->l_head->l_hatom)) {
            x->default_ioi = hatom_getdouble(&args->l_head->l_hatom);
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "bE44", "e", names);
        x->e_ob.l_inlet_hot[1] = true;

        x->assembly_line_status = 0;
        
        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_assemble_free(t_buf_assemble *x)
{
    llll_free(x->gains);
    llll_free(x->offsets);
    earsbufobj_free((t_earsbufobj *)x);
}

void buf_assemble_bang(t_buf_assemble *x)
{
    t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);

    if (x->assembly_line_status == 1 || x->assembly_line_status == 2) {
        ears_buffer_assemble_close((t_object *)x, out, (e_ears_normalization_modes)x->normalization_mode, x->curr_length_samps);
    }
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);

    x->assembly_line_status = 3; // done
    
}

void buf_assemble_once(t_buf_assemble *x)
{
    if (earsbufobj_get_instore_size((t_earsbufobj *)x, 0) < 1)
        return;
    
    t_llll *gains = llll_clone(x->gains);
    t_llll *offsets = llll_clone(x->offsets);
    
    t_llll *gains_linear = llll_get();
    
    t_llllelem *gain_el = gains->l_head;
    t_llllelem *offset_el = offsets->l_head;
    t_buffer_obj *buf = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    
    if (x->interp_offsets > 0) {
        double offset_samps = 0;

        if (gain_el)
            llll_appendllll(gains_linear, earsbufobj_llllelem_to_linear_and_samples((t_earsbufobj *)x, gain_el, buf));
        else
            llll_appenddouble(gains_linear, 1.);
        
        if (offset_el) {
            switch (x->onset_mode) {
                case EARS_ASSEMBLE_ONSETMODE_INTERONSETINTERVAL:
                    offset_samps = x->last_offset_samps + earsbufobj_time_to_fsamps((t_earsbufobj *)x, hatom_getdouble(&offset_el->l_hatom), buf);
                    break;
                case EARS_ASSEMBLE_ONSETMODE_TAILONSETINTERVAL:
                    offset_samps = x->curr_length_samps + earsbufobj_time_to_fsamps((t_earsbufobj *)x, hatom_getdouble(&offset_el->l_hatom), buf);
                    break;
                case EARS_ASSEMBLE_ONSETMODE_STANDARD:
                default:
                    offset_samps = earsbufobj_time_to_fsamps((t_earsbufobj *)x, hatom_getdouble(&offset_el->l_hatom), buf);
                    break;
            }
            offset_samps = MAX(0, offset_samps);
            x->last_offset_samps = offset_samps;
        }
        // TO DO
//        ears_buffer_assemble_subsampleprec((t_object *)x, inbufs, num_buffers, out, gains_linear, offsets_samps, (e_ears_normalization_modes)normalization_mode, earsbufobj_get_slope_mapping((t_earsbufobj *)x), (e_ears_resamplingpolicy)x->e_ob.l_resamplingpolicy, x->e_ob.l_resamplingfilterwidth);

    } else {
        long offset_samps = 0;

        if (gain_el)
            llll_appendllll(gains_linear, earsbufobj_llllelem_to_linear_and_samples((t_earsbufobj *)x, gain_el, buf));
        else
            llll_appenddouble(gains_linear, 1.);
        
        if (offset_el) {
            switch (x->onset_mode) {
                case EARS_ASSEMBLE_ONSETMODE_INTERONSETINTERVAL:
                    offset_samps = x->last_offset_samps + earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&offset_el->l_hatom), buf);
                    break;
                case EARS_ASSEMBLE_ONSETMODE_TAILONSETINTERVAL:
                    offset_samps = x->curr_length_samps + earsbufobj_time_to_fsamps((t_earsbufobj *)x, hatom_getdouble(&offset_el->l_hatom), buf);
                    break;
                case EARS_ASSEMBLE_ONSETMODE_STANDARD:
                default:
                    offset_samps = earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&offset_el->l_hatom), buf);
                    break;
            }
            
            offset_samps = MAX(0, offset_samps);
            x->last_offset_samps = offset_samps;
        } else {
            long default_ioi_samps = earsbufobj_time_to_samps((t_earsbufobj *)x, x->default_ioi, buf);
            if (x->onset_mode == EARS_ASSEMBLE_ONSETMODE_INTERONSETINTERVAL && x->assembly_line_status == 2) {
                offset_samps = x->last_offset_samps + default_ioi_samps;
            } else if (x->onset_mode == EARS_ASSEMBLE_ONSETMODE_TAILONSETINTERVAL && x->assembly_line_status == 2) {
                offset_samps = x->curr_length_samps + default_ioi_samps;
            } else {
                offset_samps = x->last_offset_samps;
            }
            offset_samps = MAX(0, offset_samps);
            x->last_offset_samps = offset_samps;
        }
        
        
        ears_buffer_assemble_once((t_object *)x, out, buf, gains_linear, offset_samps, earsbufobj_get_slope_mapping((t_earsbufobj *)x), (e_ears_resamplingpolicy)x->e_ob.l_resamplingpolicy, x->e_ob.l_resamplingfilterwidth, (e_ears_resamplingmode)x->e_ob.l_resamplingmode, &x->curr_length_samps, &x->allocated_samps);
        
        x->assembly_line_status = 2;
    }
    
    llll_free(gains);
    llll_free(gains_linear);
    llll_free(offsets);
}

void buf_assemble_anything(t_buf_assemble *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 1) {
            long num_buffers = parsed->l_size;
            
            if (num_buffers >= 1 && parsed->l_head) {
                if (num_buffers > 1) {
                    object_warn((t_object *)x, "Buffers must be assembled one at a time:");
                    object_warn((t_object *)x, "    only first buffer is retained,");
                    object_warn((t_object *)x, "    use ears.mix~ if you want to assemble them all at once.");
                }
                
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 1, true);
            
                if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
                    t_symbol *buf = hatom_getsym(&parsed->l_head->l_hatom);

                    // storing input buffer(s)
                    earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 0, buf);
                    
                    if (x->assembly_line_status == 0 || x->assembly_line_status == 3) {
                        t_buffer_obj *from = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
                        t_buffer_obj *to = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);

                        x->curr_length_samps = 0;
                        x->last_offset_samps = 0;
                        
                        long numframes = EARS_BUFFER_ASSEMBLE_ALLOCATION_STEP_SEC * ears_buffer_get_sr((t_object *)x, from);

                        ears_buffer_copy_format_and_set_size_samps((t_object *)x, from, to, numframes);
                        ears_buffer_clear((t_object *)x, to);
                        
                        x->allocated_samps = numframes;
                        x->assembly_line_status = 1;
                    }
                    
                    buf_assemble_once(x);
                    
                }
            }
        } else if (inlet == 2) {
            llll_free(x->gains);
            x->gains = llll_clone(parsed);
        } else if (inlet == 3) {
            llll_free(x->offsets);
            x->offsets = llll_clone(parsed);
        }
    }
    llll_free(parsed);
}


