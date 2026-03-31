/**
	@file
	ears.ebur128.c
 
	@name
	ears.ebur128~
 
	@realname
	ears.ebur128~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
    EBU R 128 statistics
 
	@description
    Implements the EBU R 128 standard for loudness normalisation

	@discussion
 
	@category
	ears basic
 
	@keywords
	gain, ebur128, loudness, lufs, lu
 
	@seealso
	ears.gain~, ears.dynamic~, ears.*~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"
#include "ebur128.h"


typedef struct _buf_ebur128 {
    t_earsbufobj       e_ob;
    
    char            true_peak;
    char            channelwise_peaks;
    
    t_llll          *channelmap;
} t_buf_ebur128;



// Prototypes
t_buf_ebur128*         buf_ebur128_new(t_symbol *s, short argc, t_atom *argv);
void			buf_ebur128_free(t_buf_ebur128 *x);
void			buf_ebur128_bang(t_buf_ebur128 *x);
void			buf_ebur128_anything(t_buf_ebur128 *x, t_symbol *msg, long ac, t_atom *av);

void buf_ebur128_assist(t_buf_ebur128 *x, void *b, long m, long a, char *s);
void buf_ebur128_inletinfo(t_buf_ebur128 *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(ebur128)

DEFINE_LLLL_ATTR_DEFAULT_GETTER(t_buf_ebur128, channelmap, buf_ebur128_getattr_channelmap);
DEFINE_LLLL_ATTR_DEFAULT_SETTER(t_buf_ebur128, channelmap, buf_ebur128_setattr_channelmap);

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.ebur128~",
                         (method)buf_ebur128_new,
                         (method)buf_ebur128_free,
                         sizeof(t_buf_ebur128),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the EBU R 128 specifics.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(ebur128)

    
    llllobj_class_add_out_attr(c, LLLL_OBJ_VANILLA);
    
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_ampunit_attr(c);

    CLASS_ATTR_CHAR(c, "truepeak", 0, t_buf_ebur128, true_peak);
    CLASS_ATTR_STYLE_LABEL(c,"truepeak",0,"onoff","Use True Peak Calculation");
    CLASS_ATTR_BASIC(c, "truepeak", 0);
    // @description Toggles true peak calculation (default: on).

    CLASS_ATTR_CHAR(c, "channelwisepeaks", 0, t_buf_ebur128, channelwise_peaks);
    CLASS_ATTR_STYLE_LABEL(c,"channelwisepeaks",0,"onoff","Calculate Peaks Channel-Wise");
    CLASS_ATTR_BASIC(c, "channelwisepeaks", 0);
    // @description Calculate Channel-wise Peaks (default: off).

    CLASS_ATTR_LLLL(c, "channelmap", 0, t_buf_ebur128, channelmap, buf_ebur128_getattr_channelmap, buf_ebur128_setattr_channelmap);
    CLASS_ATTR_STYLE_LABEL(c,"channelmap",0,"text","Channel Map");
    // @description Sets a custom channel map, if needed. One symbol per channel is expected; symbols can be:
    // "left", "right", "center", "leftsurround", "rightsurround", "dualmono".


    
    // llllobj_class_add_default_bach_attrs_and_methods(c, LLLL_OBJ_VANILLA);
 class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_ebur128_assist(t_buf_ebur128 *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        if (a == 0)
            sprintf(s, "double/llll: Global Loudness (LUFS)"); // @out 0 @type double/llll @digest Global loudness
        else if (a == 1)
            sprintf(s, "double/llll: Short-Term Loudness (LUFS)"); // @out 1 @type double/llll @digest Short-term loudness
        else if (a == 2)
            sprintf(s, "double/llll: Momentary Loudness (LUFS)"); // @out 2 @type double/llll @digest Momentary loudness
        else if (a == 3)
            sprintf(s, "double/llll: Loudness Range (LUFS)"); // @out 3 @type double/llll @digest Loudness range
        else if (a == 4) {
            const char *unit = ears_ampunit_to_abbrev((e_ears_ampunit)x->e_ob.l_ampunit);
            sprintf(s, "double/llll: Peak %s", unit); // @out 4 @type double/llll @digest True peak or sample peak depending on the <m>truepeak</m> attribute
        }
    }
}

void buf_ebur128_inletinfo(t_buf_ebur128 *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_ebur128 *buf_ebur128_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_ebur128 *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_ebur128*)object_alloc_debug(s_tag_class);
    if (x) {
        x->channelmap = llll_get();
        x->true_peak = true;
        x->channelwise_peaks = false;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE); // EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E", "44444", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_ebur128_free(t_buf_ebur128 *x)
{
    llll_free(x->channelmap);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_ebur128_bang(t_buf_ebur128 *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    t_llll *loudness_global_ll = llll_get();
    t_llll *loudness_momentary_ll = llll_get();
    t_llll *loudness_shortterm_ll = llll_get();
    t_llll *loudness_range_ll = llll_get();
    t_llll *peaks_ll = llll_get();
    
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);

    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
    
    ebur128_state** sts = (ebur128_state**)sysmem_newptr(num_buffers * sizeof(ebur128_state*));

    for (long count = 0; count < num_buffers; count++) {
        double loudness_global = -INFINITY, loudness_momentary = -INFINITY, loudness_shortterm = -INFINITY;
        double loudness_range = -INFINITY, peak = -INFINITY;
        t_llll *these_peaks_ll = x->channelwise_peaks ? llll_get() : NULL;

        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        
        t_atom_long    channelcount = buffer_getchannelcount(in);
        t_atom_long    framecount   = buffer_getframecount(in);
        t_atom_long    sr = (t_atom_long)buffer_getsamplerate(in);

        sts[count] = ebur128_init((int)channelcount,
                                  (long)sr,
                                  EBUR128_MODE_I | EBUR128_MODE_LRA | EBUR128_MODE_TRUE_PEAK | EBUR128_MODE_SAMPLE_PEAK);

        if (!sts[count]) {
            object_error((t_object *)x, "Could not calculate EBUR128 for buffer No. %ld", count+1);
        } else {
            
            if (x->channelmap && x->channelmap->l_size > 0) {
                int c = 0;
                for (t_llllelem *el = x->channelmap->l_head; el && c < channelcount; el = el->l_next, c++) {
                    t_symbol *s = hatom_getsym(&el->l_hatom);
                    int eburchtype = EBUR128_UNUSED;
                    if (s == _llllobj_sym_left || s == _llllobj_sym_l) {
                        eburchtype = EBUR128_LEFT;
                    } else if (s == _llllobj_sym_right || s == _llllobj_sym_r) {
                        eburchtype = EBUR128_RIGHT;
                    } else if (s == gensym("center") || s == gensym("c")) {
                        eburchtype = EBUR128_CENTER;
                    } else if (s == gensym("leftsurround") || s == gensym("ls")) {
                        eburchtype = EBUR128_LEFT_SURROUND;
                    } else if (s == gensym("rightrurround") || s == gensym("rs")) {
                        eburchtype = EBUR128_RIGHT_SURROUND;
                    } else if (s == gensym("dualmono") || s == gensym("dm")) {
                        eburchtype = EBUR128_DUAL_MONO;
                    }
                    ebur128_set_channel(sts[count], c, eburchtype);
                }
            }
            // set channel map?
            /* example: set channel map (note: see ebur128.h for the default map) */
            /*
            if (file_info.channels == 5) {
              ebur128_set_channel(sts[i], 0, EBUR128_LEFT);
              ebur128_set_channel(sts[i], 1, EBUR128_RIGHT);
              ebur128_set_channel(sts[i], 2, EBUR128_CENTER);
              ebur128_set_channel(sts[i], 3, EBUR128_LEFT_SURROUND);
              ebur128_set_channel(sts[i], 4, EBUR128_RIGHT_SURROUND);
            }
             */
            
            float *in_sample = ears_buffer_locksamples(in);
            
            if (!in_sample) {
//                err = EARS_ERR_CANT_READ;
                object_error((t_object *)x, EARS_ERROR_BUF_CANT_READ);
            } else {
/*                for (long f = 0; f < framecount; f += sr) { // one second at a time?
                    ebur128_add_frames_float(sts[count], in_sample + f * channelcount, (sr < framecount - f ? sr : framecount - f));
                } */

                ebur128_add_frames_float(sts[count], in_sample, framecount);

                ebur128_loudness_global(sts[count], &loudness_global);
                ebur128_loudness_momentary(sts[count], &loudness_momentary);
                ebur128_loudness_shortterm(sts[count], &loudness_shortterm);
 
                ebur128_loudness_range(sts[count], &loudness_range);

                if (x->true_peak) {
                    for (int ch = 0; ch < channelcount; ch++) {
                        double ch_peak = -INFINITY;
                        if (ebur128_true_peak(sts[count], ch, &ch_peak) != EBUR128_SUCCESS) {
                            object_error((t_object *)x, "Could not calculate peak for buffer No. %ld", count+1);
                        } else {
                            ch_peak = ears_convert_ampunit(ch_peak, EARS_AMPUNIT_LINEAR, (e_ears_ampunit)x->e_ob.l_ampunit);
                        }
                        if (x->channelwise_peaks) {
                            llll_appenddouble(these_peaks_ll, ch_peak);
                        } else {
                            peak = MAX(peak, ch_peak);
                        }
                    }
                } else {
                    for (int ch = 0; ch < channelcount; ch++) {
                        double ch_peak = -INFINITY;
                        if (ebur128_sample_peak(sts[count], ch, &ch_peak) != EBUR128_SUCCESS) {
                            object_error((t_object *)x, "Could not calculate peak for buffer No. %ld", count+1);
                        } else {
                            ch_peak = ears_convert_ampunit(ch_peak, EARS_AMPUNIT_LINEAR, (e_ears_ampunit)x->e_ob.l_ampunit);
                        }
                        if (x->channelwise_peaks) {
                            llll_appenddouble(these_peaks_ll, ch_peak);
                        } else {
                            peak = MAX(peak, ch_peak);
                        }
                    }
                }
                
//                post("loudnes global %.2f", loudness_global);
//                post("loudnes range %.2f", loudness_range);
            }
            
            ears_buffer_unlocksamples(in);
        }

        llll_appenddouble(loudness_shortterm_ll, loudness_shortterm);
        llll_appenddouble(loudness_momentary_ll, loudness_momentary);
        llll_appenddouble(loudness_global_ll, loudness_global);
        llll_appenddouble(loudness_range_ll, loudness_range);

        if (x->channelwise_peaks) {
            llll_appendllll(peaks_ll, these_peaks_ll);
        } else {
            llll_appenddouble(peaks_ll, peak);
        }

        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }
    
