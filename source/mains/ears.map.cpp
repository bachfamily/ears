//
//  ears.cpp
//  ears
//
//  Created by andreaagostini on 03/04/2021.
//

const int EARSMAP_MAX_INPUT_BUFFERS = 16;
const int EARSMAP_MAX_OUTPUT_BUFFERS = EARSMAP_MAX_INPUT_BUFFERS;
const int EARSMAP_MAX_VS = 4096;
const int EARSMAP_MAX_DATA_INLETS = 16;
const int EARSMAP_MAX_DATA_OUTLETS = EARSMAP_MAX_DATA_INLETS;


#include <stdio.h>

#include <ext.h>
#include <ext_obex.h>
#include <z_dsp.h>
#include <ext_wind.h>
#include <ext_buffer.h>
#include <jpatcher_api.h>

#include <unordered_set>
#include <map>
#include <vector>


// Fixes a compiler error with object_method_direct when building the code as cpp
// Shouldn't be necessary with recent version of the Max SDK, but it won't hurt either
#define object_method_direct_cpp(rt, sig, x, s, ...) ((rt (*)sig)object_method_direct_getmethod((t_object *)x, s))((t_object*)object_method_direct_getobject((t_object *)x, s), __VA_ARGS__)

t_class *earsmap_class;

/*
 things to communicate with:
 - ears.in~:
    keeps a reference to an audio buffer where it will look for samples.
    ears.map~ keeps a sparse matrix of audio buffers
 
 
 */

class earsInAudioOutlet
{
public:
    t_object *obj;
    t_atom_long index;
    
    earsInAudioOutlet(t_object *o, t_atom_long i) : obj(o), index(i) { }
};



typedef std::vector<earsInAudioOutlet> clientList;


class audioPacket
{
public:
    t_sample audio[EARSMAP_MAX_VS]; // this is taken by ears.in~ objects which will read from here audio to be output
    t_atom_long buffer;
    t_atom_long channel;
    clientList clients;
    
    audioPacket(t_atom_long buf, t_atom_long chan, t_object *c, t_atom_long idx) : buffer(buf), channel(chan) {
        setClient(c, idx);
    }
    
    void setClient(t_object *c, t_atom_long idx) {
        if (c) {
            clients.push_back(earsInAudioOutlet(c, idx));
        }
    }
    
    void initializeClients() {
        for (auto c : clients) {
            t_object *o = c.obj;
            t_atom_long i = c.index;
            object_method(o, gensym("initialize"), i, audio);
        }
    }
    
};


class channelMap
{
public:
    std::map<t_atom_long, audioPacket*> theMap;
    t_atom_long buffer;
    
    channelMap(t_atom_long buf) : buffer(buf) { }

    ~channelMap() {
        for (auto i: theMap) {
            delete i.second;
        }
    }
    
    void insert(t_atom_long chan_from, t_atom_long chan_to, t_object *c, t_atom_long index) {
        for (t_atom_long chan = chan_from; chan <= chan_to; c++) {
            if (theMap.find(chan) == theMap.end()) {
                theMap[chan] = new audioPacket(buffer, chan, c, index);
            } else if (c) {
                theMap[chan]->setClient(c, index);
            }
        }
    }
    
    void initializeClients() {
        for (auto i : theMap) {
            i.second->initializeClients();
        }
    }
    
    void removeClient(t_atom_long chan, t_object *c) {
        audioPacket *ap = theMap[chan];
        for (auto i = ap->clients.begin(); i != ap->clients.end(); i++) {
            if ((*i).obj == c) {
                ap->clients.erase(i);
            }
        }
        if (ap->clients.size() == 0) {
            delete ap;
            theMap.erase(chan);
        }
    }
};


class bufferMap
{
public:
    std::map<t_atom_long, channelMap*> theMap;
    
    virtual ~bufferMap() {
        for (auto i: theMap)
            delete i.second;
    }
    
