/**
	@file
	ears.tempogram.c
 
	@name
	ears.tempogram~
 
	@realname
	ears.tempogram~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Compute tempograms
 
	@description
	Computes a spectrogram-like representation of tempo over time
 
	@discussion
 
	@category
	ears spectral
 
	@keywords
	buffer, tempogram, spectrogram, tempo, rhythm, transform, spectrum
 
	@seealso
	ears.spectrogram~, ears.essentia~
	
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
#include "ears.essentia_commons.h"

typedef struct _buf_tempogram {
    t_earsbufobj       e_ob;

    double  bigframesize;
    double  bigoverlap;

    long  mintempo;
    long  maxtempo;
    long  maxpeaks;
    long downmix;
} t_buf_tempogram;



// Prototypes
t_buf_tempogram*     buf_tempogram_new(t_symbol *s, short argc, t_atom *argv);
void			buf_tempogram_free(t_buf_tempogram *x);
void			buf_tempogram_bang(t_buf_tempogram *x);
void			buf_tempogram_anything(t_buf_tempogram *x, t_symbol *msg, long ac, t_atom *av);

void buf_tempogram_assist(t_buf_tempogram *x, void *b, long m, long a, char *s);
void buf_tempogram_inletinfo(t_buf_tempogram *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(tempogram)

/**********************************************************************/
// Class Definition and Life Cycle

int C74_EXPORT main(void)
{
    ears_essentia_init();
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.tempogram~",
                         (method)buf_tempogram_new,
                         (method)buf_tempogram_free,
                         sizeof(t_buf_tempogram),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(tempogram)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_antimeunit_attr(c);

    earsbufobj_class_add_framesize_attr(c);
    earsbufobj_class_add_hopsize_attr(c);
    earsbufobj_class_add_numframes_attr(c);
    earsbufobj_class_add_overlap_attr(c);
    earsbufobj_class_add_wintype_attr(c);
    earsbufobj_class_add_winstartfromzero_attr(c);
    
    earsbufobj_class_add_poly_attr(c);

    CLASS_ATTR_LONG(c, "downmix",    0,    t_buf_tempogram, downmix);
    CLASS_ATTR_STYLE_LABEL(c, "downmix", 0, "onoff", "Downmix to Mono");
    CLASS_ATTR_BASIC(c, "downmix", 0);
    // @description Toggles the ability to downmix all the channels into one. If this flag is not set, then
    // one buffer per channel is output. By default downmix is 1.

    CLASS_ATTR_LONG(c, "mintempo",    0,    t_buf_tempogram, mintempo);
    CLASS_ATTR_STYLE_LABEL(c, "mintempo", 0, "text", "Minimum Tempo");
    CLASS_ATTR_BASIC(c, "mintempo", 0);
    CLASS_ATTR_CATEGORY(c, "mintempo", 0, "Analysis");
    // @description Sets the minimum tempo for analysis.
    
    CLASS_ATTR_LONG(c, "maxtempo",    0,    t_buf_tempogram, maxtempo);
    CLASS_ATTR_STYLE_LABEL(c, "maxtempo", 0, "text", "Maximum Tempo");
    CLASS_ATTR_BASIC(c, "maxtempo", 0);
    CLASS_ATTR_CATEGORY(c, "maxtempo", 0, "Analysis");
    // @description Sets the maximum tempo for analysis.

    CLASS_ATTR_LONG(c, "maxpeaks",    0,    t_buf_tempogram, maxpeaks);
    CLASS_ATTR_STYLE_LABEL(c, "maxpeaks", 0, "text", "Maximum Number Of Peaks");
    CLASS_ATTR_BASIC(c, "maxpeaks", 0);
    CLASS_ATTR_CATEGORY(c, "maxpeaks", 0, "Analysis");
    // @description Maximum number of peaks to be considered at each spectrum.
    
    CLASS_ATTR_DOUBLE(c, "bigframesize",    0,    t_buf_tempogram, bigframesize);
    CLASS_ATTR_STYLE_LABEL(c, "bigframesize", 0, "text", "Big Frame Size");
    CLASS_ATTR_BASIC(c, "bigframesize", 0);
    CLASS_ATTR_CATEGORY(c, "bigframesize", 0, "Analysis");
    // @description Sets the frame size for the big tempo analysis frames.

    CLASS_ATTR_DOUBLE(c, "bigoverlap",    0,    t_buf_tempogram, bigoverlap);
    CLASS_ATTR_STYLE_LABEL(c, "bigoverlap", 0, "text", "Big Overlap");
    CLASS_ATTR_BASIC(c, "bigoverlap", 0);
    CLASS_ATTR_CATEGORY(c, "bigoverlap", 0, "Analysis");
    // @description Sets the overlap for the big tempo analysis frames.

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_tempogram_assist(t_buf_tempogram *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
            // @in 0 @type symbol @digest Buffer containing audio
            // @description Source audio buffer
        sprintf(s, "symbol: Source Buffer");
    } else {
        // @out 0 @type symbol @digest Buffer containing output tempogram
        sprintf(s, "symbol: Buffer Containing Tempogram");
    }
}

