/**
	@file
	ears.filterempty.c
 
	@name
	ears.filterempty~
 
	@realname
	ears.filterempty~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Filter out empty buffers
 
	@description
	Only let non-empty buffers through
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, filter, empty, sieve, remove, amplitude
 
	@seealso
	ears.normalize~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_filterempty {
    t_earsbufobj       e_ob;
    
    double             ampthresh;
} t_buf_filterempty;



// Prototypes
t_buf_filterempty*         buf_filterempty_new(t_symbol *s, short argc, t_atom *argv);
void			buf_filterempty_free(t_buf_filterempty *x);
void			buf_filterempty_bang(t_buf_filterempty *x);
void			buf_filterempty_anything(t_buf_filterempty *x, t_symbol *msg, long ac, t_atom *av);

void buf_filterempty_assist(t_buf_filterempty *x, void *b, long m, long a, char *s);
void buf_filterempty_inletinfo(t_buf_filterempty *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(filterempty)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.filterempty~",
                         (method)buf_filterempty_new,
                         (method)buf_filterempty_free,
                         sizeof(t_buf_filterempty),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(filterempty)
    
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_ampunit_attr(c);
//    earsbufobj_class_add_outname_attr(c);
//    earsbufobj_class_add_naming_attr(c);

    
    CLASS_ATTR_DOUBLE(c, "ampthresh", 0, t_buf_filterempty, ampthresh);
    CLASS_ATTR_STYLE_LABEL(c,"ampthresh",0,"text","Amplitude Threshold");
    // @description Sets the amplitude threshold below which a buffer is considered to be empty.
    // Defaults to 0. (in linear scale).

    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_filterempty_assist(t_buf_filterempty *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_filterempty_inletinfo(t_buf_filterempty *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_filterempty *buf_filterempty_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_filterempty *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_filterempty*)object_alloc_debug(s_tag_class);
    if (x) {
        earsbufobj_init((t_earsbufobj *)x,  EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name ampthresh @optional 1 @type number
        // @digest Amplitude threshold
        // @description Sets the amplitude threshold for a buffer to be considered empty
        // (default is 0 in linear scale).

        // @arg 2 @name amp_unit @optional 1 @type symbol
        // @digest Amplitude unit
        // @description Sets the amplitude unit (see <m>ampunit</m> attribute).

        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
        
        if (args && args->l_head) {
            x->ampthresh = hatom_getdouble(&args->l_head->l_hatom);
            if (args->l_head->l_next && hatom_gettype(&args->l_head->l_next->l_hatom) == H_SYM) {
                t_atom av;
                atom_setsym(&av, hatom_getsym(&args->l_head->l_next->l_hatom));
                object_attr_setvalueof(x, gensym("ampunit"), 1, &av);
            }
        }
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E", "E", names);

        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_filterempty_free(t_buf_filterempty *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_filterempty_bang(t_buf_filterempty *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);
    if (num_buffers > 0) {
        
        //    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
        //    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
        
        t_symbol **syms = (t_symbol **)bach_newptr(num_buffers * sizeof(t_symbol));
        long num_syms = 0;
        double ampthresh_linear = earsbufobj_amplitude_to_linear((t_earsbufobj *)x, x->ampthresh);
        
        
        
        earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);
        for (long count = 0; count < num_buffers; count++) {
            
            t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
            double amp = 0;
            t_ears_err err = ears_buffer_get_maxabs((t_object *)x, in, &amp);
            
            if (err == EARS_ERR_NONE && amp > ampthresh_linear) {
                syms[num_syms] = earsbufobj_get_inlet_buffer_name((t_earsbufobj *)x, 0, count);
                num_syms++;
            }
            
            if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
        }
        
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, num_syms, true);
        for (long i = 0; i < num_syms; i++)
            earsbufobj_store_buffer((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, i, syms[i]);

        earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
        
        bach_freeptr(syms);
    }
//    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, out_count, true);
//    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_filterempty_anything(t_buf_filterempty *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long num_bufs = llll_get_num_symbols_root(parsed);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_filterempty_bang(x);
        }
    }
    llll_free(parsed);
}


