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
	ears.collect~, ears.expr~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_iter {
    t_earsbufobj       e_ob;
    long               e_num_inlets;
    long               e_iterationmode;
    t_symbol           *buffers[LLLL_MAX_INLETS];
    long               e_iteration_mode;
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

int C74_EXPORT main(void)
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
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

    earsbufobj_class_add_blocking_attr(c);
//    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_naming_attr(c);

    CLASS_ATTR_LONG(c, "iterationmode",    0,    t_buf_iter, e_iterationmode);
    CLASS_ATTR_STYLE_LABEL(c, "iterationmode", 0, "enumindex", "Iteration Mode");
    CLASS_ATTR_FILTER_CLIP(c, "iterationmode", 0, 2);
    CLASS_ATTR_ENUMINDEX(c, "iterationmode", 0, "Shortest Longest Zeros");

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_iter_assist(t_buf_iter *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a < x->e_num_inlets) // @in 0 @loop 1 @type buffer @digest buffer whose sample must be iterated
            sprintf(s, "llll %ld", a + 1);
    } else {
        if (a == x->e_num_inlets + 1)
            sprintf(s, "int: Sample Index");    // @out 1 @type int @digest Sample Index
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
        
        // @arg 1 @name numbuffers @optional 1 @type number
        // @digest Number of buffers
        // @description Sets the number of buffers to be iterated in parallel.

        t_llll *args = llll_parse(true_ac, argv);
        
        if (args && args->l_head && is_hatom_number(&args->l_head->l_hatom)) {
            x->e_num_inlets = CLAMP(hatom_getlong(&args->l_head->l_hatom), 0, LLLL_MAX_INLETS-1);
        }
        

        attr_args_process(x, argc, argv);
        
        char inlets[LLLL_MAX_INLETS];
        char outlets[LLLL_MAX_OUTLETS];
        outlets[0] = 'i';
        for (long i = 0; i < x->e_num_inlets; i++) {
            inlets[i] = 'e';
            outlets[i+1] = 'z';
        }
        outlets[x->e_num_inlets+1] = 'b';
        inlets[x->e_num_inlets] = 0;
        outlets[x->e_num_inlets+2] = 0;
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
    long minsamps = LONG_MAX, maxsamps = 0;
    long maxnumchannels = 0;
    long iterationmode = x->e_iterationmode;
    
    for (long i = 0; i < num_inlets; i++) {
        buffer[i] = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, i, 0);
        samps[i] = buffer[i] ? buffer_locksamples(buffer[i]) : NULL;
        sizesamps[i] = buffer_getframecount(buffer[i]);
        if (sizesamps[i] > maxsamps)
            maxsamps = sizesamps[i];
        if (sizesamps[i] < minsamps)
            minsamps = sizesamps[i];
        numchannels[i] = buffer_getchannelcount(buffer[i]);
        if (numchannels[i] > maxnumchannels)
            maxnumchannels = numchannels[i];
    }

    // all buffers are now locked. Let's send out the information
    long limitsamps = (iterationmode == 0 ? minsamps : maxsamps);

    t_atom *av = (t_atom *)bach_newptr((maxnumchannels + 1) * sizeof(t_atom));
    for (long f = 0; f < limitsamps; f++) {
        earsbufobj_outlet_int((t_earsbufobj *)x, num_inlets+1, f);
        
        for (long i = num_inlets - 1; i >= 0; i--) {

            if (numchannels[i] == 1) {
                if (f < sizesamps[i]) {
                    atom_setfloat(av, samps[i][f]);
                    earsbufobj_outlet_anything((t_earsbufobj *)x, i+1, _sym_float, 1, av);
                } else if (iterationmode == 2) {
                    atom_setfloat(av, 0.);
                    earsbufobj_outlet_anything((t_earsbufobj *)x, i+1, _sym_float, 1, av);
                }
            } else {
                if (f < sizesamps[i]) {
                    for (long c = 0; c < numchannels[i]; c++)
                        atom_setfloat(av+c, samps[i][f*numchannels[i] + c]);
                    earsbufobj_outlet_anything((t_earsbufobj *)x, i+1, _sym_list, numchannels[i], av);
                } else if (iterationmode == 2) {
                    for (long c = 0; c < numchannels[i]; c++)
                        atom_setfloat(av+c, 0.);
                    earsbufobj_outlet_anything((t_earsbufobj *)x, i+1, _sym_list, numchannels[i], av);
                }
            }
        }
    }
    
    for (long i = 0; i < num_inlets; i++) {
        if (samps[i] && buffer[i])
            buffer_unlocksamples(buffer[i]);
    }
    
    bach_freeptr(av);
    
    // output bang
    earsbufobj_outlet_bang((t_earsbufobj *)x, 0);
}


void buf_iter_anything(t_buf_iter *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        t_symbol *buffername = hatom_getsym(&parsed->l_head->l_hatom);
        if (buffername) {
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, 1, true);
            earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, 0, buffername);
            if (inlet == 0)
                buf_iter_bang(x);
        }
    }
    llll_free(parsed);
}


