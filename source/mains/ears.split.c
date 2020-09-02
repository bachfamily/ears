/**
	@file
	ears.split.c
 
	@name
	ears.split~
 
	@realname
	ears.split~
 
    @type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Split a buffer into multiple buffers
 
	@description
	Chops a buffer into multiple buffers, according to fixed lengths, silence thresholds or attacks.
 
	@discussion
 
	@category
	ears buffer operations
 
	@keywords
	buffer, split, chop, segment
 
	@seealso
	ears.slice~, ears.trim~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_split {
    t_earsbufobj        e_ob;
    
    long                e_mode; //< one of the e_ears_split_modes
    char                e_partial_segments; // output partial segments (for duration split)
    char                e_keep_silence; // keep silence (For silence split)

    t_llll              *params; // either the segment_duration, or  num_segments, or a threshold, depending on the e_mode
} t_buf_split;



// Psplitotypes
t_buf_split*         buf_split_new(t_symbol *s, short argc, t_atom *argv);
void			buf_split_free(t_buf_split *x);
void			buf_split_bang(t_buf_split *x);
void			buf_split_anything(t_buf_split *x, t_symbol *msg, long ac, t_atom *av);

void buf_split_assist(t_buf_split *x, void *b, long m, long a, char *s);
void buf_split_inletinfo(t_buf_split *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(split)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.split~",
                         (method)buf_split_new,
                         (method)buf_split_free,
                         sizeof(t_buf_split),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll in the first inlet with buffer names will trigger the slicing; the right portion of buffers is
    // output from the left outlet, and left portion of buffers is output from the left outlet.

    // @method number @digest Set split position
    // @description A number in the second inlet sets the split position (in the unit defined by the <m>timeunit</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(split)


    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

    CLASS_ATTR_LONG(c, "mode", 0, t_buf_split, e_mode);
    CLASS_ATTR_STYLE_LABEL(c,"mode",0,"enumindex","Split Mode");
    CLASS_ATTR_ENUMINDEX(c,"mode", 0, "Duration Number List Silence");
    CLASS_ATTR_BASIC(c, "mode", 0);
    // @description Sets the split mode: <br />
    // 0 (Duration): buffer is split into equally long chunks, whose duration is set; <br />
    // 1 (Number): buffer is split into a fixed number of buffers (such number is set); <br />
    // 2 (List): buffer is split via a series of split points given as parameter (right inlet); <br />
    // 3 (Silence): buffer is split according to silence regions.

    CLASS_ATTR_CHAR(c, "partials", 0, t_buf_split, e_partial_segments);
    CLASS_ATTR_STYLE_LABEL(c,"partials",0,"onoff","Output Partial Segments");
    CLASS_ATTR_BASIC(c, "partials", 0);
    // @description Toggles the ability to output partial segments in <b>Duration</b> <m>mode</m>.

    CLASS_ATTR_CHAR(c, "keepsilence", 0, t_buf_split, e_keep_silence);
    CLASS_ATTR_STYLE_LABEL(c,"keepsilence",0,"onoff","Keep Silence");
    CLASS_ATTR_BASIC(c, "keepsilence", 0);
    // @description Toggles the ability to keep the trailing silence in <b>Silence</b> <m>mode</m>.

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_split_assist(t_buf_split *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Name"); // @in 0 @type symbol/llll @digest Incoming buffer name
        else
            sprintf(s, "number/list/llll: Algorithm parameters (segment duration, or number, or list of split points, or algorithm threshold"); // @in 1 @type number/llll @digest Depending on the <m>mode</m> this can be the segment duration, the number of segments, the list of split point the threshold
    } else {
        if (a == 0)
            sprintf(s, "symbol/list: Segmented Buffer Names"); // @out 0 @type symbol/list @digest Outputs a list with buffer names for each segmented buffer
        else
            sprintf(s, "llll: Wrapped Start and End points for Regions"); // @out 1 @type llll @digest Outputs a list of couples containing the start and end points for each region (according to the <m>timeunit</m>)
    }
}

void buf_split_inletinfo(t_buf_split *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_split *buf_split_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_split *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_split*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_mode = EARS_SPLIT_MODE_DURATION;
        
        earsbufobj_init((t_earsbufobj *)x, 0);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name algorithm_parameters @optional 1 @type number
        // @digest Segment duration or number or list of split points or threshold
        // @description Depending on the <m>mode</m> attribute, sets the segment duration, the number of segments, or the list of split points or the threshold

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            x->params = llll_clone(args);
        } else {
            x->params = llll_make();
        }
        
        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "e4", "4E", names);

        llll_free(args);
        llll_free(names);
        
    }
    return x;
}


void buf_split_free(t_buf_split *x)
{
    llll_free(x->params);
    earsbufobj_free((t_earsbufobj *)x);
}





void buf_split_get_splitpoints(t_buf_split *x, t_object *buf, t_llll **start, t_llll **end)
{
    t_earsbufobj *e_ob = (t_earsbufobj *)x;
    e_ears_split_modes mode = (e_ears_split_modes)x->e_mode;
    long size_samps = ears_buffer_get_size_samps((t_object *)e_ob, buf);
    
    switch (mode) {
        case EARS_SPLIT_MODE_NUMBER:
        {
            long num_buffers = x->params && x->params->l_head ? hatom_getlong(&x->params->l_head->l_hatom) : 0;

            *start = llll_get();
            *end = llll_get();
            llll_appendlong(*start, 0);
            for (long i = 1; i < num_buffers; i++) {
                long this_samp = (long)round(i * size_samps/num_buffers);
                llll_appendlong(*end, this_samp);
                llll_appendlong(*start, this_samp);
            }
            llll_appendlong(*end, size_samps);
        }
            break;
            
        case EARS_SPLIT_MODE_DURATION:
        {
            double duration_samps = earsbufobj_input_to_fsamps(e_ob, x->params && x->params->l_head ? hatom_getdouble(&x->params->l_head->l_hatom) : 0, buf);

            *start = llll_get();
            *end = llll_get();
            llll_appendlong(*start, 0);
            for (double c = duration_samps; c < size_samps; c+=duration_samps) {
                long this_samp = (long)round(c);
                if (this_samp + duration_samps > size_samps) { // last shorter one
                    llll_appendlong(*end, this_samp);
                    if (x->e_partial_segments) {
                        llll_appendlong(*start, this_samp);
                        llll_appendlong(*end, size_samps);
                    }
                } else {
                    llll_appendlong(*end, this_samp);
                    llll_appendlong(*start, this_samp);
                }
            }
        }
            break;
            
        case EARS_SPLIT_MODE_LIST:
        {
            *start = llll_get();
            *end = llll_get();
            if (x->params && x->params->l_size > 0) {
                double prev_samp = 0;
                double this_samp = 0;
                for (t_llllelem *el = x->params->l_head; el; el = el->l_next) {
                    this_samp = earsbufobj_input_to_fsamps(e_ob, hatom_getdouble(&el->l_hatom), buf);
                    if (this_samp < size_samps) {
                        if (this_samp > prev_samp) {
                            llll_appendlong(*start, prev_samp);
                            llll_appendlong(*end, this_samp);
                        }
                    } else {
                        llll_appendlong(*start, prev_samp);
                        llll_appendlong(*end, size_samps);
                        break;
                    }
                    prev_samp = this_samp;
                }
                if (this_samp < size_samps) {
                    llll_appendlong(*start, this_samp);
                    llll_appendlong(*end, size_samps);
                }
            }
        }
            break;
            
        case EARS_SPLIT_MODE_SILENCE:
        {
            double thresh = (x->params && x->params->l_head ? earsbufobj_input_to_linear((t_earsbufobj *)x, hatom_getdouble(&x->params->l_head->l_hatom)) : 0);
            double min_dur = (x->params && x->params->l_head && x->params->l_head->l_next ? earsbufobj_input_to_samps((t_earsbufobj *)x, hatom_getdouble(&x->params->l_head->l_next->l_hatom), buf) : 0);

            t_ears_err err = ears_buffer_get_split_points_samps_silence((t_object *)x, buf, thresh, min_dur, start, end, x->e_keep_silence);
        }
            break;
            
        default:
            *start = llll_get();
            *end = llll_get();
            break;
    }
}




void buf_split_bang(t_buf_split *x)
{
    if (earsbufobj_get_num_inlet_stored_buffers((t_earsbufobj *)x, 0, false) > 0) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
        
        t_llll *start = NULL, *end = NULL;
        buf_split_get_splitpoints(x, in, &start, &end);
        
        long num_out_buffers = start->l_size;
        
        earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_out_buffers, true);
        
        long *start_array = NULL, *end_array = NULL;
        llll_to_long_array(start, &start_array);
        llll_to_long_array(end, &end_array);
        
        t_buffer_obj *dest[num_out_buffers];
        for (long i = 0; i < num_out_buffers; i++)
            dest[i] = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, i);
        
        ears_buffer_split((t_object *)x, in, dest, start_array, end_array, num_out_buffers);
        
        t_llll *out = llll_get();
        t_llllelem *s, *e;
        t_atom temp;
        for (s = start->l_head, e = end->l_head; s && e; s = s->l_next, e = e->l_next) {
            t_llll *this_ll = llll_get();
            earsbufobj_samps_to_atom((t_earsbufobj *)x, hatom_getlong(&s->l_hatom), in, &temp);
            llll_appendatom(this_ll, &temp);
            earsbufobj_samps_to_atom((t_earsbufobj *)x, hatom_getlong(&e->l_hatom), in, &temp);
            llll_appendatom(this_ll, &temp);
            llll_appendllll(out, this_ll);
        }
        earsbufobj_outlet_llll((t_earsbufobj *)x, 1, out);
        llll_free(out);
        
        bach_freeptr(start_array);
        bach_freeptr(end_array);
        
        earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
    }
}


void buf_split_anything(t_buf_split *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 1, true);
                earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 0, hatom_getsym(&parsed->l_head->l_hatom));
                
                buf_split_bang(x);
            }
            
        } else if (inlet == 1) {
            llll_free(x->params);
            x->params = llll_clone(parsed);
        }
    }
    llll_free(parsed);
}


