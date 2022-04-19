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
 
 @author
 Andrea Agostini, partly based upon work by Alexander J. Harker
 
 @digest
 Offline host for patches operating on buffers
 
 @description
 Loads a DSP patch and runs it in non-realtime
 reading from and writing to buffers

 @discussion
 
 @category
 ears process
 
 @keywords
 buffer, offline, patch, patcher, non-realtime
 
 @seealso
 ears.in, ears.in~, ears.mc.in~, ears.out, ears.out~, ears.mc.out~, ears.processinfo~, ears.tovector~, ears.fromvector~
 
 @owner
 Andrea Agostini
 */

#include <stdio.h>

#include <ext.h>
#include <ext_obex.h>
#include <ext_wind.h>
#include <ext_buffer.h>
#include <jpatcher_api.h>

#include <unordered_set>
#include <map>
#include <vector>
#include <set>
#include "ears.process_commons.h"



// Fixes a compiler error with object_method_direct when building the code as cpp
// Shouldn't be necessary with recent versions of the Max SDK, but it won't hurt either
#define object_method_direct_cpp(rt, sig, x, s, ...) ((rt (*)sig)object_method_direct_getmethod((t_object *)x, s))((t_object*)object_method_direct_getobject((t_object *)x, s), __VA_ARGS__)

t_class *earsprocess_class;

template <typename T>
class earsprocessMultiMap
{
public:
    earsprocessMultiMap() : maxIdx(0) { }
    
    std::map<t_atom_long, std::unordered_set<T>*> theMap;
    
    int maxIdx;
    
    void insert(t_atom_long idx, T what) {
        if (theMap.find(idx) == theMap.end()) {
            std::unordered_set<T>* s = new std::unordered_set<T>;
            theMap[idx] = s;
            //.insert(std::pair<t_atom_long, std::unordered_set<T>*>(idx, s));
        }
        theMap[idx]->insert(what);
        if (maxIdx < idx)
            maxIdx = idx;
    }
    
    void remove(t_atom_long idx, T what) {
        if (theMap.find(idx) == theMap.end())
            return;
        theMap[idx]->erase(what);
        if (theMap[idx]->size() == 0) {
            delete theMap[idx];
            theMap.erase(idx);
        }
    }
    
    void clear() {
        for (auto o: theMap) {
            delete o.second;
        }
        theMap.clear();
    }
};

typedef earsprocessMultiMap<t_object*> earsInsByIndex;

class earsOuts : public earsprocessMultiMap<t_object *>
{
public:
    using earsprocessMultiMap::earsprocessMultiMap;
    
    void setOutlets(long nOutlets, void** outlets) {
        for (auto i: theMap) {
            t_atom_long index = i.first;
            auto oSet = i.second;
            void *out = index <= nOutlets ? outlets[index] : nullptr;
            for (t_object* o: *oSet)
                object_method(o, gensym("setoutlet"), index, out);
        }
    }
};

typedef std::unordered_set<t_object*> objectSet;

typedef struct _earsprocess
{
    t_earsbufobj e_ob;
    
    t_symbol* inbuf[EARS_PROCESS_MAX_INPUT_BUFFERS];
    
    t_patcher *parent_patch;

    t_bool running;
    
    long vs;
    double sr;

    long policy;
    double tail;
    
    long scalarmode;
    long reload;
    
    long autoclock;
    t_symbol *clock_name;
    
    t_symbol *patch_name;
    
    t_bool stopped;
    
    t_atom_long nBufInlets;
    t_atom_long nBufOutlets;
    t_atom_long nOutBufChans[EARS_PROCESS_MAX_OUTPUT_BUFFERS];
    t_atom_long nDataOutlets;
    
    earsInsByIndex *theInsByIndex;
    std::set<t_object*> *theIns;
    //earsOuts *theOuts;
    std::set<t_object*> *theOuts;
    void *dataOutlets[EARS_PROCESS_MAX_DATA_OUTLETS];

    t_patcher *client_patch;
    long client_argc;
    t_atom client_argv[256];
    
    objectSet* earsInTildeObjects;
    objectSet* earsOutTildeObjects;
    objectSet* earsprocessinfoObjects;
    
    t_bool generator;
    
} t_earsprocess;



void *earsprocess_new(t_symbol *s, long argc, t_atom *argv);
void earsprocess_free(t_earsprocess *x);
void earsprocess_assist(t_earsprocess *x, void *b, long m, long a, char *s);
void earsprocess_inletinfo(t_earsprocess *x, void *b, long a, char *t);


void earsprocess_deletepatch(t_earsprocess *x, t_symbol *msg, long argc, t_atom *argv);
void earsprocess_clear(t_earsprocess *x);
void earsprocess_patchername(t_earsprocess *x, t_symbol *s, long argc, t_atom *argv);


void earsprocess_bang(t_earsprocess *x);
void earsprocess_bang_do(t_earsprocess *x, t_symbol *s, t_atom_long ac, t_atom *av);

void earsprocess_stop(t_earsprocess *x);
void earsprocess_int(t_earsprocess *x, t_atom_long n);
void earsprocess_float(t_earsprocess *x, t_atom_float f);
void earsprocess_list(t_earsprocess *x, t_symbol *s, t_atom_long ac, t_atom* av);
void earsprocess_anything(t_earsprocess *x, t_symbol *s, t_atom_long argc, t_atom *argv);

void earsprocess_dsp64(t_earsprocess *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);

void earsprocess_dblclick(t_earsprocess *x);
void earsprocess_open(t_earsprocess *x);
void earsprocess_pclose(t_earsprocess *x);
void earsprocess_wclose(t_earsprocess *x);

void earsprocess_pupdate(t_earsprocess *x, void *b, t_patcher *p);
void *earsprocess_subpatcher(t_earsprocess *x, long index, void *arg);
void earsprocess_parentpatcher(t_earsprocess *x, t_patcher **parent);

void earsprocess_setupOutObjects(t_earsprocess *x);

