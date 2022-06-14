/**
	@file
	ears.collect.c
 
	@name
	ears.collect~
 
	@realname
	ears.collect~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Collect buffer samples
 
	@description
	Create buffers by collecting individual samples.
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, collect
 
	@seealso
	ears.iter~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"


#define EARS_COLLECT_ALLOCATION_STEP_SAMPS (44100*8)

typedef struct _buf_collect {
    t_earsbufobj       e_ob;
    long               e_num_outlets;
    float              **collections;
    long               *collections_numchannels;
    long               *collections_size;
    long               *collections_allocated;
    long               e_mode;
    long               e_curr_idx;
    long               e_autoclear;
    long               e_keepmem;
    double             e_sr;
} t_buf_collect;



// Prototypes
t_buf_collect*         buf_collect_new(t_symbol *s, short argc, t_atom *argv);
void			buf_collect_free(t_buf_collect *x);
void			buf_collect_bang(t_buf_collect *x);
void			buf_collect_anything(t_buf_collect *x, t_symbol *msg, long ac, t_atom *av);

void buf_collect_assist(t_buf_collect *x, void *b, long m, long a, char *s);
void buf_collect_inletinfo(t_buf_collect *x, void *b, long a, char *t);

void buf_collect_clear(t_buf_collect *x);
void buf_collect_reset(t_buf_collect *x);

// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(collect)

/**********************************************************************/
// Class Definition and Life Cycle

void C74_EXPORT ext_main(void* moduleRef)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    t_class *c;
    
    CLASS_NEW_CHECK_SIZE(c, "ears.collect~",
                         (method)buf_collect_new,
                         (method)buf_collect_free,
                         sizeof(t_buf_collect),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method bang @digest Output collected buffers
    // @description A bang in the leftmost inlet causes the collected buffers to be output

    // @method float/list @digest Collect samples
    // @description A float or a list in the middle inlets causes the corresponding samples to be collected (one sample for each channel)

    // @method int @digest Set sample index
    // @description An integer in the rightmost inlet sets the sample index. This is not strictly needed, as an auto-increment system is
    // in place by default, but may be useful in situations where the sample collection is not linear
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(collect)

    // @method clear @digest Clear existing buffers
    // @description The <m>clear</m> message
    // sets all the samples of the buffers to be collected to 0.
    class_addmethod(c, (method) buf_collect_clear, "clear", 0);

    // @method reset @digest Reset sample counting
    // @description The <m>reset</m> message only resets the sample counting (restarting from 0), not clearing the existing buffers
    class_addmethod(c, (method) buf_collect_reset, "reset", 0);

    
//    earsbufobj_class_add_blocking_attr(c);
//    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

    CLASS_ATTR_DOUBLE(c, "sr", 0, t_buf_collect, e_sr);
    CLASS_ATTR_STYLE_LABEL(c,"sr",0,"text","Output Sample Rate");
    CLASS_ATTR_BASIC(c, "sr", 0);
    // @description Sets the sample rate for the output buffer. If zero (default) then the current Max sample rate is used.

    CLASS_ATTR_LONG(c, "mode",    0,    t_buf_collect, e_mode);
    CLASS_ATTR_STYLE_LABEL(c, "mode", 0, "enumindex", "Mode");
    CLASS_ATTR_FILTER_CLIP(c, "mode", 0, 1);
    CLASS_ATTR_ENUMINDEX(c, "mode", 0, "Overwrite Overlap-Add");
    CLASS_ATTR_BASIC(c, "mode", 0);
    // @description Sets the collections mode: <br />
    // 0 = Overwrite: new values at the same index of others overwrite the previous ones; <br />
    // 1 = Overlap-Add: new values at the same index of others sums with the previous ones; <br />

    CLASS_ATTR_LONG(c, "autoclear",    0,    t_buf_collect, e_autoclear);
    CLASS_ATTR_STYLE_LABEL(c, "autoclear", 0, "enumindex", "Autoclear");
    CLASS_ATTR_FILTER_CLIP(c, "autoclear", 0, 1);
    CLASS_ATTR_ENUMINDEX(c, "autoclear", 0, "Don't Restart Counting Clear Buffers");
    CLASS_ATTR_BASIC(c, "autoclear", 0);
    // @description Toggles the ability to automatically clear the inner buffer storage when buffers are output.
    // 0 = Don't: no autoclear; <br />
    // 1 = Restart Counting: only restart the automatic counting of sample indices; <br />
    // 2 = Clear Buffers: restart automatic counting and clear existing buffers; <br />


    
    CLASS_ATTR_LONG(c, "keepmemory",    0,    t_buf_collect, e_keepmem);
    CLASS_ATTR_STYLE_LABEL(c, "keepmemory", 0, "onoff", "Keep Memory Allocation While Clearing");
    CLASS_ATTR_FILTER_CLIP(c, "keepmemory", 0, 1);
    // @description Toggles the ability to keep the allocated memory while clearing.
    // This means that filling in new buffers will be faster. Set this to zero if you want to
    // have more free RAM.

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_collect_assist(t_buf_collect *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "bang When Done");   // @out 0 @type bang @digest Bang to Output Data
                                            // @description Sends a bang to output collected data
        else if (a <= x->e_num_outlets)
            sprintf(s, "list: Samples For Each Channel");    // @out 1 @loop 1 @type int/list @digest Samples
                                                            // @description Incoming samples to be collected (one inlet for each buffer channel)
        else
            sprintf(s, "int: Sample Index");    // @out 2 @type int @digest Sample Index
                                                // @description Sample index is optional; if not set, an auto-counting mechanism is put in place

    } else {
        if (a < x->e_num_outlets) // @in 0 @loop 1 @type symbol @digest Buffer(s) with collected samples
            sprintf(s, "symbol: Buffer %ld", a + 1);
    }
}

