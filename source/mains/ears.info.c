/**
	@file
	ears.info.c
 
	@name
	ears.info~
 
	@realname
	ears.info~
 
	@type
	object
 
	@module
	ears
 
	@author
	Daniele Ghisi
 
	@digest
	Get information on buffers
 
	@description
	Retrieves information about buffers
 
	@discussion
 
	@category
	ears basic
 
	@keywords
	buffer, info, get, retrieve, rms, amplitude, peak
 
	@seealso
	info~, ears.essentia~
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"


enum {
    EARS_BUF_INFO_NONE = 0,
    EARS_BUF_INFO_MIN,
    EARS_BUF_INFO_MAX,
    EARS_BUF_INFO_MAXABS,
    EARS_BUF_INFO_RMS,
    EARS_BUF_INFO_DURATION,
    EARS_BUF_INFO_NUMCHANNELS,
    EARS_BUF_INFO_SR,
    EARS_BUF_INFO_NUMSAMPLES,
    // spectral buffer stuff
    EARS_BUF_INFO_SPECTRAL,
    EARS_BUF_INFO_AUDIOSR,
    EARS_BUF_INFO_BINSIZE,
    EARS_BUF_INFO_BINOFFSET,
    EARS_BUF_INFO_SPECTYPE,
    EARS_BUF_INFO_SPECFREQUNIT,
};

typedef struct _buf_info {
    t_earsbufobj       e_ob;
    
    long               outlet_info[LLLL_MAX_OUTLETS];
    long               num_outlets;
} t_buf_info;



// Prototypes
t_buf_info*         buf_info_new(t_symbol *s, short argc, t_atom *argv);
void			buf_info_free(t_buf_info *x);
void			buf_info_bang(t_buf_info *x);
void			buf_info_anything(t_buf_info *x, t_symbol *msg, long ac, t_atom *av);

void buf_info_assist(t_buf_info *x, void *b, long m, long a, char *s);
void buf_info_inletinfo(t_buf_info *x, void *b, long a, char *t);


// Globals and Statics
static t_class	*s_info_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(info)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.info~",
                         (method)buf_info_new,
                         (method)buf_info_free,
                         sizeof(t_buf_info),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method symbol/llll @digest Get buffer information
    // @description A symbol or an llll with a single symbol is considered as a buffer name, whose information (depending on the arguments) is output.
    EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(info) // TODO: should we NOT defer ears.info~?
    
    earsbufobj_add_common_methods(c);
    
    earsbufobj_class_add_ampunit_attr(c);
    earsbufobj_class_add_timeunit_attr(c);

    
    class_register(CLASS_BOX, c);
    s_info_class = c;
    ps_event = gensym("event");
    return 0;
}


long sym2info(t_symbol *s)
{
    if (s == gensym("min"))
        return EARS_BUF_INFO_MIN;

    if (s == gensym("max"))
        return EARS_BUF_INFO_MAX;

    if (s == gensym("maxabs"))
        return EARS_BUF_INFO_MAXABS;

    if (s == gensym("rms"))
        return EARS_BUF_INFO_RMS;

    if (s == gensym("length") || s == gensym("duration"))
        return EARS_BUF_INFO_DURATION;

    if (s == gensym("numchannels"))
        return EARS_BUF_INFO_NUMCHANNELS;

    if (s == gensym("sr"))
        return EARS_BUF_INFO_SR;

    if (s == gensym("numsamples"))
        return EARS_BUF_INFO_NUMSAMPLES;

    if (s == gensym("spectral"))
        return EARS_BUF_INFO_SPECTRAL;

    if (s == gensym("audiosr"))
        return EARS_BUF_INFO_AUDIOSR;

    if (s == gensym("binsize"))
        return EARS_BUF_INFO_BINSIZE;
    
    if (s == gensym("binoffset"))
        return EARS_BUF_INFO_BINOFFSET;
    
    if (s == gensym("spectype"))
        return EARS_BUF_INFO_SPECTYPE;
    
    if (s == gensym("binunit"))
        return EARS_BUF_INFO_SPECFREQUNIT;

    return EARS_BUF_INFO_NONE;
}

t_symbol *info2sym(long info)
{
    switch (info) {
        case EARS_BUF_INFO_MIN: return gensym("min");
        case EARS_BUF_INFO_MAX: return gensym("max");
        case EARS_BUF_INFO_MAXABS: return gensym("maxabs");
        case EARS_BUF_INFO_RMS: return gensym("rms");
        case EARS_BUF_INFO_DURATION: return gensym("duration");
        case EARS_BUF_INFO_NUMCHANNELS: return gensym("numchannels");
        case EARS_BUF_INFO_SR: return gensym("sr");
        case EARS_BUF_INFO_NUMSAMPLES: return gensym("numsamples");
        case EARS_BUF_INFO_SPECTRAL: return gensym("spectral");
        case EARS_BUF_INFO_AUDIOSR: return gensym("audiosr");
        case EARS_BUF_INFO_BINSIZE: return gensym("binsize");
        case EARS_BUF_INFO_BINOFFSET: return gensym("binoffset");
        case EARS_BUF_INFO_SPECTYPE: return gensym("spectype");
        case EARS_BUF_INFO_SPECFREQUNIT: return gensym("binunit");
        default: return _llllobj_sym_none;
    }
}



t_symbol *info2type(t_buf_info *x, long info)
{
    switch (info) {
        case EARS_BUF_INFO_MIN: 
        case EARS_BUF_INFO_MAX: 
        case EARS_BUF_INFO_MAXABS:
        case EARS_BUF_INFO_RMS:
            return _sym_float;
            
        case EARS_BUF_INFO_DURATION:
            switch (x->e_ob.l_timeunit) {
                case EARS_TIMEUNIT_MS: return _sym_float;
                case EARS_TIMEUNIT_SECONDS: return _sym_float;
                case EARS_TIMEUNIT_SAMPS: return _sym_int;
                default: return _sym_float;
            };
            break;
            
        case EARS_BUF_INFO_NUMCHANNELS:
        case EARS_BUF_INFO_NUMSAMPLES:
        case EARS_BUF_INFO_SPECTRAL:
            return _sym_int;
        
        case EARS_BUF_INFO_SR:
        case EARS_BUF_INFO_AUDIOSR:
        case EARS_BUF_INFO_BINSIZE:
        case EARS_BUF_INFO_BINOFFSET:
            return _sym_float;


        case EARS_BUF_INFO_SPECTYPE:
        case EARS_BUF_INFO_SPECFREQUNIT:
            return _sym_symbol;
            
        default:
            return _sym_float;
    }
}

t_ears_err buf_info_get_analysis(t_buf_info *x, t_buffer_obj *buf, long info, t_atom *res)
{
    t_ears_err err = EARS_ERR_NONE;
    double val = 0;
    switch (info) {
        case EARS_BUF_INFO_MIN:
            err = ears_buffer_get_minmax((t_object *)x, buf, &val, NULL);
            val = earsbufobj_linear_to_output((t_earsbufobj *)x, val);
            atom_setfloat(res, val);
            break;
            
        case EARS_BUF_INFO_MAX:
            err = ears_buffer_get_minmax((t_object *)x, buf, NULL, &val);
            val = earsbufobj_linear_to_output((t_earsbufobj *)x, val);
            atom_setfloat(res, val);
            break;
            
        case EARS_BUF_INFO_MAXABS:
            err = ears_buffer_get_maxabs((t_object *)x, buf, &val);
            if (err == EARS_ERR_EMPTY_BUFFER)
                object_warn((t_object *)x, EARS_ERROR_BUF_EMPTY_BUFFER);
            val = earsbufobj_linear_to_output((t_earsbufobj *)x, val);
            atom_setfloat(res, val);
            break;

        case EARS_BUF_INFO_RMS:
            err = ears_buffer_get_rms((t_object *)x, buf, &val);
            val = earsbufobj_linear_to_output((t_earsbufobj *)x, val);
            atom_setfloat(res, val);
            break;
            
        case EARS_BUF_INFO_DURATION:
            switch (x->e_ob.l_timeunit) {
                case EARS_TIMEUNIT_MS:
                    atom_setfloat(res, ears_buffer_get_size_ms((t_object *)x, buf));
                    break;
                case EARS_TIMEUNIT_SECONDS:
                    atom_setfloat(res, ears_buffer_get_size_ms((t_object *)x, buf)/1000.);
                    break;
                case EARS_TIMEUNIT_SAMPS:
                    atom_setlong(res, ears_buffer_get_size_samps((t_object *)x, buf));
                    break;
                default:
                    atom_setfloat(res, 1.);
                    break;
            };
            break;
            
        case EARS_BUF_INFO_NUMCHANNELS:
            atom_setlong(res, ears_buffer_get_numchannels((t_object *)x, buf));
            break;

        case EARS_BUF_INFO_NUMSAMPLES:
            atom_setlong(res, ears_buffer_get_size_samps((t_object *)x, buf));
            break;

        case EARS_BUF_INFO_SR:
            atom_setfloat(res, ears_buffer_get_sr((t_object *)x, buf));
            break;

        case EARS_BUF_INFO_SPECTRAL:
            atom_setlong(res, ears_buffer_is_spectral((t_object *)x, buf));
            break;

        case EARS_BUF_INFO_AUDIOSR:
            atom_setfloat(res, ears_spectralbuf_get_original_audio_sr((t_object *)x, buf));
            break;

        case EARS_BUF_INFO_BINSIZE:
            atom_setfloat(res, ears_spectralbuf_get_binsize((t_object *)x, buf));
            break;
            
        case EARS_BUF_INFO_BINOFFSET:
            atom_setfloat(res, ears_spectralbuf_get_binoffset((t_object *)x, buf));
            break;
            
        case EARS_BUF_INFO_SPECTYPE:
            atom_setsym(res, ears_spectralbuf_get_spectype((t_object *)x, buf));
            break;
            
        case EARS_BUF_INFO_SPECFREQUNIT:
            atom_setsym(res, ears_frequnit_to_symbol(ears_spectralbuf_get_binunit((t_object *)x, buf)));
            break;
            
        default:
            atom_setfloat(res, 0.);
            break;
    }
    
    return err;
}




void buf_info_assist(t_buf_info *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
    } else {
        t_symbol *sym = info2sym(x->outlet_info[a]);
        sprintf(s, "number/list: %s", sym ? sym->s_name : (char *)"unknown property");
        // @out 0 @loop 1 @type number/list @digest Property value for the corresponding key
    }
}

void buf_info_inletinfo(t_buf_info *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_info *buf_info_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_info *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_info*)object_alloc_debug(s_info_class);
    if (x) {

        earsbufobj_init((t_earsbufobj *)x, 0);
        
        // @arg 0 @name tags @optional 0 @type symbol/list
        // @digest Required buffer information tags
        // @description A list of symbols among the following ones: "min", "max", "maxabs", "rms", "length" (or "duration"),
        // "numchannels", "sr", "numsamples". Plus, for spectral buffers: "spectral", "audiosr", "binsize", "binoffset",
        // "spectype", "binunit". For every symbol an outlet is created, which
        // will output the corresponding information.
        

        t_llll *args = llll_parse(true_ac, argv);
        
        char outlet_str[LLLL_MAX_OUTLETS + 1];
        x->num_outlets = 0;
        for (t_llllelem *elem = args->l_head; elem && x->num_outlets < LLLL_MAX_OUTLETS; elem = elem->l_next) {
            t_symbol *tagsym = hatom_getsym(&elem->l_hatom);
            x->outlet_info[x->num_outlets] = sym2info(tagsym);
            if (x->outlet_info[x->num_outlets] == EARS_BUF_INFO_NONE) {
                object_error((t_object *)x, "Unknown tag: '%s'", tagsym ? tagsym->s_name : "???");
                llll_free(args);
                return NULL;
            }
            outlet_str[x->num_outlets] = 'z';
            x->num_outlets++;
        }
        outlet_str[x->num_outlets] = 0;
        
        if (x->num_outlets <= 0) {
            llll_free(args);
            object_free_debug(x); // unlike freeobject(), this works even if the argument is NULL
            return NULL;
        }
        
        
        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E", outlet_str, NULL);

        llll_free(args);
    }
    return x;
}


void buf_info_free(t_buf_info *x)
{
    earsbufobj_free((t_earsbufobj *)x);
}



void buf_info_bang(t_buf_info *x)
{
    long num_buffers = earsbufobj_get_instore_size((t_earsbufobj *)x, 0);

    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    earsbufobj_mutex_lock((t_earsbufobj *)x);
    earsbufobj_init_progress((t_earsbufobj *)x, num_buffers);

    long num_outlets = x->num_outlets;
    t_atom *av[LLLL_MAX_OUTLETS];
    
    for (long i = 0; i < num_outlets; i++)
        av[i] = (t_atom *)bach_newptr(num_buffers * sizeof(t_atom));
    
    for (long count = 0; count < num_buffers; count++) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        for (long i = 0; i < num_outlets; i++)
            buf_info_get_analysis(x, in, x->outlet_info[i], av[i]+count);
        if (earsbufobj_iter_progress((t_earsbufobj *)x, count, num_buffers)) break;
    }

    earsbufobj_mutex_unlock((t_earsbufobj *)x);
    
    for (long i = 0; i < num_outlets; i++)
        earsbufobj_outlet_anything((t_earsbufobj *)x, i, num_buffers > 1 || info2type(x, x->outlet_info[i]) == _sym_symbol ? _llllobj_sym_list : info2type(x, x->outlet_info[i]), num_buffers, av[i]);

    for (long i = 0; i < num_outlets; i++)
        bach_freeptr(av[i]);
}


void buf_info_anything(t_buf_info *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = earsbufobj_proxy_getinlet((t_earsbufobj *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
            long num_bufs = llll_get_num_symbols_root(parsed);

            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_bufs, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_info_bang(x);
        }
    }
    llll_free(parsed);
}