void earsprocess_earsintildecreated(t_earsprocess *x, t_atom_long ioNum, t_object *in);
void earsprocess_earsintildedeleted(t_earsprocess *x, t_object *in);
void earsprocess_earsouttildecreated(t_earsprocess *x, t_atom_long ioNum, t_object *out);
void earsprocess_earsouttildedeleted(t_earsprocess *x, t_object *out);

void earsprocess_earsincreated(t_earsprocess *x, t_object *in, t_atom_long n, t_atom_long *index);
void earsprocess_earsoutcreated(t_earsprocess *x, long maxindex, t_object *obj);
void earsprocess_earsindeleted(t_earsprocess *x, t_object *in, t_atom_long n, t_atom_long *index);
void earsprocess_earsoutdeleted(t_earsprocess *x, t_object *obj);

void earsprocess_earsprocessinfocreated(t_earsprocess *x, t_object *obj);
void earsprocess_earsprocessinfodeleted(t_earsprocess *x, t_object *obj);

void earsprocess_autoclock(t_earsprocess *x, t_patcher *p);

t_max_err earsprocess_set_duration(t_earsprocess *x, t_object *attr, long argc, t_atom *argv);
t_max_err earsprocess_get_duration(t_earsprocess *x, t_object *attr, long *argc, t_atom **argv);

t_max_err earsprocess_set_vs(t_earsprocess *x, t_object *attr, long argc, t_atom *argv);
t_max_err earsprocess_get_ownsdspchain(t_earsprocess *x, t_object *attr, long *argc, t_atom **argv);



t_llll *emptyLl;

typedef enum {
    eDURPOLICY_SHORTEST = 0,
    eDURPOLICY_LONGEST = 1,
    eDURPOLICY_FIXED = 2
} e_durationPolicies;


