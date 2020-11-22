/**
	@file
	ears.process.c
 
	@name
	ears.process~
 
	@realname
	ears.process~
 
	@type
	object
 
	@module
	ears
 
    @status
    hidden
 
	@author
	Daniele Ghisi
 
	@digest
	Process buffers through patches
 
	@description
	Processes the incoming buffers through a poly~-like system
 
	@discussion
 
	@category
	ears process
 
	@keywords
	buffer, process, apply, poly
 
	@seealso
    
	
	@owner
	Daniele Ghisi
 */

#include "ext.h"
#include "ext_obex.h"
#include "llllobj.h"
#include "llll_commons_ext.h"
#include "bach_math_utilities.h"
#include "ears.object.h"

#define MAX_NUM_PATCHES 4096
#define MAX_ARGS 50

////////////////////////////// Structure for patch and related data //////////////////////////////

typedef struct _patchspace
{
    t_patcher*            the_patch;
    struct _dspchain*    the_dspchain;
    t_symbol*            patch_name_in;
    char                patch_name[256];
    short                patch_path;
    
    short                x_argc;                // Arguments (stored in case of reload / update)
    t_atom                x_argv[MAX_ARGS];
    
    double**            out_ptrs;            // Pointer to Audio Out Buffers
    
    char                patch_valid;
    char                patch_on;
    
} t_patchspace;

////////////////////////////////////// ins/outs ////////////////////////////////////////////////

typedef void *t_outvoid;

typedef struct _inout
{
    t_object    s_obj;
    long        s_index;
    void*        s_outlet;
    
} t_inout;

typedef struct _io_infos
{
    long sig_ins;
    long sig_outs;
    long ins;
    long outs;
    
    long sig_ins_maxindex;
    long sig_outs_maxindex;
    long ins_maxindex;
    long outs_maxindex;
    
    long extra_sig_ins;
    long extra_sig_outs;
    long extra_ins;
    long extra_outs;
    
    long extra_sig_ins_maxindex;
    long extra_sig_outs_maxindex;
    long extra_ins_maxindex;
    long extra_outs_maxindex;
    
} t_io_infos;


/////// the actual structure

typedef struct _buf_pan {
    t_earsbufobj       e_ob;
    t_llll             *params;
    
    t_patcher *parent_patch;
    
    // Patch Data and Variables
    
    t_patchspace *patch_space_ptrs[MAX_NUM_PATCHES];
    long patch_spaces_allocated;
    
    t_int32_atomic patch_is_loading;
    
    long target_index;
    
    long last_vec_size;
    long last_samp_rate;
    
    // IO Variables
    
    long mode_default_numins;
    long mode_default_numouts;
    
    long instance_sig_ins;
    long instance_sig_outs;
    long instance_ins;
    long instance_outs;
    
    long extra_sig_ins;
    long extra_sig_outs;
    long extra_ins;
    long extra_outs;
    
    long declared_sig_ins;
    long declared_sig_outs;
    long declared_ins;
    long declared_outs;
    
    void **sig_ins;
    void **sig_outs;
    
    t_outvoid *in_table;            // table of non-signal inlets
    t_outvoid *out_table;            // table of non-signal outlets
    long num_proxies;                // number of proxies
    
    // Hoa stuff
    Processor<Hoa2d, t_sample>::Harmonics*  f_ambi2D;
    Processor<Hoa3d, t_sample>::Harmonics*  f_ambi3D;
    ulong                                   f_order;
    t_symbol*                               f_mode;
    e_hoa_object_type                       f_object_type;

} t_buf_process;


enum {
    EARS_PAN_RANGETYPE_CUSTOM = 0,
    EARS_PAN_RANGETYPE_0_1 = 1,
    EARS_PAN_RANGETYPE_m1_1 = 2,
    EARS_PAN_RANGETYPE_NUM_0based = 3,
    EARS_PAN_RANGETYPE_NUM_1based = 4,
};


// Prototypes
t_buf_process*         buf_process_new(t_symbol *s, short argc, t_atom *argv);
void			buf_process_free(t_buf_process *x);
void			buf_process_bang(t_buf_process *x);
void			buf_process_anything(t_buf_process *x, t_symbol *msg, long ac, t_atom *av);

void buf_process_assist(t_buf_process *x, void *b, long m, long a, char *s);
void buf_process_inletinfo(t_buf_process *x, void *b, long a, char *t);


void buf_process_dblclick(t_buf_process *x);
void buf_process_open(t_buf_process *x, t_symbol *msg, short argc, t_atom *argv);
void buf_process_doopen(t_buf_process *x, t_symbol *s, short argc, t_atom *argv);
void buf_process_wclose(t_buf_process *x, t_symbol *msg, short argc, t_atom *argv);
void buf_process_dowclose(t_buf_process *x, t_symbol *s, short argc, t_atom *argv);


// Globals and Statics
static t_class	*s_tag_class = NULL;
static t_symbol	*ps_event = NULL;

EARSBUFOBJ_ADD_IO_METHODS(pan)

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
    
    CLASS_NEW_CHECK_SIZE(c, "ears.pan",
                         (method)buf_process_new,
                         (method)buf_process_free,
                         sizeof(t_buf_process),
                         (method)NULL,
                         A_GIMME,
                         0L);
    
    // @method list/llll @digest Process buffers
    // @description A list or llll with buffer names will trigger the buffer processing and output the processed
    // buffer names (depending on the <m>naming</m> attribute).
    EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(pan)
    
    earsbufobj_class_add_outname_attr(c);
    earsbufobj_class_add_naming_attr(c);
    
    // @method open @digest open a patcher instance for viewing.
    // @description The word open, followed by a number, opens the specified instance of the patcher (depending on the process mode). You can view the activity of any instance of the patcher up to the number of loaded instances. With no arguments, the open message opens the instance that is currently the target (see the <m>target</m> message).
    // @marg 0 @name instance-index @optional 1 @type int
    class_addmethod(c, (method)buf_process_open,                        "open",                    A_GIMME,  0);
    
    // @method wclose @digest close a numbered patcher instance's window.
    // @description Closes the instance window.
    // @marg 0 @name instance-index @optional 1 @type int
    class_addmethod(c, (method)buf_process_wclose,                    "wclose",                A_GIMME,  0);
    
    // @method (mouse) @digest double-click to open a display window to view loaded patch contents.
    // @description Double-clicking opens a display window of the instance that is currently the target.
    class_addmethod(c, (method)buf_process_dblclick,                    "dblclick",                A_CANT,   0);
    
    
    class_register(CLASS_BOX, c);
    s_tag_class = c;
    ps_event = gensym("event");
    return 0;
}

void buf_process_assist(t_buf_process *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a == 0)
            sprintf(s, "symbol/list/llll: Incoming Buffer Names"); // @in 0 @type symbol/list/llll @digest Incoming buffer names
        else
            sprintf(s, "number/llll/symbol: pan position"); // @in 1 @type number @digest Pan position, envelope or buffer
    } else {
        sprintf(s, "symbol/list: Output Buffer Names"); // @out 0 @type symbol/list @digest Output buffer names
    }
}

void buf_process_inletinfo(t_buf_process *x, void *b, long a, char *t)
{
    if (a)
        *t = 1;
}


