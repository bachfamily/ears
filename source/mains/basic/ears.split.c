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
	ears basic
 
	@keywords
	buffer, split, chop, segment
 
	@seealso
	ears.slice~, ears.trim~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_split {
    t_earsbufobj        e_ob;
    
    long                e_mode; //< one of the e_ears_split_modes
    char                e_partial_segments; // output partial segments (for duration split)
    char                e_keep_silence; // keep silence (For silence and onset split)

    double              e_overlap; // < Overlap amount (depending on the time unit)
    
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

t_max_err buf_split_setattr_mode(t_buf_split *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            if (s == gensym("list") || s == gensym("List"))
                x->e_mode = EARS_SPLIT_MODE_LIST;
            else if (s == gensym("duration") || s == gensym("Duration"))
                x->e_mode = EARS_SPLIT_MODE_DURATION;
            else if (s == gensym("number") || s == gensym("Number"))
                x->e_mode = EARS_SPLIT_MODE_NUMBER;
            else if (s == gensym("silence") || s == gensym("Silence"))
                x->e_mode = EARS_SPLIT_MODE_SILENCE;
            else if (s == gensym("onset") || s == gensym("Onset"))
                x->e_mode = EARS_SPLIT_MODE_ONSET;
            else
                object_error((t_object *)x, "Unsupported attribute value.");
        } else if (atom_gettype(argv) == A_LONG){
            x->e_mode = atom_getlong(argv);
        } else {
            object_error((t_object *)x, "Unsupported attribute value.");
        }
    }
    return MAX_ERR_NONE;
}