int C74_EXPORT main()
{
    common_symbols_init();
    llllobj_common_symbols_init();
    
    if (llllobj_check_version(bach_get_current_llll_version()) || llllobj_test()) {
        ears_error_bachcheck();
        return 1;
    }
    
    CLASS_NEW_CHECK_SIZE(earsprocess_class, "ears.process~", (method)earsprocess_new, (method)earsprocess_free, sizeof(t_earsprocess), NULL, A_GIMME, 0);
    
    
    
    class_addmethod(earsprocess_class, (method)earsprocess_assist, "assist", A_CANT, 0);
    
    // @method wopen @digest Open patcher
    // @description The <m>wopen</m> message
    // opens the patcher window loaded in the object, if any.
    class_addmethod(earsprocess_class, (method)earsprocess_open, "wopen", 0);
    
    
    // @method (doubleclick) @digest Open patcher
    // @description Double-clicking on the object
    // opens the patcher window loaded in the object, if any.
    class_addmethod(earsprocess_class, (method)earsprocess_open, "dblclick", A_CANT, 0);
    
    // @method wclose @digest Closes patcher
    // @description The <m>wclose</m> message
    // closes the patcher window loaded in the object, if any.
    class_addmethod(earsprocess_class, (method)earsprocess_wclose, "wclose", 0);
    

    // @method bang @digest Run process
    // @description
    // A <m>bang</m> in any of the buffer inlets causes the object
    // to perform the process with the latest received
    // buffer names and parameters. <br/>
    // A <m>bang</m> in any other inlet is routed
    // to the client patch through its <o>ears.in</o> objects.
    class_addmethod(earsprocess_class, (method)earsprocess_bang, "bang", 0);
    
    // @method stop @digest Stop process
    // @description
    // The <m>stop</m> message in any of the buffer inlets causes the object
    // to stop immediately the process it is performing;
    // in any other inlet, it is routed
    // to the client patch through its <o>ears.in</o> objects.
    class_addmethod(earsprocess_class, (method)earsprocess_stop, "stop", 0);


    // @method patchername @digest Load patch
    // @description
    // The <m>patchername</m> message in any of the buffer inlets causes the object
    // to stop immediately the process it is performing;
    // in any other inlet, it is routed
    // to the client patch through its <o>ears.in</o> objects.
    class_addmethod(earsprocess_class, (method)earsprocess_patchername, "patchername", A_DEFER, 0);
    
    // @method int @digest Passed to ears.in
    // @description
    // An integer received in one of <o>ears.process~</o>'s inlets
    // is passed to the corresponding <o>ears.in</o> object
    // in the loaded patch.<br/>
    // According to whether each instance of <o>ears.in</o> is in direct mode or not,
    // the integer will be passed immediately to the receiving patch,
    // or at the beginning of the first buffer iteration.
    class_addmethod(earsprocess_class, (method)earsprocess_int, "int", A_LONG, 0);
    
    // @method float @digest Passed to ears.in
    // @description
    // A float received in one of <o>ears.process~</o>'s inlets
    // is passed to the corresponding <o>ears.in</o> object
    // in the loaded patch.<br/>
    // According to whether each instance of <o>ears.in</o> is in direct mode or not,
    // the float will be passed immediately to the receiving patch,
    // or at the beginning of the first buffer iteration.
    class_addmethod(earsprocess_class, (method)earsprocess_float, "float", A_FLOAT, 0);
    
    // @method list @digest Passed to ears.in
    // @description
    // A list received in one of <o>ears.process~</o>'s inlets
    // is passed to the corresponding <o>ears.in</o> object
    // in the loaded patch.<br/>
    // According to whether each instance of <o>ears.in</o> is in direct mode or not,
    // the list will be passed immediately to the receiving patch,
    // or iterated along with the incoming buffers.
    class_addmethod(earsprocess_class, (method)earsprocess_list, "list", A_GIMME, 0);
    
    // @method symbol/llll @digest Function depends on inlet
    // @description
    // If an llll with buffer names is passed to one of the buffer inlets,
    // The contents of the corresponding buffers will be passed
    // to the loaded patcher through the patcher's
    // <o>ears.in~</o> and <o>ears.mc.in~</o> objects,
    // for being processed by the DSP chain of the patcher itself.
    // The names of the buffers containing the processed audio,
    // as output by the patcher's <o>ears.out~</o> and <o>ears.mc.out~</o> objects,
    // will be subsequently output.<br/>
    // Lllls received in non-buffer inlets will be passed to the loaded patcher
    // through its <o>ears.in</o> objects.<br/>
    // If the llll is received in the first inlet, the processing will be triggered.<br/>
    // A single symbol, or any message starting with a symbol, will be treated as an llll anyway.<br/>
    // According to whether each instance of <o>ears.in</o> is in direct mode or not,
    // the llll will be passed immediately to the receiving patch,
    // or iterated along with the incoming buffers.
    class_addmethod(earsprocess_class, (method)earsprocess_anything, "anything", A_GIMME, 0);

    class_addmethod(earsprocess_class, (method)earsprocess_pupdate, "pupdate", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_subpatcher, "subpatcher", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_parentpatcher, "parentpatcher", A_CANT, 0);
    
    class_addmethod(earsprocess_class, (method)earsprocess_earsintildecreated, "ears.in~_created", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_earsouttildecreated, "ears.out~_created", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_earsintildedeleted, "ears.in~_deleted", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_earsouttildedeleted, "ears.out~_deleted", A_CANT, 0);

    class_addmethod(earsprocess_class, (method)earsprocess_earsincreated, "ears.in_created", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_earsoutcreated, "ears.out_created", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_earsindeleted, "ears.in_deleted", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_earsoutdeleted, "ears.out_deleted", A_CANT, 0);
    
    class_addmethod(earsprocess_class, (method)earsprocess_earsprocessinfocreated, "ears.processinfo~_created", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_earsprocessinfodeleted, "ears.processinfo~_deleted", A_CANT, 0);
    
    class_addmethod(earsprocess_class, (method)earsprocess_assist, "assist", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_inletinfo, "inletinfo", A_CANT, 0);

    earsbufobj_add_common_methods(earsprocess_class, 1);

    earsbufobj_class_add_outname_attr(earsprocess_class);
    earsbufobj_class_add_naming_attr(earsprocess_class);
    
    llllobj_class_add_default_bach_attrs_and_methods(earsprocess_class, LLLL_OBJ_VANILLA);

    CLASS_ATTR_OBJ(earsprocess_class, "ownsdspchain", ATTR_SET_OPAQUE | ATTR_SET_OPAQUE_USER, t_earsprocess, e_ob);
    CLASS_ATTR_ACCESSORS(earsprocess_class, "ownsdspchain", (method) earsprocess_get_ownsdspchain, NULL);
    CLASS_ATTR_INVISIBLE(earsprocess_class, "ownsdspchain", 0);
    
    
    
    CLASS_ATTR_LONG(earsprocess_class, "vs", 0, t_earsprocess, vs);
    CLASS_ATTR_ACCESSORS(earsprocess_class, "vs", NULL, (method) earsprocess_set_vs);
    CLASS_ATTR_LABEL(earsprocess_class, "vs", 0, "Vector Size");
    // @description
    // The signal vector size of the loaded patcher, expressed in samples.
    // It must be a power of 2 between 1 and 4096.
    // The default is 128.


    CLASS_ATTR_DOUBLE(earsprocess_class, "sr", 0, t_earsprocess, sr);
    CLASS_ATTR_FILTER_MIN(earsprocess_class, "sr", 0);
    CLASS_ATTR_LABEL(earsprocess_class, "sr", 0, "Default Sample Rate");
    // @description
    // The default sample rate of the loaded patcher,
    // only used for generator patches
    // (i.e., patches with no buffer inlets):
    // if a patcher has buffer inlets, the sample rate of each run of the patcher
    // corresponds to the sample rate of the incoming buffers.<br/>
    // The default is 0, meaning that generator patches will run at the system sample rate.

    
    CLASS_ATTR_DOUBLE(earsprocess_class, "tail", 0, t_earsprocess, tail);
    CLASS_ATTR_LABEL(earsprocess_class, "tail", 0, "Tail Duration");
    CLASS_ATTR_FILTER_MIN(earsprocess_class, "tail", 0);
    // @description
    // The <m>tail</m> attribute adds a duration (always measured in milliseconds)
    // to the processing duration as established by the <m>duration</m> attribute.
    // It defaults to 0.
    // If the loaded patch has no buffer inlets and the tail is not set to a value higher than 0 ms,
    // no processing will take place.
    
    
    CLASS_ATTR_LONG(earsprocess_class, "duration", 0, t_earsprocess, policy);
    CLASS_ATTR_ENUMINDEX(earsprocess_class,"duration", 0, "Shortest Longest Fixed");
    CLASS_ATTR_STYLE_LABEL(earsprocess_class, "duration", 0, "enumindex", "Duration Policy");
    // @description
    // The <m>duration</m> attribute controls how the duration of the incoming buffers
    // affects the duration of the resulting ones.<br/>
    // If the duration policy is set to <m>0</m> (<b>shortest</b>), as per the default,
    // the duration of the processed (and therefore resulting) audio
    // is the duration of the shortest incoming buffer
    // plus the tail. If a list of buffers is passed, to be processed in sequence,
    // the processing duration refers to the shortest buffer in each iteration.<br/>
    // If the duration policy is set to <m>1</m> (<b>longest</b>),
    // the duration of the processed audio
    // is the duration of the longest incoming buffer plus the tail.<br/>
    // If the duration policy is set to <m>2</m> (<b>fixed</b>),
    // the duration of the processed audio is set to the value of the tail,
    // regardless of the duration of the incoming buffers.<br/>
    // If the loaded patcher has a single buffer inlet,
    // the <m>shortest</m> and <m>longest</m> policies are equivalent. If it has no buffer inlets, all three are equivalent
    // and the duration will anyway correspond to the value of the <m>tail</m> attribute.<br/>

    
    
    CLASS_ATTR_LONG(earsprocess_class, "scalarmode", 0, t_earsprocess, scalarmode);
    CLASS_ATTR_STYLE_LABEL(earsprocess_class, "scalarmode", 0, "onoff", "Scalar Mode");
    CLASS_ATTR_FILTER_CLIP(earsprocess_class, "scalarmode", 0, 1);
    // @description
    // When set to 1, if a buffer inlet receives a single buffer
    // while other inlets receive lists of buffers,
    // then the single buffer will be iterated repeatedly against the list of buffers, until the end of the shortest list.<br/>
    // When set to 0 (as per the default), if a buffer inlet receives a single buffer
    // no iterator will be performed, and only the first buffer of each inlet will be processed.
    
    /*
    CLASS_ATTR_LONG(earsprocess_class, "reload", 0, t_earsprocess, reload);
    CLASS_ATTR_STYLE_LABEL(earsprocess_class, "reload", 0, "onoff", "Reload");
    CLASS_ATTR_FILTER_CLIP(earsprocess_class, "reload", 0, 1);
    // @description
    // When set to 1, the patch is reloaded before each run.
    // In this case, initialisation data received through <o>ears.in</o> objects
    // have to be sent again after each reloading,
    // and the time required by the project might increase considerably,
    // especially for complex patches.
    // On the other hand, this guarantees that the patch
    // is always "clean" before running.<br/>
    // When set to 0 (as per the default), the patch is only loaded
    // when <o>ears.process</o> is instantiated
    // or when the <m>patchername</m> message is received.
    */
     
    CLASS_ATTR_LONG(earsprocess_class, "autoclock", 0, t_earsprocess, autoclock);
    CLASS_ATTR_STYLE_LABEL(earsprocess_class, "autoclock", 0, "onoff", "Automatic Clock Message");
    CLASS_ATTR_FILTER_CLIP(earsprocess_class, "autoclock", 0, 1);
    // @description
    // The <o>ears.process~</o> objects manages an internal <o>clock</o>
    // to control scheduler-based objects such as
    // <o>metro</o> and <o>delay</o>, the playback capabilities
    // of <o>bach.roll</o> and <o>bach.score</o>, and more.
    // In this way, their timing will be correct with respect to
    // the non-realtime operation of the loaded patch,
    // rather than the physical time of the outside world.
    // The internal clock is synced to the non-realtime signal vector,
    // as with "Scheduler in Overdrive" and "Scheduler in Audio Interrupt" both on.<br/>
    // If the <o>autoclock</o> attribute is set to 1 (as per the default),
    // all the objects that declare a <o>clock</o> message or attribute, including the ones mentioned above,
    // will automatically have their clock set by <o>ears.process~</o> before each run of the patch.<br/>
    // If <o>autoclock</o> is set to 0, the <o>clock</o> method is not called
    // and it is the user's responsibility to pass the clock, whose name can be obtained from <o>ears.processinfo~</o>, to the relevant objects.<br/>
    // Notice that there might scheduler-based objects not accepting the clock method (such as <o>mtr</o>, <o>thresh</o> and <o>quickthresh</o> as of Max 8.1.1).
    // There is currently no way to use such objects with correct timing inside <o>ears.process~</o>.
    
    emptyLl = llll_get();
    
    class_register(CLASS_BOX, earsprocess_class);

    
    return 0;
}

