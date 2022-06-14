/**
	@file
	ears.timesquash.c
 
	@name
	ears.timesquash~
 
	@realname
	ears.timesquash~
 
	@type
	object
 
	@module
	ears

	@author
	Daniele Ghisi
 
	@digest
	Content-aware time abridgement
 
	@description
    Resizes a buffer while preserving original content
 
	@discussion
    The module uses an implementation of a seam-carving algorithm.
 
	@category
    ears time and pitch
 
	@keywords
	buffer, timesquash, carve, seam, timestretch, content, preserve
 
	@seealso
	ears.rubberband~, ears.soundtouch~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"
#include "ears.spectral.h"



typedef struct _buf_timesquash {
    t_earsbufobj       e_ob;

    t_llll             *e_howmuch;
    char               mode;

    // Time-algorithm
    double             timeblock;
    double             xfade;
    char               xfade_type;
    double             xfade_curve;
    long               extended_output;

    // Spectral algorithm:
    long               e_energy_mode;
    long             e_compensate_phases;
    double            e_forward_energy;
    long              e_forward_energy_type;
    long              e_forward_energy_embed;
    double            e_regularization;
    long              e_batch_size;
    long              e_batch_interrupt;
    
    // Griffin-Lim
    long            e_num_griffin_lim_iter;
    long            e_griffin_lim_bleed_width;
    char            e_griffin_lim_vertical;
    char            e_griffin_lim_randomize;
} t_buf_timesquash;



// Prototypes
t_buf_timesquash*         buf_timesquash_new(t_symbol *s, short argc, t_atom *argv);
void			buf_timesquash_free(t_buf_timesquash *x);
void			buf_timesquash_bang(t_buf_timesquash *x);
void			buf_timesquash_anything(t_buf_timesquash *x, t_symbol *msg, long ac, t_atom *av);

void buf_timesquash_assist(t_buf_timesquash *x, void *b, long m, long a, char *s);
void buf_timesquash_inletinfo(t_buf_timesquash *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(timesquash)

/**********************************************************************/
// Class Definition and Life Cycle