t_buf_process *buf_process_new(t_symbol *s, short argc, t_atom *argv)
{
    t_buf_process *x;
    long true_ac = attr_args_offset(argc, argv);
    
    x = (t_buf_process*)object_alloc_debug(s_tag_class);
    if (x) {
        x->args = llll_make();
        
        earsbufobj_init((t_earsbufobj *)x, 0);

        // @arg 0 @name outnames @optional 1 @type symbol
        // @digest Output buffer names
        // @description @copy EARS_DOC_OUTNAME_ATTR

        // @arg 1 @name patchername @optional 0 @type symbol
        // @digest Patcher name
        // @description Sets the file name or pathof the patcher to be applied

        t_llll *args = llll_parse(true_ac, argv);
        
        t_symbol *patch_name_entered = NULL;
        t_llll *names = NULL;
        t_llllelem *patchername_el = args->l_head;
        if (args && args->l_head) {
            if (hatom_gettype(&args->l_head->l_hatom) == H_LLLL && llll_contains_only_symbols_and_at_least_one(hatom_getllll(&args->l_head->l_hatom))) {
                names = llll_get();
                llll_appendhatom_clone(names, &args->l_head->l_hatom);
                llll_behead(args);
                patchername_el = args->l_head->l_next;
            } else if (hatom_gettype(&args->l_head->l_hatom) == H_SYM && args->l_head->l_next && hatom_gettype(&args->l_head->l_next->l_hatom) != H_SYM)) {
                names = symbol2llll(hatom_getsym(&args->l_head->l_hatom));
                llll_behead(args);
                patchername_el = args->l_head->l_next;
            }
        }
        if (patchername_el && hatom_gettype(&patchername_el->l_hatom))
            patch_name_entered = hatom_getsym(&patchername_el->l_hatom);
        

        attr_args_process(x, argc, argv);
        
        earsbufobj_setup((t_earsbufobj *)x, "E4", "E", names);

        llll_free(args);
        llll_free(names);
        
        
        
        
        t_symbol *tempsym;
        long i;
        int first_int = 1;
        short ac = 0;
        t_atom av[MAX_ARGS];
        long number_of_instances_to_load = 0;
        
        x->patch_spaces_allocated = 0;
        x->target_index = 0;
        
        x->last_vec_size = 64;
        x->last_samp_rate = 44100;
        
        x->in_table = x->out_table = 0;
        
        x->patch_is_loading = 0;
        
        x->declared_sig_ins = x->declared_ins = x->instance_ins = x->instance_sig_ins = x->extra_ins = x->extra_sig_ins = x->mode_default_numins = 0;
        x->declared_sig_outs = x->declared_outs = x->instance_outs = x->instance_sig_outs = x->extra_outs = x->extra_sig_outs = x->mode_default_numouts = 0;
        
        
        // Check the order or the number of instances :
        first_int = 1; // atom_getlong(argv); // TO DO
        
        // Initialise patcher symbol
        object_obex_lookup(x, gensym("#P"), &x->parent_patch);                                    // store reference to parent patcher
        
        // load a single instance to query io informations
        
        t_io_infos io_infos;
        t_hoa_err loaded_patch_err = buf_process_get_patch_filename_io_context(x, patch_name_entered, &io_infos);
        
        if (loaded_patch_err != HOA_ERR_NONE)
            return x;
        
        // default io config depends on object type (2d/3d) and mode :
        
        if (x->f_mode == hoa_sym_harmonics)
        {
            if (x->f_object_type == HOA_OBJECT_2D)
                x->mode_default_numins = x->mode_default_numouts = x->f_ambi2D->getNumberOfHarmonics();
            else if (x->f_object_type == HOA_OBJECT_3D)
                x->mode_default_numins = x->mode_default_numouts = x->f_ambi3D->getNumberOfHarmonics();
        }
        else if (x->f_mode == hoa_sym_planewaves)
        {
            x->mode_default_numins = x->mode_default_numouts = first_int;
        }
        
        // Set object io depending on contextual io of the current patcher
        
        if (io_infos.sig_ins > 0)
            x->instance_sig_ins = x->declared_sig_ins = x->mode_default_numins;
        
        if (io_infos.sig_outs > 0)
            x->instance_sig_outs = x->declared_sig_outs = x->mode_default_numouts;
        
        if (io_infos.ins > 0)
            x->instance_ins = x->declared_ins = x->mode_default_numins;
        
        if (io_infos.outs > 0)
            x->instance_outs = x->declared_outs = x->mode_default_numouts;
        
        // --- add extra ins and outs (if necessary)
        
        x->extra_sig_ins    = io_infos.extra_sig_ins_maxindex;
        x->extra_sig_outs    = io_infos.extra_sig_outs_maxindex;
        x->extra_ins        = io_infos.extra_ins_maxindex;
        x->extra_outs        = io_infos.extra_outs_maxindex;
        
        x->declared_sig_ins        += x->extra_sig_ins;
        x->declared_sig_outs    += x->extra_sig_outs;
        x->declared_ins            += x->extra_ins;
        x->declared_outs        += x->extra_outs;
        
        // --- Create signal in/out buffers and zero
        
        x->num_proxies = max(x->instance_sig_ins, x->instance_ins) + max(x->extra_sig_ins, x->extra_ins);
        
        x->declared_sig_ins = x->declared_ins = x->num_proxies;
        
        x->sig_ins    = (void **) malloc(x->declared_sig_ins * sizeof(void *));
        x->sig_outs = (void **) malloc(x->declared_sig_outs * sizeof(void *));
        
        for (i = 0; i < x->declared_sig_ins; i++)
            x->sig_ins[i] = 0;
        for (i = 0; i < x->declared_sig_outs; i++)
            x->sig_outs[i] = 0;
        
        
        // io schema :
        // ins : instance in (mixed sig/ctrl) -- extra (mixed sig/ctrl)
        // outs : instance sig outs -- extra sig -- instance ctrl outs -- extra ctrl
        
        // Make non-signal inlets
        
        if (x->declared_ins)
        {
            x->in_table = (t_outvoid *)t_getbytes(x->declared_ins * sizeof(t_outvoid));
            for (i = 0; i < x->declared_ins; i++)
                x->in_table[i] = outlet_new(0L, 0L);                                            // make generic unowned inlets
        }
        
        // Make signal ins
/*
        dsp_setup((t_pxobject *) x, x->num_proxies);
        x->x_obj.z_misc = Z_NO_INPLACE;                                                            // due to output zeroing!!
        
        // Make non-signal instance and extra outlets
        
        if (x->declared_outs)
        {
            x->out_table = (t_outvoid *) t_getbytes(x->declared_outs * sizeof(t_outvoid));
            
            // non-signal extra outlets
            if (x->extra_outs)
                for (i = x->declared_outs - 1; i >= (x->declared_outs - x->extra_outs); i--)
                    x->out_table[i] = outlet_new((t_object *)x, 0);
            
            // non-signal instance outlets
            for (i = x->declared_outs - x->extra_outs - 1; i >= 0; i--)
                x->out_table[i] = outlet_new((t_object *)x, 0);
        }
        
        // Make signal extra outlets
        
        for (i = 0; i < x->declared_sig_outs; i++)
            outlet_new((t_object *)x, "signal");
        */
        
        // Load patches and initialise all instances
        
        if (patch_name_entered && loaded_patch_err == HOA_ERR_NONE)
        {
            if (x->f_mode == hoa_sym_harmonics)
            {
                if (x->f_object_type == HOA_OBJECT_2D)
                    number_of_instances_to_load = x->f_ambi2D->getNumberOfHarmonics();
                else if (x->f_object_type == HOA_OBJECT_3D)
                    number_of_instances_to_load = x->f_ambi3D->getNumberOfHarmonics();
            }
            else if (x->f_mode == hoa_sym_planewaves)
            {
                number_of_instances_to_load = first_int;
            }
            
            for (i = 0; i < number_of_instances_to_load; i++)
            {
                buf_process_loadpatch(x, i, patch_name_entered, ac, av);
            }
        }
        
        
    }
    return x;
}


