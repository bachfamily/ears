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
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"


#define EARS_COLLECT_ALLOCATION_STEP_SAMPS (44100*8)

typedef struct _buf_collect {
    t_earsbufobj       e_ob;
    long               e_num_outlets;
    
    long               e_num_buffers;
    long               e_num_buffers_allocated;
    float              ***e_collections;
    long               **e_collections_numchannels;
    long               **e_collections_size;
    long               **e_collections_allocated;
    t_symbol           ***e_buffer_names;
    
    long               e_mode;
    long               e_curr_idx;
    long               e_curr_buffer;
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
        return;
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
        else if (a == x->e_num_outlets)
            sprintf(s, "int: Sample Index");    // @out 2 @type int @digest Sample Index
                                                // @description Sample index is optional; if not set, an auto-counting mechanism is put in place
        else
            sprintf(s, "symbol/list: Buffer Models");    // @out 3 @type int @digest Buffer Models
                                                         // @description Models for buffer to be collected

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
        x->e_curr_buffer = -1;
        x->e_autoclear = 2;
        x->e_keepmem = 1;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
        
        // @arg 0 @name outname @optional 1 @type symbol
        // @digest Output buffer name
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name numbuffers @optional 1 @type number
        // @digest Number of buffers
        // @description Sets the number of buffers to be collected in parallel.

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head && is_hatom_number(&args->l_head->l_hatom)) {
            x->e_num_outlets = CLAMP(hatom_getlong(&args->l_head->l_hatom), 1, LLLL_MAX_OUTLETS-2);
        }
        
        // one element for each outlet
        // one element for each buffer
        // (one element for each sample, for e_collections)
        x->e_collections = (float ***)bach_newptr(x->e_num_outlets * sizeof(float **));
        x->e_collections_size = (long **)bach_newptr(x->e_num_outlets * sizeof(long *));
        x->e_collections_numchannels = (long **)bach_newptr(x->e_num_outlets * sizeof(long *));
        x->e_collections_allocated = (long **)bach_newptr(x->e_num_outlets * sizeof(long *));
        x->e_buffer_names = (t_symbol ***)bach_newptr(x->e_num_outlets * sizeof(t_symbol **));

        for (long i = 0; i < x->e_num_outlets; i++) {
            x->e_collections[i] = (float **)bach_newptrclear(1 * sizeof(float *));
            x->e_collections_size[i] = (long *)bach_newptrclear(1 * sizeof(long));
            x->e_collections_numchannels[i] = (long *)bach_newptrclear(1 * sizeof(long));
            x->e_collections_allocated[i] = (long *)bach_newptrclear(1 * sizeof(long));
            x->e_buffer_names[i] = (t_symbol **)bach_newptrclear(1 * sizeof(t_symbol *));

            x->e_collections[i][0] = (float *)bach_newptrclear(EARS_COLLECT_ALLOCATION_STEP_SAMPS * sizeof(float));
            x->e_collections_allocated[i][0] = EARS_COLLECT_ALLOCATION_STEP_SAMPS;
            x->e_collections_size[i][0] = 0;
        }
        x->e_num_buffers = 0;
        x->e_num_buffers_allocated = 1;

        attr_args_process(x, argc, argv);

        char inlets[LLLL_MAX_INLETS];
        char outlets[LLLL_MAX_OUTLETS];
        inlets[0] = 'b';
        for (long i = 0; i < x->e_num_outlets; i++) {
            inlets[i+1] = 'z';
            outlets[i] = 'e';
        }
        inlets[x->e_num_outlets+2] = 'i'; // doesn't matter
        inlets[x->e_num_outlets+1] = 'i'; // doesn't matter
        outlets[x->e_num_outlets] = 0;
        inlets[x->e_num_outlets+3] = 0;
        earsbufobj_setup((t_earsbufobj *)x, inlets, outlets, names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_collect_free(t_buf_collect *x)
{
    for (long i = 0; i < x->e_num_outlets; i++) {
        for (long b = 0; b < x->e_num_buffers_allocated; b++) {
            bach_freeptr(x->e_collections[i][b]);
        }
        bach_freeptr(x->e_collections_size[i]);
        bach_freeptr(x->e_collections_allocated[i]);
        bach_freeptr(x->e_buffer_names[i]);
        bach_freeptr(x->e_collections_numchannels[i]);
        bach_freeptr(x->e_collections[i]);
    }
    bach_freeptr(x->e_collections);
    bach_freeptr(x->e_collections_size);
    bach_freeptr(x->e_collections_numchannels);
    bach_freeptr(x->e_collections_allocated);
    bach_freeptr(x->e_buffer_names);
    earsbufobj_free((t_earsbufobj *)x);
}


void buf_collect_clear_do(t_buf_collect *x)
{
    // clear storage
    for (long i = 0; i < x->e_num_outlets; i++) {
        for (long b = 0 ; b < x->e_num_buffers_allocated; b++) {
            x->e_collections_size[i][b] = 0;
        }
        
        if (!x->e_keepmem) {
            for (long b = 0; b < x->e_num_buffers_allocated; b++) {
                bach_freeptr(x->e_collections[i][b]);
                x->e_collections_allocated[i][b] = 0;
                x->e_buffer_names[i][b] = NULL;
            }
            
            x->e_collections[i] = (float **)bach_resizeptrclear(x->e_collections[i], 1 * sizeof(float *));
            x->e_collections_size[i] = (long *)bach_resizeptrclear(x->e_collections_size[i], 1 * sizeof(long));
            x->e_collections_numchannels[i] = (long *)bach_resizeptrclear(x->e_collections_numchannels[i], 1 * sizeof(long));
            x->e_collections_allocated[i] = (long *)bach_resizeptrclear(x->e_collections_allocated[i], 1 * sizeof(long));
            x->e_buffer_names[i] = (t_symbol **)bach_resizeptrclear(x->e_buffer_names[i], 1 * sizeof(t_symbol *));

            
            x->e_collections[i][0] = (float *)bach_newptr(EARS_COLLECT_ALLOCATION_STEP_SAMPS * sizeof(float));
            x->e_collections_allocated[i][0] = EARS_COLLECT_ALLOCATION_STEP_SAMPS;
            x->e_buffer_names[i][0] = NULL;
        }
    }
    
    if (!x->e_keepmem) {
        x->e_num_buffers_allocated = 1;
    }

    x->e_curr_idx = 0;
    x->e_curr_buffer = -1;
}

void buf_collect_clear(t_buf_collect *x)
{
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    buf_collect_clear_do(x);
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
}

void buf_collect_reset_do(t_buf_collect *x)
{
    x->e_curr_idx = 0;
    x->e_curr_buffer = -1;
}

void buf_collect_reset(t_buf_collect *x)
{
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    buf_collect_reset_do(x);
    earsbufobj_mutex_unlock((t_earsbufobj *)x);
}


void buf_collect_bang(t_buf_collect *x)
{
    long num_buffers = x->e_curr_buffer < 0 ? 1 : x->e_curr_buffer + 1;
    long num_outlets = x->e_num_outlets;
    
    for (long c = 0; c < x->e_num_outlets; c++)
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, c, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);

    if (x->e_ob.l_bufouts_naming == EARSBUFOBJ_NAMING_COPY) {
        // refreshing outlet names by hand
        long store;
        t_earsbufobj *e_ob = (t_earsbufobj *)x;
        for (store = 0; store < e_ob->l_numbufouts; store++) {
            long num_stored_bufs = x->e_ob.l_outstore[0].num_stored_bufs;
            if (num_stored_bufs > 0) {
                t_symbol **s = (t_symbol **)bach_newptrclear(num_stored_bufs * sizeof(t_symbol *));
                t_buffer_obj **o = (t_buffer_obj **)bach_newptrclear(num_stored_bufs * sizeof(t_buffer_obj *));
                long j, c = 0;
                for (j = 0; j < num_stored_bufs; j++) {
                    t_symbol *name = earsbufobj_get_outlet_buffer_name(e_ob, store, j);
                    t_buffer_obj *buf = earsbufobj_get_outlet_buffer_obj(e_ob, store, j);
                    if (name) {
                        s[c] = name;
                        o[c] = buf;
                        c++;
                    }
                }
                
                // Now we change the outlet names
                for (j = 0; j < num_stored_bufs && j < x->e_num_buffers_allocated; j++) { // was: j < c
                    if (e_ob->l_outstore[store].stored_buf[j].l_status != EARSBUFOBJ_BUFSTATUS_USERNAMED &&
                        (!(e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_STATIC && (j < c && s[j])))) {
                        earsbufobj_buffer_link(e_ob, EARSBUFOBJ_OUT, store, j, x->e_buffer_names[store][j]);
                    }
                }
                
                bach_freeptr(s);
                bach_freeptr(o);
            }
        }
    } else {
        earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    }
    
    earsbufobj_init_progress((t_earsbufobj *)x, num_outlets * num_buffers);
    
    for (long c = 0; c < num_outlets; c++) {
        for (long b = 0; b < num_buffers; b++) {
            t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, c, b);
            t_buffer_obj *model = (b < x->e_num_buffers_allocated && x->e_buffer_names[c][b] ? ears_buffer_getobject(x->e_buffer_names[c][b]) : NULL);
            
            if (model) {
                ears_buffer_copy_format((t_object *)x, model, out, true);
                ears_buffer_set_size_and_numchannels((t_object *)x, out, x->e_collections_size[c][b], x->e_collections_numchannels[c][b]);
            } else {
                ears_buffer_set_sr((t_object *)x, out, x->e_sr <= 0 ? ears_get_current_Max_sr() : x->e_sr);
                ears_buffer_set_size_and_numchannels((t_object *)x, out, x->e_collections_size[c][b], x->e_collections_numchannels[c][b]);
            }
            
            float *sample = ears_buffer_locksamples(out);
            if (!sample) {
                object_error((t_object *)x, EARS_ERROR_BUF_CANT_READ);
            } else {
                t_atom_long    channelcount = buffer_getchannelcount(out);        // number of floats in a frame
                t_atom_long    framecount   = buffer_getframecount(out);            // number of floats long the buffer is for a single channel
                
                if (channelcount != x->e_collections_numchannels[c][b] || framecount != x->e_collections_size[c][b]) {
                    object_error((t_object *)x, "Internal mismatch in buffer length.");
                } else {
                    bach_copyptr(x->e_collections[c][b], sample, framecount * channelcount * sizeof(float));
                }
                buffer_setdirty(out);
            }
            ears_buffer_unlocksamples(out);
            
            if (earsbufobj_iter_progress((t_earsbufobj *)x, c * num_buffers + b, num_outlets * num_buffers)) break;
        }
    }

    if (x->e_autoclear == 2) {
        // clear storage
        buf_collect_clear_do(x);
    } else if (x->e_autoclear == 1) {
        buf_collect_reset_do(x);
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
            if (msg == _llllobj_sym_bach_llll) {
                // support native lllls by hand, because we need the float input to be fast: we sacrifice lllls, one isn't supposed to use them.
                t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av, LLLL_PARSE_RETAIN);
                if (parsed && parsed->l_head && is_hatom_number(&parsed->l_head->l_hatom))
                    x->e_curr_idx = hatom_getlong(&parsed->l_head->l_hatom);
                llll_free(parsed);
            } else {
                x->e_curr_idx = atom_getlong(av);
            }
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    } else if (inlet == x->e_num_outlets + 2) {
        t_llll *ll = earsbufobj_parse_gimme((t_earsbufobj *)x, LLLL_OBJ_VANILLA, msg, ac, av);
        if (ll) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            
            x->e_curr_buffer++;
            
            long b = x->e_curr_buffer;
            if (b >= x->e_num_buffers_allocated) {
                for (long c = 0; c < x->e_num_outlets; c++) {
                    x->e_collections[c] = (float **)bach_resizeptr(x->e_collections[c], (b+1) * sizeof(float *));
                    x->e_collections_size[c] = (long *)bach_resizeptr(x->e_collections_size[c], (b+1) * sizeof(long));
                    x->e_collections_numchannels[c] = (long *)bach_resizeptr(x->e_collections_numchannels[c], (b+1) * sizeof(long));
                    x->e_collections_allocated[c] = (long *)bach_resizeptr(x->e_collections_allocated[c], (b+1) * sizeof(long));
                    for (long j = x->e_num_buffers_allocated; j < b+1; j++) {
                        x->e_collections[c][j] = (float *)bach_resizeptrclear(x->e_collections[c][j], EARS_COLLECT_ALLOCATION_STEP_SAMPS * sizeof(float));
                        x->e_collections_allocated[c][j] = EARS_COLLECT_ALLOCATION_STEP_SAMPS;
                        x->e_buffer_names[c][j] = NULL;
                    }
                }
                x->e_num_buffers_allocated = b+1;
            }
            
            long c = 0;
            if (x->e_curr_buffer >= 0 && x->e_curr_buffer < x->e_num_buffers_allocated) {
                for (t_llllelem *el = ll->l_head; el; el = el->l_next, c++) {
                    x->e_buffer_names[c][x->e_curr_buffer] = hatom_getsym(&el->l_hatom);
                }
            }
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
        llll_free(ll);
    } else {
        if (ac) {
            long c = inlet - 1;
            long acok = ac;
            t_atom *avok = av;
            bool mustfree_avok = false;

            earsbufobj_mutex_lock((t_earsbufobj *)x);
            long b = x->e_curr_buffer;

            if (msg == _llllobj_sym_bach_llll) {
                // support native lllls by hand, because we need the float input to be fast: we sacrifice lllls, one isn't supposed to use them.
                t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av, LLLL_PARSE_RETAIN);
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
                if (b < 0)
                    b = 0;

                if (b >= x->e_num_buffers_allocated) {
                    for (long c = 0; c < x->e_num_outlets; c++) {
                        x->e_collections[c] = (float **)bach_resizeptr(x->e_collections[c], (b+1) * sizeof(float *));
                        x->e_collections_size[c] = (long *)bach_resizeptr(x->e_collections_size[c], (b+1) * sizeof(long));
                        x->e_collections_numchannels[c] = (long *)bach_resizeptr(x->e_collections_numchannels[c], (b+1) * sizeof(long));
                        x->e_collections_allocated[c] = (long *)bach_resizeptr(x->e_collections_allocated[c], (b+1) * sizeof(long));
                        for (long j = x->e_num_buffers_allocated; j < b+1; j++) {
                            x->e_collections[c][j] = (float *)bach_resizeptrclear(x->e_collections[c][j], EARS_COLLECT_ALLOCATION_STEP_SAMPS * sizeof(float));
                            x->e_collections_allocated[c][j] = EARS_COLLECT_ALLOCATION_STEP_SAMPS;
                            x->e_buffer_names[c][j] = NULL;
                        }
                    }
                    x->e_num_buffers_allocated = b+1;
                }

                if (c >= 0 && c < x->e_num_outlets) {
                    if (x->e_collections_size[c][b] == 0) {
                        // let's begin collecting
                        x->e_collections_numchannels[c][b] = acok;
                    } else {
                        if (acok != x->e_collections_numchannels[c][b]) {
                            object_warn((t_object *)x, "Mismatch in number of channels");
                            acok = MIN(acok, x->e_collections_numchannels[c][b]);
                        }
                    }
                    
                    while (idx * x->e_collections_numchannels[c][b] >= x->e_collections_allocated[c][b]) {
                        x->e_collections_allocated[c][b] += EARS_COLLECT_ALLOCATION_STEP_SAMPS;
                        x->e_collections[c][b] = (float *)bach_resizeptr(x->e_collections[c][b], x->e_collections_allocated[c][b] * sizeof(float));
                        for (long i = x->e_collections_size[c][b] * x->e_collections_numchannels[c][b]; i < x->e_collections_allocated[c][b]; i++)
                            x->e_collections[c][b][i] = 0.;
                    }
                    
                    for (long i = 0; i < acok; i++) {
                        if (mode == 0)
                            x->e_collections[c][b][idx * x->e_collections_numchannels[c][b] + i] = (float)(atom_getfloat(avok + i));
                        else
                            x->e_collections[c][b][idx * x->e_collections_numchannels[c][b] + i] += (float)(atom_getfloat(avok + i));
                    }
                    x->e_collections_size[c][b] = MAX(x->e_collections_size[c][b], idx+1);
                    
                    x->e_curr_idx++;
                }
            }
            
            if (mustfree_avok)
                bach_freeptr(avok);
            
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        }
    }
}