t_max_err t_buf_timesquash_setattr_extout(t_buf_timesquash *x, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (!x->e_ob.l_is_creating)
            object_error((t_object *)x, "The 'extout' attribute can only be set in the object box.");
        else if (atom_gettype(argv) == A_LONG)
            x->extended_output = atom_getlong(argv);
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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.timesquash~",
                         (method)buf_timesquash_new,
                         (method)buf_timesquash_free,
                         sizeof(t_buf_timesquash),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a parameter (depending on the <m>mode</m>).
    // For "repeat" mode, the parameter is the number of repetitions.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(timesquash)

    // @method number @digest Set timesquash
    // @description A number in the second inlet sets the timesquash parameter (depending on the <m>ampunit</m>).

    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_slopemapping_attr(c);

    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);
    earsbufobj_class_add_overlap_attr(c);
    earsbufobj_class_add_wintype_attr_ansyn(c);
    earsbufobj_class_add_winstartfromzero_attr(c);

    CLASS_ATTR_LONG(c, "extout",    0,    t_buf_timesquash, extended_output);
    CLASS_ATTR_STYLE_LABEL(c, "extout", 0, "onoff", "Extended Output");
    CLASS_ATTR_ACCESSORS(c, "extout", NULL, t_buf_timesquash_setattr_extout);
    // @description Also output seams and energy matrices from the right outlet.
    // This attribute is static and can only be set inside the object box.

    CLASS_ATTR_CHAR(c, "mode", 0, t_buf_timesquash, mode);
    CLASS_ATTR_STYLE_LABEL(c,"mode",0,"enumindex","Algorithm Mode");
    CLASS_ATTR_ENUMINDEX(c,"mode", 0, "Time-Domain Frequency-Domain");
    CLASS_ATTR_BASIC(c, "mode", 0);
    // @description Sets the working mode: either in the Time-Domain or in the Frequency-Domain (default)


    CLASS_ATTR_DOUBLE(c, "timeblock", 0, t_buf_timesquash, timeblock);
    CLASS_ATTR_STYLE_LABEL(c,"timeblock",0,"text","Time Block");
    CLASS_ATTR_BASIC(c, "timeblock", 0);
    CLASS_ATTR_CATEGORY(c, "timeblock", 0, "Time-Domain Algorithm");
    // @description Sets the duration of each time block to be removed in the <m>antimeunit</m>

    
    CLASS_ATTR_CHAR(c, "xfadetype", 0, t_buf_timesquash, xfade_type);
    CLASS_ATTR_STYLE_LABEL(c,"xfadetype",0,"enumindex","Crossfade Type");
    CLASS_ATTR_ENUMINDEX(c,"xfadetype", 0, "None Linear Sine (Equal Power) Curve S-Curve");
    CLASS_ATTR_BASIC(c, "xfadetype", 0);
    CLASS_ATTR_CATEGORY(c, "xfadetype", 0, "Time-Domain Algorithm");
    // @description Sets the cross fade type: 0 = None, 1 = Linear, 2 = Sine (Equal Power, default), 3 = Curve, 4 = S-Curve

    
    CLASS_ATTR_DOUBLE(c, "xfadecurve", 0, t_buf_timesquash, xfade_curve);
    CLASS_ATTR_STYLE_LABEL(c,"xfadecurve",0,"text","Crossfade Curve");
    CLASS_ATTR_CATEGORY(c, "xfadecurve", 0, "Time-Domain Algorithm");
    // @description Sets the curve parameter for the crossfade (for fades of type Curve and S-Curve only).
    // The parameters goes from -1 to 1, 0 being linear (default).

    CLASS_ATTR_DOUBLE(c, "xfade", 0, t_buf_timesquash, xfade);
    CLASS_ATTR_STYLE_LABEL(c,"xfade",0,"text","Crossfade Duration");
    CLASS_ATTR_BASIC(c, "xfade", 0);
    CLASS_ATTR_CATEGORY(c, "xfade", 0, "Time-Domain Algorithm");
    // @description Sets the duration of the crossfade, whose unit is set via the <m>antimeunit</m> attribute.

    
    
    CLASS_ATTR_DOUBLE(c, "forwardenergy", 0, t_buf_timesquash, e_forward_energy);
    CLASS_ATTR_STYLE_LABEL(c,"forwardenergy",0,"text","Forward Energy Contribution");
    CLASS_ATTR_CATEGORY(c, "forwardenergy", 0, "Frequency-Domain Algorithm");
    // @description Sets the amount of 'forward energy' contribution, i.e. the energy that
    // is obtained by neighboring pixel when a seam pixel is removed.

    
    CLASS_ATTR_LONG(c, "forwardenergytype", 0, t_buf_timesquash, e_forward_energy_type);
    CLASS_ATTR_STYLE_LABEL(c,"forwardenergytype",0,"text","Forward Energy Type");
    CLASS_ATTR_ENUMINDEX(c,"forwardenergytype", 0, "Gradient L1 Norm Energy L2 Norm");
    CLASS_ATTR_CATEGORY(c, "forwardenergytype", 0, "Frequency-Domain Algorithm");
    // @description Sets the forward energy type.

    CLASS_ATTR_LONG(c, "forwardenergyembed", 0, t_buf_timesquash, e_forward_energy_embed);
    CLASS_ATTR_STYLE_LABEL(c,"forwardenergyembed",0,"onoff","Embed Forward Energy In Cumulative Energy");
    CLASS_ATTR_CATEGORY(c, "forwardenergyembed", 0, "Frequency-Domain Algorithm");
    // @description Toggles the ability to embed the forward energy in the cumulative energy matrix.
    // If zero, the forward energy is used as additional weight when finding the best path
    // in the cumulative energy matrix.

    
    CLASS_ATTR_DOUBLE(c, "forwardenergy", 0, t_buf_timesquash, e_forward_energy);
    CLASS_ATTR_STYLE_LABEL(c,"forwardenergy",0,"text","Forward Energy Contribution");
    CLASS_ATTR_BASIC(c, "forwardenergy", 0);
    CLASS_ATTR_CATEGORY(c, "forwardenergy", 0, "Frequency-Domain Algorithm");
    // @description Sets the amount of 'forward energy' contribution, i.e. the energy that
    // is obtained by neighboring pixel when a seam pixel is removed.

    
    
    CLASS_ATTR_DOUBLE(c, "regularization", 0, t_buf_timesquash, e_regularization);
    CLASS_ATTR_STYLE_LABEL(c,"regularization",0,"text","Regularization");
    CLASS_ATTR_BASIC(c, "regularization", 0);
    CLASS_ATTR_FILTER_CLIP(c, "reguluarization", 0, 1);
    CLASS_ATTR_CATEGORY(c, "regularization", 0, "Frequency-Domain Algorithm");
    // @description A value between 0 and 1 that sets how much the energy of a bin smears on the
    // ones above and below. For a regularization value of 1, energy is constant vertically
    // across the spectrum, and hence all seams are forced to be purely vertical.

    
    CLASS_ATTR_LONG(c, "phasehandling", 0, t_buf_timesquash, e_compensate_phases);
    CLASS_ATTR_STYLE_LABEL(c,"phasehandling",0,"enumindex","Phase Handling");
    CLASS_ATTR_ENUMINDEX(c,"phasehandling", 0, "Keep Compensate Griffin-Lim");
    CLASS_ATTR_BASIC(c, "phasehandling", 0);
    CLASS_ATTR_CATEGORY(c, "phasehandling", 0, "Frequency-Domain Algorithm");
    // @description Toggles the ability to compensate phase changes.
    
    CLASS_ATTR_LONG(c, "energy", 0, t_buf_timesquash, e_energy_mode);
    CLASS_ATTR_STYLE_LABEL(c,"energy",0,"enumindex","Energy Function");
    CLASS_ATTR_ENUMINDEX(c,"energy", 0, "Magnitude Gradient Magnitude Sobel");
    CLASS_ATTR_CATEGORY(c, "energy", 0, "Frequency-Domain Algorithm");
    CLASS_ATTR_BASIC(c, "energy", 0);
    // @description Sets the energy function used for computing seams.

    CLASS_ATTR_LONG(c, "batchsize", 0, t_buf_timesquash, e_batch_size);
    CLASS_ATTR_STYLE_LABEL(c,"batchsize",0,"text","Batch Size");
    CLASS_ATTR_CATEGORY(c, "batchsize", 0, "Frequency-Domain Algorithm");
    CLASS_ATTR_FILTER_MIN(c, "batchsize", 1);
    // @description Sets the number of seams processed in batch.

    CLASS_ATTR_LONG(c, "batchinterrupt", 0, t_buf_timesquash, e_batch_interrupt);
    CLASS_ATTR_STYLE_LABEL(c,"batchinterrupt",0,"onoff","Interrupt Batch When Crossing");
    CLASS_ATTR_CATEGORY(c, "batchinterrupt", 0, "Frequency-Domain Algorithm");
    // @description Toggles the ability to end the batch whenever a seam crossing is encountered.
    // In this case, a new batch will begin.

    