t_max_err earsprocess_set_vs(t_earsprocess *x, t_object *attr, long argc, t_atom *argv)
{
    if (argc != 1) {
        object_error((t_object *) x, "vs: wrong number of arguments");
        return MAX_ERR_GENERIC;
    }
    if (atom_gettype(argv) != A_LONG) {
        object_error((t_object *) x, "vs: wrong argument type");
        return MAX_ERR_GENERIC;
    }
    long new_vs = atom_getlong(argv);
    long good_vs;
    for (good_vs = 1; good_vs < new_vs && good_vs < 4096; good_vs *= 2)
        ;
    x->vs = good_vs;
    return MAX_ERR_NONE;
}



t_max_err earsprocess_get_ownsdspchain(t_earsprocess *x, t_object *attr, long *argc, t_atom **argv)
{
    char alloc = false;
    
    if (atom_alloc(argc, argv, &alloc))
        return MAX_ERR_GENERIC;
    
    // This prevents getdspchain on child patchers from walking up past this object
    atom_setlong(*argv,1);
    return MAX_ERR_NONE;
}




// args:
// 1. patch name

/*
 Gonna work like this:
 - The patch (if any) gets loaded
 - ears.in~, ears.out~, ears.in and ears.out are all created
 - When ears.in is created, it calls the "ears.in_created" method here communicating its own index and outlet address;
    The maximum ears.in number is kept, and the list of addresses of all the outlets associated with each index is kept.
 - When ears.out is created, it calls the "ears.out_created" method here communicating its own address and number;
    The maximum ears.out number is kept, and the list of addresses of all the objects associated with each index is kept.
 - Inlets and outlets are created.
 - The "setoutlet" method of all the ears.out objects is called, and the corresponding outlets and outlet numbers are passed to them.
 - The initialize member functions of both bufferMaps are called, so as to pass to the ears.in~ and ears.out~ objects the audio buffers they'll use
 
 */


void earsprocess_earsintildecreated(t_earsprocess *x, t_atom_long ioNum, t_object *in)
{
    x->earsInTildeObjects->insert(in);
    x->generator = false;
    if (ioNum > x->nBufInlets)
        x->nBufInlets = ioNum;
}

void earsprocess_earsintildedeleted(t_earsprocess *x, t_object *in)
{
    x->earsInTildeObjects->erase(in);
}

void earsprocess_earsouttildecreated(t_earsprocess *x, t_atom_long ioNum, t_object *out)
{
    x->earsOutTildeObjects->insert(out);
    if (ioNum > x->nBufOutlets)
        x->nBufOutlets = ioNum;
}

