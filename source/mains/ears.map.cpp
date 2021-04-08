//
//  ears.cpp
//  ears
//
//  Created by andreaagostini on 03/04/2021.
//




#include <stdio.h>

#include <ext.h>
#include <ext_obex.h>
#include <z_dsp.h> // ???
#include <ext_wind.h>
#include <ext_buffer.h>
#include <jpatcher_api.h>

#include <unordered_set>
#include <map>
#include <vector>

#include "ears.map.h"



// Fixes a compiler error with object_method_direct when building the code as cpp
// Shouldn't be necessary with recent version of the Max SDK, but it won't hurt either
#define object_method_direct_cpp(rt, sig, x, s, ...) ((rt (*)sig)object_method_direct_getmethod((t_object *)x, s))((t_object*)object_method_direct_getobject((t_object *)x, s), __VA_ARGS__)

t_class *earsmap_class;

template <typename T>
class earsmapMultiMap
{
public:
    earsmapMultiMap() : maxIdx(0) { }
    
    std::map<t_atom_long, std::unordered_set<T>> theMap;
    
    int maxIdx;
    
    void insert(t_atom_long idx, T what) {
        if (theMap.find(idx) == theMap.end()) {
            theMap.insert(std::pair<t_atom_long, std::unordered_set<T>>(idx, std::unordered_set<T>()));
        }
        theMap[idx].insert(what);
        if (maxIdx < idx)
            maxIdx = idx;
    }
    
    void remove(t_atom_long idx, T what) {
        theMap[idx].erase(what);
        if (theMap[idx].size() == 0) {
            theMap.erase(idx);
        }
    }
    
    void clear() {
        theMap.clear();
    }
};

typedef earsmapMultiMap<void*> earsInOutlets;

class earsOuts : public earsmapMultiMap<t_object *>
{
public:
    using earsmapMultiMap::earsmapMultiMap;
    
    void setOutlets(long nOutlets, void** outlets) {
        for (auto i: theMap) {
            t_atom_long index = i.first;
            auto oSet = i.second;
            void *out = index <= nOutlets ? outlets[index] : nullptr;
            for (t_object* o: oSet)
                object_method(o, gensym("setoutlet"), index, out);
        }
    }
};

typedef std::unordered_set<t_object*> objectSet;

typedef struct _earsmap
{
    t_object x_obj;
    
    t_symbol* inbuf[EARSMAP_MAX_INPUT_BUFFERS];
    
    t_patcher *parent_patch;

    t_bool running;
    
    void *m_proxy[EARSMAP_MAX_INPUT_BUFFERS + EARSMAP_MAX_DATA_INLETS + 1];
    long m_in;
    void *m_outlets[EARSMAP_MAX_OUTPUT_BUFFERS + EARSMAP_MAX_DATA_OUTLETS + 1];
    
    long nrt_vs;
    t_symbol *clock_name;
    
    t_bool stopped;
    
    t_atom_long nBufInlets;
    t_atom_long nBufOutlets;

    
    earsInOutlets *theInOutlets;
    earsOuts *theOuts;

    t_patcher *client_patch;
    long client_argc;
    t_atom client_argv[256];
    
    objectSet* earsInTildeObjects;
    objectSet* earsOutTildeObjects;
    objectSet* earsMapinfoObjects;
    
} t_earsmap;



void *earsmap_new(t_symbol *s, long argc, t_atom *argv);
void earsmap_free(t_earsmap *x);
void earsmap_assist(t_earsmap *x, void *b, long m, long a, char *s);


void earsmap_deletepatch(t_earsmap *x, t_symbol *msg, long argc, t_atom *argv);
void earsmap_clear(t_earsmap *x);
void earsmap_loadpatch(t_earsmap *x, t_symbol *s, long argc, t_atom *argv);


void earsmap_bang(t_earsmap *x);
void earsmap_stop(t_earsmap *x);
void earsmap_int(t_earsmap *x, t_atom_long n);
void earsmap_float(t_earsmap *x, t_atom_float f);
void earsmap_anything(t_earsmap *x, t_symbol *s, t_atom_long argc, t_atom *argv);

void earsmap_addclient(t_earsmap *x, t_object *client);
void earsmap_removeclient(t_earsmap *x, t_object *client);


