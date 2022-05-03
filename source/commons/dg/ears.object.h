/**
	@file
	ears.object.h
	Common utilities header for the objects in the buffer ears sublibrary
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_OBJECT_H_
#define _EARS_BUF_OBJECT_H_

#include "ears.commons.h"
#include <vector>


#define EARSBUFOBJ_ADD_DEFERRED_METHODS(NAME) \
void buf_ ## NAME ## _bang_handlethread(t_buf_ ## NAME *x); \
void buf_ ## NAME ## _anything_handlethread(t_buf_ ## NAME *x, t_symbol *msg, long ac, t_atom *av); \
void buf_ ## NAME ## _bang_handlethread(t_buf_ ## NAME *x) \
{ \
    switch (x->e_ob.l_blocking) { \
        case EARSBUFOBJ_BLOCKING_OWNTHREAD: \
        break; \
        case EARSBUFOBJ_BLOCKING_SCHEDULER: \
            schedule(x, (method)buf_ ## NAME ##_bang, 0, NULL, 0, NULL); \
        break; \
        case EARSBUFOBJ_BLOCKING_MAINTHREAD: \
        default: \
            defer(x, (method)buf_ ## NAME ##_bang, NULL, 0, NULL); \
        break; \
    } \
} \
\
void buf_ ## NAME ## _anything_handlethread(t_buf_ ## NAME *x, t_symbol *msg, long ac, t_atom *av) \
{ \
    x->e_ob.l_curr_proxy = proxy_getinlet((t_object *) x); \
    if (!x->e_ob.l_inlet_hot[x->e_ob.l_curr_proxy]) { \
        buf_ ## NAME ##_anything(x, msg, ac, av); \
        return; \
    } \
    switch (x->e_ob.l_blocking) { \
        case EARSBUFOBJ_BLOCKING_OWNTHREAD: \
        break; \
        case EARSBUFOBJ_BLOCKING_SCHEDULER: \
            schedule(x, (method)buf_ ## NAME ##_anything, 0, msg, ac, av); \
        break; \
        case EARSBUFOBJ_BLOCKING_MAINTHREAD: \
        default: \
            defer(x, (method)buf_ ## NAME ##_anything, msg, ac, av); \
        break; \
    } \
} \


#define EARSBUFOBJ_ADD_INT_FLOAT_METHODS(NAME) \
void buf_ ## NAME ## _int_handlethread(t_buf_ ## NAME *x, t_atom_long num); \
void buf_ ## NAME ## _float_handlethread(t_buf_ ## NAME *x, t_atom_float num); \
\
void buf_ ## NAME ## _int_handlethread(t_buf_ ## NAME *x, t_atom_long num) \
{ \
    t_atom argv[1]; \
    atom_setlong(argv, num); \
    buf_ ## NAME ## _anything_handlethread(x, _sym_list, 1, argv); \
} \
\
void buf_ ## NAME ## _float_handlethread(t_buf_ ## NAME *x, t_atom_float num) \
{ \
    t_atom argv[1]; \
    atom_setfloat(argv, num); \
    buf_ ## NAME ## _anything_handlethread(x, _sym_list, 1, argv); \
} \



#define EARSBUFOBJ_ADD_IO_METHODS(NAME) \
EARSBUFOBJ_ADD_DEFERRED_METHODS(NAME) \
EARSBUFOBJ_ADD_INT_FLOAT_METHODS(NAME) \


#define EARSBUFOBJ_DECLARE_COMMON_METHODS_HANDLETHREAD(NAME) \
class_addmethod(c, (method)buf_ ## NAME ## _int_handlethread,       "int",       A_LONG, 0); \
class_addmethod(c, (method)buf_ ## NAME ## _float_handlethread,     "float",     A_FLOAT, 0); \
class_addmethod(c, (method)buf_ ## NAME ## _anything_handlethread,  "anything",  A_GIMME, 0); \
class_addmethod(c, (method)buf_ ## NAME ## _anything_handlethread,  "list",      A_GIMME, 0); \
class_addmethod(c, (method)buf_ ## NAME ## _bang_handlethread,      "bang",     0); \
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


#define EARS_DEFAULT_RESAMPLING_WINDOW_WIDTH (11)   ///< Default resampling window size. Should we increase this?
#define EARS_DEFAULT_WRITE_FORMAT (_sym_int16)      ///< Default output write format for saving files.
                                                    ///  This is coherent with Max's default



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
    EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES = 2,       ///< Supports naming copy, i.e. "inplace" modification
} e_earsbufobj_flag;

typedef enum _earsbufobj_blocking
{
    EARSBUFOBJ_BLOCKING_OWNTHREAD = 0,    ///< Processing happens in an own separate thread
    EARSBUFOBJ_BLOCKING_MAINTHREAD = 1,   ///< Processing happens in the main thread
    EARSBUFOBJ_BLOCKING_SCHEDULER = 2,    ///< Processing happens in the scheduler thread
} e_earsbufobj_blocking;


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
    char                    l_outlet_types[LLLL_MAX_OUTLETS];  ///< Current indices of the used generated outname
    t_int32					l_numbufouts;	///< how many buffer outlets
    t_earsbufobj_store      *l_outstore;	///< the out stores
    t_llll                  *l_outnames;    ///< Output names, could be a level2 list if outlets have multiple buffers
    char                    l_bufouts_naming;   ///< One of the e_earsbufobj_namings.
                                                /// Names for buffer outlets are static, i.e. "destructive” operation mode.
// stuff for cycling over a finite list of generated output names
    t_llll                  *l_generated_outnames;    ///< Updated list of automatically generated output buffer names
                                                      ///  (one subllll for each buffer outlet)
    long                    l_generated_outname_count[LLLL_MAX_OUTLETS];  ///< Current indices of the used generated outname

    bool                    l_inlet_hot[LLLL_MAX_INLETS];  ///<1 for Inlets that are hot

    // threading
    char                    l_blocking;   ///< One of the e_earsbufobj_blocking
    
    
    
    // Settings
    char                    l_timeunit;      ///< Time unit
    char                    l_ampunit;      ///< Amplitude unit
    char                    l_antimeunit;      ///< Time unit for analysis
    char                    l_envtimeunit;      ///< Time unit for envelopes
    char                    l_envampunit;      ///< Amplitude unit for envelopes
    char                    l_pitchunit;      ///< Pitch unit
    char                    l_frequnit;       ///< Frequency unit
    char                    l_angleunit;      ///< Angle unit
    
    // Resampling modes
    char                    l_resamplingpolicy;
    long                    l_resamplingfilterwidth;
    
    // analysis
    double                  a_framesize;
    double                  a_hopsize;
    t_atom                  a_numframes;
    double                  a_overlap;
    t_symbol                *a_wintype;
    char                    a_winnorm; //< if set, window is normalized to have area of 1 and then scaled by a factor of 2
    long                    a_zeropadding;
    char                    a_zerophase;
    char                    a_lastframetoendoffile;
    char                    a_winstartfromzero;
    
    char                    l_slopemapping; ///< Slope mapping (one of the #e_slope_mapping)

    t_systhread_mutex       l_mutex;        ///< A mutex
    
    long                    l_flags;        ///< A combination of the e_earsbufobj_flag
    
    char                    l_is_freeing;   ///< 1 when object is being freed;
    long                    l_curr_proxy;  ///< Filled with the number of the proxy being used
    char                    l_buffer_size_changed; ///< 1 when buffer size has changed
} t_earsbufobj;



void ears_error_bachcheck();


/// Buffer mechanisms
void ears_hashtabs_setup();
t_hashtab *ears_hashtab_get();
void ears_hashtab_inccount(t_symbol *name);
void ears_hashtab_store(t_symbol *name);
t_atom_long ears_hashtab_getcount(t_symbol *name);
void earsbufobj_buffer_release(t_earsbufobj *e_ob, e_earsbufobj_in_out where, long store, long bufferidx);

// spectrogram table
t_hashtab *ears_hashtab_spectrograms_get();
long ears_hashtab_spectrograms_store(t_symbol *buffername, void *data);
void *ears_hashtab_spectrograms_retrieve(t_symbol *buffername);


// Proxy mechanism
long earsbufobj_proxy_getinlet(t_earsbufobj *e_ob);


// Handling generated out names
t_llll *earsbufobj_generated_names_llll_getlist(t_llll *ll, long pos1, long pos2);
t_llllelem *earsbufobj_generated_names_llll_getsymbol(t_llll *ll, long pos1, long pos2, long pos3);
void earsbufobj_generated_names_llll_subssymbol(t_llll *ll, t_symbol *sym, long pos1, long pos2, long pos3);



/// Accessors
t_max_err earsbufobj_setattr_outname(t_earsbufobj *e_ob, t_object *attr, long argc, t_atom *argv);
t_max_err earsbufobj_notify(t_earsbufobj *e_ob, t_symbol *s, t_symbol *msg, void *sender, void *data);

/// Methods and attributes
void earsbufobj_add_common_methods(t_class *c, long flags = 0);
void earsbufobj_class_add_outname_attr(t_class *c);
void earsbufobj_class_add_blocking_attr(t_class *c);
void earsbufobj_class_add_timeunit_attr(t_class *c);
void earsbufobj_class_add_antimeunit_attr(t_class *c);
void earsbufobj_class_add_ampunit_attr(t_class *c);
void earsbufobj_class_add_envtimeunit_attr(t_class *c);
void earsbufobj_class_add_envampunit_attr(t_class *c);
void earsbufobj_class_add_pitchunit_attr(t_class *c);
void earsbufobj_class_add_frequnit_attr(t_class *c);
void earsbufobj_class_add_angleunit_attr(t_class *c);
void earsbufobj_class_add_naming_attr(t_class *c);
void earsbufobj_class_add_slopemapping_attr(t_class *c);
void earsbufobj_class_add_framesize_attr(t_class *c);
void earsbufobj_class_add_hopsize_attr(t_class *c);
void earsbufobj_class_add_numframes_attr(t_class *c);
void earsbufobj_class_add_overlap_attr(t_class *c);
void earsbufobj_class_add_wintype_attr(t_class *c);
void earsbufobj_class_add_winstartfromzero_attr(t_class *c);
void earsbufobj_class_add_winnormalized_attr(t_class *c);
void earsbufobj_class_add_zerophase_attr(t_class *c);
void earsbufobj_class_add_zeropadding_attr(t_class *c);

void earsbufobj_class_add_resamplingpolicy_attr(t_class *c);
void earsbufobj_class_add_resamplingfiltersize_attr(t_class *c);

/// Basic API
void earsbufobj_buffer_link(t_earsbufobj *e_ob, e_earsbufobj_in_out where, long store_index, long buffer_index, t_symbol *buf_name);
void earsbufobj_init(t_earsbufobj *e_ob, long flags = 0);
void earsbufobj_setup(t_earsbufobj *e_ob, const char *in_types, const char *out_types, t_llll *outlet_names);
void earsbufobj_init_and_setup(t_earsbufobj *e_ob, const char *in_types, const char *out_types, t_llll *outlet_names, long flags = 0);
bool earsbufobj_is_buf_autoassigned(t_earsbufobj *e_ob, e_earsbufobj_in_out inout, long store, long bufferidx);
bool earsbufobj_is_buf_autoassigned_or_copied(t_earsbufobj *e_ob, e_earsbufobj_in_out where, long store, long bufferidx);
void earsbufobj_free(t_earsbufobj *e_ob);
void earsbufobj_resize_store(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long new_size, char also_create_unique_buffers);
long llll_get_num_symbols_root(t_llll *ll);
long earsbufobj_store_buffer_list(t_earsbufobj *e_ob, t_llll *buffers, long store_idx);
t_llll *earsbufobj_parse_gimme(t_earsbufobj *e_ob, e_llllobj_obj_types type, t_symbol *msg, long ac, t_atom *av);

t_max_err earsbufobj_setattr_naming(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv);
void earsbufobj_release_generated_outnames(t_earsbufobj *e_ob);

t_llll *earsbufobj_extract_names_from_args(t_earsbufobj *e_ob, t_llll *args, char assign_naming_policy = true);

e_slope_mapping earsbufobj_get_slope_mapping(t_earsbufobj *e_ob);

t_object *earsbufobj_get_inlet_buffer_obj(t_earsbufobj *e_ob, long store_idx, long buffer_idx, bool update_buffer_obj = true);
t_symbol *earsbufobj_get_inlet_buffer_name(t_earsbufobj *e_ob, long store_idx, long buffer_idx);

t_object *earsbufobj_get_outlet_buffer_obj(t_earsbufobj *e_ob, long store_idx, long buffer_idx);
t_symbol *earsbufobj_get_outlet_buffer_name(t_earsbufobj *e_ob, long store_idx, long buffer_idx);
long earsbufobj_outlet_to_bufstore(t_earsbufobj *e_ob, long outlet);

long earsbufobj_get_instore_size(t_earsbufobj *e_ob, long store_idx);
void earsbufobj_store_buffer(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx, t_symbol *buffername);
void earsbufobj_store_empty_buffer(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx);
void earsbufobj_importreplace_buffer(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx, t_symbol *filename);



void earsbufobj_outlet_buffer(t_earsbufobj *e_ob, long outnum);
void earsbufobj_outlet_anything(t_earsbufobj *e_ob, long outnum, t_symbol *s, long ac, t_atom *av);
void earsbufobj_outlet_symbol_list(t_earsbufobj *e_ob, long outnum, long numsymbols, t_symbol **s);
void earsbufobj_outlet_llll(t_earsbufobj *e_ob, long outnum, t_llll *ll);
void earsbufobj_outlet_bang(t_earsbufobj *e_ob, long outnum);
void earsbufobj_shoot_llll(t_earsbufobj *e_ob, long outnum);
void earsbufobj_gunload_llll(t_earsbufobj *e_ob, long outnum, t_llll *ll);

t_earsbufobj_store *earsbufobj_get_store(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long index);
void earsbufobj_store_copy_format(t_earsbufobj *e_ob, e_earsbufobj_in_out source, long source_store_idx, long source_buffer_idx, e_earsbufobj_in_out dest, long dest_store_idx, long dest_buffer_idx);
void earsbufobj_refresh_outlet_names(t_earsbufobj *e_ob, char force_refresh_even_if_static = false);
long earsbufobj_get_num_inlet_stored_buffers(t_earsbufobj *e_ob, long store_idx, char remove_empty_buffers);
long earsbufobj_get_num_outlet_stored_buffers(t_earsbufobj *e_ob, long store_idx, char remove_empty_buffers);


t_earsbufobj_store *earsbufobj_get_store(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long index);
t_buffer_obj *earsbufobj_get_stored_buffer_obj(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx);
t_symbol *earsbufobj_get_stored_buffer_name(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx);

void earsbufobj_mutex_lock(t_earsbufobj *e_ob);
void earsbufobj_mutex_unlock(t_earsbufobj *e_ob);


// as symbol_unique() but also accounts for outlet modes
t_symbol *earsbufobj_output_get_symbol_unique(t_earsbufobj *e_ob, long outstore_idx, long buffer_idx, e_earsbufobj_bufstatus *status = NULL);




//// BUFFER UNIT CONVERSIONS
double earsbufobj_time_to_fsamps(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, bool is_envelope = false, bool is_analysis = false);
long earsbufobj_time_to_samps(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, bool is_envelope = false, bool is_analysis = false);
double earsbufobj_time_to_ms(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, bool is_envelope = false, bool is_analysis = false);
double earsbufobj_time_to_durationratio(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, bool is_envelope = false, bool is_analysis = false);
double earsbufobj_pitch_to_cents(t_earsbufobj *e_ob, double value);
double earsbufobj_pitch_to_hz(t_earsbufobj *e_ob, double value);
double earsbufobj_convert_timeunit(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, e_ears_timeunit new_timeunit, bool is_envelope = false, bool is_analysis = false); // generic one
double earsbufobj_freq_to_hz(t_earsbufobj *e_ob, double value);
double earsbufobj_freq_to_midi(t_earsbufobj *e_ob, double value);
double earsbufobj_freq_to_cents(t_earsbufobj *e_ob, double value);

// Convenience non-earsobj versions
double ears_convert_timeunit(double value, t_buffer_obj *buf, e_ears_timeunit from, e_ears_timeunit to);
double ears_convert_ampunit(double value, t_buffer_obj *buf, e_ears_ampunit from, e_ears_ampunit to);
double ears_convert_frequnit(double value, t_buffer_obj *buf, e_ears_frequnit from, e_ears_frequnit to);
double ears_convert_pitchunit(double value, t_buffer_obj *buf, e_ears_pitchunit from, e_ears_pitchunit to);
double ears_convert_angleunit(double value, t_buffer_obj *buf, e_ears_angleunit from, e_ears_angleunit to);
// vector-of-floats in-place versions
void ears_convert_timeunit(std::vector<float> &vec, t_buffer_obj *buf, e_ears_timeunit from, e_ears_timeunit to);
void ears_convert_ampunit(std::vector<float> &vec, t_buffer_obj *buf, e_ears_ampunit from, e_ears_ampunit to);
void ears_convert_frequnit(std::vector<float> &vec, t_buffer_obj *buf, e_ears_frequnit from, e_ears_frequnit to);
void ears_convert_pitchunit(std::vector<float> &vec, t_buffer_obj *buf, e_ears_pitchunit from, e_ears_pitchunit to);
void ears_convert_angleunit(std::vector<float> &vec, t_buffer_obj *buf, e_ears_angleunit from, e_ears_angleunit to);



long earsbufobj_atom_to_samps(t_earsbufobj *e_ob, t_atom *v, t_buffer_obj *buf);
void earsbufobj_samps_to_atom(t_earsbufobj *e_ob, long samps, t_buffer_obj *buf, t_atom *a);

double earsbufobj_amplitude_to_linear(t_earsbufobj *e_ob, double value);
double earsbufobj_amplitude_to_db(t_earsbufobj *e_ob, double value);
double earsbufobj_angle_to_radians(t_earsbufobj *e_ob, double value);
double earsbufobj_angle_to_degrees(t_earsbufobj *e_ob, double value);
t_llll *earsbufobj_llllelem_to_linear(t_earsbufobj *e_ob, t_llllelem *elem);
t_llll *earsbufobj_llllelem_to_linear_and_samples(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf);
t_llll *earsbufobj_llllelem_to_env_samples(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf);
t_llll *earsbufobj_pitch_llllelem_to_cents_and_samples(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf);



// export
void earsbufobj_write_buffer(t_earsbufobj *e_ob, t_object *buf, t_symbol *msg, t_symbol *exportpath, t_symbol *format);


double earsbufobj_linear_to_output(t_earsbufobj *e_ob, double value);

t_llll *earsbufobj_llllelem_convert_envtimeunit_and_normalize_range(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf, e_ears_timeunit dest_envtimeunit, double orig_from, double orig_to, char convert_from_decibels);
t_llll *earsbufobj_llll_convert_envtimeunit_and_normalize_range(t_earsbufobj *e_ob, t_llll *ll, t_buffer_obj *buf, e_ears_timeunit dest_envtimeunit, double orig_from, double orig_to, char convert_from_decibels);

// returns true if the s is _ = or !
t_bool earsbufobj_is_sym_naming_mech(t_symbol *s);


#endif // _EARS_BUF_OBJECT_H_