    void insert(t_atom_long buf, t_atom_long chan_from, t_atom_long chan_to, t_object *c, t_atom_long index) {
        
        if (theMap.find(buf) == theMap.end()) {
            theMap[buf] = new channelMap(buf);
            //.insert(std::pair<t_atom_long, channelMap>(buf, channelMap(buf)));
        }
        theMap[buf]->insert(chan_from, chan_to, c, index);
    }
    
    // can be much optimized for the cases where there is superposition between old and new
    void replace(t_atom_long old_buf,
                 t_atom_long old_chan_from, t_atom_long old_chan_to,
                 t_atom_long new_buf,
                 t_atom_long new_chan_from, t_atom_long new_chan_to,
                 t_object *c, t_atom_long index) {
        channelMap *chMap = theMap[old_buf];
        for (t_atom_long ch = old_chan_from; ch <= old_chan_to; ch++) {
            chMap->removeClient(ch, c);
        }
        chMap->insert(new_chan_from, new_chan_to, c, index);
    }
    
    void initializeClients() {
        for (auto i : theMap) {
            i.second->initializeClients();
        }
    }
    
};


//typedef std::map<t_atom_long, audioPacket> channelMap;
//typedef std::map<t_atom_long, channelMap> bufferMap;


template <typename T>
class earsmapMultiMap
{
public:
    std::map<t_atom_long, std::unordered_set<T>> theMap;
    
    void insert(t_atom_long idx, T what) {
        if (theMap.find(idx) == theMap.end()) {
            theMap.insert(std::pair<t_atom_long, std::unordered_set<T>>(idx, std::unordered_set<T>()));
        }
        theMap[idx].insert(what);
    }
};

typedef earsmapMultiMap<void*> earsInOutlets;

class earsOuts : earsmapMultiMap<t_object *>
{
public:
    
};

typedef struct _earsmap
{
    t_pxobject x_obj;
    
    t_patcher *parent_patch;
    
    bufferMap *inBufMap; // for the ears.in~ objects
    bufferMap *outBufMap; // for the ears.out~ objects

    //outletSet *client_outlet[EARSMAP_MAX_DATA_OUTLETS]; // the outlets of the ears.in objects

    t_bool running;
    
    void *m_proxy[EARSMAP_MAX_INPUT_BUFFERS + EARSMAP_MAX_DATA_INLETS + 1];
    long m_in;
    void *m_outlets[EARSMAP_MAX_OUTPUT_BUFFERS + EARSMAP_MAX_DATA_OUTLETS + 1];
    
    long nrt_vs;
    t_symbol *clock_name;
    
    t_bool stopped;
    
    t_atom_long nBufInlets;
    t_atom_long nBufOutlets;
    t_atom_long nDataInlets;
    t_atom_long nDataOutlets;
    
    earsInOutlets *theInOutlets;
    
    t_patcher *client_patch;
    long client_argc;
    t_atom client_argv[256];
    
    t_symbol *outbufname; // ignored for now
    
} t_earsmap;



void *earsmap_new(t_symbol *s, long argc, t_atom *argv);
void earsmap_free(t_earsmap *x);
void earsmap_assist(t_earsmap *x, void *b, long m, long a, char *s);


void earsmap_deletepatch(t_earsmap *x, t_symbol *msg, long argc, t_atom *argv);
void earsmap_clear(t_earsmap *x);
void earsmap_loadpatch(t_earsmap *x, t_symbol *s, long argc, t_atom *argv);


void earsmap_bang(t_earsmap *x);
void earsmap_stop(t_earsmap *x);


void earsmap_addclient(t_earsmap *x, t_object *client);
void earsmap_removeclient(t_earsmap *x, t_object *client);


void earsmap_registerclientoutlet(t_earsmap *x, t_atom_long inlet_num, void* inobj);
void* earsmap_passoutlet(t_earsmap *x, t_atom_long outlet_num);