void earsmap_registerclientoutlet(t_earsmap *x, t_atom_long inlet_num, void* inobj);
void* earsmap_passoutlet(t_earsmap *x, t_atom_long outlet_num);




void earsmap_dsp64(t_earsmap *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);

void earsmap_dblclick(t_earsmap *x);
void earsmap_open(t_earsmap *x);
void earsmap_pclose(t_earsmap *x);
void earsmap_wclose(t_earsmap *x);

void earsmap_pupdate(t_earsmap *x, void *b, t_patcher *p);
void *earsmap_subpatcher(t_earsmap *x, long index, void *arg);
void earsmap_parentpatcher(t_earsmap *x, t_patcher **parent);

void earsmap_earsintildecreated(t_earsmap *x, t_atom_long index, t_object *in);
void earsmap_earsintildedeleted(t_earsmap *x, t_object *in);
void earsmap_earsouttildecreated(t_earsmap *x, t_atom_long index, t_object *out);
void earsmap_earsouttildedeleted(t_earsmap *x, t_object *out);

void earsmap_earsincreated(t_earsmap *x, t_atom_long index, void *outlet);
void earsmap_earsoutcreated(t_earsmap *x, t_atom_long index, t_object *obj);
void earsmap_earsindeleted(t_earsmap *x, t_atom_long index, void *outlet);
void earsmap_earsoutdeleted(t_earsmap *x, t_atom_long index, t_object *obj);

void earsmap_earsmapinfocreated(t_earsmap *x, t_object *obj);
void earsmap_earsmapinfodeleted(t_earsmap *x, t_object *obj);




t_max_err earsmap_set_vs(t_earsmap *x, t_object *attr, long argc, t_atom *argv);
t_max_err earsmap_get_ownsdspchain(t_earsmap *x, t_object *attr, long *argc, t_atom **argv);




int C74_EXPORT main()
{
    earsmap_class = class_new("ears.map~", (method)earsmap_new, (method)earsmap_free, sizeof(t_earsmap), NULL, A_GIMME, 0);
    
    //class_addmethod(earsmap_class, (method)earsmap_dsp64, "dsp64", A_CANT, 0);
    
    //class_addmethod(earsmap_class, (method)earsmap_assist, "assist", A_CANT, 0);
    class_addmethod(earsmap_class, (method)earsmap_open, "open", 0);
    class_addmethod(earsmap_class, (method)earsmap_open, "dblclick", A_CANT, 0);
    class_addmethod(earsmap_class, (method)earsmap_wclose, "wclose", 0);
    
    class_addmethod(earsmap_class, (method)earsmap_pupdate, "pupdate", A_CANT, 0);
    class_addmethod(earsmap_class, (method)earsmap_subpatcher, "subpatcher", A_CANT, 0);
    class_addmethod(earsmap_class, (method)earsmap_parentpatcher, "parentpatcher", A_CANT, 0);
    
    
    class_addmethod(earsmap_class, (method)earsmap_anything, "anything", A_GIMME, 0);

    class_addmethod(earsmap_class, (method)earsmap_bang, "bang", 0);
    
    //class_addmethod(earsmap_class, (method)earsmap_stop, "stop", A_CANT, 0);
    //class_addmethod(earsmap_class, (method)earsmap_int, "int", A_LONG, 0);
    //class_addmethod(earsmap_class, (method)earsmap_float, "float", A_FLOAT, 0);
    //class_addmethod(earsmap_class, (method)earsmap_anything, "list", A_GIMME, 0);
    //class_addmethod(earsmap_class, (method)earsmap_anything, "anything", A_GIMME, 0);
    
    //class_addmethod(earsmap_class, (method)earsmap_clear, "clear", 0);
    class_addmethod(earsmap_class, (method)earsmap_loadpatch, "loadpatch", A_DEFER, 0);
    
    class_addmethod(earsmap_class, (method)earsmap_earsintildecreated, "ears.in~_created", A_CANT, 0);
    class_addmethod(earsmap_class, (method)earsmap_earsouttildecreated, "ears.out~_created", A_CANT, 0);
    class_addmethod(earsmap_class, (method)earsmap_earsintildedeleted, "ears.in~_deleted", A_CANT, 0);
    class_addmethod(earsmap_class, (method)earsmap_earsouttildedeleted, "ears.out~_deleted", A_CANT, 0);

    class_addmethod(earsmap_class, (method)earsmap_earsincreated, "ears.in_created", A_CANT, 0);
    class_addmethod(earsmap_class, (method)earsmap_earsoutcreated, "ears.out_created", A_CANT, 0);
    class_addmethod(earsmap_class, (method)earsmap_earsindeleted, "ears.in_deleted", A_CANT, 0);
    class_addmethod(earsmap_class, (method)earsmap_earsoutdeleted, "ears.out_deleted", A_CANT, 0);
    
    class_addmethod(earsmap_class, (method)earsmap_earsmapinfocreated, "ears.mapinfo~_created", A_CANT, 0);
    class_addmethod(earsmap_class, (method)earsmap_earsmapinfodeleted, "ears.mapinfo~_deleted", A_CANT, 0);

    CLASS_ATTR_OBJ(earsmap_class, "ownsdspchain", ATTR_SET_OPAQUE | ATTR_SET_OPAQUE_USER, t_earsmap, x_obj);
    CLASS_ATTR_ACCESSORS(earsmap_class, "ownsdspchain", (method) earsmap_get_ownsdspchain, NULL);
    CLASS_ATTR_INVISIBLE(earsmap_class, "ownsdspchain", 0);
    
    CLASS_ATTR_LONG(earsmap_class, "vs", 0, t_earsmap, nrt_vs);
    CLASS_ATTR_ACCESSORS(earsmap_class, "vs", NULL, (method) earsmap_set_vs);
    
    
    CLASS_ATTR_ATOM_VARSIZE(earsmap_class, "args", 0, t_earsmap, client_argv, client_argc, 256);
    
    //class_dspinit(earsmap_class);
    
    class_register(CLASS_BOX, earsmap_class);
    
    return 0;
}