void buf_process_free(t_buf_process *x)
{
    t_patchspace *patch_space_ptr;
    long i;
    
//    dsp_free((t_pxobject *)x);
    
    // Free patches
    
    for (i = 0; i < x->patch_spaces_allocated; i++)
    {
        patch_space_ptr = x->patch_space_ptrs[i];
        buf_process_free_patch_and_dsp (x, patch_space_ptr);
        
        if (patch_space_ptr)
            freebytes((char *) patch_space_ptr, sizeof(t_patchspace));
    }
    
    if (x->declared_sig_ins)
        free(x->sig_ins);
    
    if (x->declared_sig_outs)
        free(x->sig_outs);
    
    for (i = 0; i < x->declared_ins; i++)
        object_free((t_object*)x->in_table[i]);
    
    if (x->in_table)
        freebytes(x->in_table, x->declared_ins * sizeof(t_outvoid));
    
    if (x->out_table)
        freebytes(x->out_table, x->declared_outs * sizeof(t_outvoid));
    
    llll_free(x->params);
    earsbufobj_free((t_earsbufobj *)x);
}




void buf_process_bang(t_buf_process *x)
{
    long num_buffers = ((t_earsbufobj *)x)->l_instore[0].num_stored_bufs;
    long num_out_channels = x->num_out_channels;
    e_ears_pan_modes pan_mode = (e_ears_pan_modes)x->pan_mode;
    e_ears_pan_laws pan_law = (e_ears_pan_laws)x->pan_law;
    
    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
    earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, num_buffers, true);
    
    t_llllelem *el = x->pan->l_head;
    for (long count = 0; count < num_buffers; count++, el = el && el->l_next ? el->l_next : el) {
        t_buffer_obj *in = earsbufobj_get_inlet_buffer_obj((t_earsbufobj *)x, 0, count);
        t_buffer_obj *out = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, 0, count);

        t_llll *pans01 = earsbufobj_llllelem_remap_y_to_0_1_and_x_to_samples((t_earsbufobj *)x, el, in, x->pan_range[0], x->pan_range[1], false);
        

        
        if (pans01->l_depth == 1 && pans01->l_head) {
            
            if (hatom_gettype(&pans01->l_head->l_hatom) == H_SYM) {
                // pan is another buffer!
                t_buffer_ref *ref = buffer_ref_new((t_object *)x, hatom_getsym(&pans01->l_head->l_hatom));
                ears_buffer_pan1d_buffer((t_object *)x, in, out, num_out_channels, buffer_ref_getobject(ref), pan_mode, pan_law, x->multichannel_spread, x->compensate_multichannel_gain_to_avoid_clipping);
                object_free(ref);
            } else {
                // pan is a single number
                ears_buffer_pan1d((t_object *)x, in, out, num_out_channels, hatom_getdouble(&pans01->l_head->l_hatom), pan_mode, pan_law, x->multichannel_spread, x->compensate_multichannel_gain_to_avoid_clipping);
            }
        } else {
            // pan is an envelope in llll form
            ears_buffer_pan1d_envelope((t_object *)x, in, out, num_out_channels, pans01, pan_mode, pan_law, x->multichannel_spread, x->compensate_multichannel_gain_to_avoid_clipping);
        }
        
        llll_free(pans01);
    }
    
    earsbufobj_outlet_buffer((t_earsbufobj *)x, 0);
}


void buf_process_anything(t_buf_process *x, t_symbol *msg, long ac, t_atom *av)
{
    long inlet = proxy_getinlet((t_object *) x);

    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, msg, ac, av);
    if (!parsed) return;
    
    if (parsed && parsed->l_head) {
        if (inlet == 0) {
//            earsbufobj_refresh_outlet_names((t_earsbufobj *)x);
            
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, 0, parsed->l_size, true);
            earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, 0, parsed->l_size, true);
            
            earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, 0);
            
            buf_process_bang(x);
            
        } else if (inlet == 1) {
            llll_free(x->params);
            x->params = llll_clone(parsed);
        }
    }
    llll_free(parsed);
}


// ========================================================================================================================================== //
// Patcher Loading / Deleting
// ========================================================================================================================================== //

void buf_process_loadexit(t_buf_process *x, long replace_symbol_pointers, void *previous, void *previousindex)
{
    if (replace_symbol_pointers)
    {
        hoa_sym_HoaProcessor->s_thing = (struct object*)previous;
        hoa_sym_HoaProcessorPatchIndex->s_thing = (struct object*)previousindex;
    }
    ATOMIC_DECREMENT_BARRIER(&x->patch_is_loading);
}

