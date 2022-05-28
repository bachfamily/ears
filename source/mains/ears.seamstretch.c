/**
	@file
	ears.seamstretch.c
 
	@name
	ears.seamstretch~
 
	@realname
	ears.seamstretch~
 
	@type
	object
 
	@module
	ears

	@author
	Daniele Ghisi
 
	@digest
	Content-aware timestretch
 
	@description
    Resizes a buffer while preserving original content
 
	@discussion
    The module uses an implementation of a seam-carving algorithm.
 
	@category
    ears time and pitch
 
	@keywords
	buffer, seamstretch, carve, seam, timestretch, content, preserve
 
	@seealso
	ears.rubberband~, ears.soundtouch~
	
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

typedef struct _buf_seamstretch {
    t_earsbufobj       e_ob;
    long               e_energy_mode;
    t_llll             *e_howmuch;
    
    double             e_weighting_amount;
    double             e_weighting_stdev;
    
    long             e_compensate_phases;
} t_buf_seamstretch;



// Prototypes
t_buf_seamstretch*         buf_seamstretch_new(t_symbol *s, short argc, t_atom *argv);
void			buf_seamstretch_free(t_buf_seamstretch *x);
void			buf_seamstretch_bang(t_buf_seamstretch *x);
void			buf_seamstretch_anything(t_buf_seamstretch *x, t_symbol *msg, long ac, t_atom *av);

void buf_seamstretch_assist(t_buf_seamstretch *x, void *b, long m, long a, char *s);
void buf_seamstretch_inletinfo(t_buf_seamstretch *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(seamstretch)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.seamstretch~",
                         (method)buf_seamstretch_new,
                         (method)buf_seamstretch_free,
                         sizeof(t_buf_seamstretch),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a parameter (depending on the <m>mode</m>).
    // For "repeat" mode, the parameter is the number of repetitions.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(seamstretch)

    // @method number @digest Set seamstretch
    // @description A number in the second inlet sets the seamstretch parameter (depending on the <m>ampunit</m>).

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);

    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);
    earsbufobj_class_add_overlap_attr(c);
    earsbufobj_class_add_wintype_attr(c);
    earsbufobj_class_add_winstartfromzero_attr(c);


    
    CLASS_ATTR_LONG(c, "phasecompensation", 0, t_buf_seamstretch, e_compensate_phases);
    CLASS_ATTR_STYLE_LABEL(c,"phasecompensation",0,"onoff","Phase Compensation");
    CLASS_ATTR_BASIC(c, "phasecompensation", 0);
    // @description Toggles the ability to compensate phase changes.
    
    CLASS_ATTR_LONG(c, "energy", 0, t_buf_seamstretch, e_energy_mode);
    CLASS_ATTR_STYLE_LABEL(c,"energy",0,"enumindex","Energy Function");
    CLASS_ATTR_ENUMINDEX(c,"energy", 0, "Magnitude Gradient Magnitude");
    CLASS_ATTR_BASIC(c, "energy", 0);
    // @description Sets the normalization mode for the output buffer:
    // 0 = Never, does not normalize; 1 = Always, always normalizes to 1.; 2 = Overload Protection Only, only
    // normalizes to 1. if some samples exceed in modulo 1.

    CLASS_ATTR_DOUBLE(c, "uniformity", 0, t_buf_seamstretch, e_weighting_amount);
    CLASS_ATTR_STYLE_LABEL(c,"uniformity",0,"text","Force Uniformity");
    CLASS_ATTR_BASIC(c, "uniformity", 0);
    // @description Sets the amount of uniformity enforced by the algorithm. Zero means: no constraint for the seams:
    // whichever is in ; 1 means: enforce uniformity by penalizing seams in similar position position as previous ones
    // (via the uniformitywintime attribute)

    CLASS_ATTR_DOUBLE(c, "uniformitytimeinfluence", 0, t_buf_seamstretch, e_weighting_stdev);
    CLASS_ATTR_STYLE_LABEL(c,"uniformitytimeinfluence",0,"text","Uniformity Time Influence");
    // @description Sets the influence of uniformity constraints in the <m>antimeunit</m> enforced by the uniformity algorithm.
    // This corresponds to the standard deviation of the penalizing gaussian curve.

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_seamstretch_assist(t_buf_seamstretch *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Amplitude Buffers"); // @in 0 @type symbol/list/llll @digest Incoming amplitude buffer names
        else if (a == 1)                                      // @description One amplitude buffer for each channel
            sprintf(s, "symbol/list/llll: Phase Buffers"); // @in 1 @type symbol/list/llll @digest Incoming phase buffer names
        else                                                // @description One phase buffer for each channel
            sprintf(s, "number/llll: Stretch factor or duration"); // @in 2 @type number/llll @digest Stretch factor or duration
                                                                    // @description Sets the stretch factor or duration depending on the <m>timeunit</m> attribute.
    } else {
        if (a == 0)
            sprintf(s, "symbol/list: Amplitude Buffers"); // @out 0 @type symbol/list/llll @digest Output amplitude buffer names
        else if (a == 1)                                       // @description One amplitude buffer for each channel
            sprintf(s, "symbol/list: Phase Buffers"); // @out 1 @type symbol/list/llll @digest Incoming phase buffer names
                                                           // @description One phase buffer for each channel
        else
            sprintf(s, "list: Initial Energy Map And Seams"); // @out 2 @type list @digest Energy map and seams
                                                    // @description Outputs two buffer: the used energy map at the first iteration and the position of the carved seams
    }
}

void buf_seamstretch_inletinfo(t_buf_seamstretch *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_seamstretch *buf_seamstretch_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_seamstretch *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_seamstretch*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_howmuch = llll_from_text_buf("1.");
        x->e_energy_mode = EARS_SEAM_CARVE_MODE_MAGNITUDE;
        x->e_weighting_amount = 0;
        x->e_weighting_stdev = 44100; //in the <antimeunit>
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name timestretch @optional 1 @type number
        // @digest Timestretch factor or duration
        // @description Sets the stretch factor or duration depending on the <m>timeunit</m> attribute.

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            llll_free(x->e_howmuch);
            x->e_howmuch = llll_clone(args);
        }

        x->e_ob.l_timeunit = EARS_TIMEUNIT_DURATION_RATIO;
        x->e_ob.a_wintype = gensym("sqrthann");

        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_seamstretch_free(t_buf_seamstretch *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}


/*
void buf_seamstretch_bang(t_buf_seamstretch *x)
{
    long num_buffers = MIN(earsbufobj_get_instore_size((t_earsbufobj *)x, 0), earsbufobj_get_instore_size((t_earsbufobj *)x, 1));
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 1, num_buffers, true);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 2, 2, true);

    if (num_buffers > 0) {
        earsbufobj_updateprogress((t_earsbufobj *)x, 0.);
        earsbufobj_mutex_lock((t_earsbufobj *)x);
        t_buffer_obj *in_amps[num_buffers]; // @ANDREA: I KNOW...
        t_buffer_obj *out_amps[num_buffers]; // @ANDREA: I KNOW...
        t_buffer_obj *in_phases[num_buffers]; // @ANDREA: I KNOW...
        t_buffer_obj *out_phases[num_buffers]; // @ANDREA: I KNOW...
        t_buffer_obj *energy_map = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 2, 0);
        t_buffer_obj *seam_path = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 2, 1);
        for (long count = 0; count < num_buffers; count++) {
            in_amps[count] = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
            in_phases[count] = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 1, count);
            out_amps[count] = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
            out_phases[count] = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, count);
        }

        long delta_samps = earsbufobj_time_to_durationdifference_samps((t_earsbufobj *)x, x->e_howmuch, in_amps[0], EARSBUFOBJ_CONVERSION_FLAG_USEORIGINALAUDIOSRFORSPECTRALBUFFERS);
        double framesize_samps = 2*(ears_buffer_get_numchannels((t_object *)x, in_amps[0])-1);
        double hopsize_samps = ears_spectralbuf_get_original_audio_sr((t_object *)x, in_amps[0]) * 1./ears_buffer_get_sr((t_object *)x, in_amps[0]);
        long delta_frames = (long)round(delta_samps / hopsize_samps);

        ears_buffer_spectral_seam_carve((t_object *)x, num_buffers, in_amps, in_phases, out_amps, out_phases, energy_map, seam_path, delta_frames, framesize_samps, hopsize_samps, x->e_energy_mode, (updateprogress_fn)earsbufobj_updateprogress, x->e_compensate_phases);
        
        earsbufobj_mutex_unlock((t_earsbufobj *)x);
        earsbufobj_updateprogress((t_earsbufobj *)x, 1.);

        earsbufobj_outlet_buffer((t_earsbufobj *)x, 2);
        earsbufobj_outlet_buffer((t_earsbufobj *)x, 1);
        earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
    }
}
*/