void buf_collect_inletinfo(t_buf_collect *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_collect *buf_collect_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_collect *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_collect*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_num_outlets = 1;
        x->e_curr_idx = 0;
        x->e_autoclear = 2;
        x->e_keepmem = 1;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_NONE);
        
        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name numbuffers @optional 1 @type number
        // @digest Number of buffers
        // @description Sets the number of buffers to be collected in parallel.

        t_llll *args = llll_parse(true_ac, argv);
        
        if (args && args->l_head && is_hatom_number(&args->l_head->l_hatom)) {
            x->e_num_outlets = CLAMP(hatom_getlong(&args->l_head->l_hatom), 1, LLLL_MAX_OUTLETS-2);
        }
        
        x->collections = (float **)bach_newptr(x->e_num_outlets * sizeof(float *));
        x->collections_size = (long *)bach_newptr(x->e_num_outlets * sizeof(long));
        x->collections_numchannels = (long *)bach_newptr(x->e_num_outlets * sizeof(long));
        x->collections_allocated = (long *)bach_newptr(x->e_num_outlets * sizeof(long));
        for (long i = 0; i < x->e_num_outlets; i++) {
            x->collections[i] = (float *)bach_newptrclear(EARS_COLLECT_ALLOCATION_STEP_SAMPS * sizeof(float));
            x->collections_allocated[i] = EARS_COLLECT_ALLOCATION_STEP_SAMPS;
            x->collections_size[i] = 0;
        }

        attr_args_process(x, argc, argv);
        
        char inlets[LLLL_MAX_INLETS];
        char outlets[LLLL_MAX_OUTLETS];
        inlets[0] = 'b';
        for (long i = 0; i < x->e_num_outlets; i++) {
            inlets[i+1] = 'z';
            outlets[i] = 'e';
        }
        inlets[x->e_num_outlets+1] = 'b';
        outlets[x->e_num_outlets] = 0;
        inlets[x->e_num_outlets+2] = 0;
        earsbufobj_setup((t_earsbufobj *)x, inlets, outlets, NULL);

        llll_free(args);
    }
    return x;
}