t_ears_err buf_process_loadpatch(t_buf_process *x, long index, t_symbol *patch_name_in, short argc, t_atom *argv)
{
    t_patchspace *patch_space_ptr = 0;
    t_object *previous;
    t_object *previousindex;
    t_fourcc type;
    t_fourcc filetypelist = 'JSON';
    long patch_spaces_allocated = x->patch_spaces_allocated;
    long harmonic_hdegree, harmonic_argument;
    long i;
    
    short patch_path;
    short saveloadupdate;
    char filename[MAX_FILENAME_CHARS];
    char windowname[280];
    t_patcher *p;
    
    // Check that this object is not loading in another thread
    
    if (ATOMIC_INCREMENT_BARRIER(&x->patch_is_loading) > 1)
    {
        object_error((t_object*)x, "Patch is loading in another thread");
        buf_process_loadexit(x, 0, 0, 0);
        return EARS_ERR_GENERIC;
    }
    
    // Find a free patch if no index is given
    
    if (index < 0)
    {
        for (index = 0; index < patch_spaces_allocated; index++)
            if (x->patch_space_ptrs[index]->the_patch == 0)
                break;
    }
    
    // Check that the index is valid
    
    if (index >= MAX_NUM_PATCHES)
    {
        object_error((t_object*)x, "Max number of patcher loaded exceeded");
        buf_process_loadexit(x, 0, 0, 0);
        return EARS_ERR_GENERIC;
    }
    
    // Create patchspaces up until the last allocated index (if necessary) and store the pointer
    
    for (i = patch_spaces_allocated; i < index + 1; i++)
        buf_process_new_patch_space (x, i);
    
    patch_space_ptr = x->patch_space_ptrs[index];
    
    // Free the old patch - the new patch is not yet valid, but we switch it on so it can be switched off at loadbang time
    
    patch_space_ptr->patch_valid = 0;
    buf_process_free_patch_and_dsp(x, patch_space_ptr);
    buf_process_init_patch_space(patch_space_ptr);
    patch_space_ptr->patch_on = 1;
    
    // Bind to the loading symbols and store the old symbols
    
    previous = hoa_sym_HoaProcessor->s_thing;
    previousindex = hoa_sym_HoaProcessorPatchIndex->s_thing;
    
    hoa_sym_HoaProcessor->s_thing = (t_object *) x;
    hoa_sym_HoaProcessorPatchIndex->s_thing = (t_object *) (index + 1);
    
    // Try to locate a file of the given name that is of the correct type
    
    strncpy_zero(filename, patch_name_in->s_name, MAX_FILENAME_CHARS);
    
    // if filetype does not exists
    if (locatefile_extended(filename, &patch_path, &type, &filetypelist, 1))
    {
        object_error((t_object*)x, "patcher \"%s\" not found !", filename);
        buf_process_loadexit(x, 1, previous, previousindex);
        return HOA_ERR_FILE_NOT_FOUND;
    }
    
    // Check the number of rarguments (only up to 16 allowed right now)
    
    if (argc > MAX_ARGS)
        argc = MAX_ARGS;
    
    // Load the patch (don't interrupt dsp)
    
    saveloadupdate = dsp_setloadupdate(false);
    p = (t_patcher*) intload(filename, patch_path, 0 , argc, argv, false);
    dsp_setloadupdate(saveloadupdate);
    
    // Check something has loaded
    
    if (!p)
    {
        object_error((t_object*)x, "error loading %s", filename);
        buf_process_loadexit(x, 1, previous, previousindex);
        return HOA_ERR_FAIL;
    }
    
    // Check that it is a patcher that has loaded
    
    if (!ispatcher((t_object*)p))
    {
        object_error((t_object*)x, "%s is not a patcher file", filename);
        object_free((t_object *)p);
        buf_process_loadexit(x, 1, previous, previousindex);
        return HOA_ERR_FAIL;
    }
    
    // Change the window name to : "patchname [hdegree arg]" (if mode no or post)
    
    if (x->f_mode == hoa_sym_harmonics)
    {
        if (x->f_object_type == HOA_OBJECT_2D)
        {
            harmonic_argument = x->f_ambi2D->getHarmonicOrder(index);
            sprintf(windowname, "%s [%ld]", patch_name_in->s_name, harmonic_argument);
        }
        else if (x->f_object_type == HOA_OBJECT_3D)
        {
            harmonic_hdegree = x->f_ambi3D->getHarmonicDegree(index);
            harmonic_argument = x->f_ambi3D->getHarmonicOrder(index);
            sprintf(windowname, "%s [%ld %ld]", patch_name_in->s_name, harmonic_hdegree, harmonic_argument);
        }
    }
    else
    {
        sprintf(windowname, "%s (%ld)", patch_name_in->s_name, index+1);
    }
    
    jpatcher_set_title(p, gensym(windowname));
    
    // Set the relevant associations
    buf_process_patcher_descend((t_patcher *)p, (t_intmethod) buf_process_setsubassoc, x, x);
    
    // Link inlets and outlets
    if (x->declared_ins)
        buf_process_patcher_descend((t_patcher *)p, (t_intmethod) buf_process_linkinlets, x, x);
    
    // Copy all the relevant data into the patch space
    
    patch_space_ptr->the_patch = (t_patcher*)p;
    patch_space_ptr->patch_name_in = patch_name_in;
    
    strcpy(patch_space_ptr->patch_name, filename);
    patch_space_ptr->patch_path = patch_path;
    
    patch_space_ptr->x_argc = argc;
    for (i = 0; i < argc; i++)
        patch_space_ptr->x_argv[i] = argv[i];
    
    // Compile the dspchain in case dsp is on
    
    buf_process_dsp_internal (patch_space_ptr, x->last_vec_size, x->last_samp_rate);
    
    // The patch is valid and ready to go
    
    patch_space_ptr->patch_valid = 1;
    
    // Return to previous state
    
    buf_process_loadexit(x, 1, previous, previousindex);
    
    //object_method(patch_space_ptr->the_patch, gensym("loadbang")); // useless (intload() func do it for us)
    
    return HOA_ERR_NONE;
}

// ========================================================================================================================================== //
// Messages in passed on to the patcher via the "in" objects / Voice targeting
// ========================================================================================================================================== //


void buf_process_bang(t_buf_process *x)
{
    long index = proxy_getinlet((t_object *)x);
    long target_index = x->target_index;
    
    if (index >= x->declared_ins || target_index == -1)
        return;
    
    if (target_index)
    {
        buf_process_target(x, target_index, index, hoa_sym_bang, 0, 0);
        //post("buf_process_target target_index = %ld", target_index);
    }
    else
    {
        outlet_bang(x->in_table[index]);
        //post("outlet_bang");
    }
}

void buf_process_int(t_buf_process *x, long n)
{
    long index = proxy_getinlet((t_object *)x);    // proxy index
    long target_index = x->target_index;
    
    if (index >= x->declared_ins || target_index == -1)
        return;
    
    if (target_index)
    {
        t_atom n_atom;
        atom_setlong (&n_atom, n);
        buf_process_target(x, target_index, index, hoa_sym_int, 1, &n_atom);
    }
    else
        outlet_int(x->in_table[index], n);
}

void buf_process_float(t_buf_process *x, double f)
{
    long index = proxy_getinlet((t_object *)x);    // proxy index
    long target_index = x->target_index;
    
    if (index >= x->declared_ins || target_index == -1)
        return;
    
    if (target_index)
    {
        t_atom f_atom;
        atom_setfloat(&f_atom, f);
        buf_process_target(x, target_index, index, hoa_sym_float, 1, &f_atom);
    }
    else
        outlet_float(x->in_table[index], f);
}

void buf_process_list(t_buf_process *x, t_symbol *s, short argc, t_atom *argv)
{
    long index = proxy_getinlet((t_object *)x);    // proxy index
    long target_index = x->target_index;
    
    if (index >= x->declared_ins || target_index == -1)
        return;
    
    if (target_index)
        buf_process_target(x, target_index, index, hoa_sym_list, argc, argv);
    else
        outlet_list(x->in_table[index], hoa_sym_list, argc, argv);
}

void buf_process_anything(t_buf_process *x, t_symbol *s, short argc, t_atom *argv)
{
    long index = proxy_getinlet((t_object *)x);    // proxy index
    long target_index = x->target_index;
    
    if (index >= x->declared_ins || target_index == -1)
        return;
    
    if (target_index)
        buf_process_target(x, target_index, index, s, argc, argv);
    else
        outlet_anything(x->in_table[index], s, argc, argv);
}

void buf_process_target(t_buf_process *x, long target_index, long index, t_symbol *msg, short argc, t_atom *argv)
{
    t_args_struct pass_args;
    
    pass_args.msg = msg;
    pass_args.argc = argc;
    pass_args.argv = argv;
    pass_args.index = index + 1;
    
    if (target_index >= 1 && target_index <= x->patch_spaces_allocated)
    {
        t_patcher *p = x->patch_space_ptrs[target_index - 1]->the_patch;
        
        if (x->patch_space_ptrs[target_index - 1]->patch_valid)
            buf_process_patcher_descend(p, (t_intmethod) buf_process_targetinlets, &pass_args, x);
    }
}

// - inlet and outlet linking using the in and out objects
short buf_process_targetinlets(t_patcher *p, t_args_struct *args)
{
    t_box *b;
    t_inout *io;
    
    for (b = jpatcher_get_firstobject(p); b; b = jbox_get_nextobject(b))
    {
        if (jbox_get_maxclass(b) == hoa_sym_in)
        {
            io = (t_inout *) jbox_get_object(b);
            
            if (io->s_index == args->index)
                buf_process_output_typed_message(io->s_obj.o_outlet, args);
        }
    }
    return (0);
}

