/**
	@file
	ears.expr.c
 
	@name
	ears.expr~
 
	@realname
	ears.expr~
 
    @hiddenalias
    ears.expr

	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Evaluate mathematical expressions on buffers
 
	@description
	Performs arithmetic operations on buffers
 
	@discussion
 
	@category
	ears buffer operations
 
	@keywords
	buffer, expr, sum, add, operation, arithmetic, arithmetics
 
	@seealso
	ears.mix~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"



typedef struct _buf_expr {
    t_earsbufobj       e_ob;
    
    char                normalization_mode;
    
    long                n_maxvars;
    t_lexpr                *n_lexpr;
    t_llll                *n_dummy;
    t_llll                *n_empty;
    t_bach_atomic_lock    n_lock;
} t_buf_expr;



// Prototypes
t_buf_expr*         buf_expr_new(t_symbol *s, short argc, t_atom *argv);
void			buf_expr_free(t_buf_expr *x);
void			buf_expr_bang(t_buf_expr *x);
void			buf_expr_anything(t_buf_expr *x, t_symbol *msg, long ac, t_atom *av);

void buf_expr_assist(t_buf_expr *x, void *b, long m, long a, char *s);
void buf_expr_inletinfo(t_buf_expr *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(expr)


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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.expr~",
                         (method)buf_expr_new,
                         (method)buf_expr_free,
                         sizeof(t_buf_expr),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(expr)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);

    
    CLASS_ATTR_CHAR(c, "normalize", 0, t_buf_expr, normalization_mode);
    CLASS_ATTR_STYLE_LABEL(c,"normalize",0,"enumindex","Normalize Output");
    CLASS_ATTR_ENUMINDEX(c,"normalize", 0, "Never Always Overload Protection Only");
    CLASS_ATTR_BASIC(c, "normalize", 0);
    // @description Sets the normalization mode for the output buffer:
    // 0 = Never, does not normalize; 1 = Always, always normalizes to 1.; 2 = Overload Protection Only, only
    // normalizes to 1. if some samples exceed in modulo 1.
    

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_expr_assist(t_buf_expr *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET)
        sprintf(s, "symbol/llll: Argument buffer"); // @in 0 @loop 1 @type symbol/llll @digest Buffer to be given as argument
    else {
        sprintf(s, "symbol: Output buffer"); // @out 0 @type symbol @digest Resulting buffer
    }
}



void buf_expr_inletinfo(t_buf_expr *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


char is_all_alphanumerical(t_symbol *s)
{
    if (!s)
        return false;
    
    long len = strlen(s->s_name);
    
    if (len == 0)
        return false;
    
    for (long i = 0; i < len; i++)
        if (!isalnum(s->s_name[i]))
            return false;
    
    return true;
}

t_buf_expr *buf_expr_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_expr *x;
    long true_ac = attr_args_offset(argc, argv);
    long proxies = 0;
    t_atom *true_av = argv;
    
    x = (t_buf_expr*)object_alloc_debug(s_tag_class);
    if (x) {

        x->normalization_mode = EARS_NORMALIZE_OVERLOAD_PROTECTION_ONLY;
        
        
        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        t_llll *names = NULL;
        if (true_ac && atom_gettype(true_av) == A_SYM && is_all_alphanumerical(atom_getsym(true_av))) {
            // must not be a lexprable stuff!
            names = llll_get();
            llll_appendsym(names, atom_getsym(true_av));
            true_ac--;
            true_av++;
        }

        
        
        // @arg 1 @name expression @optional 0 @type anything @digest Expression to evaluate
        // @description The expression can contain variables, in the form <m>$in</m>, <m>$rn</m>, <m>$fn</m>, <m>$xn</m>, where <m>n</m> stands for an inlet number.
        // <m>$in</m> will refer to an integer value extracted from the llll received in the n-th inlet;
        // likewise, <m>$rn</m> will refer to a rational or integer value and <m>$fn</m> to a floating-point value.
        // <m>$xn</m> will refer to an untyped value, meaning that the type of the value extracted from the llll will be taken as-is, including symbols (see the description of the <m>llll</m> method).
        // Unless specific needs, <m>$xn</m> should be considered the preferred way to express variables.
        // For a complete list of the mathematical operators and functions supported please refer to <o>expr</o>'s help file.
        
        if (true_ac) {
            x->n_lexpr = lexpr_new(true_ac, true_av, 0, NULL, (t_object *) x);
            if (x->n_lexpr) {
                //                object_post((t_object *) x, "good expr!");
                x->n_maxvars = x->n_lexpr->l_numvars;
            } else {
                object_error((t_object *) x, "Bad expression");
                //                post("true_ac = %ld, av = %p", true_ac, av);
            }
        }

        /*
        i = true_ac;
        while (i < ac - 1) {
            t_symbol *symattr = atom_getsym(av + i);
            if (!symattr || *symattr->s_name != '@') {
                object_error((t_object *) x, "Bad argument at position %ld", i);
                i++;
                break;
            }
            const char *attrname = symattr->s_name + 1;
            i++;
            if (!strcmp(attrname, "maxvars")) {
                long type = atom_gettype(av + i);
                if (type == A_LONG || type == A_FLOAT) {
                    t_atom_long val = atom_getlong(av + i);
                    x->n_maxvars = CLAMP(val, 0, LLLL_MAX_INLETS);
                    i++;
                } else
                    object_error((t_object *) x, "Bad value for ins attribute");
            } else if (!strcmp(attrname, "scalarmode")) {
                long type = atom_gettype(av + i);
                if (type == A_LONG || type == A_FLOAT) {
                    t_atom_long val = atom_getlong(av + i);
                    x->n_scalarmode = CLAMP(val, 0, 1);
                    i++;
                } else
                    object_error((t_object *) x, "Bad value for scalarmode attribute");
            } else if (!strcmp(attrname, "out")) {
                llllobj_obj_setout((t_llllobj_object *) x, NULL, 1, av + i);
                i++;
            } else
                object_error((t_object *) x, "Unknown attribute %s", attrname);
        }
        if (x->n_maxvars < 1)
            x->n_maxvars = 1;
         */
        
        llllobj_obj_setup((t_llllobj_object *) x, x->n_maxvars, "4");
        
        proxies = CLAMP(x->n_maxvars - 1, 0, LLLL_MAX_INLETS);
        
        x->n_dummy = llll_get();
        llll_appendobj(x->n_dummy, NULL, 0, WHITENULL_llll);
        llll_retain(x->n_dummy);
        x->n_empty = llll_get();
        llll_retain(x->n_empty);

        
        proxies = MIN(proxies, LLLL_MAX_INLETS);
        char temp[LLLL_MAX_INLETS + 2];
        long i = 0;
        temp[0] = 'e';
        for (i = 0; i < proxies; i++)
            temp[i+1] = 'e';
        temp[i+1] = 0;
        
        earsbufobj_init((t_earsbufobj *)x, EARSBUFOBJ_FLAG_DONT_DUPLICATE_INPUT_BUFFERS);
        attr_args_process(x, argc, argv);
        earsbufobj_setup((t_earsbufobj *)x, temp, "e", names);

        llll_free(names);
    }
    return x;
}