void earsprocess_earsouttildedeleted(t_earsprocess *x, t_object *out)
{
    x->earsOutTildeObjects->erase(out);
}

void earsprocess_earsincreated(t_earsprocess *x, t_object *in, t_atom_long n, t_atom_long *index) {
    for (int i = 0; i < n; i++)
        x->theInsByIndex->insert(*(index++), in);
    x->theIns->insert(in);
}

void earsprocess_earsindeleted(t_earsprocess *x, t_object *in, t_atom_long n, t_atom_long *index) {
    for (int i = 0; i < n; i++)
        x->theInsByIndex->remove(*(index++), in);
    x->theIns->erase(in);
}

void earsprocess_earsoutcreated(t_earsprocess *x, long maxindex, t_object *obj) {
    x->theOuts->insert(obj);
    if (x->nDataOutlets < maxindex)
        x->nDataOutlets = maxindex;
}

void earsprocess_earsoutdeleted(t_earsprocess *x, t_object *obj) {
    x->theOuts->erase(obj);
}

void earsprocess_earsprocessinfocreated(t_earsprocess *x, t_object *obj)
{
    x->earsprocessinfoObjects->insert(obj);
}

void earsprocess_earsprocessinfodeleted(t_earsprocess *x, t_object *obj)
{
    x->earsprocessinfoObjects->erase(obj);
}