void buf_process_user_target(t_buf_process *x, t_symbol *msg, short argc, t_atom *argv)
{
    x->target_index = -1;
    
    if (argc && argv)
    {
        if (atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("all"))
        {
            x->target_index = 0;
        }
        else if (atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("none"))
        {
            x->target_index = -1;
        }
        else if (atom_gettype(argv) == A_LONG)
        {
            if (x->f_mode == hoa_sym_harmonics)
            {
                long harm_degree, harm_order = 0;
                
                if (x->f_object_type == HOA_OBJECT_2D)
                {
                    harm_order = atom_getlong(argv);
                    
                    if (abs(harm_order) <= x->f_ambi2D->getDecompositionOrder())
                        x->target_index = x->f_ambi2D->getHarmonicIndex(0, harm_order) + 1;
                    
                    // bad target target none
                    if (x->target_index <= 0 || x->target_index > x->patch_spaces_allocated)
                    {
                        object_warn((t_object *)x, "target [%ld] doesn't match any patcher instance", harm_order);
                        x->target_index = -1;
                    }
                }
                else if (x->f_object_type == HOA_OBJECT_3D)
                {
                    harm_degree = atom_getlong(argv);
                    
                    if (argc > 1 && atom_gettype(argv+1) == A_LONG)
                        harm_order = atom_getlong(argv+1);
                    
                    if (harm_degree <= x->f_ambi3D->getDecompositionOrder() && abs(harm_order) <= harm_degree)
                        x->target_index = x->f_ambi3D->getHarmonicIndex(harm_degree, harm_order) + 1;
                    
                    // bad target target none
                    if (x->target_index <= 0 || x->target_index > x->patch_spaces_allocated)
                    {
                        object_warn((t_object *)x, "target [%ld, %ld] doesn't match any patcher instance", harm_degree, harm_order);
                        x->target_index = -1;
                    }
                }
            }
            else if (x->f_mode == hoa_sym_planewaves)
            {
                x->target_index = atom_getlong(argv);
                
                // bad target target none
                if (x->target_index <= 0 || x->target_index > x->patch_spaces_allocated)
                {
                    object_warn((t_object *)x, "target (%ld) doesn't match any patcher instance", x->target_index);
                    x->target_index = -1;
                }
            }
        }
    }
}

void buf_process_user_mute(t_buf_process *x, t_symbol *msg, short argc, t_atom *argv)
{
    int state = 0;
    int index = -1;
    
    if (argc && argv)
    {
        if (atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("all"))
        {
            index = 0;
            
            if (argc > 1 && atom_gettype(argv+1) == A_LONG)
                state = atom_getlong(argv+1) != 0;
        }
        else if (atom_gettype(argv) == A_LONG)
        {
            if (x->f_mode == hoa_sym_harmonics)
            {
                long harm_degree, harm_order = 0;
                
                if (x->f_object_type == HOA_OBJECT_2D)
                {
                    harm_order = atom_getlong(argv);
                    
                    if (abs(harm_order) <= x->f_ambi2D->getDecompositionOrder())
                        index = x->f_ambi2D->getHarmonicIndex(0, harm_order) + 1;
                    
                    // bad target target none
                    if (index <= 0 || index > x->patch_spaces_allocated)
                    {
                        object_error((t_object *)x, "mute [%ld] doesn't match any patcher instance", harm_order);
                        index = -1;
                    }
                    
                    if (argc > 1 && atom_gettype(argv+1) == A_LONG)
                        state = atom_getlong(argv+1) != 0;
                }
                else if (x->f_object_type == HOA_OBJECT_3D)
                {
                    harm_degree = Math<long>::clip(atom_getlong(argv), 0, x->f_ambi3D->getDecompositionOrder());
                    
                    if (argc > 1 && atom_gettype(argv+1) == A_LONG)
                        harm_order = atom_getlong(argv+1);
                    
                    if (harm_degree <= x->f_ambi3D->getDecompositionOrder() && abs(harm_order) <= harm_degree)
                        index = x->f_ambi3D->getHarmonicIndex(harm_degree, harm_order) + 1;
                    
                    // bad target target none
                    if (index <= 0 || index > x->patch_spaces_allocated)
                    {
                        object_error((t_object *)x, "target [%ld, %ld] doesn't match any patcher instance", harm_degree, harm_order);
                        index = -1;
                    }
                    
                    if (argc > 2 && atom_gettype(argv+2) == A_LONG)
                        state = atom_getlong(argv+2) != 0;
                }
            }
            else if (x->f_mode == hoa_sym_planewaves)
            {
                index = atom_getlong(argv);
                
                // bad target target none
                if (index <= 0 || index > x->patch_spaces_allocated)
                {
                    object_warn((t_object *)x, "mute (%ld) doesn't match any patcher instance", x->target_index);
                    index = -1;
                }
                
                if (argc > 1 && atom_gettype(argv+1) == A_LONG)
                    state = atom_getlong(argv+1) != 0;
            }
        }
    }
    
    // mute patch(es) and send bang to hoa.thisprocess~ object(s)
    
    if (index == 0)
    {
        for (int i=0; i < x->patch_spaces_allocated; i++)
        {
            buf_process_client_set_patch_on(x, i+1, !state);
            buf_process_patcher_descend(x->patch_space_ptrs[i]->the_patch, (t_intmethod)buf_process_send_mutechange, x, x);
        }
    }
    else if (index > 0 && index <= x->patch_spaces_allocated)
    {
        buf_process_client_set_patch_on(x, index, !state);
        buf_process_patcher_descend(x->patch_space_ptrs[index-1]->the_patch, (t_intmethod)buf_process_send_mutechange, x, x);
    }
}

// - send a "mutechange" message to all hoa.thisprocess~ objects in the patch
short buf_process_send_mutechange(t_patcher *p, t_args_struct *args)
{
    t_box *b;
    t_object* thisprocess;
    
    for (b = jpatcher_get_firstobject(p); b; b = jbox_get_nextobject(b))
    {
        if (jbox_get_maxclass(b) == gensym("hoa.thisprocess~"))
        {
            thisprocess = (t_object *) jbox_get_object(b);
            object_method(thisprocess, gensym("mutechange"));
        }
    }
    return (0);
}

// report muted instance info as a list in a specied message outlet
void buf_process_mutemap(t_buf_process *x, long n)
{
    int outlet_index = n;
    if (outlet_index < 1 || outlet_index > x->declared_outs) return;
    
    //t_atom list[x->patch_spaces_allocated];
    
    t_atom *list = NULL;
    list = (t_atom *)sysmem_newptr(sizeof(x->patch_spaces_allocated));
    
    for (int i=0; i<x->patch_spaces_allocated; i++)
        atom_setlong(list+i, !x->patch_space_ptrs[i]->patch_on);
    
    outlet_list(x->out_table[outlet_index-1], NULL, x->patch_spaces_allocated, list);
}

void buf_process_out_message(t_buf_process *x, t_args_struct *args)
{
    long index = args->index;
    if (args->index > 0 && args->index <= x->declared_outs)
        buf_process_output_typed_message(x->out_table[index-1], args);
}

void buf_process_output_typed_message(void* outletptr, t_args_struct *args)
{
    if (outletptr)
    {
        if (args->msg == hoa_sym_bang)
            outlet_bang(outletptr);
        else if (args->msg == hoa_sym_int)
            outlet_int(outletptr, atom_getlong(args->argv));
        else if (args->msg == hoa_sym_float)
            outlet_float(outletptr, atom_getfloat(args->argv));
        else if (args->msg == hoa_sym_list)
            outlet_list(outletptr, 0L, args->argc, args->argv);
        else
            outlet_anything (outletptr, args->msg, args->argc, args->argv);
    }
}

// ========================================================================================================================================== //
// Perform and DSP Routines
// ========================================================================================================================================== //

void buf_process_perform64 (t_buf_process *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long vec_size, long flags, void *userparam)
{
    for (int i = 0; i < x->declared_sig_ins; i++)
        x->sig_ins[i] = ins[i];
    
    // Zero Outputs
    for (int i = 0; i < x->declared_sig_outs; i++)
        memset(outs[i], 0, sizeof(double) * vec_size);
    
    if (x->x_obj.z_disabled)
        return;
    
    t_patchspace **patch_space_ptrs = x->patch_space_ptrs;
    t_patchspace *next_patch_space_ptr = 0;
    
    t_dspchain *next_dspchain = 0;
    
    long patch_spaces_allocated = x->patch_spaces_allocated;
    long index = 0;
    
    for (int i = 0; i < patch_spaces_allocated; i++)
    {
        if (++index >= patch_spaces_allocated)
            index -= patch_spaces_allocated;
        
        next_patch_space_ptr = patch_space_ptrs[index];
        next_dspchain = next_patch_space_ptr->the_dspchain;
        
        if (next_patch_space_ptr->patch_valid && next_patch_space_ptr->patch_on && next_dspchain)
        {
            next_patch_space_ptr->out_ptrs = outs;
            dspchain_tick(next_dspchain);
        }
    }
}