void earsmap_int(t_earsmap *x, t_atom_long n);
void earsmap_float(t_earsmap *x, t_atom_float f);
void earsmap_anything(t_earsmap *x, t_symbol *s, long argc, t_atom *argv);


void earsmap_dsp64(t_earsmap *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);

void earsmap_dblclick(t_earsmap *x);
void earsmap_open(t_earsmap *x);
void earsmap_pclose(t_earsmap *x);
void earsmap_wclose(t_earsmap *x);

void earsmap_pupdate(t_earsmap *x, void *b, t_patcher *p);
void *earsmap_subpatcher(t_earsmap *x, long index, void *arg);
void earsmap_parentpatcher(t_earsmap *x, t_patcher **parent);

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
    
    //class_addmethod(earsmap_class, (method)earsmap_bang, "bang", 0);
    
    //class_addmethod(earsmap_class, (method)earsmap_stop, "stop", A_CANT, 0);
    //class_addmethod(earsmap_class, (method)earsmap_int, "int", A_LONG, 0);
    //class_addmethod(earsmap_class, (method)earsmap_float, "float", A_FLOAT, 0);
    //class_addmethod(earsmap_class, (method)earsmap_anything, "list", A_GIMME, 0);
    //class_addmethod(earsmap_class, (method)earsmap_anything, "anything", A_GIMME, 0);
    
    //class_addmethod(earsmap_class, (method)earsmap_clear, "clear", 0);
    class_addmethod(earsmap_class, (method)earsmap_loadpatch, "loadpatch", A_DEFER, 0);
    
    //class_addmethod(earsmap_class, (method) earsmap_addclient, "addclient", A_CANT, 0);
    //class_addmethod(earsmap_class, (method) earsmap_addclient, "removeclient", A_CANT, 0);
    
    //class_addmethod(earsmap_class, (method) earsmap_registerclientoutlet, "registerclientoutlet", A_CANT, 0);
    //class_addmethod(earsmap_class, (method) earsmap_passoutlet, "passoutlet", A_CANT, 0);
    
    
    CLASS_ATTR_OBJ(earsmap_class, "ownsdspchain", ATTR_SET_OPAQUE | ATTR_SET_OPAQUE_USER, t_earsmap, x_obj);
    CLASS_ATTR_ACCESSORS(earsmap_class, "ownsdspchain", (method) earsmap_get_ownsdspchain, NULL);
    CLASS_ATTR_INVISIBLE(earsmap_class, "ownsdspchain", 0);
    
    CLASS_ATTR_LONG(earsmap_class, "vs", 0, t_earsmap, nrt_vs);
    CLASS_ATTR_ACCESSORS(earsmap_class, "vs", NULL, (method) earsmap_set_vs);
    
    
    CLASS_ATTR_ATOM_VARSIZE(earsmap_class, "args", 0, t_earsmap, client_argv, client_argc, 256);
    
    class_dspinit(earsmap_class);
    
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
 - When ears.in~ is created, it calls the "ears.in~_created" method here once per declared outlet, communicating its own buffer number, channel number, outlet index and address. All these are stored in the corresponding bufferMap.
 - When ears.out~ is created, it calls the "ears.out~_created" method here once per declared inlet, communicating its own buffer number, channel number, outlet index and address. All these are stored in the corresponding bufferMap.
 - Inlets and outlets are created.
 - The "getoutlet" method of all the ears.out objects is called, and the corresponding outlets and outlet numbers are passed to them.
 - The initialize member functions of both bufferMaps are called, so as to pass to the ears.in~ and ears.out~ objects the audio buffers they'll use
 
 */

void earsmap_earsInTildeCreated(t_earsmap *x, t_atom_long buf,
                                t_atom_long chan_from, t_atom_long chan_to,
                                t_object *client, t_atom_long idx) {
    if (buf > x->nBufInlets)
        x->nBufInlets = buf;
    x->inBufMap->insert(buf, chan_from, chan_to, client, idx);
}

