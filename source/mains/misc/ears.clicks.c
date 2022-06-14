/**
	@file
	ears.clicks.c
 
	@name
	ears.clicks~
 
	@realname
	ears.clicks~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Put clicks at time points
 
	@description
    Creates a buffer containing a set of clicks at specific onsets
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, click, clicks, collect
 
	@seealso
	ears.fromsamps~, ears.mix~, ears.assemble~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"


typedef struct _buf_clicks {
    t_earsbufobj       e_ob;
    t_llll             *gains;
    t_buffer_obj       *impulse;
    double      sr;
} t_buf_clicks;



// Prototypes
t_buf_clicks*         buf_clicks_new(t_symbol *s, short argc, t_atom *argv);
void			buf_clicks_free(t_buf_clicks *x);
void			buf_clicks_bang(t_buf_clicks *x);
void			buf_clicks_anything(t_buf_clicks *x, t_symbol *msg, long ac, t_atom *av);

void buf_clicks_assist(t_buf_clicks *x, void *b, long m, long a, char *s);
void buf_clicks_inletinfo(t_buf_clicks *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(clicks)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.clicks~",
                         (method)buf_clicks_new,
                         (method)buf_clicks_free,
                         sizeof(t_buf_clicks),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Create buffer
    // @description A list or llll will be interpreted as a set of  that will be put inside a buffer.
    // If the llll has depth 2, each levels of parentheses is considered to be a channel.
    // The buffer name will be then output.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(clicks)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_blocking_attr(c);
    earsbufobj_class_add_timeunit_attr(c);
    earsbufobj_class_add_ampunit_attr(c);

    CLASS_ATTR_DOUBLE(c, "sr", 0, t_buf_clicks, sr);
    CLASS_ATTR_STYLE_LABEL(c,"sr",0,"text","Output Sample Rate");
    CLASS_ATTR_BASIC(c, "sr", 0);
    // @description Sets the sample rate for the output buffer. If zero (default) then the current Max sample rate is used.

    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_clicks_assist(t_buf_clicks *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "llll: Onsets"); // @in 0 @type llll @digest Onsets
            // @description List of onsets (unit depends on the <m>timeunit</m> attribute)
        else if (a == 1)
            sprintf(s, "llll: Gains"); // @in 1 @type llll @digest Gains
            // @description List of gains (unit depends on the <m>ampunit</m> attribute)
        else
            sprintf(s, "llll: Impulse"); // @in 1 @type llll @digest Impulse samples
    } else {
        sprintf(s, "symbol: Output Buffer"); // @out 0 @type symbol @digest Output Buffer
    }
}

void buf_clicks_inletinfo(t_buf_clicks *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_clicks *buf_clicks_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_clicks *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_clicks*)object_alloc_debug(s_tag_class);
    if (x) {
        earsbufobj_init((t_earsbufobj *)x, 0);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR
        
        
        t_llll *args = llll_parse(true_ac, argv);
        t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);

        attr_args_process(x, argc, argv);

        x->gains = llll_make();
        x->impulse = ears_buffer_make(NULL);
        ears_buffer_set_sr((t_object *)x, x->impulse, x->sr > 0 ? x->sr : ears_get_current_Max_sr());
        t_llll *temp = llll_from_text_buf("[1]");
        ears_buffer_from_llll((t_object *)x, x->impulse, temp, 1);
        llll_free(temp);
        
        earsbufobj_setup((t_earsbufobj *)x, "444", "e", names);

        
        llll_free(args);
        llll_free(names);
    }
    return x;
}


void buf_clicks_free(t_buf_clicks *x)
{
    llll_free(x->gains);
    ears_buffer_free(x->impulse);
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_clicks_bang(t_buf_clicks *x)
{
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


long convert_timeunit_fn(void *data, t_hatom *a, const t_llll *address){
    t_buf_clicks *x = (t_buf_clicks *) data;
    
    if (is_hatom_number(a) && x->e_ob.l_timeunit != EARS_TIMEUNIT_SAMPS){
        hatom_setdouble(a, earsbufobj_convert_timeunit((t_earsbufobj *)x, hatom_getdouble(a), x->impulse, EARS_TIMEUNIT_SAMPS));
    }
    return 0;
}

long convert_ampunit_fn(void *data, t_hatom *a, const t_llll *address){
    t_buf_clicks *x = (t_buf_clicks *) data;
    
    if (is_hatom_number(a) && x->e_ob.l_ampunit != EARS_AMPUNIT_LINEAR){
        hatom_setdouble(a, ears_convert_ampunit(hatom_getdouble(a), (e_ears_ampunit)x->e_ob.l_ampunit, EARS_AMPUNIT_LINEAR));
    }
    return 0;
}


void buf_clicks_anything(t_buf_clicks *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed) {
        if (inlet == 2) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            t_llll *temp = llll_clone(parsed);
            llll_wrap_once(&temp);
            ears_buffer_set_sr((t_object *)x, x->impulse, x->sr > 0 ? x->sr : ears_get_current_Max_sr());
            ears_buffer_from_llll((t_object *)x, x->impulse, temp, 1);
            llll_free(temp);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else if (inlet == 1) {
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            llll_free(x->gains);
            x->gains = llll_clone(parsed);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
        } else if (inlet == 0){
            earsbufobj_mutex_lock((t_earsbufobj *)x);
            if (parsed->l_depth == 1) // flat llll = 1 channel only
                llll_wrap_once(&parsed);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, 1, true);
            t_llll *onsets_samps = llll_clone(parsed);
            t_llll *gains_linear = llll_clone(x->gains);
            if (gains_linear->l_depth == 1) // flat llll = 1 channel only
                llll_wrap_once(&gains_linear);
            llll_funall(onsets_samps, (fun_fn) convert_timeunit_fn, x, 1, -1, FUNALL_ONLY_PROCESS_ATOMS);
            llll_funall(gains_linear, (fun_fn) convert_ampunit_fn, x, 1, -1, FUNALL_ONLY_PROCESS_ATOMS);
            ears_buffer_from_clicks((t_object *)x, earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, 0), onsets_samps, gains_linear, x->impulse);
            llll_free(onsets_samps);
            llll_free(gains_linear);
            earsbufobj_mutex_unlock((t_earsbufobj *)x);
            buf_clicks_bang(x);
        }
    }
    llll_free(parsed);
}