void buf_process_dsp64 (t_buf_process *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    t_patchspace *patch_space_ptr;
    
    // Do internal dsp compile (for each valid patch)
    
    for (int i = 0; i < x->patch_spaces_allocated; i++)
    {
        patch_space_ptr = x->patch_space_ptrs[i];
        if (patch_space_ptr->patch_valid)
        {
            buf_process_dsp_internal(patch_space_ptr, maxvectorsize, (long)samplerate);
        }
    }
    
    x->last_vec_size = maxvectorsize;
    x->last_samp_rate = (long)samplerate;
    
    object_method(dsp64, gensym("dsp_add64"), x, buf_process_perform64, 0, NULL);
}

void buf_process_dsp_internal (t_patchspace *patch_space_ptr, long vec_size, long samp_rate)
{
    // Free the old dspchain
    
    if (patch_space_ptr->the_dspchain)
        object_free((t_object *)patch_space_ptr->the_dspchain);
    
    // Recompile
    
    patch_space_ptr->the_dspchain = dspchain_compile(patch_space_ptr->the_patch, vec_size, samp_rate);
}


// ========================================================================================================================================== //
// Patcher Link Inlets / Outlets
// ========================================================================================================================================== //

void buf_process_init_io_infos(t_io_infos* io_infos)
{
    io_infos->sig_ins = 0;
    io_infos->sig_outs = 0;
    io_infos->ins = 0;
    io_infos->outs = 0;
    
    io_infos->sig_ins_maxindex = 0;
    io_infos->sig_outs_maxindex = 0;
    io_infos->ins_maxindex = 0;
    io_infos->outs_maxindex = 0;
    
    io_infos->extra_sig_ins = 0;
    io_infos->extra_sig_outs = 0;
    io_infos->extra_ins = 0;
    io_infos->extra_outs = 0;
    
    io_infos->extra_sig_ins_maxindex = 0;
    io_infos->extra_sig_outs_maxindex = 0;
    io_infos->extra_ins_maxindex = 0;
    io_infos->extra_outs_maxindex = 0;
}

t_ears_err buf_process_get_patch_filename_io_context(t_buf_process *x, t_symbol *patch_name_in, t_io_infos* io_infos)
{
    t_fourcc type;
    t_fourcc filetypelist = 'JSON';
    
    short patch_path;
    char filename[MAX_FILENAME_CHARS];
    t_patcher *p;
    
    buf_process_init_io_infos(io_infos);
    
    if (!patch_name_in)
    {
        object_error((t_object*)x, "No patch name entered!");
        buf_process_loadexit(x, 0, 0, 0);
        return EARS_ERR_NO_FILE;
    }
    
    // Check that this object is not loading in another thread
    
    if (ATOMIC_INCREMENT_BARRIER(&x->patch_is_loading) > 1)
    {
        object_error((t_object*)x, "Patch is loading in another thread");
        buf_process_loadexit(x, 0, 0, 0);
        return HOA_ERR_FAIL;
    }
    
    // Try to locate a file of the given name that is of the correct type
    
    strncpy_zero(filename, patch_name_in->s_name, MAX_FILENAME_CHARS);
    
    // if filetype does not exists
    if (locatefile_extended(filename, &patch_path, &type, &filetypelist, 1))
    {
        object_error((t_object*)x, "Patcher \"%s\" not found !", filename);
        buf_process_loadexit(x, 0, 0, 0);
        return HOA_ERR_FILE_NOT_FOUND;
    }
    
    // Load the patch
    p = (t_patcher*) intload(filename, patch_path, 0, 0, NULL, false);
    
    // Check something has loaded
    
    if (!p)
    {
        object_error((t_object*)x, "Error loading %s", filename);
        buf_process_loadexit(x, 0, 0, 0);
        return HOA_ERR_FAIL;
    }
    
    // Check that it is a patcher that has loaded
    
    if (!ispatcher((t_object*)p))
    {
        object_error((t_object*)x, "%s is not a patcher file", filename);
        object_free((t_object *)p);
        buf_process_loadexit(x, 0, 0, 0);
        return HOA_ERR_FAIL;
    }
    
    // no error, so we can check our IO context
    
    buf_process_get_patch_io_context(p, io_infos);
    
    object_free((t_object *)p);
    
    buf_process_loadexit(x, 0, 0, 0);
    return HOA_ERR_NONE;
}

short buf_process_get_patch_io_context(t_patcher *p, t_io_infos* io_infos)
{
    t_box *b;
    t_object *io;
    t_symbol* objclassname;
    long extra;
    
    buf_process_init_io_infos(io_infos);
    
    for (b = jpatcher_get_firstobject(p); b; b = jbox_get_nextobject(b))
    {
        objclassname = jbox_get_maxclass(b);
        
        if (objclassname == hoa_sym_sigin || objclassname == hoa_sym_in ||
            objclassname == hoa_sym_sigout || objclassname == hoa_sym_out)
        {
            extra = 0;
            io = jbox_get_object(b);
            
            extra = object_attr_getlong(io, hoa_sym_extra);
            
            if (objclassname == hoa_sym_sigin)
            {
                if (extra > 0) io_infos->extra_sig_ins++;
                else io_infos->sig_ins++;
                
                io_infos->extra_sig_ins_maxindex = max(extra, io_infos->extra_sig_ins_maxindex);
            }
            else if (objclassname == hoa_sym_in)
            {
                if (extra > 0) io_infos->extra_ins++;
                else io_infos->ins++;
                
                io_infos->extra_ins_maxindex = max(extra, io_infos->extra_ins_maxindex);
            }
            else if (objclassname == hoa_sym_sigout)
            {
                if (extra > 0) io_infos->extra_sig_outs++;
                else io_infos->sig_outs++;
                
                io_infos->extra_sig_outs_maxindex = max(extra, io_infos->extra_sig_outs_maxindex);
            }
            else if (objclassname == hoa_sym_out)
            {
                if (extra > 0) io_infos->extra_outs++;
                else io_infos->outs++;
                
                io_infos->extra_outs_maxindex = max(extra, io_infos->extra_outs_maxindex);
            }
        }
    }
    
    return 0;
}

// - inlet linking and removal using hoa.in object

short buf_process_linkinlets(t_patcher *p, t_buf_process *x)
{
    t_box *b;
    t_inout *io;
    
    for (b = jpatcher_get_firstobject(p); b; b = jbox_get_nextobject(b))
    {
        if (jbox_get_maxclass(b) == hoa_sym_in)
        {
            io = (t_inout *) jbox_get_object(b);
            
            if (io->s_index <= x->declared_ins)
                outlet_add(x->in_table[io->s_index - 1], (struct inlet *)io->s_obj.o_outlet);
        }
    }
    return 0;
}

short buf_process_unlinkinlets(t_patcher *p, t_buf_process *x)
{
    t_box *b;
    t_inout *io;
    
    for (b = jpatcher_get_firstobject(p); b; b = jbox_get_nextobject(b))
    {
        if (jbox_get_maxclass(b) == hoa_sym_in)
        {
            io = (t_inout *) jbox_get_object(b);
            
            if (io->s_index <= x->declared_ins)
            {
                outlet_rm(x->in_table[io->s_index - 1], (struct inlet *)io->s_obj.o_outlet);
            }
        }
    }
    return (0);
}

