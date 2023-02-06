/**
	@file
	ears.unpack.c
 
	@name
	ears.unpack~
 
	@realname
	ears.unpack~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Unpack buffer channels
 
	@description
	Splits buffer channels into a different buffers
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, unpack, combine, channel, buffer
 
	@seealso
	ears.pack~, ears.channel~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "math/bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_unpack {
    t_earsbufobj       e_ob;
    t_llll             *n_channels;
    long               n_numoutbufs;
} t_buf_unpack;



// Prototypes
t_buf_unpack*         buf_unpack_new(t_symbol *s, short argc, t_atom *argv);
void			buf_unpack_free(t_buf_unpack *x);
void			buf_unpack_bang(t_buf_unpack *x);
void			buf_unpack_anything(t_buf_unpack *x, t_symbol *msg, long ac, t_atom *av);

void buf_unpack_assist(t_buf_unpack *x, void *b, long m, long a, char *s);
void buf_unpack_inletinfo(t_buf_unpack *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;



EARSBUFOBJ_ADD_IO_METHODS(unpack)


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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.unpack~",
                         (method)buf_unpack_new,
                         (method)buf_unpack_free,
                         sizeof(t_buf_unpack),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method symbol/llll @digest Unpack buffer channels
    // @description An incoming symbol or llll will be considered as a buffer name, whose channels should be unpacked and output as
    // separate buffers.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(unpack)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_naming_attr(c);

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
}

void buf_unpack_assist(t_buf_unpack *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET)
        sprintf(s, "symbol/list/llll: Buffer"); // @in 0 @type symbol/llll @digest Buffer whose channels should be split
    else {
        if (a < x->n_numoutbufs - 1)
            sprintf(s, "symbol: Channel group No. %ld", a+1); // @out 0 @loop 1 @type symbol @digest Channel group
        else
            sprintf(s, "symbol: Remaining channels"); // @out -1 @type symbol @digest Remaining channels
    }
}



void buf_unpack_inletinfo(t_buf_unpack *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_unpack *buf_unpack_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_unpack *x;
    long true_ac = attr_args_offset(argc, argv);
    long outlets = 1;
    
    x = (t_buf_unpack*)object_alloc_debug(s_tag_class);
    if (x) {
        earsbufobj_init((t_earsbufobj *)x, 0);
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name channels @optional 0 @type list
        // @digest Channels to be unpacked
        // @description Sets the list of channels to be unpacked
        
        x->n_channels = args ? llll_clone(args) : llll_get();

        outlets = args && args->l_head ? args->l_size + 1 : 1;
        
        attr_args_process(x, argc, argv); // this must be called before llllobj_obj_setup

        outlets = MIN(outlets, LLLL_MAX_OUTLETS);
        char temp[LLLL_MAX_OUTLETS + 2];
        long i = 0;
        for (i = 0; i < outlets; i++)
            temp[i] = 'e';
        temp[i] = 0;
        
        x->n_numoutbufs = outlets;
        
        earsbufobj_setup((t_earsbufobj *)x, "e", temp, names);
        
        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_unpack_free(t_buf_unpack *x)
{
    llll_free(x->n_channels);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_unpack_bang(t_buf_unpack *x)
{
    long numoutbufs = x->n_numoutbufs;
    t_llllelem *el = x->n_channels->l_head;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    for (long i = 0; i < numoutbufs; i++)
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, i, 1, true);
    
    t_object *orig = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    if (orig) {
        long num_orig_channels = ears_buffer_get_numchannels((t_object *)x, orig);
        t_rational sum_channels_args_rat = llll_sum_of_rat_llll(x->n_channels); //it'll be a long :)
        long sum_channels_args = rat_num(sum_channels_args_rat);
        
        long channels[EARS_MAX_NUM_CHANNELS];
        long offset = 0;
        for (long i = 0; i < MAX(num_orig_channels, sum_channels_args); i++)
            channels[i] = i;

        for (long i = 0; i < numoutbufs; i++, el = el ? el->l_next : NULL) {
            t_object *thisout = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, i, 0);
            long this_num_channels = el ? hatom_getlong(&el->l_hatom) : MAX(0, num_orig_channels - offset);
            if (this_num_channels == 0) {
                earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, i, 0, false);
            } else {
                ears_buffer_extractchannels((t_object *)x, orig, thisout, this_num_channels, channels + offset);
            }
            offset += this_num_channels;
        }
    }
    
    for (long i = numoutbufs - 1; i >= 0; i--)
        earsbufobj_outlet_buffer((t_earsbufobj *)x, i);
}


void buf_unpack_anything(t_buf_unpack *x, t_symbol *msg, long ac, t_atom *av)
{
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (hatom_gettype(&parsed->l_head->l_hatom) == H_SYM) {
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 1, true);
            earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, 0, hatom_getsym(&parsed->l_head->l_hatom));
        }
        
        buf_unpack_bang(x);
    }
    llll_free(parsed);
}