void buf_expr_free(t_buf_expr *x)
{
    
//    t_llll *stored = x->e_ob.l_ob.l_incache[0].s_ll;
    lexpr_free(x->n_lexpr);
    llll_release(x->n_dummy);
    llll_release(x->n_empty);
//    if (stored != x->n_dummy)
        llll_free(x->n_dummy); // otherwise it will be released by llllobj_obj_free
//    if (stored != x->n_empty)
        llll_free(x->n_empty); // otherwise it will be released by llllobj_obj_free
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_expr_bang(t_buf_expr *x)
{
    char normalization_mode = x->normalization_mode;
    long num_arguments = x->e_ob.l_numins;
    t_buffer_obj **arguments = (t_buffer_obj **)bach_newptrclear(num_arguments * sizeof(t_buffer_obj *)); // initialized with all NULLs
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
    
    for (long i = 0; i < num_arguments; i++) {
        if (((t_earsbufobj *)x)->l_instore[i].num_stored_bufs > 0)
            arguments[i] = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, i, 0);
    }

    t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0);
    ears_buffer_expr((t_object *)x, x->n_lexpr, arguments, num_arguments, out, (e_ears_normalization_modes)normalization_mode);
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
    bach_freeptr(arguments);
}


void buf_expr_anything(t_buf_expr *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);
        
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, parsed->l_size, true);
        earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, inlet, inlet == 0);
        
        if (inlet == 0)
            buf_expr_bang(x);
    }
    llll_free(parsed);
}