// ========================================================================================================================================== //
// Patcher Window stuff
// ========================================================================================================================================== //


void buf_process_dblclick(t_buf_process *x)
{
    for (int i = 0; i < x->patch_spaces_allocated; i++)
    {
        if (x->patch_space_ptrs[i]->the_patch)
        {
            // open the current target instance
            buf_process_open(x, NULL, 0, NULL);
            break;
        }
    }
}

void buf_process_open(t_buf_process *x, t_symbol *msg, short argc, t_atom *argv)
{
    long index = 0;
    t_atom a;
    
    if (argc && argv && atom_gettype(argv) == A_LONG)
    {
        if (x->f_mode == hoa_sym_harmonics)
        {
            long harm_degree, harm_order = 0;
            
            if (x->f_object_type == HOA_OBJECT_2D)
            {
                harm_order = atom_getlong(argv);
                
                if (abs(harm_order) <= x->f_ambi2D->getDecompositionOrder())
                    index = x->f_ambi2D->getHarmonicIndex(0, harm_order) + 1;
                
                // bad target target none
                if (index <= 0 || index > x->patch_spaces_allocated)
                {
                    object_error((t_object *)x, "open [%ld] doesn't match any patcher instance", harm_order);
                    index = -1;
                }
            }
            else if (x->f_object_type == HOA_OBJECT_3D)
            {
                harm_degree = atom_getlong(argv);
                
                if (argc > 1 && atom_gettype(argv+1) == A_LONG)
                    harm_order = atom_getlong(argv+1);
                
                if (harm_degree <= x->f_ambi3D->getDecompositionOrder() && abs(harm_order) <= harm_degree)
                    index = x->f_ambi3D->getHarmonicIndex(harm_degree, harm_order) + 1;
                
                // bad target target none
                if (index <= 0 || index > x->patch_spaces_allocated)
                {
                    object_error((t_object *)x, "open [%ld, %ld] doesn't match any patcher instance", harm_degree, harm_order);
                    index = -1;
                }
            }
        }
        else if (x->f_mode == hoa_sym_planewaves)
        {
            index = atom_getlong(argv);
            
            // bad target target none
            if (index <= 0 || index > x->patch_spaces_allocated)
            {
                object_error((t_object *)x, "no patcher instance (%ld)", index);
                index = -1;
            }
        }
    }
    else
    {
        index = max<long>(x->target_index, 1);
    }
    
    atom_setlong (&a, index - 1);
    
    if (index < 1 || index > x->patch_spaces_allocated) return;
    if (!x->patch_space_ptrs[index - 1]->patch_valid) return;
    
    defer(x,(method)buf_process_doopen, 0L, 1, &a);
}

void buf_process_doopen(t_buf_process *x, t_symbol *s, short argc, t_atom *argv)
{
    long index = atom_getlong(argv);
    
    if (x->patch_space_ptrs[index]->the_patch)
        mess0((t_object *)x->patch_space_ptrs[index]->the_patch, hoa_sym_front);        // this will always do the right thing
}

void buf_process_wclose(t_buf_process *x, t_symbol *msg, short argc, t_atom *argv)
{
    long index = 0;
    t_atom a;
    
    if (argc && argv && atom_gettype(argv) == A_LONG)
    {
        if (x->f_mode == hoa_sym_harmonics)
        {
            long harm_degree, harm_order = 0;
            
            if (x->f_object_type == HOA_OBJECT_2D)
            {
                harm_order = atom_getlong(argv);
                
                if (abs(harm_order) <= x->f_ambi2D->getDecompositionOrder())
                    index = x->f_ambi2D->getHarmonicIndex(0, harm_order) + 1;
                
                // bad target target none
                if (index <= 0 || index > x->patch_spaces_allocated)
                {
                    object_error((t_object *)x, "wclose [%ld] doesn't match any patcher instance", harm_order);
                    index = -1;
                }
            }
            else if (x->f_object_type == HOA_OBJECT_3D)
            {
                harm_degree = atom_getlong(argv);
                
                if (argc > 1 && atom_gettype(argv+1) == A_LONG)
                    harm_order = atom_getlong(argv+1);
                
                if (harm_degree <= x->f_ambi3D->getDecompositionOrder() && abs(harm_order) <= harm_degree)
                    index = x->f_ambi3D->getHarmonicIndex(harm_degree, harm_order) + 1;
                
                // bad target target none
                if (index <= 0 || index > x->patch_spaces_allocated)
                {
                    object_error((t_object *)x, "wclose [%ld, %ld] doesn't match any patcher instance", harm_degree, harm_order);
                    index = -1;
                }
            }
        }
        else if (x->f_mode == hoa_sym_planewaves)
        {
            index = atom_getlong(argv);
            
            // bad target target none
            if (index <= 0 || index > x->patch_spaces_allocated)
            {
                object_error((t_object *)x, "no patcher instance (%ld)", index);
                index = -1;
            }
        }
    }
    else if (argc && argv && atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("all"))
    {
        for (int i = 0; i < x->patch_spaces_allocated; i++)
        {
            atom_setlong (&a, i);
            defer(x,(method)buf_process_dowclose, 0L, 1, &a);
        }
        return;
    }
    
    if (index < 1 || index > x->patch_spaces_allocated) return;
    if (!x->patch_space_ptrs[index - 1]->patch_valid) return;
    
    atom_setlong (&a, index - 1);
    defer(x,(method)buf_process_dowclose, 0L, 1, &a);
}

void buf_process_dowclose(t_buf_process *x, t_symbol *s, short argc, t_atom *argv)
{
    long index = atom_getlong(argv);
    
    if (index < 0) return;
    if (index >= x->patch_spaces_allocated) return;
    if (!x->patch_space_ptrs[index]->patch_valid) return;
    
    if (x->patch_space_ptrs[index]->the_patch)
        object_method(x->patch_space_ptrs[index]->the_patch, gensym("wclose"), x);
}


// ========================================================================================================================================== //
// Patcher Utilities (these deal with various updating and necessary behind the scens state stuff)
// ========================================================================================================================================== //


short buf_process_patcher_descend(t_patcher *p, t_intmethod fn, void *arg, t_buf_process *x)
{
    t_box *b;
    t_patcher *p2;
    long index;
    t_object *assoc = NULL;
    object_method(p, hoa_sym_getassoc, &assoc);                // Avoid recursion into a poly / pfft / hoa.process~
    if (assoc && (t_buf_process *) assoc != x)
        return 0;
    
    // CHANGED - DO NOT PASS x AS ARG
    if ((*fn)(p, arg))
        return (1);
    
    for (b = jpatcher_get_firstobject(p); b; b = jbox_get_nextobject(b))
    {
        if (b)
        {
            index = 0;
            while ((p2 = (t_patcher*)object_subpatcher(jbox_get_object(b), &index, arg)))
                if (buf_process_patcher_descend(p2, fn, arg, x))
                    return 1;
        }
    }
    
    return (0);
}

short buf_process_setsubassoc(t_patcher *p, t_buf_process *x)
{
    t_object *assoc;
    object_method(p, hoa_sym_getassoc, &assoc);
    if (!assoc)
        object_method(p, hoa_sym_setassoc, x);
    object_method(p, hoa_sym_noedit, 1);
    
    return 0;
}