// 1 arg: patch name
// 2 args: buffer name (llll), patch name: [ foo bar ] patchname
void *earsprocess_new(t_symbol *objname, long argc, t_atom *argv)
{
    // @arg 0 @name patcher name @optional 1 @type symbol
    // @digest Patcher name
    // @description Sets the name of the patch to be loaded   
    // @arg 1 @name outnames @optional 1 @type symbol/llll
    // @digest Output buffer names
    // @description @copy EARS_DOC_OUTNAME_ATTR
    
    t_earsprocess *x = (t_earsprocess *) object_alloc(earsprocess_class);
    t_symbol *patchname = nullptr;
    
    earsbufobj_init((t_earsbufobj*) x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
    
    x->autoclock = 1;
    
    long true_ac = attr_args_offset(argc, argv);
    attr_args_process(x, argc - true_ac, argv + true_ac);
    
    t_llll* args = llll_parse(true_ac, argv);
    
    char intypes[2048];
    char outtypes[2048];
    int i;
    t_bool ok = true;
    
    switch (args->l_size) {
        case 0:
            break;
        case 1: { // it's !_= or the patcher name or an llll with the buffer names
            t_hatom *h = &args->l_head->l_hatom;
            switch (hatom_gettype(h)) {
                case H_SYM: {
                    t_symbol *s = hatom_getsym(h);
                    if (!earsbufobj_is_sym_naming_mech(s)) {
                        // patcher name
                        patchname = s;
                        llll_clear(args);
                    } // if it is !=_ then keep them, otherwise it is the patcher name
                    break;
                }
                case H_LLLL:
                    break; // earsbufobj_extract_names_from_args() will take care of it
                default:
                    ok = false;
                    break;
            }
            break;
        }
        case 2: {
            t_llllelem *el = args->l_head;
            t_hatom *h = &el->l_hatom;
            t_symbol *s = hatom_getsym(h);
            if (s == nullptr) {
                ok = false;
                break;
            }
            if (earsbufobj_is_sym_naming_mech(s)) {
                el = el->l_next;
                h = &el->l_hatom;
                switch(hatom_gettype(h)) {
                    case H_SYM:
                        patchname = hatom_getsym(h);
                        llll_destroyelem(el);
                        break;
                    case H_LLLL:
                        break;
                    default:
                        ok = false;
                        break;
                }
            } else {
                patchname = hatom_getsym(h);
                if (patchname) {
                    llll_destroyelem(el); // what follows is surely the buffer names
                } else {
                    ok = false;
                }
            }
            break;
        }
        case 3: {
            t_llllelem *el = args->l_head->l_next; // the first must be a symbol
            // earsbufobj_extract_names_from_args() will take care of it
            patchname = hatom_getsym(&el->l_hatom);
            if (!patchname) {
                ok = false;
            } else {
                llll_destroyelem(el);
            }
            break;
        }
        default:
            ok = false;
            break;
    }
    
    if (!ok) {
        object_error((t_object *) x, "Bad arguments");
        llll_free(args);
        object_free(x);
        return nullptr;
    }
    
    t_llll *names = earsbufobj_extract_names_from_args((t_earsbufobj *)x, args);
    
    x->theInsByIndex = new earsInsByIndex;
    x->theIns = new std::set<t_object*>;
    x->theOuts = new std::set<t_object*>;
    x->earsInTildeObjects = new objectSet;
    x->earsOutTildeObjects = new objectSet;
    x->earsprocessinfoObjects = new objectSet;

    // Get parent patcher
    
    x->parent_patch = (t_patcher *)gensym("#P")->s_thing;
    
    // Load patch
    
    x->nBufInlets = 0;
    if (patchname)
        earsprocess_patchername(x, patchname, 0, NULL);
    
    // e = buffer; E = buffer list;

 
    int t;
    for (i = x->theInsByIndex->maxIdx - 1, t = 0; i >= 0; i--, t++) {
        intypes[t] = '4';
    }
    for (i = x->nBufInlets - 1; i >= 0; i--, t++) {
        intypes[t] = 'E';
    }
    intypes[t] = 0;
    
    for (i = x->nDataOutlets - 1, t = 0; i >= 0; i--, t++) {
        outtypes[t] = '4';
    }
    for (i = x->nBufOutlets - 1; i >= 0; i--, t++) {
        outtypes[t] = 'E';
    }
    outtypes[t] = 0;

    earsbufobj_setup((t_earsbufobj *) x, intypes, outtypes, names);
    llll_free(args);
    llll_free(names);

    earsprocess_setupOutObjects(x);

    if (x->vs == 0)
        x->vs = 128;
    
    x->clock_name = symbol_unique();
    x->running = true;
    return x;
}

void earsprocess_free(t_earsprocess *x)
{
    earsprocess_wclose(x);
    object_free(x->client_patch);
    
    delete x->theInsByIndex;
    delete x->theIns;
    delete x->theOuts;
    delete x->earsInTildeObjects;
    delete x->earsOutTildeObjects;
    delete x->earsprocessinfoObjects;
    
    earsbufobj_free((t_earsbufobj*) x);
}


int earsprocess_setSubAssoc(t_patcher *p, t_object *x)
{
    t_object *assoc;
    
    object_method(p, gensym("getassoc"), &assoc);
    if (!assoc)
        object_method(p, gensym("setassoc"), x);
    
    return 0;
}

void earsprocess_setupOutObjects(t_earsprocess *x)
{
    for (t_object* o: *x->theOuts) {
        object_method(o, gensym("setoutlets"), x->nDataOutlets, x->dataOutlets);
    }
}

void earsprocess_patchername(t_earsprocess *x, t_symbol *patchname, long ac, t_atom *av)
{
    x->theInsByIndex->clear();
    x->theOuts->clear();
    x->earsInTildeObjects->clear();
    x->earsOutTildeObjects->clear();
    x->earsprocessinfoObjects->clear();

    x->generator = true;
    
    if (ac == 0) {
        ac = x->client_argc;
        av = x->client_argv;
    }
    
    t_fourcc validTypes[3];
    
    // Set the valid types to test for
    
    validTypes[0] = FOUR_CHAR_CODE('maxb');
    validTypes[1] = FOUR_CHAR_CODE('TEXT');
    validTypes[2] = FOUR_CHAR_CODE('JSON');
    
    char name[2048];
    strncpy_zero(name, patchname->s_name, 2048);
    
    short outvol;
    t_fourcc outtype;
    
    if (locatefile_extended(name, &outvol, &outtype, validTypes, 3)) {
        object_error((t_object *) x, "Can't find %s", name);
        return;
    }
    
    if (x->client_patch) {
        object_free(x->client_patch);
    }
    
    t_symbol *ps_earsprocess = gensym(EARS_PROCESS_SPECIALSYM);
    t_symbol *ps_inhibit_subpatcher_vis = gensym("inhibit_subpatcher_vis");
    t_symbol *ps_PAT = gensym("#P");
    
    // Store the old loading symbols
    
    t_object *previous = ps_earsprocess->s_thing;
    t_object *save_inhibit_state = ps_inhibit_subpatcher_vis->s_thing;
    t_patcher *saveparent = (t_patcher *)ps_PAT->s_thing;
    
    // Bind to the loading symbols
    
    ps_earsprocess->s_thing = (t_object *) x;
    ps_inhibit_subpatcher_vis->s_thing = (t_object *) -1;
    ps_PAT->s_thing = x->parent_patch;
    
    // Load the patch (don't interrupt dsp and use setclass to allow Modify Read-Only)
    
    short savedLoadUpdate = dsp_setloadupdate(false);
    loadbang_suspend();
    x->client_patch = (t_patcher *)intload(name, outvol, 0, ac, av, false);
    //object_method(x->client_patch, gensym("setclass"));
    
    // Restore previous loading symbol bindings
    
    ps_earsprocess->s_thing = previous;
    ps_inhibit_subpatcher_vis->s_thing = save_inhibit_state;
    ps_PAT->s_thing = (t_object *) saveparent;

    if (!x->client_patch) {
        object_error((t_object *) x, "Couldn't load %s", name);
        loadbang_resume();
        dsp_setloadupdate(savedLoadUpdate);
        return;
    }
    
    x->patch_name = patchname;
    
    if (!ispatcher((t_object *)x->client_patch)) {
        object_error((t_object *) x, "%s is not a patch", name);
        object_free(x->client_patch); // good idea??
        loadbang_resume();
        dsp_setloadupdate(savedLoadUpdate);
        return;
    }
    
    // connect ears.in~ / .out~ / .in / .out here?
    
    long result = 0;
    
    // this apparently associates the patcher to the objects it contains
    // what is the difference wrt the "iterate" method?
    object_method(x->client_patch, _sym_traverse, earsprocess_setSubAssoc, x, &result);
    
    loadbang_resume();
    dsp_setloadupdate(savedLoadUpdate);
    
    if (x->running) {
        earsprocess_setupOutObjects(x);
    }
    
    return;
}



void earsprocess_open(t_earsprocess *x)
{
    if (!x->client_patch)
        return;
    object_method((t_object *) x->client_patch, _sym_vis);
}

void earsprocess_wclose(t_earsprocess *x)
{
    if (!x->client_patch)
        return;
    object_method((t_object *) x->client_patch, _sym_wclose);
}


// never called apparently
void earsprocess_pupdate(t_earsprocess *x, void *b, t_patcher *p)
{
    //
}

void *earsprocess_subpatcher(t_earsprocess *x, long index, void *arg)
{
    // Report subpatchers if requested by an object that is not a dspchain
    
    if ((t_ptr_uint) arg > 1)
        if (!NOGOOD(arg))
            if (ob_sym(arg) == gensym("dspchain"))
                return nullptr;
    
    if (index == 0)
        return (void*) x->client_patch;
    else
        return nullptr;
}

void earsprocess_parentpatcher(t_earsprocess *x, t_patcher **parent)
{
    *parent = x->parent_patch;
}

void earsprocess_int(t_earsprocess *x, t_atom_long i)
{
    t_atom a[1];
    atom_setlong(a, i);
    long inlet = proxy_getinlet((t_object *) x);
    if (inlet >= x->nBufInlets && inlet < x->theInsByIndex->maxIdx) {
        earsprocess_anything(x, _sym_int, 1, a);
    } else {
        object_error((t_object *) x, "Doesn't understand int in inlet %ld", inlet + 1);
    }
}

void earsprocess_float(t_earsprocess *x, t_atom_float f)
{
    t_atom a[1];
    atom_setfloat(a, f);
    long inlet = proxy_getinlet((t_object *) x);
    if (inlet >= x->nBufInlets && inlet < x->theInsByIndex->maxIdx + x->nBufInlets) {
        earsprocess_anything(x, _sym_float, 1, a);
    } else {
        object_error((t_object *) x, "Doesn't understand float in inlet %ld", inlet + 1);
    }
}

void earsprocess_list(t_earsprocess *x, t_symbol *s, t_atom_long ac, t_atom* av)
{
    long inlet = proxy_getinlet((t_object *) x);
    if (inlet >= x->nBufInlets && inlet < x->theInsByIndex->maxIdx + x->nBufInlets) {
        earsprocess_anything(x, s, ac, av);
    } else {
        object_error((t_object *) x, "Doesn't understand list in inlet %ld", inlet + 1);
    }
}

void earsprocess_anything(t_earsprocess *x, t_symbol *s, t_atom_long ac, t_atom* av)
{
    long inlet = proxy_getinlet((t_object *) x);
    if (inlet >= x->nBufInlets && inlet < x->theInsByIndex->maxIdx + x->nBufInlets) {
        t_llll *ll = llllobj_parse_retain_and_store((t_object *) x, LLLL_OBJ_VANILLA, s, ac, av, inlet);
        //t_llll *ll = llllobj_parse_llll((t_object *) x, LLLL_OBJ_VANILLA, s, ac, av, LLLL_PARSE_RETAIN);
        if (!ll)
            return;
        for (t_object* in: *x->theInsByIndex->theMap[inlet - x->nBufInlets + 1]) {
            object_method(in, gensym("llll"), ll, inlet - x->nBufInlets + 1);
        }
        if (inlet == 0)
            earsprocess_bang(x);
        llll_release(ll);
        return;
    }
    
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, s, ac, av);
    
    if (!parsed)
        return;
    
    if (parsed->l_head) {
        long num_bufs = llll_get_num_symbols_root(parsed);
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, num_bufs, true);
        earsbufobj_store_buffer_list((t_earsbufobj *)x, parsed, inlet);
    }
    
    if (inlet == 0)
        earsprocess_bang(x);
    
    llll_free(parsed);
}