void buf_tempogram_inletinfo(t_buf_tempogram *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_tempogram *buf_tempogram_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_tempogram *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_tempogram*)object_alloc_debug(s_tag_class);
    if (x) {
        x->downmix = 1;
        x->mintempo = 30;
        x->maxtempo = 560;
        x->bigframesize = 44100*4;
        x->bigoverlap = 16;
        x->maxpeaks = 50;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_NONE);

        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        attr_args_process(x, argc, argv);

        earsbufobj_setup((t_earsbufobj *)x, "e", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_tempogram_free(t_buf_tempogram *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}


t_ears_essentia_analysis_params buf_tempogram_get_params(t_buf_tempogram *x, t_buffer_obj *buf)
{
    t_ears_essentia_analysis_params params = earsbufobj_get_essentia_analysis_params((t_earsbufobj *)x, buf);
    params.TEMPO_minBpm = x->mintempo;
    params.TEMPO_maxBpm = x->maxtempo;
    params.TEMPO_bigFrameSize = ears_convert_timeunit(x->bigframesize, buf, (e_ears_timeunit)x->e_ob.l_antimeunit, EARS_TIMEUNIT_SECONDS);
    params.TEMPO_bigOverlap = x->bigoverlap;
    params.TEMPO_maxPeaks = x->maxpeaks;
    return params;
}

void buf_tempogram_bang(t_buf_tempogram *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    long downmix = x->downmix;
    std::vector<essentia::Real> frequencyBands = {0, 50, 100, 150, 200, 300, 400, 510, 630, 770, 920, 1080, 1270, 1480, 1720, 2000, 2320, 2700, 3150, 3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500, 20500, 27000};
    
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);

    if (num_buffers > 0) { // must be just 1 buffer
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
        long num_channels = ears_buffer_get_numchannels((t_object *)x, in);

        if (downmix) {
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
            earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
            
            t_buffer_obj *out1 = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
            std::vector<float> data = ears_buffer_get_sample_vector_mono((t_object *)x, in);
            
            t_ears_essentia_analysis_params params = buf_tempogram_get_params(x, in);

            ears_vector_tempogram((t_object *)x, data, ears_buffer_get_sr((t_object *)x, in), frequencyBands, out1, &params);
        } else if (num_channels > 0){
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_channels, true);
            earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
            
            for (long c = 0; c < num_channels; c++) {
                t_buffer_obj *out1 = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, c);
                std::vector<float> data = ears_buffer_get_sample_vector_channel((t_object *)x, in, c);
                
                t_ears_essentia_analysis_params params = buf_tempogram_get_params(x, in);

                ears_vector_tempogram((t_object *)x, data, ears_buffer_get_sr((t_object *)x, in), frequencyBands, out1, &params);
            }
        } else {
            object_error((t_object *)x, "No channels in buffer!");
        }
    }
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}



void buf_tempogram_anything(t_buf_tempogram *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
            
//                earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
            
            if (inlet == 0) {
                long num_bufs = llll_get_num_symbols_root(parsed);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_bufs, true);
                earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
                
                buf_tempogram_bang(x);
            }
        }
    }
    llll_free(parsed);
}