t_max_err earsmap_set_vs(t_earsmap *x, t_object *attr, long argc, t_atom *argv)
{
    if (argc != 1) {
        object_error((t_object *) x, "Wrong number of arguments");
        return MAX_ERR_GENERIC;
    }
    if (atom_gettype(argv) != A_LONG) {
        object_error((t_object *) x, "Wrong argument type");
        return MAX_ERR_GENERIC;
    }
    long new_vs = atom_getlong(argv);
    long good_vs;
    for (good_vs = 1; good_vs < new_vs && good_vs < 4096; good_vs *= 2)
        ;
    x->nrt_vs = good_vs;
    return MAX_ERR_NONE;
}



t_max_err earsmap_get_ownsdspchain(t_earsmap *x, t_object *attr, long *argc, t_atom **argv)
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


void earsmap_earsintildecreated(t_earsmap *x, t_atom_long bufIndex, t_object *in)
{
    x->earsInTildeObjects->insert(in);
    if (bufIndex > x->nBufInlets)
        x->nBufInlets = bufIndex;
}

void earsmap_earsintildedeleted(t_earsmap *x, t_object *in)
{
    x->earsInTildeObjects->erase(in);
}

void earsmap_earsouttildecreated(t_earsmap *x, t_atom_long bufIndex, t_object *out)
{
    x->earsOutTildeObjects->insert(out);
    if (bufIndex > x->nBufOutlets)
        x->nBufOutlets = bufIndex;
}

void earsmap_earsouttildedeleted(t_earsmap *x, t_object *out)
{
    x->earsOutTildeObjects->erase(out);
}

void earsmap_earsincreated(t_earsmap *x, t_atom_long index, void *outlet) {
    x->theInOutlets->insert(index, outlet);
}

void earsmap_earsoutcreated(t_earsmap *x, t_atom_long index, t_object *obj) {
    x->theOuts->insert(index, obj);
}

void earsmap_earsindeleted(t_earsmap *x, t_atom_long index, void *outlet) {
    x->theInOutlets->remove(index, outlet);
}

void earsmap_earsoutdeleted(t_earsmap *x, t_atom_long index, t_object *obj) {
    x->theOuts->remove(index, obj);
}

void earsmap_earsmapinfocreated(t_earsmap *x, t_object *obj)
{
    x->earsMapinfoObjects->insert(obj);
}

void earsmap_earsmapinfodeleted(t_earsmap *x, t_object *obj)
{
    x->earsMapinfoObjects->erase(obj);
}