void earsprocess_bang(t_earsprocess *x)
{
    if (!x->client_patch)
        return;
    defer(x, (method) earsprocess_bang_do, NULL, 0, NULL);
}

void earsprocess_bang_do(t_earsprocess *x, t_symbol *s, t_atom_long ac, t_atom *av)
{
    double sr;
    
    if (x->reload)
        earsprocess_patchername((t_earsprocess*) x, x->patch_name, x->client_argc, x->client_argv);
    
    // Get number of buffers on which to iterate
    long nIterations = LONG_MAX;
    long i;
    for (i = 0; i < x->nBufInlets; i++) {
        long this_num_buf = earsbufobj_get_num_inlet_stored_buffers((t_earsbufobj *)x, i, false);
        if (x->scalarmode && this_num_buf == 1) {
            // nothing to do
        } else {
            nIterations = MIN(nIterations, this_num_buf);
        }
    }
    for (; i < x->nBufInlets + x->theInsByIndex->maxIdx; i++) {
        t_llll *ll = llllobj_get_store_contents((t_object *) x, LLLL_OBJ_VANILLA, i, 0);
        long this_size = ll->l_size;
        llll_release(ll);
        if (x->scalarmode && this_size == 1) {
            // nothing to do
        } else {
            nIterations = MIN(nIterations, this_size);
        }
    }
    if (nIterations == LONG_MAX ||
        (nIterations == 0 && x->generator))
        nIterations = 1;

    earsbufobj_refresh_outlet_names((t_earsbufobj *)x);

    for (long i = 0; i < x->nBufOutlets; i++)
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, i, nIterations, true);
    
    for (int iteration = 0; iteration < nIterations; iteration++) {
        bufferData bufs[EARS_PROCESS_MAX_INPUT_BUFFERS];
        t_atom bufDurs[EARS_PROCESS_MAX_INPUT_BUFFERS];

        t_atom_long maxDur = 0;
        t_atom_long minDur = 0;
        
        sr = 0;
        
        if (!x->generator) {
            for (long i = 0; i < x->nBufInlets; i++) {
                t_atom_long bufNum = (x->scalarmode &&
                    earsbufobj_get_num_inlet_stored_buffers((t_earsbufobj *)x, i, false) == 1) ?
                    0 : iteration;
                t_symbol *name = earsbufobj_get_inlet_buffer_name((t_earsbufobj*) x, i, bufNum);
                if (name) {
                    bufs[i].set((t_object*) x, name);
                    t_atom_long fr = bufs[i].frames;
                    if (fr > maxDur)
                        maxDur = fr;
                    if (minDur == 0 || fr < minDur)
                        minDur = fr;
                    sr = bufs[i].sr;
                    atom_setfloat(bufDurs + i, fr / sr * 1000.);
                }
            }
        }
        
        if (sr == 0) {
            sr = x->sr ? x->sr : sys_getsr();
            atom_setfloat(bufDurs, 0.);
        }
        
        double mssr = sr / 1000.;
        t_atom_long duration = x->tail * mssr;
        
        switch (x->policy) {
            case eDURPOLICY_SHORTEST: duration += minDur; break;
            case eDURPOLICY_LONGEST: duration += maxDur; break;
            default: break;
        }
        
        if (duration < 0)
            duration = 0;
                
        x->stopped = false;
        
        t_atom scarg;
        atom_setsym(&scarg, x->clock_name);
        t_object *setclock = (t_object *) newinstance(gensym("setclock"), 1, &scarg);

        for (t_object* i: *x->theIns) {
            object_method(i, gensym("iteration"), iteration + 1);
        }
        
        if (iteration == 0) {
            earsprocess_autoclock(x, x->client_patch);
            for (t_object* o : *x->earsprocessinfoObjects) {
                object_method(o, gensym("prepare"), &scarg);
            }
        }
        
        for (t_object* o : *x->earsprocessinfoObjects) {
            object_method(o, gensym("start"), x->nBufInlets, bufDurs);
        }
        
        const t_atom_long vs = x->vs;

        audioChanMap chanMap;
        
        for (t_object* o : *x->earsInTildeObjects) {
            object_method(o, gensym("setbuffers"), bufs);
        }
        
        for (t_object* o : *x->earsOutTildeObjects) {
            object_method(o, gensym("setchanmap"), &chanMap);
        }
        
        t_dspchain* chain = dspchain_compile(x->client_patch, vs, sr);
        
        // check and compile again,
        // because the compilation won't work for gen~ the first time
        if (chain && chain->c_broken)
        {
            object_free(chain);
            chain = dspchain_compile(x->client_patch, vs, sr);
        }
        

        if (chain) {
            
            t_atom_long s;
            for (s = 0; s < duration && !x->stopped; s += vs) {
                object_method_double(setclock, _sym_float, s / mssr, NULL);
                for (t_object* o : *x->earsprocessinfoObjects) {
                    object_method(o, gensym("output_position"));
                }
                dspchain_tick(chain);
            }
            
            for (t_object* o : *x->earsprocessinfoObjects) {
                object_method(o, gensym("output_position"));
            }
            
            if (x->stopped)
                duration = s;
            
            object_free((t_object *) chain);
            
            for (int i = 0; i < x->nBufInlets; i++)
                bufs[i].unlock();
            
            bufferData outBuf[EARS_PROCESS_MAX_OUTPUT_BUFFERS];
            
            for (int i = 0; i < x->nBufOutlets; i++) {            
                t_buffer_obj *buf = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, i, iteration);
                ears_buffer_set_sr((t_object *)x, buf, sr);
                int nChans = chanMap.chansPerBuf[i];
                ears_buffer_set_size_and_numchannels((t_object *)x, buf, duration, MAX(nChans, 1));

                outBuf[i].set((t_object *) x, earsbufobj_get_outlet_buffer_name((t_earsbufobj *)x, i, iteration));
                for (int c = 1; c <= outBuf[i].chans; c++) {
                    audioChannel *ch = chanMap.retrieveChannel(i + 1, c);
                    if (ch) {
                        ch->copyToBuffer(outBuf + i, duration);
                    }
                }
            }
        }
        

        for (const auto &i : *x->earsprocessinfoObjects) {
            object_method(i, gensym("end"));
        }
        
        for (t_object* o: *x->theOuts) {
            object_method(o, gensym("iteration"), iteration + 1);
        }
        
        object_free(setclock);
    }
    
    for (int i = x->nDataOutlets - 1; i >= 0; i--) {
        llll_retain(emptyLl);
        llllobj_gunload_llll((t_object *) x, LLLL_OBJ_VANILLA, emptyLl, i);
    }
    
    for (t_object* o: *x->theOuts) {
        object_method(o, gensym("finalize"), x->nBufOutlets);
    }
    
    for (int i = x->nDataOutlets + x->nBufOutlets - 1; i >= x->nBufOutlets; i--) {
        if (llllobj_get_loaded_llll((t_object *) x, LLLL_OBJ_VANILLA, i) != emptyLl) {
            llllobj_shoot_llll((t_object *) x, LLLL_OBJ_VANILLA, i);
        }
    }
    
    for (int i = x->nBufOutlets - 1; i >= 0 ; i--)
        earsbufobj_outlet_buffer((t_earsbufobj *)x, i);
}

