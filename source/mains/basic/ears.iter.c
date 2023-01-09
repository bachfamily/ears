/**
	@file
	ears.iter.c
 
	@name
	ears.iter~
 
	@realname
	ears.iter~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Iterate over buffer samples
 
	@description
	Outputs buffer samples iteratively.
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, iter, iterate
 
	@seealso
	ears.collect~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_iter {
    t_earsbufobj       e_ob;
    long               e_num_inlets;
    long               e_iterationmode;
    long               e_lengthmode;
    t_symbol           *buffers[LLLL_MAX_INLETS];
} t_buf_iter;



// Prototypes
t_buf_iter*         buf_iter_new(t_symbol *s, short argc, t_atom *argv);
void			buf_iter_free(t_buf_iter *x);
void			buf_iter_bang(t_buf_iter *x);
void			buf_iter_anything(t_buf_iter *x, t_symbol *msg, long ac, t_atom *av);

void buf_iter_assist(t_buf_iter *x, void *b, long m, long a, char *s);
void buf_iter_inletinfo(t_buf_iter *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(iter)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.iter~",
                         (method)buf_iter_new,
                         (method)buf_iter_free,
                         sizeof(t_buf_iter),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Function depends on inlet
    // @description A list or llll in the first inlet is supposed to contain buffer names and will
    // trigger the buffer processing and output the processed buffer names (depending on the <m>naming</m> attribute). <br />
    // A number or an llll in the second inlet is expected to contain a iter parameter (depending on the <m>ampunit</m>) or
    // an envelope (also see <m>envampunit</m>).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(iter)

    // @method number @digest Set iter
    // @description A number in the second inlet sets the iter parameter (depending on the <m>ampunit</m>).

 //   earsbufobj_class_add_blocking_attr(c);
//    earsbufobj_class_add_ampunit_attr(c);
//    earsbufobj_class_add_naming_attr(c);

    CLASS_ATTR_LONG(c, "iterationmode",    0,    t_buf_iter, e_iterationmode);
    CLASS_ATTR_STYLE_LABEL(c, "iterationmode", 0, "enumindex", "Iteration Mode");
    CLASS_ATTR_FILTER_CLIP(c, "iterationmode", 0, 2);
    CLASS_ATTR_ENUMINDEX(c, "iterationmode", 0, "Shortest Longest Zeros");
    // @description Sets the iteration mode: <br />
    // 0 = Iterate until the shortest list of buffers is over; <br />
    // 1 = Iterate until the longest list of buffers is over; <br />
    // 2 = Iterate until the longest list of buffers is over and output zeros to pad.

    CLASS_ATTR_LONG(c, "lengthmode",    0,    t_buf_iter, e_lengthmode);
    CLASS_ATTR_STYLE_LABEL(c, "lengthmode", 0, "enumindex", "Length Mode");
    CLASS_ATTR_FILTER_CLIP(c, "lengthmode", 0, 2);
    CLASS_ATTR_ENUMINDEX(c, "lengthmode", 0, "Shortest Longest Zeros");
    // @description Sets the length mode: <br />
    // 0 = Iterate until the shortest buffer is over; <br />
    // 1 = Iterate until the longest buffer is over; <br />
    // 2 = Iterate until the longest buffer is over and output zeros to pad the shorter buffers.

    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_iter_assist(t_buf_iter *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a < x->e_num_inlets) // @in 0 @loop 1 @type symbol @digest Buffer whose sample must be iterated
            sprintf(s, "symbol: Buffer %ld", a + 1);
    } else {
        if (a == x->e_num_inlets + 2)
            sprintf(s, "symbol/list: Buffers Being Iterated");    // @out 3 @type symbol/buffer @digest Buffers being iterated
        else if (a == x->e_num_inlets + 1)
            sprintf(s, "int: Sample Index");    // @out 2 @type int @digest Sample Index
        else if (a == 0)
            sprintf(s, "bang When Done");    // @out 0 @type bang @digest Bang When Done
                                            // @description Sends a bang when the iteration has ended
        else
            sprintf(s, "list: Samples For Each Channel");    // @out 1 @loop 1 @type int/list @digest Samples
                                                            // @description The samples outlet outputs a list of samples (one for each buffer channel)
    }
}

void buf_iter_inletinfo(t_buf_iter *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_iter *buf_iter_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_iter *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_iter*)object_alloc_debug(s_tag_class);
    if (x) {
        x->e_num_inlets = 1;
        
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_NONE);
        
        // @arg 0 @name numbuffers @optional 1 @type number
        // @digest Number of buffers
        // @description Sets the number of buffers to be iterated in parallel.

        t_llll *args = llll_parse(true_ac, argv);
        
        if (args && args->l_head && is_hatom_number(&args->l_head->l_hatom)) {
            x->e_num_inlets = CLAMP(hatom_getlong(&args->l_head->l_hatom), 1, LLLL_MAX_INLETS-3);
        }
        

        attr_args_process(x, argc, argv);
        
        char inlets[LLLL_MAX_INLETS];
        char outlets[LLLL_MAX_OUTLETS];
        outlets[0] = 'z';
        outlets[1] = 'i';
        for (long i = 0; i < x->e_num_inlets; i++) {
            inlets[i] = 'E';
            outlets[i+2] = 'z';
        }
        outlets[x->e_num_inlets+2] = 'b';
        inlets[x->e_num_inlets] = 0;
        outlets[x->e_num_inlets+3] = 0;
        earsbufobj_setup((t_earsbufobj *)x, inlets, outlets, NULL);

        llll_free(args);
    }
    return x;
}


void buf_iter_free(t_buf_iter *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_iter_bang(t_buf_iter *x)
{
    long num_inlets = x->e_num_inlets;
    float *samps[LLLL_MAX_INLETS];
    t_object *buffer[LLLL_MAX_INLETS];
    long sizesamps[LLLL_MAX_INLETS];
    long numchannels[LLLL_MAX_INLETS];
    long maxnumchannels = 0;
    long iterationmode = x->e_iterationmode;
    long lengthmode = x->e_lengthmode;
    
    long num_buffers[LLLL_MAX_INLETS];
    long minnumbuffers = -1, maxnumbuffers = -1;
    for (long i = 0; i < num_inlets; i++) {
        num_buffers[i] = earsbufobj_get_instore_size((t_earsbufobj *)x, i);
        if (minnumbuffers < 0 || num_buffers[i] < minnumbuffers)
            minnumbuffers = num_buffers[i];
        if (maxnumbuffers < 0 || num_buffers[i] > maxnumbuffers)
            maxnumbuffers = num_buffers[i];
    }
    long max_iter_num_buffers = (iterationmode == 0 ? minnumbuffers : maxnumbuffers);
    
    t_symbol *these_buffers[LLLL_MAX_INLETS];
    
    for (long b = 0; b < max_iter_num_buffers; b++) {
        long minsamps = LONG_MAX, maxsamps = 0;

        for (long i = 0; i < num_inlets; i++) {
            t_object *buf = b < num_buffers[i] ? earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, i, b) : NULL;
            these_buffers[i] = buf ? ears_buffer_get_name((t_object *)x, buf) : _llllobj_sym_none;
        }
        earsbufobj_outlet_symbol_list((t_earsbufobj *)x, num_inlets+2, num_inlets, these_buffers);
        
        for (long i = 0; i < num_inlets; i++) {
            buffer[i] = b < num_buffers[i] ? earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, i, b) : NULL;
            samps[i] = buffer[i] ? ears_buffer_locksamples(buffer[i]) : NULL;
            sizesamps[i] = buffer[i] ? buffer_getframecount(buffer[i]) : 0;
            if (buffer[i] && sizesamps[i] > maxsamps)
                maxsamps = sizesamps[i];
            if (buffer[i] && sizesamps[i] < minsamps)
                minsamps = sizesamps[i];
            numchannels[i] = buffer[i] ? buffer_getchannelcount(buffer[i]) : 0;
            if (buffer[i] && numchannels[i] > maxnumchannels)
                maxnumchannels = numchannels[i];
        }
        
        // all buffers are now locked. Let's send out the information
        long limitsamps = (lengthmode == 0 ? minsamps : maxsamps);
        
        t_atom *av = (t_atom *)bach_newptr((maxnumchannels + 1) * sizeof(t_atom));
        for (long f = 0; f < limitsamps; f++) {
            earsbufobj_outlet_int((t_earsbufobj *)x, num_inlets+1, f);
            
            for (long i = num_inlets - 1; i >= 0; i--) {
                
                if (numchannels[i] == 1) {
                    if (f < sizesamps[i]) {
                        atom_setfloat(av, samps[i][f]);
                        earsbufobj_outlet_anything((t_earsbufobj *)x, i+1, _sym_float, 1, av);
                    } else if (lengthmode == 2) {
                        atom_setfloat(av, 0.);
                        earsbufobj_outlet_anything((t_earsbufobj *)x, i+1, _sym_float, 1, av);
                    }
                } else {
                    if (f < sizesamps[i]) {
                        for (long c = 0; c < numchannels[i]; c++)
                            atom_setfloat(av+c, samps[i][f*numchannels[i] + c]);
                        earsbufobj_outlet_anything((t_earsbufobj *)x, i+1, _sym_list, numchannels[i], av);
                    } else if (lengthmode == 2 || (!buffer[i] && iterationmode == 2)) {
                        for (long c = 0; c < (buffer[i] ? numchannels[i] : 1); c++)
                            atom_setfloat(av+c, 0.);
                        earsbufobj_outlet_anything((t_earsbufobj *)x, i+1, _sym_list, (buffer[i] ? numchannels[i] : 1), av);
                    }
                }
            }
        }
        
        for (long i = 0; i < num_inlets; i++) {
            if (samps[i] && buffer[i])
                ears_buffer_unlocksamples(buffer[i]);
        }
        
        bach_freeptr(av);
    }
    
    // output bang
    earsbufobj_outlet_bang((t_earsbufobj *)x, 0);
}


void buf_iter_anything(t_buf_iter *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        long num_bufs = llll_get_num_symbols_root(parsed);
        
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, num_bufs, true);
        earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, inlet);
        
        if (inlet == 0)
            buf_iter_bang(x);
    }
    llll_free(parsed);
}


