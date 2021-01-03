/**
	@file
	ears.object.h
	Common utilities header for the objects in the buffer ears sublibrary
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_OBJECT_H_
#define _EARS_BUF_OBJECT_H_

#include "ears.commons.h"



#define EARSBUFOBJ_ADD_DEFERRED_METHODS(NAME) \
void buf_ ## NAME ## _bang_deferred(t_buf_ ## NAME *x); \
void buf_ ## NAME ## _anything_deferred(t_buf_ ## NAME *x, t_symbol *msg, long ac, t_atom *av); \
void buf_ ## NAME ## _bang_deferred(t_buf_ ## NAME *x) \
{ \
defer(x, (method)buf_ ## NAME ##_bang, NULL, 0, NULL); \
} \
void buf_ ## NAME ## _anything_deferred(t_buf_ ## NAME *x, t_symbol *msg, long ac, t_atom *av) \
{ \
x->e_ob.l_curr_proxy = proxy_getinlet((t_object *) x); \
defer(x, (method)buf_ ## NAME ##_anything, msg, ac, av); \
} \


#define EARSBUFOBJ_ADD_INT_FLOAT_METHODS(NAME) \
void buf_ ## NAME ## _int_deferred(t_buf_ ## NAME *x, t_atom_long num); \
void buf_ ## NAME ## _float_deferred(t_buf_ ## NAME *x, t_atom_float num); \
\
void buf_ ## NAME ## _int_deferred(t_buf_ ## NAME *x, t_atom_long num) \
{ \
    t_atom argv[1]; \
    atom_setlong(argv, num); \
    buf_ ## NAME ## _anything_deferred(x, _sym_list, 1, argv); \
} \
\
void buf_ ## NAME ## _float_deferred(t_buf_ ## NAME *x, t_atom_float num) \
{ \
    t_atom argv[1]; \
    atom_setfloat(argv, num); \
    buf_ ## NAME ## _anything_deferred(x, _sym_list, 1, argv); \
} \



#define EARSBUFOBJ_ADD_IO_METHODS(NAME) \
EARSBUFOBJ_ADD_DEFERRED_METHODS(NAME) \
EARSBUFOBJ_ADD_INT_FLOAT_METHODS(NAME) \


#define EARSBUFOBJ_DECLARE_COMMON_METHODS_DEFER(NAME) \
class_addmethod(c, (method)buf_ ## NAME ## _int_deferred,       "int",       A_LONG, 0); \
class_addmethod(c, (method)buf_ ## NAME ## _float_deferred,     "float",     A_FLOAT, 0); \
class_addmethod(c, (method)buf_ ## NAME ## _anything_deferred,  "anything",  A_GIMME, 0); \
class_addmethod(c, (method)buf_ ## NAME ## _anything_deferred,  "list",      A_GIMME, 0); \
class_addmethod(c, (method)buf_ ## NAME ## _bang_deferred,      "bang",     0); \
\
class_addmethod(c, (method)buf_ ## NAME ## _assist,                "assist",                A_CANT,        0); \
class_addmethod(c, (method)buf_ ## NAME ## _inletinfo,            "inletinfo",            A_CANT,        0); \
\
earsbufobj_add_common_methods(c); \



#define EARSBUFOBJ_DECLARE_COMMON_METHODS(NAME) \
class_addmethod(c, (method)buf_ ## NAME ## _int,                "int",            A_LONG, 0); \
class_addmethod(c, (method)buf_ ## NAME ## _float,                "float",            A_FLOAT, 0); \
class_addmethod(c, (method)buf_ ## NAME ## _anything,            "anything",            A_GIMME, 0); \
class_addmethod(c, (method)buf_ ## NAME ## _anything,            "list",            A_GIMME, 0); \
class_addmethod(c, (method)buf_ ## NAME ## _bang,               "bang",     0); \
\
class_addmethod(c, (method)buf_ ## NAME ## _assist,                "assist",                A_CANT,        0); \
class_addmethod(c, (method)buf_ ## NAME ## _inletinfo,            "inletinfo",            A_CANT,        0); \
\
earsbufobj_add_common_methods(c); \





typedef enum _earsbufobj_in_out
{
    EARSBUFOBJ_IN = 0,
    EARSBUFOBJ_OUT = 1
} e_earsbufobj_in_out;

typedef enum _ears_bufstatus
{
    EARSBUFOBJ_BUFSTATUS_NONE = 0,
    EARSBUFOBJ_BUFSTATUS_COPIED = 1,
    EARSBUFOBJ_BUFSTATUS_USERNAMED = 2,
    EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED = 3
} e_earsbufobj_bufstatus;



typedef struct earsbufobj_stored_buffer
{
    t_buffer_ref              *l_ref;
    t_symbol                  *l_name;
    t_object                  *l_buf;
    e_earsbufobj_bufstatus    l_status;
} t_earsbufobj_stored_buffer;


typedef struct earsbufobj_store
{
    long                        num_stored_bufs;
    t_earsbufobj_stored_buffer  *stored_buf;
    long                        max_num_stored_bufs; // currently can only be 0 (=no limit) or 1 (=single buffer)
} t_earsbufobj_store;




typedef enum _earsbufobj_namings
{
    EARSBUFOBJ_NAMING_COPY = 0,   ///< Output buffers are NOT cloned, and copied whenever possible from the input buffers
    EARSBUFOBJ_NAMING_STATIC = 1,   ///< Output buffers are "static": we always use the same ones
    EARSBUFOBJ_NAMING_DYNAMIC = 2,  ///< Output buffers are "dynamic": new buffers are created at each output
                                    /// (but can be cycled via the "cycle" message)
} e_earsbufobj_namings;


typedef enum _earsbufobj_flag
{
    EARSBUFOBJ_FLAG_NONE = 0,
    EARSBUFOBJ_FLAG_DUPLICATE_INPUT_BUFFERS = 1,   ///< Input buffers are not cloned inside the input stores
    EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES = 2,   ///< Supports naming copy, i.e. "inplace" modification
} e_earsbufobj_flag;


typedef struct _earsbufobj
{
    t_llllobj_object		l_ob;			///< the t_object from which we inherit
    
    // Inlets
    t_int32					l_numins;       ///< how many inlets do we have
    void                    **l_proxy;      ///< inlets
    long                    l_in;           ///< place to store proxy number
    t_int32					l_numbufins;    ///< how many buffer inlets do we have
    t_earsbufobj_store      *l_instore;     ///< the in stores
    
    // Outlets
// (the first two fields are handled directly via the llllobj_object structure
//    t_int32					l_numouts;      ///< how many outlets
//    void					**l_outlet;     ///< the outlets
    t_int32					l_numbufouts;	///< how many buffer outlets
    t_earsbufobj_store      *l_outstore;	///< the out stores
    t_llll                  *l_outnames;    ///< Output names, could be a level2 list if outlets have multiple buffers
    e_earsbufobj_namings    l_bufouts_naming;   ///< Names for buffer outlets are static, i.e. "destructive” operation mode.

    // stuff for cycling over a finite list of generated output names
    t_llll                  *l_generated_outnames;    ///< Updated list of automatically generated output buffer names
                                                      ///  (one subllll for each buffer outlet
    long                    l_generated_outname_count[LLLL_MAX_OUTLETS];  ///< Current indices of the used generated outname

    // Settings
    char                    l_timeunit;      ///< Time unit
    char                    l_ampunit;      ///< Amplitude unit
    char                    l_envtimeunit;      ///< Time unit for envelopes
    char                    l_envampunit;      ///< Amplitude unit for envelopes
    char                    l_pitchunit;      ///< Pitch unit
    char                    l_angleunit;      ///< Angle unit

    t_systhread_mutex       l_mutex;        ///< A mutex
    
    long                    l_flags;        ///< A combination of the e_earsbufobj_flag
    
    char                    l_is_freeing;   ///< 1 when object is being freed;
    long                    l_curr_proxy;  ///< Filled with the number of the proxy being used
} t_earsbufobj;




void ears_error_bachcheck();

/// Buffer mechanisms
void ears_hashtab_setup();
t_hashtab *ears_hashtab_get();
void ears_buffer_store(t_symbol *name);
void ears_hashtab_inccount(t_symbol *name);
t_atom_long ears_hashtab_getcount(t_symbol *name);
void earsbufobj_buffer_release_raw(t_earsbufobj *e_ob, t_object *buf, t_symbol *name, char mustfree);
void earsbufobj_buffer_release(t_earsbufobj *e_ob, e_earsbufobj_in_out where, long store, long bufferidx, bool prevent_from_freeing = false);

// Proxy mechanism
long earsbufobj_proxy_getinlet(t_earsbufobj *e_ob);


/// Accessors
t_max_err earsbufobj_setattr_outname(t_earsbufobj *e_ob, t_object *attr, long argc, t_atom *argv);
t_max_err earsbufobj_notify(t_earsbufobj *e_ob, t_symbol *s, t_symbol *msg, void *sender, void *data);

/// Methods and attributes
void earsbufobj_add_common_methods(t_class *c);
void earsbufobj_class_add_outname_attr(t_class *c);
void earsbufobj_class_add_timeunit_attr(t_class *c);
void earsbufobj_class_add_ampunit_attr(t_class *c);
void earsbufobj_class_add_envtimeunit_attr(t_class *c);
void earsbufobj_class_add_envampunit_attr(t_class *c);
void earsbufobj_class_add_pitchunit_attr(t_class *c);
void earsbufobj_class_add_angleunit_attr(t_class *c);
void earsbufobj_class_add_naming_attr(t_class *c);


/// Basic API
void earsbufobj_buffer_link(t_earsbufobj *e_ob, e_earsbufobj_in_out where, long store_index, long buffer_index, t_symbol *buf_name);
void earsbufobj_init(t_earsbufobj *e_ob, long flags = 0);
void earsbufobj_setup(t_earsbufobj *e_ob, const char *in_types, const char *out_types, t_llll *outlet_names);
void earsbufobj_init_and_setup(t_earsbufobj *e_ob, const char *in_types, const char *out_types, t_llll *outlet_names, long flags = 0);
bool earsbufobj_is_buf_autoassigned(t_earsbufobj *e_ob, e_earsbufobj_in_out inout, long store, long bufferidx);
void earsbufobj_free(t_earsbufobj *e_ob);
void earsbufobj_resize_store(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long new_size, char also_create_unique_buffers);
void earsbufobj_store_buffer_list(t_earsbufobj *e_ob, t_llll *buffers, long store_idx);
t_llll *earsbufobj_parse_gimme(t_earsbufobj *e_ob, e_llllobj_obj_types type, t_symbol *msg, long ac, t_atom *av);

t_max_err earsbufobj_setattr_naming(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv);
void earsbufobj_release_generated_outnames(t_earsbufobj *e_ob);

t_llll *earsbufobj_extract_names_from_args(t_earsbufobj *e_ob, t_llll *args, char assign_naming_policy = true);

t_buffer_ref *earsbufobj_get_inlet_buffer_ref(t_earsbufobj *e_ob, long store_idx, long buffer_idx);
t_object *earsbufobj_get_inlet_buffer_obj(t_earsbufobj *e_ob, long store_idx, long buffer_idx);
t_symbol *earsbufobj_get_inlet_buffer_name(t_earsbufobj *e_ob, long store_idx, long buffer_idx);

t_buffer_ref *earsbufobj_get_outlet_buffer_ref(t_earsbufobj *e_ob, long store_idx, long buffer_idx);
t_object *earsbufobj_get_outlet_buffer_obj(t_earsbufobj *e_ob, long store_idx, long buffer_idx);
t_symbol *earsbufobj_get_outlet_buffer_name(t_earsbufobj *e_ob, long store_idx, long buffer_idx);

void earsbufobj_store_buffer(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx, t_symbol *buffername);
void earsbufobj_store_empty_buffer(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx);
void earsbufobj_importreplace_buffer(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx, t_symbol *filename);



void earsbufobj_outlet_buffer(t_earsbufobj *e_ob, long outnum);
void earsbufobj_outlet_anything(t_earsbufobj *e_ob, long outnum, t_symbol *s, long ac, t_atom *av);
void earsbufobj_outlet_symbol_list(t_earsbufobj *e_ob, long outnum, long numsymbols, t_symbol **s);
void earsbufobj_outlet_llll(t_earsbufobj *e_ob, long outnum, t_llll *ll);
void earsbufobj_outlet_bang(t_earsbufobj *e_ob, long outnum);

t_earsbufobj_store *earsbufobj_get_store(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long index);
void earsbufobj_store_copy_format(t_earsbufobj *e_ob, e_earsbufobj_in_out source, long source_store_idx, long source_buffer_idx, e_earsbufobj_in_out dest, long dest_store_idx, long dest_buffer_idx);
void earsbufobj_refresh_outlet_names(t_earsbufobj *e_ob, char force_refresh_even_if_static = false);
long earsbufobj_get_num_inlet_stored_buffers(t_earsbufobj *e_ob, long store_idx, char remove_empty_buffers);
long earsbufobj_get_num_outlet_stored_buffers(t_earsbufobj *e_ob, long store_idx, char remove_empty_buffers);


t_earsbufobj_store *earsbufobj_get_store(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long index);
t_buffer_ref *earsbufobj_get_stored_buffer_ref(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx);
t_buffer_obj *earsbufobj_get_stored_buffer_obj(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx);
t_symbol *earsbufobj_get_stored_buffer_name(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx);

void earsbufobj_mutex_lock(t_earsbufobj *e_ob);
void earsbufobj_mutex_unlock(t_earsbufobj *e_ob);


// as symbol_unique() but also accounts for outlet modes
t_symbol *earsbufobj_output_get_symbol_unique(t_earsbufobj *e_ob, long outstore_idx, long buffer_idx, e_earsbufobj_bufstatus *status = NULL);




//// BUFFER UNIT CONVERSIONS
double earsbufobj_input_to_fsamps(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, bool is_envelope = false);
long earsbufobj_input_to_samps(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, bool is_envelope = false);
double earsbufobj_input_to_ms(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, bool is_envelope = false);
double earsbufobj_input_to_ratio(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, bool is_envelope = false);
double earsbufobj_input_convert_timeunit(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, e_ears_timeunit new_timeunit, bool is_envelope = false); // generic one


long earsbufobj_atom_to_samps(t_earsbufobj *e_ob, t_atom *v, t_buffer_obj *buf);
void earsbufobj_samps_to_atom(t_earsbufobj *e_ob, long samps, t_buffer_obj *buf, t_atom *a);

double earsbufobj_input_to_linear(t_earsbufobj *e_ob, double value);
double earsbufobj_input_to_db(t_earsbufobj *e_ob, double value);
double earsbufobj_input_to_radians(t_earsbufobj *e_ob, double value);
t_llll *earsbufobj_llllelem_to_linear(t_earsbufobj *e_ob, t_llllelem *elem);
t_llll *earsbufobj_llllelem_to_linear_and_samples(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf);
t_llll *earsbufobj_llllelem_to_env_samples(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf);
t_llll *earsbufobj_llllelem_to_cents_and_samples(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf);



// export
void earsbufobj_write_buffer(t_earsbufobj *e_ob, t_object *buf, t_symbol *msg, t_symbol *exportpath, t_symbol *format);


double earsbufobj_linear_to_output(t_earsbufobj *e_ob, double value);

t_llll *earsbufobj_llllelem_convert_envtimeunit_and_normalize_range(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf, e_ears_timeunit dest_envtimeunit, double orig_from, double orig_to, char convert_from_decibels);
t_llll *earsbufobj_llll_convert_envtimeunit_and_normalize_range(t_earsbufobj *e_ob, t_llll *ll, t_buffer_obj *buf, e_ears_timeunit dest_envtimeunit, double orig_from, double orig_to, char convert_from_decibels);


#endif // _EARS_BUF_OBJECT_H_