void earsmap_earsmapinfodeleted(t_earsmap *x, t_object *obj);

void *earsmap_new(t_symbol *s, long argc, t_atom *argv)
{
    t_earsmap *x = (t_earsmap *) object_alloc(earsmap_class);
    t_symbol *patchname = nullptr;
    
    long true_ac = attr_args_offset(argc, argv);
    attr_args_process(x, argc - true_ac, argv + true_ac);
    
    if (argc && atom_gettype(argv) == A_SYM)
        patchname = atom_getsym(argv);
    
    //dsp_setup((t_pxobject *) x, 0); // necessary?
    
    x->theInOutlets = new earsInOutlets;
    x->theOuts = new earsOuts;
    x->earsInTildeObjects = new objectSet;
    x->earsOutTildeObjects = new objectSet;
    x->earsMapinfoObjects = new objectSet;

    // Get parent patcher
    
    x->parent_patch = (t_patcher *)gensym("#P")->s_thing;
    
    // Load patch
    
    if (patchname)
        earsmap_loadpatch(x, patchname, 0, NULL);
    
    // first inlet is always given (we'll get the buffer name from there)
    for (t_atom_long i = x->nBufInlets + x->theInOutlets->maxIdx - 1; i > 0; i--) {
        x->m_proxy[i] = proxy_new((t_object *) x, i, &x->m_in);
    }
    
    // there might as well be no outlets
    for (t_atom_long i = x->nBufOutlets + x->theOuts->maxIdx; i > 0; i--) {
        x->m_outlets[i] = outlet_new((t_object *) x, NULL);
    }

    if (x->nrt_vs == 0)
        x->nrt_vs = 128;
    
    x->clock_name = symbol_unique();
    x->running = true;
    return x;
}

void earsmap_free(t_earsmap *x)
{
    earsmap_wclose(x);
    delete x->theInOutlets;
    delete x->theOuts;
    delete x->earsInTildeObjects;
    delete x->earsOutTildeObjects;
    delete x->earsMapinfoObjects;
    
    object_free(x->client_patch);
}


int earsmap_setSubAssoc(t_patcher *p, t_object *x)
{
    t_object *assoc;
    
    object_method(p, gensym("getassoc"), &assoc);
    if (!assoc)
        object_method(p, gensym("setassoc"), x);
    
    return 0;
}

void earsmap_loadpatch(t_earsmap *x, t_symbol *patchname, long ac, t_atom *av)
{
    
    x->theInOutlets->clear();
    x->theOuts->clear();
    x->earsInTildeObjects->clear();
    x->earsOutTildeObjects->clear();
    x->earsMapinfoObjects->clear();

    
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
    
    t_symbol *ps_earsmap = gensym(EARSMAP_SPECIALSYM);
    t_symbol *ps_inhibit_subpatcher_vis = gensym("inhibit_subpatcher_vis");
    t_symbol *ps_PAT = gensym("#P");
    
    // Store the old loading symbols
    
    t_object *previous = ps_earsmap->s_thing;
    t_object *save_inhibit_state = ps_inhibit_subpatcher_vis->s_thing;
    t_patcher *saveparent = (t_patcher *)ps_PAT->s_thing;
    
    // Bind to the loading symbols
    
    ps_earsmap->s_thing = (t_object *) x;
    ps_inhibit_subpatcher_vis->s_thing = (t_object *) -1;
    ps_PAT->s_thing = x->parent_patch;
    
    // Load the patch (don't interrupt dsp and use setclass to allow Modify Read-Only)
    
    short savedLoadUpdate = dsp_setloadupdate(false);
    loadbang_suspend();
    x->client_patch = (t_patcher *)intload(name, outvol, 0, ac, av, false);
    object_method(x->client_patch, gensym("setclass"));
    
    // Restore previous loading symbol bindings
    
    ps_earsmap->s_thing = previous;
    ps_inhibit_subpatcher_vis->s_thing = save_inhibit_state;
    ps_PAT->s_thing = (t_object *) saveparent;

    if (!x->client_patch) {
        object_error((t_object *) x, "Couldn't load %s", name);
        loadbang_resume();
        dsp_setloadupdate(savedLoadUpdate);
        return;
    }
    
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
    object_method(x->client_patch, gensym("traverse"), earsmap_setSubAssoc, x, &result);
    
    loadbang_resume();
    dsp_setloadupdate(savedLoadUpdate);
    
    return;
}