//    CLASS_ATTR_DOUBLE(c, "uniformity", 0, t_buf_timesquash, e_weighting_amount);
//    CLASS_ATTR_STYLE_LABEL(c,"uniformity",0,"text","Force Uniformity");
//    CLASS_ATTR_BASIC(c, "uniformity", 0);
    // @exclude all
    // @description Sets the amount of uniformity enforced by the algorithm. Zero means: no constraint for the seams:
    // whichever is in ; 1 means: enforce uniformity by penalizing seams in similar position position as previous ones
    // (via the uniformitywintime attribute)

//    CLASS_ATTR_DOUBLE(c, "uniformitytimeinfluence", 0, t_buf_timesquash, e_weighting_stdev);
//    CLASS_ATTR_STYLE_LABEL(c,"uniformitytimeinfluence",0,"text","Uniformity Time Influence");
    // @exclude all
    // @description Sets the influence of uniformity constraints in the <m>antimeunit</m> enforced by the uniformity algorithm.
    // This corresponds to the standard deviation of the penalizing gaussian curve.


    
    CLASS_ATTR_LONG(c, "glnumiter", 0, t_buf_timesquash, e_num_griffin_lim_iter);
    CLASS_ATTR_STYLE_LABEL(c,"glnumiter",0,"text","Number of Griffin-Lim Iterations");
    CLASS_ATTR_CATEGORY(c, "glnumiter", 0, "Phase Retrieval");
    // @description Sets the number of Griffin-Lim iterations, in case the corresponding method is chosen for <m>phasehandling</m>.

    CLASS_ATTR_LONG(c, "glbleed", 0, t_buf_timesquash, e_griffin_lim_bleed_width);
    CLASS_ATTR_STYLE_LABEL(c,"glbleed",0,"text","Bleed Extension for Griffin-Lim Algorithm");
    CLASS_ATTR_CATEGORY(c, "glbleed", 0, "Phase Retrieval");
    CLASS_ATTR_FILTER_MIN(c, "glbleed", 0);
    // @description Sets the number of frames of invalidation for the Griffin-Lim Algorithm, in case the corresponding method is chosen for <m>phasehandling</m>.

    CLASS_ATTR_CHAR(c, "glrect", 0, t_buf_timesquash, e_griffin_lim_vertical);
    CLASS_ATTR_STYLE_LABEL(c,"glrect",0,"onoff","Rectangular Griffin-Lim Regions");
    CLASS_ATTR_CATEGORY(c, "glrect", 0, "Phase Retrieval");
    // @description Toggles the ability to invalidate whole rectangular regions whose phases are recomputed via the Griffin-Lim algorithm,
    // in case the corresponding method is chosen for <m>phasehandling</m>.

    CLASS_ATTR_CHAR(c, "glrandomize", 0, t_buf_timesquash, e_griffin_lim_randomize);
    CLASS_ATTR_STYLE_LABEL(c,"glrandomize",0,"onoff","Randomize Griffin-Lim Phases");
    CLASS_ATTR_CATEGORY(c, "glrandomize", 0, "Phase Retrieval");
    // @description Toggles the ability to randomize the phases of the regions subject to the Griffin-Lim algorithm,
    // in case the corresponding method is chosen for <m>phasehandling</m>.

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_timesquash_assist(t_buf_timesquash *x, void *b, long m, long a, char *s)
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
            sprintf(s, "list: Seams and Initial Energy Maps"); // @out 2 @type list @digest Seams, energy map and cumulative energy map
                                                    // @description If the <m>extout</m> attribute is set, from the right outlet come three buffers, containing:
                                                    // the position of the carved seams, the energy map at the first iteration and the cumulative energy map at the first iteration
    }
}