void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return;
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
    // output from the left outlet, and left portion of buffers is output from the left outlet. <br />
    // A number, list or llll in the second inlet sets the split parameter(s): <br />
    // in Duration mode: the duration (unit set by <m>timeunit</m> attribute); <br />
    // in Number mode: the number of output segments; <br />
    // in List mode: the position of split points (unit set by <m>timeunit</m> attribute); <br />
    // in Silence mode: amplitude threshold for silence (unit set by <m>ampunit</m> attribute)
    // and the minimum silence duration (unit set by <m>timeunit</m> attribute); <br />
    // in Onset mode: amplitude threshold for attacks (unit set by <m>ampunit</m> attribute)
    // and the minimum distance between attacks (unit set by <m>timeunit</m> attribute); <br />

    // @method number @digest Set split parameter
    // @description A number in the second inlet sets the split parameter (see <m>list/llll</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(split)


    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

    earsbufobj_class_add_polyout_attr(c);

    CLASS_ATTR_LONG(c, "mode", 0, t_buf_split, e_mode);
    CLASS_ATTR_STYLE_LABEL(c,"mode",0,"enumindex","Split Mode");
    CLASS_ATTR_ENUMINDEX(c,"mode", 0, "Duration Number List Silence Onset");
    CLASS_ATTR_ACCESSORS(c, "mode", NULL, buf_split_setattr_mode);
    CLASS_ATTR_BASIC(c, "mode", 0);
    // @description Sets the split mode: <br />
    // 0 (Duration): buffer is split into equally long chunks, whose duration is set; <br />
    // 1 (Number): buffer is split into a fixed number of buffers (such number is set); <br />
    // 2 (List): buffer is split via a series of split points given as parameter (right inlet); <br />
    // 3 (Silence): buffer is split according to silence regions. <br />
    // 4 (Onset): buffer is split according to onset thresholds (set in the right inlet). <br />
    // Symbols: "duration", "number", "list" and "silence" can be used while defining the attribute in the object box.

    CLASS_ATTR_CHAR(c, "partials", 0, t_buf_split, e_partial_segments);
    CLASS_ATTR_STYLE_LABEL(c,"partials",0,"onoff","Output Partial Segments");
    CLASS_ATTR_BASIC(c, "partials", 0);
    // @description Toggles the ability to output partial segments in <b>Duration</b> <m>mode</m>.

    CLASS_ATTR_CHAR(c, "keepsilence", 0, t_buf_split, e_keep_silence);
    CLASS_ATTR_STYLE_LABEL(c,"keepsilence",0,"onoff","Keep Silence");
    CLASS_ATTR_BASIC(c, "keepsilence", 0);
    // @description Toggles the ability to keep the trailing silence in <b>Silence</b> <m>mode</m>,
    // or the portion of buffer before the first attack in <b>Onset</b> mode.

    CLASS_ATTR_DOUBLE(c, "overlap", 0, t_buf_split, e_overlap);
    CLASS_ATTR_STYLE_LABEL(c,"overlap",0,"text","Overlap Time");
    CLASS_ATTR_BASIC(c, "overlap", 0);
    // @description Sets the overlap time (for <b>Duration</b>, <b>Number</b> modes only),
    // in the unit specified by the <m>timeunit</m> attribute.

    
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
            sprintf(s, "number/list/llll: Algorithm parameters (segment duration, or number, or list of split points, or algorithm threshold"); // @in 1 @type number/llll @digest Depending on the <m>mode</m> this can be the segment duration,
                            // the number of segments, the list of split points (either a plain list, or a list of wrapped elements
                            // defining the start and the end of each segment), or the silence threshold
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

            if (num_buffers <= 0) {
                object_error((t_object *)e_ob, "Cannot set number of segments to a negative number or zero.");
                object_error((t_object *)e_ob, "    Defaulting to a single segment.");
                num_buffers = 1;
            }

            *start = llll_get();
            *end = llll_get();
            llll_appendlong(*start, 0);

            double overlap_samps = earsbufobj_time_to_fsamps(e_ob, x->e_overlap, buf);
            double duration_samps = (size_samps + (num_buffers - 1) * overlap_samps)/num_buffers;
            
            if (overlap_samps >= duration_samps) {
                object_error((t_object *)e_ob, "Overlap duration cannot be greater than or equal to the segment duration.");
                object_error((t_object *)e_ob, "    Setting overlap to zero.");
                overlap_samps = 0;
                duration_samps = size_samps/num_buffers;
            }

            for (long i = 1; i < num_buffers; i++) {
                long this_samp = (long)round(i * (duration_samps - overlap_samps));
                llll_appendlong(*end, this_samp + overlap_samps);
                llll_appendlong(*start, this_samp);
            }
            llll_appendlong(*end, size_samps);
        }
            break;
            
        case EARS_SPLIT_MODE_DURATION:
        {
            double duration_samps = earsbufobj_time_to_fsamps(e_ob, x->params && x->params->l_head ? hatom_getdouble(&x->params->l_head->l_hatom) : 0, buf);
            double overlap_samps = earsbufobj_time_to_fsamps(e_ob, x->e_overlap, buf);
            
            if (overlap_samps >= duration_samps) {
                object_error((t_object *)e_ob, "Overlap duration cannot be greater than or equal to the segment duration.");
                object_error((t_object *)e_ob, "    Setting overlap to zero.");
                overlap_samps = 0;
            }

            *start = llll_get();
            *end = llll_get();
            llll_appendlong(*start, 0);
            for (double c = duration_samps - overlap_samps; c < size_samps; c += duration_samps - overlap_samps) {
                long this_samp = (long)round(c);
                if (this_samp + duration_samps > size_samps) { // last shorter one
                    llll_appendlong(*end, this_samp + overlap_samps);
                    if (x->e_partial_segments && this_samp + overlap_samps < size_samps) {
                        llll_appendlong(*start, this_samp + overlap_samps);
                        llll_appendlong(*end, size_samps);
                    }
                } else {
                    llll_appendlong(*end, this_samp + overlap_samps);
                    llll_appendlong(*start, this_samp);
                }
            }
            if ((*end)->l_size < (*start)->l_size)
                llll_appendlong(*end, size_samps);
        }
            break;
            
        case EARS_SPLIT_MODE_LIST:
        {
            *start = llll_get();
            *end = llll_get();
            if (x->params && x->params->l_size > 0 && x->params->l_depth == 1) { // plain list of elements
                double prev_samp = 0;
                double this_samp = 0;
                for (t_llllelem *el = x->params->l_head; el; el = el->l_next) {
                    double overlap_samps = earsbufobj_time_to_fsamps(e_ob, x->e_overlap, buf);
                    this_samp = earsbufobj_time_to_fsamps(e_ob, hatom_getdouble(&el->l_hatom), buf);
                    if (this_samp < size_samps) {
                        if (this_samp > prev_samp) {
                            llll_appendlong(*start, prev_samp);
                            llll_appendlong(*end, MIN(this_samp + overlap_samps, size_samps));
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
            } else if (x->params && x->params->l_size > 0 && x->params->l_depth == 2) { // list with start/end
                for (t_llllelem *el = x->params->l_head; el; el = el->l_next) {
                    if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                        t_llll *ll = hatom_getllll(&el->l_hatom);
                        if (ll && ll->l_head && ll->l_head->l_next) {
                            double start_samp = earsbufobj_time_to_fsamps(e_ob, hatom_getdouble(&ll->l_head->l_hatom), buf);
                            double end_samp = earsbufobj_time_to_fsamps(e_ob, hatom_getdouble(&ll->l_head->l_next->l_hatom), buf);
                            if (start_samp < end_samp) {
                                llll_appendlong(*start, start_samp);
                                llll_appendlong(*end, end_samp);
                            } else {
                                object_warn((t_object *)e_ob, "End point comes before (or coincides with) starting point.");
                                object_warn((t_object *)e_ob, "    Ignoring segment.");
                            }
                        } else {
                            object_warn((t_object *)e_ob, "Wrong syntax in split point list.");
                        }
                    } else {
                        object_warn((t_object *)e_ob, "Wrong syntax in split point list.");
                    }
                }
            }
        }
            break;
            
        case EARS_SPLIT_MODE_SILENCE:
        {
            double thresh = (x->params && x->params->l_head ? earsbufobj_amplitude_to_linear((t_earsbufobj *)x, hatom_getdouble(&x->params->l_head->l_hatom)) : 0);
            double min_dur = (x->params && x->params->l_head && x->params->l_head->l_next ? earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&x->params->l_head->l_next->l_hatom), buf) : 0);

            t_ears_err err = ears_buffer_get_split_points_samps_silence((t_object *)x, buf, thresh, min_dur, start, end, x->e_keep_silence);
            if (err)
                object_error((t_object *)x, "Error finding split points.");
        }
            break;

            
        case EARS_SPLIT_MODE_ONSET:
        {
            double attackthresh = (x->params && x->params->l_head ? earsbufobj_amplitude_to_linear((t_earsbufobj *)x, hatom_getdouble(&x->params->l_head->l_hatom)) : 1.);
            double releasethresh = (x->params && x->params->l_head && x->params->l_head->l_next ? earsbufobj_amplitude_to_linear((t_earsbufobj *)x, hatom_getdouble(&x->params->l_head->l_next->l_hatom)) : 0.5);
            double min_dur = (x->params && x->params->l_size >= 3 ? earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&x->params->l_head->l_next->l_next->l_hatom), buf) : 0);
            double lookahead = (x->params && x->params->l_size >= 4 ? earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&x->params->l_head->l_next->l_next->l_next->l_hatom), buf) : 0);
            double smooth = (x->params && x->params->l_size >= 5 ? earsbufobj_time_to_samps((t_earsbufobj *)x, hatom_getdouble(&x->params->l_head->l_next->l_next->l_next->l_next->l_hatom), buf) : 0);

            t_ears_err err = ears_buffer_get_split_points_samps_onset((t_object *)x, buf, attackthresh, releasethresh, min_dur, lookahead, smooth, start, end, x->e_keep_silence);
            if (err)
                object_error((t_object *)x, "Error finding split points.");
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
        
        if (!start_array || !end_array) {
            object_error((t_object *)x, "Error while splitting");
            if (start_array)
                bach_freeptr(start_array);
            if (end_array)
                bach_freeptr(end_array);
            return;
        }
        
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
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 1, true);
                earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 0, hatom_getsym(&parsed->l_head->l_hatom));
                
                buf_split_bang(x);
            } else {
                object_error((t_object *)x, EARS_ERROR_BUF_NOT_A_BUFFER);
            }
            
        } else if (inlet == 1) {
            llll_free(x->params);
            x->params = llll_clone(parsed);
        }
    }
    llll_free(parsed);
}