void earsmap_open(t_earsmap *x)
{
    if (x->client_patch)
        object_method((t_object *) x->client_patch, gensym("front"));
}

void earsmap_wclose(t_earsmap *x)
{
    if (x->client_patch)
        object_method((t_object *) x->client_patch, gensym("wclose"));
}


// check: does Max call this when the patch gets updated?
void earsmap_pupdate(t_earsmap *x, void *b, t_patcher *p)
{
    object_post((t_object *) x, "pupdate");
}

void *earsmap_subpatcher(t_earsmap *x, long index, void *arg)
{
    // Report subpatchers if requested by an object that is not a dspchain
    object_post((t_object *) x, "subpatcher %ld %p", index, arg);
    
    if ((t_ptr_uint) arg > 1)
        if (!NOGOOD(arg))
            if (ob_sym(arg) == gensym("dspchain"))
                return nullptr;
    
    if (index == 0)
        return (void*) x->client_patch;
    else
        return nullptr;
}

void earsmap_parentpatcher(t_earsmap *x, t_patcher **parent)
{
    *parent = x->parent_patch;
}

void earsmap_anything(t_earsmap *x, t_symbol *s, t_atom_long ac, t_atom* av)
{
    long inlet = proxy_getinlet((t_object *) x);
    
    if (inlet >= x->nBufInlets) {
        // send to ears.in
        return;
    }
    
    if (ac != 0) {
        object_error((t_object*) x, "bad buffer name");
        return;
    }
    
    x->inbuf[inlet] = s;
    
    if (inlet == 0)
        earsmap_bang(x);
}


void earsmap_bang(t_earsmap *x)
{
    
    if (!x->client_patch)
        return;
    
    long inlet = proxy_getinlet((t_object *) x);
    
    if (inlet > 0) {
        earsmap_anything(x, _sym_bang, 0, NULL);
        return;
    }
    
    bufferData bufs[EARSMAP_MAX_INPUT_BUFFERS];
    double sr;
    int i;
    for (i = 0; i < x->nBufInlets; i++) {
        bufs[i].set((t_object*) x, x->inbuf[i]);
    }
    
    if (i > 0) {
        sr = buffer_getsamplerate(bufs[0].obj); // assuming that the sample rate is the same for all buffers
    } else {
        sr = 44100.; // to be refined
    }
    
    const t_atom_long vs = x->nrt_vs;
    
    
    t_atom scarg;
    atom_setsym(&scarg, x->clock_name);
    
    t_object *setclock = (t_object *) newinstance(gensym("setclock"), 1, &scarg);
    //t_object *prnt = (t_object *) newinstance(gensym("print"), 0, NULL);
    
    x->stopped = false;
    
    for (t_object* o : *x->earsMapinfoObjects) {
        object_method_direct_cpp(void, (t_object*, double, double, t_symbol*), o, gensym("start"), 0, 0, x->clock_name);
    }
    
    t_dspchain* chain = dspchain_compile(x->client_patch, vs, sr);
    
    if (chain) {

        audioChanMap chanMap;
        
        for (t_object* o : *x->earsInTildeObjects) {
            object_method(o, gensym("setbuffers"), bufs);
        }
        
        for (t_object* o : *x->earsOutTildeObjects) {
            object_method(o, gensym("setchanmap"), &chanMap);
        }
        
        t_atom_long duration = 44100;
        
        for (t_atom_long s = 0; s < duration && !x->stopped; s += vs) {
            dspchain_tick(chain);
        }
        
        object_free((t_object *) chain);
        
        
        bufferData outBuf((t_object *) x, gensym("outbuf"));
        
        // how do I retrieve / create the buffers???

        for (int c = 1; c <= outBuf.chans; c++) {
            audioChannel *ch = chanMap.retrieveChannel(1, c);
            if (ch)
                ch->copyToBuffer(&outBuf, 44100);
        }
    }
    
    for (const auto &i : *x->earsMapinfoObjects) {
        object_method(i, gensym("end"));
    }
    
    outlet_anything(x->m_outlets[0], gensym("outbuf"), 0, NULL);


    object_free(setclock);
}