void earsprocess_stop(t_earsprocess *x)
{
    if (!x->client_patch)
        return;
    
    long inlet = proxy_getinlet((t_object *) x);
    if (inlet >= x->nBufInlets) {
        earsprocess_anything(x, _sym_bang, 0, NULL);
    } else {
        x->stopped = true;
    }
}

void earsprocess_autoclock(t_earsprocess *x, t_patcher *p)
{
    t_symbol *name = x->clock_name;
    for (t_box *b = jpatcher_get_firstobject(p); b; b = jbox_get_nextobject(b))
    {
        t_object *o = jbox_get_object(b);
        if (object_classname(o) == gensym("ears.process~"))
            continue;
        method c = zgetfn(o, _sym_clock);
        if (c) {
            CALL_METHOD_SAFE(void, (t_object*, t_symbol*), c, o, name);
            // was:            (c)(o, name);
        } else
            object_attr_setsym(o, _sym_clock, name);
        
        
        t_patcher *subpatch;
        long index = 0;
        
        while (b && (subpatch = (t_patcher *)object_subpatcher(o, &index, x))) {
            earsprocess_autoclock(x, subpatch);
        }
    }
}


void earsprocess_assist(t_earsprocess *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a < x->nBufInlets) {
            sprintf(s, "symbol/list/llll: Incoming Buffer Names for Inlet %ld", a + 1); // @in 0 @loop 1 @type symbol/list/llll @digest Incoming buffer name
        } else {
            sprintf(s, "llll: Incoming Data for ears.in %ld", a - x->nBufInlets + 1); // @in 1 @loop 1 @type llll @digest Incoming data for <o>ears.in</o>.
        }
    } else {
        if (a < x->nBufOutlets) {
            sprintf(s, "symbol/list/llll: Output Buffer Names for Outlet %ld", a + 1); // @out 0 @loop 1 @type symbol/llll @digest Output buffer
        } else {
            char *type;
            llllobj_get_llll_outlet_type_as_string((t_object *) x, LLLL_OBJ_VANILLA, a, &type);
            sprintf(s, "llll (%s): Output Data for ears.out %ld", type, a - x->nBufOutlets + 1); // @out 1 @loop 1 @type llll @digest Output data from <o>ears.out</o>.
        }
    }
}


void earsprocess_inletinfo(t_earsprocess *x, void *b, long a, char *t)
{
    if (a != 0)
        *t = 1;
}