void buf_seamstretch_bang(t_buf_seamstretch *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
//    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 1, 2, true);

    earsbufobj_updateprogress((t_earsbufobj *)x, 0.);
    earsbufobj_mutex_lock((t_earsbufobj *)x);

    t_buffer_obj *in_amps = ears_buffer_make(NULL);
    t_buffer_obj *in_phases = ears_buffer_make(NULL);
    t_buffer_obj *out_amps = ears_buffer_make(NULL);
    t_buffer_obj *out_phases = ears_buffer_make(NULL);
    t_buffer_obj *tempchannel = ears_buffer_make(NULL);
    long fullspectrum = 0; // must be zero! we're only shifting the lower portion of spectrum
    long unitary = 1;
    
    t_llllelem *el_howmuch = x->e_howmuch->l_head;
    for (long count = 0; count < num_buffers; count++,
         el_howmuch = el_howmuch && el_howmuch->l_next ? el_howmuch->l_next : el_howmuch) {
//        t_buffer_obj *energy_map = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, 0);
//        t_buffer_obj *seam_path = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, 1);
        t_buffer_obj *inbuf = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *outbuf = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
        double howmuch = el_howmuch ? hatom_getdouble(&el_howmuch->l_hatom) : 1.;
        double audio_sr = ears_buffer_get_sr((t_object *)x, inbuf);
        
        long num_in_chans = ears_buffer_get_numchannels((t_object *)x, inbuf);
        
        ears_buffer_copy_format((t_object *)x, inbuf, outbuf);

        for (long c = 0; c < num_in_chans; c++) {

            ears_buffer_stft((t_object *)x, inbuf, NULL, c, in_amps, in_phases, x->e_ob.a_framesize, x->e_ob.a_hopsize, x->e_ob.a_wintype ? x->e_ob.a_wintype->s_name : NULL, false, true, fullspectrum, EARS_ANGLEUNIT_RADIANS, x->e_ob.a_winstartfromzero, unitary);
            
            long delta_samps = earsbufobj_time_to_durationdifference_samps((t_earsbufobj *)x, howmuch, in_amps, EARSBUFOBJ_CONVERSION_FLAG_USEORIGINALAUDIOSRFORSPECTRALBUFFERS);
            double framesize_samps = 2*(ears_buffer_get_numchannels((t_object *)x, in_amps)-1);
            double hopsize_samps = ears_spectralbuf_get_original_audio_sr((t_object *)x, in_amps) * 1./ears_buffer_get_sr((t_object *)x, in_amps);
            long delta_frames = (long)round(delta_samps / hopsize_samps);
            double weighting_stdev_frames = earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_weighting_stdev, in_amps, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS | EARSBUFOBJ_CONVERSION_FLAG_USEORIGINALAUDIOSRFORSPECTRALBUFFERS)/hopsize_samps;
            
//            t_buffer_obj *seam_path = ears_buffer_getobject(gensym("seams"));
//            t_buffer_obj *outampsok = ears_buffer_getobject(gensym("outamps"));
//            t_buffer_obj *tempchannelok = ears_buffer_getobject(gensym("tempchannel"));

            ears_buffer_spectral_seam_carve((t_object *)x, 1, &in_amps, &in_phases, &out_amps, &out_phases, NULL, NULL /*seam_path*/, delta_frames, framesize_samps, hopsize_samps, x->e_energy_mode, (updateprogress_fn)earsbufobj_updateprogress, x->e_compensate_phases, x->e_weighting_amount, weighting_stdev_frames);
            
            ears_buffer_istft((t_object *)x, 1, &out_amps, &out_phases, tempchannel, NULL,
                              x->e_ob.a_wintype && strncmp(x->e_ob.a_wintype->s_name, "sqrt", 4) == 0 ? x->e_ob.a_wintype->s_name : "rect", true, false, fullspectrum, EARS_ANGLEUNIT_RADIANS, audio_sr, x->e_ob.a_winstartfromzero, unitary, 0);

            if (c == 0) {
                ears_buffer_set_size_and_numchannels((t_object *)x, outbuf, ears_buffer_get_size_samps((t_object *)x, tempchannel), num_in_chans);
            }
            ears_buffer_copychannel((t_object *)x, tempchannel, 0, outbuf, c);
            buffer_setdirty(outbuf);
        }
        
    }

    ears_buffer_free(in_amps);
    ears_buffer_free(in_phases);
    ears_buffer_free(out_amps);
    ears_buffer_free(out_phases);
    ears_buffer_free(tempchannel);

    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    earsbufobj_updateprogress((t_earsbufobj *)x, 1.);
    
//    earsbufobj_outlet_buffer((t_earsbufobj *)x, 1);
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}



void buf_seamstretch_anything(t_buf_seamstretch *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long num_bufs = llll_get_num_symbols_root(parsed);
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, num_bufs, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, inlet, num_bufs, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, inlet);
            
            if (inlet == 0) {
                buf_seamstretch_bang(x);
            }
        } else {
            if (parsed->l_head) {
                earsbufobj_mutex_lock((t_earsbufobj *)x);
                llll_free(x->e_howmuch);
                x->e_howmuch = llll_clone(parsed);
                earsbufobj_mutex_unlock((t_earsbufobj *)x);
            }
        }
    }
    llll_free(parsed);
}