void buf_timesquash_inletinfo(t_buf_timesquash *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_timesquash *buf_timesquash_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_timesquash *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_timesquash*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_howmuch = llll_from_text_buf("1.");
        x->e_energy_mode = EARS_SEAM_CARVE_MODE_SOBEL;
        x->mode = 1;
        x->timeblock = 4096; // samps, by default, in the <antimeunit>
        x->xfade = 4096; // samps, by default, in the <antimeunit>
        x->xfade_type = EARS_FADE_SINE;
        x->xfade_curve = 0;
        x->e_batch_size = 1;
        x->extended_output = 0;
        
        x->e_griffin_lim_vertical = 1,
        x->e_griffin_lim_randomize = 0,
        x->e_griffin_lim_bleed_width = 4;
        x->e_num_griffin_lim_iter = 10;
        x->e_compensate_phases = 2; // griffin-lim
        x->e_forward_energy = 1.; // consider forward energy
        x->e_forward_energy_embed = 1;
        x->e_forward_energy_type = 1;
        x->e_regularization = 0.01; // 10 bins on each side
        
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
        x->e_ob.a_wintype_ansyn[0] = gensym("sqrthann");
        x->e_ob.a_wintype_ansyn[1] = gensym("sqrthann");
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", x->extended_output ? "EE" : "E", names);

        object_attr_setdisabled((t_object *)x, gensym("extout"), 1);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_timesquash_free(t_buf_timesquash *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}


/*
void buf_timesquash_bang(t_buf_timesquash *x)
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



void buf_timesquash_bang(t_buf_timesquash *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long mode = x->mode;
    bool extended = x->extended_output;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    if (extended)
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 1, 3, true);

    earsbufobj_updateprogress((t_earsbufobj *)x, 0.);
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    
    if (mode == 0) { // time-domain
        t_llllelem *el_howmuch = x->e_howmuch->l_head;
        for (long count = 0; count < num_buffers; count++,
             el_howmuch = el_howmuch && el_howmuch->l_next ? el_howmuch->l_next : el_howmuch) {
            //        t_buffer_obj *energy_map = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, 0);
            //        t_buffer_obj *seam_path = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, 1);
            t_buffer_obj *inbuf = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
            t_buffer_obj *outbuf = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);
            
            double howmuch = el_howmuch ? hatom_getdouble(&el_howmuch->l_hatom) : 1.;
            
            ears_buffer_squash_waveform((t_object *)x, inbuf, outbuf,
                                        earsbufobj_time_to_durationdifference_samps((t_earsbufobj *)x, howmuch, inbuf),
                                        earsbufobj_time_to_samps((t_earsbufobj *)x, x->timeblock, inbuf, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS),
                                        earsbufobj_time_to_samps((t_earsbufobj *)x, x->e_ob.a_framesize, inbuf, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS),
                                        earsbufobj_time_to_samps((t_earsbufobj *)x, x->xfade, inbuf, EARSBUFOBJ_CONVERSION_FLAG_ISANALYSIS),
                                        (e_ears_fade_types)x->xfade_type, x->xfade_curve, (e_slope_mapping)x->e_ob.l_slopemapping);
        }
        

    } else if (mode == 1) { // frequency-domain
        
        long fullspectrum = 0; // must be zero! we're only shifting the lower portion of spectrum
        long unitary = 1;
        long num_griffin_lim_iter = x->e_num_griffin_lim_iter;
        long griffin_lim_invalidate_width = x->e_griffin_lim_bleed_width;
        bool griffin_lim_vertical = x->e_griffin_lim_vertical;
        bool griffin_lim_randomize = x->e_griffin_lim_randomize;

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
            
            if (num_in_chans <= 0) {
                object_error((t_object *)x, "Buffer has no channels!");
                continue;
            }
            
            ears_buffer_copy_format((t_object *)x, inbuf, outbuf);
            
            t_buffer_obj **in_amps = (t_buffer_obj **)bach_newptr(num_in_chans * sizeof(t_buffer_obj *));
            t_buffer_obj **out_amps = (t_buffer_obj **)bach_newptr(num_in_chans * sizeof(t_buffer_obj *));
            t_buffer_obj **in_phases = (t_buffer_obj **)bach_newptr(num_in_chans * sizeof(t_buffer_obj *));
            t_buffer_obj **out_phases = (t_buffer_obj **)bach_newptr(num_in_chans * sizeof(t_buffer_obj *));

            earsbufobj_updateprogress((t_earsbufobj *)x, 1.); // just to have a rolling line for the preprocessing

            for (long c = 0; c < num_in_chans; c++) {
                in_amps[c] = ears_buffer_make(NULL);
                in_phases[c] = ears_buffer_make(NULL);
                out_amps[c] = ears_buffer_make(NULL);
                out_phases[c] = ears_buffer_make(NULL);

                ears_buffer_stft((t_object *)x, inbuf, NULL, c, in_amps[c], in_phases[c], x->e_ob.a_framesize, x->e_ob.a_hopsize, x->e_ob.a_wintype_ansyn[0] ? x->e_ob.a_wintype_ansyn[0]->s_name : "rect", false, true, fullspectrum, EARS_ANGLEUNIT_RADIANS, x->e_ob.a_winstartfromzero, unitary);
            }
            
            
            long delta_samps = earsbufobj_time_to_durationdifference_samps((t_earsbufobj *)x, howmuch, in_amps[0], EARSBUFOBJ_CONVERSION_FLAG_USEORIGINALAUDIOSRFORSPECTRALBUFFERS);
            double framesize_samps = 2*(ears_buffer_get_numchannels((t_object *)x, in_amps[0])-1);
            double hopsize_samps = ears_spectralbuf_get_original_audio_sr((t_object *)x, in_amps[0]) * 1./ears_buffer_get_sr((t_object *)x, in_amps[0]);
            long delta_frames = (long)round(delta_samps / hopsize_samps);
                
            t_buffer_obj *seam_path = extended ? earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, 0) : NULL;
            t_buffer_obj *energymap = extended ? earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, 1) : NULL;
            t_buffer_obj *energymapcumul = extended ? earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 1, 2) : NULL;

            
            ears_buffer_spectral_seam_carve((t_object *)x, num_in_chans, in_amps, in_phases, out_amps, out_phases, energymap, energymapcumul, seam_path, delta_frames, framesize_samps, hopsize_samps, x->e_energy_mode, (updateprogress_fn)earsbufobj_updateprogress, x->e_compensate_phases, x->e_regularization, x->e_forward_energy, x->e_forward_energy_type, x->e_forward_energy_embed, fullspectrum, false, unitary, num_griffin_lim_iter, griffin_lim_invalidate_width, griffin_lim_vertical, griffin_lim_randomize, x->e_ob.a_wintype_ansyn[0] ? x->e_ob.a_wintype_ansyn[0]->s_name : "rect", x->e_ob.a_wintype_ansyn[1] ? x->e_ob.a_wintype_ansyn[1]->s_name : (x->e_ob.a_wintype_ansyn[0] ? x->e_ob.a_wintype_ansyn[0]->s_name : "rect"), x->e_batch_size, x->e_batch_interrupt);
            
            ears_buffer_istft((t_object *)x, num_in_chans, out_amps, out_phases, outbuf, NULL,
                              x->e_ob.a_wintype_ansyn[1] ? x->e_ob.a_wintype_ansyn[1]->s_name : (x->e_ob.a_wintype_ansyn[0] ? x->e_ob.a_wintype_ansyn[0]->s_name : "rect"), true, false, fullspectrum, EARS_ANGLEUNIT_RADIANS, audio_sr, x->e_ob.a_winstartfromzero, unitary);
            buffer_setdirty(outbuf);

            for (long c = 0; c < num_in_chans; c++) {
                ears_buffer_free(in_amps[c]);
                ears_buffer_free(in_phases[c]);
                ears_buffer_free(out_amps[c]);
                ears_buffer_free(out_phases[c]);
            }
            
            bach_freeptr(in_amps);
            bach_freeptr(in_phases);
            bach_freeptr(out_amps);
            bach_freeptr(out_phases);
        }
    }
    
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    earsbufobj_updateprogress((t_earsbufobj *)x, 1.);
    
    if (extended && mode == 1)
        earsbufobj_outlet_buffer((t_earsbufobj *)x, 1);
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}



void buf_timesquash_anything(t_buf_timesquash *x, t_symbol *msg, long ac, t_atom *av)
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
                buf_timesquash_bang(x);
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