void buf_collect_free(t_buf_collect *x)
{
    for (long i = 0; i < x->e_num_outlets; i++)
        bach_freeptr(x->collections[i]);
    bach_freeptr(x->collections);
    bach_freeptr(x->collections_size);
    bach_freeptr(x->collections_numchannels);
    bach_freeptr(x->collections_allocated);
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_collect_clear(t_buf_collect *x)
{
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    // clear storage
    for (long c = 0; c < x->e_num_outlets; c++) {
        if (!x->e_keepmem) {
            x->collections[c] = (float *)bach_resizeptr(x->collections[c], EARS_COLLECT_ALLOCATION_STEP_SAMPS * sizeof(float));
            x->collections_allocated[c] = EARS_COLLECT_ALLOCATION_STEP_SAMPS;
        }

        for (long i = 0; i < x->collections_allocated[c]; i++)
            x->collections[c][i] = 0.;

        x->collections_size[c] = 0;
    }
    x->e_curr_idx = 0;
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
}

void buf_collect_reset(t_buf_collect *x)
{
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    x->e_curr_idx = 0;
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
}


void buf_collect_bang(t_buf_collect *x)
{
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, x->e_num_outlets);
    
    for (long c = 0; c < x->e_num_outlets; c++) {
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, c, 1, true);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, c, 0);
        
        ears_buffer_set_sr((t_object *)x, out, x->e_sr <= 0 ? ears_get_current_Max_sr() : x->e_sr);
        ears_buffer_set_size_and_numchannels((t_object *)x, out, x->collections_size[c], x->collections_numchannels[c]);

        float *sample = buffer_locksamples(out);
        if (!sample) {
            object_error((t_object *)x, EARS_ERROR_BUF_CANT_READ);
        } else {
            t_atom_long    channelcount = buffer_getchannelcount(out);        // number of floats in a frame
            t_atom_long    framecount   = buffer_getframecount(out);            // number of floats long the buffer is for a single channel
            
            if (channelcount != x->collections_numchannels[c] || framecount != x->collections_size[c]) {
                object_error((t_object *)x, "Internal mismatch in buffer length.");
            } else {
                bach_copyptr(x->collections[c], sample, framecount * channelcount * sizeof(float));
            }
            buffer_setdirty(out);
        }
        buffer_unlocksamples(out);
        
        if (earsbufobj_iter_progress((t_earsbufobj *)x, c, x->e_num_outlets)) break;
    }

    if (x->e_autoclear == 2) {
        // clear storage
        for (long c = 0; c < x->e_num_outlets; c++) {
            if (!x->e_keepmem) {
                x->collections[c] = (float *)bach_resizeptr(x->collections[c], EARS_COLLECT_ALLOCATION_STEP_SAMPS * sizeof(float));
                x->collections_allocated[c] = EARS_COLLECT_ALLOCATION_STEP_SAMPS;
            }

            for (long i = 0; i < x->collections_allocated[c]; i++)
                x->collections[c][i] = 0.;

            x->collections_size[c] = 0;
        }
        x->e_curr_idx = 0;
    } else if (x->e_autoclear == 1) {
        x->e_curr_idx = 0;
    }

    earsbufobj_mutex_unlock((t_earsbufobj *)x);

    for (long c = x->e_num_outlets - 1; c >= 0; c--) {
        earsbufobj_outlet_buffer((t_earsbufobj *)x, c);
    }
}


void buf_collect_anything(t_buf_collect *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);
    
    if (inlet == 0) {
//        buf_collect_bang(x);
    } else if (inlet == x->e_num_outlets + 1) {
        if (ac) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            x->e_curr_idx = atom_getlong(av);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    } else {
        if (ac) {
            long c = inlet - 1;
            long acok = ac;
            t_atom *avok = av;
            bool mustfree_avok = false;

            earsbufobj_mutex_lock((t_earsbufobj *)x);

            if (msg == _llllobj_sym_bach_llll) {
                // support native lllls by hand, because we need the float input to be fast: we sacrifice lllls, one isn't supposed to use them.
                // TODO: avoid deparsing anyway
                t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
                avok = NULL;
                acok = llll_deparse(parsed, &avok, 0, LLLL_D_NONE);
                mustfree_avok = true;
                llll_free(parsed);
            }

            // float/list input
            long mode = x->e_mode;
            long idx = x->e_curr_idx;
            
            if (idx < 0) {
                object_error((t_object *)x, "Negative sample index detected and ignored.");
            } else {
                if (c >= 0 && c < x->e_num_outlets) {
                    if (x->collections_size[c] == 0) {
                        // let's begin collecting
                        x->collections_numchannels[c] = acok;
                    } else {
                        if (acok != x->collections_numchannels[c]) {
                            object_warn((t_object *)x, "Mismatch in number of channels");
                            acok = MIN(acok, x->collections_numchannels[c]);
                        }
                    }
                    
                    while (idx * x->collections_numchannels[c] >= x->collections_allocated[c]) {
                        x->collections_allocated[c] += EARS_COLLECT_ALLOCATION_STEP_SAMPS;
                        x->collections[c] = (float *)bach_resizeptr(x->collections[c], x->collections_allocated[c] * sizeof(float));
                        for (long i = x->collections_size[c] * x->collections_numchannels[c]; i < x->collections_allocated[c]; i++)
                            x->collections[c][i] = 0.;
                    }
                    
                    for (long i = 0; i < acok; i++) {
                        if (mode == 0)
                            x->collections[c][idx * x->collections_numchannels[c] + i] = (float)(atom_getfloat(avok + i));
                        else
                            x->collections[c][idx * x->collections_numchannels[c] + i] += (float)(atom_getfloat(avok + i));
                    }
                    x->collections_size[c] = MAX(x->collections_size[c], idx+1);
                    
                    x->e_curr_idx++;
                }
            }
            
            if (mustfree_avok)
                bach_freeptr(avok);
            
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
}