void earsmap_earsOutTildeCreated(t_earsmap *x, t_atom_long buf,
                                 t_atom_long chan_from, t_atom_long chan_to,
                                 t_object *client, t_atom_long idx) {
    if (buf > x->nBufOutlets)
        x->nBufOutlets = buf;
    x->outBufMap->insert(buf, chan_from, chan_to, client, idx);
}

void earsmap_earsInCreated(t_earsmap *x, t_atom_long idx, void* outlet) {
    if (idx > x->nDataInlets)
        x->nDataInlets = idx;
    x->theInOutlets->insert(idx, outlet);
}




void *earsmap_new(t_symbol *s, long argc, t_atom *argv)
{
    t_earsmap *x = (t_earsmap *) object_alloc(earsmap_class);
    t_symbol *patchname = nullptr;
    
    
    long true_ac = attr_args_offset(argc, argv);
    attr_args_process(x, argc - true_ac, argv + true_ac);
    
    if (argc && atom_gettype(argv) == A_SYM)
        patchname = atom_getsym(argv);
    
    dsp_setup((t_pxobject *) x, 0);
    
    //x->this_objs = new clientSet;
    for (int i = 0; i < EARSMAP_MAX_DATA_INLETS; i++) {
        x->client_outlet[i] = new outletSet;
    }
    
    // Get parent patcher
    
    x->parent_patch = (t_patcher *)gensym("#P")->s_thing;
    
    // Load patch
    
    if (patchname)
        earsmap_loadpatch(x, patchname, 0, NULL);
    //x->slots->load(0, patch_name_entered, ac, av, x->last_vec_size, x->last_samp_rate);
    
    /*
    // first inlet is given (we'll get the buffer name from there)
    
    for (t_atom_long i = x->num_in_objs; i > 0; i--) {
        x->m_proxy[i] = proxy_new((t_object *) x, i, &x->m_in);
    }
    
    for (t_atom_long i = x->num_out_objs; i > 0; i--) {
        x->m_outlets[i] = outlet_new((t_object *) x, NULL);
    }
    */
    x->m_outlets[0] = outlet_new((t_object *) x, NULL); // main outlet, returning the buffer
    
    if (x->nrt_vs == 0)
        x->nrt_vs = 512;
    
    x->clock_name = symbol_unique();
    x->running = true;
    return x;
}

void earsmap_free(t_earsmap *x)
{
    earsmap_wclose(x);
    if (x->inBufMap) delete x->inBufMap;
    if (x->outBufMap) delete x->outBufMap;
    object_free(x->client_patch);
}


int setSubAssoc(t_patcher *p, t_object *x)
{
    t_object *assoc;
    
    object_method(p, gensym("getassoc"), &assoc);
    if (!assoc)
        object_method(p, gensym("setassoc"), x);
    
    return 0;
}

void earsmap_loadpatch(t_earsmap *x, t_symbol *patchname, long ac, t_atom *av)
{
    if (ac == 0) {
        ac = x->client_argc;
        av = x->client_argv;
    }
    
    t_fourcc validTypes[3];
    
    // Set the valid types to test for
    
    validTypes[0] = FOUR_CHAR_CODE('maxb');
    validTypes[1] = FOUR_CHAR_CODE('TEXT');
    validTypes[2] = FOUR_CHAR_CODE('JSON');
    
    // freePatch()
    
    char name[2048];
    strncpy_zero(name, patchname->s_name, 2048);
    
    short outvol;
    t_fourcc outtype;
    
    if (locatefile_extended(name, &outvol, &outtype, validTypes, 3)) {
        object_error((t_object *) x, "Can't find %s", name);
        return;
    }
    
    t_symbol *ps_earsmap = gensym("_x_x_ears.map~_x_x_");
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
    object_method(x->client_patch, gensym("traverse"), setSubAssoc, x, &result);
    
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