//    ebur128_loudness_global_multiple(sts, (size_t) ac - 1, &loudness);
//    post("-----------\n%.2f LUFS\n", loudness);

    
    /* clean up */
    for (long count = 0; count < num_buffers; count++) {
      ebur128_destroy(&sts[count]);
    }
    sysmem_freeptr(sts);
    

    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    earsbufobj_outlet_llll((t_earsbufobj *)x, 4, peaks_ll);
    earsbufobj_outlet_llll((t_earsbufobj *)x, 3, loudness_range_ll);
    earsbufobj_outlet_llll((t_earsbufobj *)x, 2, loudness_momentary_ll);
    earsbufobj_outlet_llll((t_earsbufobj *)x, 1, loudness_shortterm_ll);
    earsbufobj_outlet_llll((t_earsbufobj *)x, 0, loudness_global_ll);
    llll_free(peaks_ll);
    llll_free(loudness_range_ll);
    llll_free(loudness_global_ll);
    llll_free(loudness_momentary_ll);
    llll_free(loudness_shortterm_ll);
}


void buf_ebur128_anything(t_buf_ebur128 *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long num_bufs = llll_get_num_symbols_root(parsed);

            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
//            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_bufs, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_ebur128_bang(x);
        }
    }
    llll_free(parsed);
}