void buf_process_pupdate(t_buf_process *x, void *b, t_patcher *p)
{
    t_patchspace *patch_space_ptr;
    
    // Reload the patcher when it's updated
    
    for (int i = 0; i < x->patch_spaces_allocated; i++)
    {
        patch_space_ptr = x->patch_space_ptrs[i];
        if (patch_space_ptr->the_patch == p)
            buf_process_loadpatch(x, i, patch_space_ptr->patch_name_in, patch_space_ptr->x_argc, patch_space_ptr->x_argv);
    }
}

void *buf_process_subpatcher(t_buf_process *x, long index, void *arg)
{
    if (arg && (long) arg != 1)
        if (!OB_INVALID(arg))                                        // arg might be good but not a valid object pointer
            if (object_classname(arg) == hoa_sym_dspchain)                // don't report subpatchers to dspchain
                return 0;
    
    if (index < x->patch_spaces_allocated)
        if (x->patch_space_ptrs[index]->patch_valid) return x->patch_space_ptrs[index]->the_patch;        // the indexed patcher
    
    return 0;
}

void buf_process_parentpatcher(t_buf_process *x, t_patcher **parent)
{
    *parent = x->parent_patch;
}

// ========================================================================================================================================== //
// Patchspace Utilities
// ========================================================================================================================================== //

// Make a new patchspace

t_patchspace *buf_process_new_patch_space (t_buf_process *x,long index)
{
    t_patchspace *patch_space_ptr;
    
    x->patch_space_ptrs[index] = patch_space_ptr = (t_patchspace *)t_getbytes(sizeof(t_patchspace));
    
    buf_process_init_patch_space (patch_space_ptr);
    x->patch_spaces_allocated++;
    
    return patch_space_ptr;
}


// Initialise a patchspace

void buf_process_init_patch_space (t_patchspace *patch_space_ptr)
{
    patch_space_ptr->the_patch = 0;
    patch_space_ptr->patch_name_in = 0;
    patch_space_ptr->patch_path = 0;
    patch_space_ptr->patch_valid = 0;
    patch_space_ptr->patch_on = 0;
    patch_space_ptr->the_dspchain = 0;
    patch_space_ptr->x_argc = 0;
    patch_space_ptr->out_ptrs = 0;
}

// Free the patch and dspchain

void buf_process_free_patch_and_dsp (t_buf_process *x, t_patchspace *patch_space_ptr)
{
    // free old patch and dspchain
    
    if (patch_space_ptr->the_dspchain)
        object_free((t_object *)patch_space_ptr->the_dspchain);
    
    if (patch_space_ptr->the_patch)
    {
        
        if (x->declared_ins)
            buf_process_patcher_descend(patch_space_ptr->the_patch, (t_intmethod) buf_process_unlinkinlets, x, x);
        
        object_free((t_object *)patch_space_ptr->the_patch);
    }
}


// ========================================================================================================================================== //
// Parent / Child Communication - Routines for owned objects to query the parent
// ========================================================================================================================================== //


// Note that objects wishing to query the parent hoa.process~ object should call the functions in hoa.process.h.
// These act as suitable wrappers to send the appropriate message to the parent object and returns values as appropriate


////////////////////////////////////////////////// Signal IO Queries //////////////////////////////////////////////////


void *buf_process_query_declared_sigins(t_buf_process *x)
{
    return (void *) x->declared_sig_ins;
}

void *buf_process_query_declared_sigouts(t_buf_process *x)
{
    return (void *) x->declared_sig_outs;
}

void *buf_process_query_sigins(t_buf_process *x)
{
    return (void *) x->sig_ins;
}

void *buf_process_query_outptrs_ptr(t_buf_process *x, long index)
{
    if (index <= x->patch_spaces_allocated)
        return &x->patch_space_ptrs[index - 1]->out_ptrs;
    else
        return 0;
}

void* buf_process_query_io_index(t_buf_process *x, long patchIndex, t_object* io)
{
    long io_index = -1;
    long extra;
    
    t_symbol* objclassname = object_classname(io);
    
    if (objclassname == hoa_sym_in || (objclassname == hoa_sym_out) ||
        (objclassname == hoa_sym_sigin) || (objclassname == hoa_sym_sigout))
    {
        extra = object_attr_getlong(io, hoa_sym_extra);
        
        //post("query ins -- patchIndex = %ld, extra = %ld", patchIndex, extra);
        
        if (objclassname == hoa_sym_in)
        {
            if ( extra > 0 && extra <= x->extra_ins)
            {
                if (x->instance_ins >= x->instance_sig_ins)
                    io_index = x->instance_ins + extra;
                else if (x->instance_ins < x->instance_sig_ins)
                    io_index = x->instance_sig_ins + extra;
            }
            else
                io_index = patchIndex;
            
            if (io_index < 1 || io_index > x->declared_ins)
                io_index = -1;
        }
        else if (objclassname == hoa_sym_out)
        {
            if ( extra > 0 && extra <= x->extra_outs)
                io_index = x->instance_outs + extra;
            else
                io_index = patchIndex;
            
            if (io_index < 1 || io_index > x->declared_outs)
                io_index = -1;
        }
        else if(objclassname == hoa_sym_sigin)
        {
            if ( extra > 0 && extra <= x->extra_sig_ins)
            {
                if (x->instance_sig_ins >= x->instance_ins)
                    io_index = x->instance_sig_ins + extra;
                else if (x->instance_sig_ins < x->instance_ins)
                    io_index = x->instance_ins + extra;
            }
            else
                io_index = patchIndex;
        }
        else if(objclassname == hoa_sym_sigout)
        {
            if ( extra > 0 && extra <= x->extra_sig_outs)
            {
                io_index = x->instance_sig_outs + extra;
            }
            else
                io_index = patchIndex;
            
            if (io_index < 1 || io_index > x->declared_sig_outs)
                io_index = -1;
        }
        
        //object_post((t_object*)io,"query ins -- io_index = %ld", io_index);
    }
    
    return (void*) io_index;
}

//////////////////////////////////////////////////// State Queries ////////////////////////////////////////////////////

void buf_process_client_set_patch_on (t_buf_process *x, long index, long state)
{
    if (state) state = 1;
    if (index <= x->patch_spaces_allocated)
    {
        x->patch_space_ptrs[index - 1]->patch_on = state;
    }
}

void *buf_process_query_ambisonic_order(t_buf_process *x)
{
    long order = x->f_ambi2D->getDecompositionOrder();
    return (void *) order;
}

void *buf_process_query_mode(t_buf_process *x)
{
    return (void *) x->f_mode;
}

void *buf_process_query_number_of_instances(t_buf_process *x)
{
    return (void *) (x->patch_spaces_allocated);
}

void *buf_process_query_is_2D(t_buf_process *x)
{
    return (void *) (x->f_object_type == HOA_OBJECT_2D);
}

t_hoa_err buf_process_query_patcherargs(t_buf_process *x, long index, long *argc, t_atom **argv)
{
    argc[0] = 0;
    argv[0] = NULL;
    if (index > 0 && index <= x->patch_spaces_allocated)
    {
        long ac = x->patch_space_ptrs[index - 1]->x_argc;
        argc[0] = ac;
        argv[0] = (t_atom *) malloc(ac * sizeof(t_atom) );
        for (int i = 0; i < ac; i++)
            argv[0][i] = x->patch_space_ptrs[index - 1]->x_argv[i];
        
        return HOA_ERR_NONE;
    }
    
    return HOA_ERR_OUT_OF_MEMORY;
}

void *buf_process_client_get_patch_on (t_buf_process *x, long index)
{
    if (index <= x->patch_spaces_allocated)
        return (void *) (long) x->patch_space_ptrs[index - 1]->patch_on;
    
    return 0;
}

