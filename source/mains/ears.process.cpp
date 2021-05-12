//
//  ears.cpp
//  ears
//
//  Created by andreaagostini on 03/04/2021.
//




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
// Shouldn't be necessary with recent version of the Max SDK, but it won't hurt either
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

typedef earsprocessMultiMap<void*> earsInOutlets;

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

    t_atom dummydur[2]; // just in case
    long durationpolicy;
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
    
    earsInOutlets *theInOutlets;
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


void earsprocess_deletepatch(t_earsprocess *x, t_symbol *msg, long argc, t_atom *argv);
void earsprocess_clear(t_earsprocess *x);
void earsprocess_loadpatch(t_earsprocess *x, t_symbol *s, long argc, t_atom *argv);


void earsprocess_bang(t_earsprocess *x);
void earsprocess_bang_do(t_earsprocess *x, t_symbol *s, t_atom_long ac, t_atom *av);

void earsprocess_stop(t_earsprocess *x);
void earsprocess_int(t_earsprocess *x, t_atom_long n);
void earsprocess_float(t_earsprocess *x, t_atom_float f);
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

void earsprocess_earsintildecreated(t_earsprocess *x, t_atom_long bufIndex, t_object *in);
void earsprocess_earsintildedeleted(t_earsprocess *x, t_object *in);
void earsprocess_earsouttildecreated(t_earsprocess *x, t_atom_long bufIndex, t_object *out);
void earsprocess_earsouttildedeleted(t_earsprocess *x, t_object *out);

void earsprocess_earsincreated(t_earsprocess *x, t_atom_long n, t_atom_long *index, void **outlet);
void earsprocess_earsoutcreated(t_earsprocess *x, long maxindex, t_object *obj);
void earsprocess_earsindeleted(t_earsprocess *x, t_atom_long n, t_atom_long *index, void **outlet);
void earsprocess_earsoutdeleted(t_earsprocess *x, t_object *obj);

void earsprocess_earsprocessinfocreated(t_earsprocess *x, t_object *obj);
void earsprocess_earsprocessinfodeleted(t_earsprocess *x, t_object *obj);

void earsprocess_autoclock(t_earsprocess *x, t_patcher *p);

t_max_err earsprocess_set_duration(t_earsprocess *x, t_object *attr, long argc, t_atom *argv);
t_max_err earsprocess_get_duration(t_earsprocess *x, t_object *attr, long *argc, t_atom **argv);

t_max_err earsprocess_set_vs(t_earsprocess *x, t_object *attr, long argc, t_atom *argv);
t_max_err earsprocess_get_ownsdspchain(t_earsprocess *x, t_object *attr, long *argc, t_atom **argv);





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
    
    earsprocess_class = class_new("ears.process~", (method)earsprocess_new, (method)earsprocess_free, sizeof(t_earsprocess), NULL, A_GIMME, 0);
    
    
    
    //class_addmethod(earsprocess_class, (method)earsprocess_assist, "assist", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_open, "open", 0);
    class_addmethod(earsprocess_class, (method)earsprocess_open, "dblclick", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_wclose, "wclose", 0);
    
    class_addmethod(earsprocess_class, (method)earsprocess_pupdate, "pupdate", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_subpatcher, "subpatcher", A_CANT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_parentpatcher, "parentpatcher", A_CANT, 0);
    
    class_addmethod(earsprocess_class, (method)earsprocess_bang, "bang", 0);
    class_addmethod(earsprocess_class, (method)earsprocess_stop, "stop", 0);

    class_addmethod(earsprocess_class, (method)earsprocess_int, "int", A_LONG, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_float, "float", A_FLOAT, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_anything, "list", A_GIMME, 0);
    class_addmethod(earsprocess_class, (method)earsprocess_anything, "anything", A_GIMME, 0);

    class_addmethod(earsprocess_class, (method)earsprocess_loadpatch, "loadpatch", A_DEFER, 0);
    
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
    
    earsbufobj_class_add_outname_attr(earsprocess_class);
    earsbufobj_class_add_naming_attr(earsprocess_class);

    CLASS_ATTR_OBJ(earsprocess_class, "ownsdspchain", ATTR_SET_OPAQUE | ATTR_SET_OPAQUE_USER, t_earsprocess, e_ob);
    CLASS_ATTR_ACCESSORS(earsprocess_class, "ownsdspchain", (method) earsprocess_get_ownsdspchain, NULL);
    CLASS_ATTR_INVISIBLE(earsprocess_class, "ownsdspchain", 0);
    
    CLASS_ATTR_LONG(earsprocess_class, "vs", 0, t_earsprocess, vs);
    CLASS_ATTR_ACCESSORS(earsprocess_class, "vs", NULL, (method) earsprocess_set_vs);
    CLASS_ATTR_LABEL(earsprocess_class, "vs", 0, "Vector Size");

    CLASS_ATTR_LONG(earsprocess_class, "sr", 0, t_earsprocess, sr);
    CLASS_ATTR_FILTER_MIN(earsprocess_class, "sr", 0);
    CLASS_ATTR_LABEL(earsprocess_class, "sr", 0, "Default Sample Rate");

    /*
    CLASS_ATTR_LONG(earsprocess_class, "durationpolicy", 0, t_earsprocess, durationpolicy);
    CLASS_ATTR_STYLE_LABEL(earsprocess_class, "durationpolicy", 0, "enumindex", "Duration Policy");
    CLASS_ATTR_FILTER_CLIP(earsprocess_class, "durationpolicy", 0, 2);
    CLASS_ATTR_ENUMINDEX(earsprocess_class, "durationpolicy", 0, "Shortest Longest Fixed");
     */
    
    
    CLASS_ATTR_ATOM_ARRAY(earsprocess_class, "duration", 0, t_earsprocess, dummydur, 2);
    CLASS_ATTR_ACCESSORS(earsprocess_class, "duration", (method) earsprocess_get_duration, (method) earsprocess_set_duration);

    //CLASS_ATTR_LONG(earsprocess_class, "durationpolicy", 0, t_earsprocess, durationpolicy);
    //CLASS_ATTR_STYLE_LABEL(earsprocess_class, "durationpolicy", 0, "enumindex", "Duration Policy");
    //CLASS_ATTR_FILTER_CLIP(earsprocess_class, "durationpolicy", 0, 2);
    //CLASS_ATTR_ENUMINDEX(earsprocess_class, "durationpolicy", 0, "Shortest Longest Fixed");

    //CLASS_ATTR_DOUBLE(earsprocess_class, "tail", 0, t_earsprocess, tail);
    //CLASS_ATTR_LABEL(earsprocess_class, "tail", 0, "Tail or Fixed Duration");
    //CLASS_ATTR_FILTER_MIN(earsprocess_class, "tail", 0);
    
    CLASS_ATTR_LONG(earsprocess_class, "scalarmode", 0, t_earsprocess, scalarmode);
    CLASS_ATTR_STYLE_LABEL(earsprocess_class, "scalarmode", 0, "onoff", "Scalar Mode");
    CLASS_ATTR_FILTER_CLIP(earsprocess_class, "scalarmode", 0, 1);
    
    CLASS_ATTR_LONG(earsprocess_class, "reload", 0, t_earsprocess, reload);
    CLASS_ATTR_STYLE_LABEL(earsprocess_class, "reload", 0, "onoff", "Reload");
    CLASS_ATTR_FILTER_CLIP(earsprocess_class, "reload", 0, 1);
    
    CLASS_ATTR_LONG(earsprocess_class, "autoclock", 0, t_earsprocess, autoclock);
    CLASS_ATTR_STYLE_LABEL(earsprocess_class, "autoclock", 0, "onoff", "Automatic Clock Message");
    CLASS_ATTR_FILTER_CLIP(earsprocess_class, "autoclock", 0, 1);

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


t_max_err earsprocess_set_duration(t_earsprocess *x, t_object *attr, long argc, t_atom *argv)
{
    t_llll *ll = llll_parse(argc, argv);
    
    if (!ll) {
        object_error((t_object *) x, "duration: bad llll");
        return MAX_ERR_GENERIC;
    }
    
    if (ll->l_size == 0 || ll->l_size > 2 || ll->l_depth > 1) {
        object_error((t_object *) x, "duration: wrong llll format");
        return MAX_ERR_GENERIC;
    }
    
    t_hatom *a = &ll->l_head->l_hatom;
    
    switch (hatom_gettype(a)) {
        case H_LONG: {
            long v = atom_getlong(argv);
            CLIP_ASSIGN(v, 0, 2);
            x->durationpolicy = v;
            break;
        }
        case H_SYM: {
            t_symbol *s = atom_getsym(argv);
            if (s == gensym("shortest") || s == gensym("s"))
                x->durationpolicy = eDURPOLICY_SHORTEST;
            else if (s == gensym("longest") || s == gensym("l"))
                x->durationpolicy = eDURPOLICY_LONGEST;
            else if (s == gensym("fixed") || s == gensym("f") || s == gensym("maximum") || s == gensym("max") || s == gensym("m")) {
                x->durationpolicy = eDURPOLICY_FIXED;
                if (x->tail <= 0 && ll->l_size == 1) {
                    object_warn((t_object *) x, "Setting maximum duration to 60000");
                    x->tail = 60000;
                }
            }
            else
                object_error((t_object *) x, "duration: unknown duration policy %s", s->s_name);
            break;
        }
        default: {
            object_error((t_object *) x, "duration: wrong type for argument 1");
            return MAX_ERR_GENERIC;
        }
    }
    
    if (ll->l_size == 1)
        return MAX_ERR_NONE;
    
    a = &ll->l_tail->l_hatom;
    
    if (!hatom_is_number(a)) {
        object_error((t_object *) x, "duration: wrong type for argument 2");
        if (x->durationpolicy == eDURPOLICY_FIXED) {
            object_warn((t_object *) x, "Setting maximum duration to 60000");
            x->tail = 60000;
        }
        return MAX_ERR_GENERIC;
    }
    
    x->tail = hatom_getdouble(a);

    return MAX_ERR_NONE;
}



t_max_err earsprocess_get_duration(t_earsprocess *x, t_object *attr, long *argc, t_atom **argv)
{
    char alloc;
    atom_alloc_array(2, argc, argv, &alloc);
    
    switch(x->durationpolicy) {
        case eDURPOLICY_LONGEST:
            atom_setsym(*argv, gensym("shortest"));
            break;
        case eDURPOLICY_FIXED:
            atom_setsym(*argv, gensym("fixed"));
            break;
        default:
            atom_setsym(*argv, gensym("shortest"));
            break;
    }
    
    atom_setfloat((*argv) + 1, x->tail);

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


void earsprocess_earsintildecreated(t_earsprocess *x, t_atom_long bufIndex, t_object *in)
{
    x->earsInTildeObjects->insert(in);
    x->generator = false;
    if (bufIndex > x->nBufInlets)
        x->nBufInlets = bufIndex;
}

void earsprocess_earsintildedeleted(t_earsprocess *x, t_object *in)
{
    x->earsInTildeObjects->erase(in);
}

void earsprocess_earsouttildecreated(t_earsprocess *x, t_atom_long bufIndex, t_object *out)
{
    x->earsOutTildeObjects->insert(out);
    if (bufIndex > x->nBufOutlets)
        x->nBufOutlets = bufIndex;
}

void earsprocess_earsouttildedeleted(t_earsprocess *x, t_object *out)
{
    x->earsOutTildeObjects->erase(out);
}

void earsprocess_earsincreated(t_earsprocess *x, t_atom_long n, t_atom_long *index, void **outlet) {
    for (int i = 0; i < n; i++)
        x->theInOutlets->insert(*(index++), *(outlet++));
}

void earsprocess_earsoutcreated(t_earsprocess *x, long maxindex, t_object *obj) {
    x->theOuts->insert(obj);
    if (x->nDataOutlets < maxindex)
        x->nDataOutlets = maxindex;
}

void earsprocess_earsindeleted(t_earsprocess *x, t_atom_long n, t_atom_long *index, void **outlet) {
    for (int i = 0; i < n; i++)
        x->theInOutlets->remove(*(index++), *(outlet++));}

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
void *earsprocess_new(t_symbol *s, long argc, t_atom *argv)
{
    t_earsprocess *x = (t_earsprocess *) object_alloc(earsprocess_class);
    t_symbol *patchname = nullptr;
    
    earsbufobj_init((t_earsbufobj*) x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
    
    x->autoclock = 1;
    
    long true_ac = attr_args_offset(argc, argv);
    attr_args_process(x, argc - true_ac, argv + true_ac);
    
    t_llll* args = llll_parse(true_ac, argv);
    
    switch (args->l_size) {
        case 0:
            break;
        case 1:
            patchname = hatom_getsym(&args->l_head->l_hatom);
            llll_clear(args);
            break;
        case 2:
            patchname = hatom_getsym(&args->l_tail->l_hatom);
            llll_betail(args);
            args = nullptr;
            break;
        default:
            object_error((t_object *) x, "too many arguments");
            llll_free(args);
            return nullptr;
            break;
    }

    x->theInOutlets = new earsInOutlets;
    x->theOuts = new std::set<t_object*>;
    x->earsInTildeObjects = new objectSet;
    x->earsOutTildeObjects = new objectSet;
    x->earsprocessinfoObjects = new objectSet;

    // Get parent patcher
    
    x->parent_patch = (t_patcher *)gensym("#P")->s_thing;
    
    // Load patch
    
    x->nBufInlets = 1;
    if (patchname)
        earsprocess_loadpatch(x, patchname, 0, NULL);
    
    // e = buffer; E = buffer list;

    char intypes[2048];
    int i;
    for (i = 0; i < x->nBufInlets; i++) {
        intypes[i] = 'E';
    }
    for (i = 0; i < x->theInOutlets->maxIdx; i++) {
        intypes[i + x->nBufInlets] = 'a';
    }
    intypes[i + x->nBufInlets] = 0;
    
    char outtypes[2048];
    for (i = x->nDataOutlets - 1; i >= 0; i--) {
        x->dataOutlets[i] = outlet_new(x, "anything");
    }
    for (i = 0; i < x->nBufOutlets; i++) {
        outtypes[i] = 'E';
    }
    outtypes[i] = 0;

    earsbufobj_setup((t_earsbufobj *) x, intypes, outtypes, args);
    llll_free(args);

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
    
    delete x->theInOutlets;
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

void earsprocess_loadpatch(t_earsprocess *x, t_symbol *patchname, long ac, t_atom *av)
{
    
    x->theInOutlets->clear();
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
    if (x->client_patch)
        object_method((t_object *) x->client_patch, _sym_front);
}

void earsprocess_wclose(t_earsprocess *x)
{
    if (x->client_patch)
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
    earsprocess_anything(x, _sym_int, 1, a);
}

void earsprocess_float(t_earsprocess *x, t_atom_float f)
{
    t_atom a[1];
    atom_setfloat(a, f);
    earsprocess_anything(x, _sym_float, 1, a);
}

void earsprocess_anything(t_earsprocess *x, t_symbol *s, t_atom_long ac, t_atom* av)
{
    long inlet = proxy_getinlet((t_object *) x);
    
    if (inlet > 0 && inlet >= x->nBufInlets) {
        // TODO: send to ears.in
        for (auto o: *x->theInOutlets->theMap[inlet - x->nBufInlets + 1]) {
            outlet_anything(o, s, ac, av);
        }
        return;
    }
    
    t_llll *parsed = earsbufobj_parse_gimme((t_earsbufobj *) x, LLLL_OBJ_VANILLA, s, ac, av);
    
    if (!parsed)
        return;
    
    if (parsed->l_head) {
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_IN, inlet, parsed->l_size, true);
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
    
    long inlet = proxy_getinlet((t_object *) x);
    
    if (inlet > 0) {
        earsprocess_anything(x, _sym_bang, 0, NULL);
    } else {
        defer(x, (method) earsprocess_bang_do, NULL, 0, NULL);
    }

}

void earsprocess_bang_do(t_earsprocess *x, t_symbol *s, t_atom_long ac, t_atom *av)
{
    double sr;
    
    if (x->reload)
        earsprocess_loadpatch((t_earsprocess*) x, x->patch_name, x->client_argc, x->client_argv);
    
    // Get number of buffers on which to iterate
    long num_buffer_to_iter = LONG_MAX;
    for (long i = 0; i < x->nBufInlets; i++) {
        long this_num_buf = earsbufobj_get_num_inlet_stored_buffers((t_earsbufobj *)x, i, false);
        if (x->scalarmode && this_num_buf == 1) {
            // nothing to do
        } else {
            num_buffer_to_iter = MIN(num_buffer_to_iter, this_num_buf);
        }
    }
    if (num_buffer_to_iter == LONG_MAX ||
        (num_buffer_to_iter == 0 && x->generator))
        num_buffer_to_iter = 1;

    for (long i = 0; i < x->nBufOutlets; i++)
        earsbufobj_resize_store((t_earsbufobj *)x, EARSBUFOBJ_OUT, i, num_buffer_to_iter, true);
    
    for (int iterBuf = 0; iterBuf < num_buffer_to_iter; iterBuf++) {
        bufferData bufs[EARS_PROCESS_MAX_INPUT_BUFFERS];
        t_atom bufDurs[EARS_PROCESS_MAX_INPUT_BUFFERS];

        t_atom_long maxDur = 0;
        t_atom_long minDur = 0;
        
        sr = 0;
        
        if (!x->generator) {
            for (long i = 0; i < x->nBufInlets; i++) {
                t_atom_long bufNum = (x->scalarmode &&
                    earsbufobj_get_num_inlet_stored_buffers((t_earsbufobj *)x, i, false) == 1) ?
                    0 : iterBuf;
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
        
        switch (x->durationpolicy) {
            case eDURPOLICY_SHORTEST: duration += minDur; break;
            case eDURPOLICY_LONGEST: duration += maxDur; break;
            default: break;
        }
        
        if (duration <= 0)
            duration = LONG_MAX;
        
        //t_object *prnt = (t_object *) newinstance(gensym("print"), 0, NULL);
        
        x->stopped = false;
        
        t_atom scarg;
        atom_setsym(&scarg, x->clock_name);
        t_object *setclock = (t_object *) newinstance(gensym("setclock"), 1, &scarg);

        if (iterBuf == 0) {
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
                dspchain_tick(chain);
            }
            
            if (x->stopped)
                duration = s;
            
            object_free((t_object *) chain);
            
            bufferData outBuf[EARS_PROCESS_MAX_OUTPUT_BUFFERS];
            
            for (int i = 0; i < x->nBufOutlets; i++) {            
                t_buffer_obj *buf = earsbufobj_get_outlet_buffer_obj((t_earsbufobj *)x, i, iterBuf);
                ears_buffer_set_sr((t_object *)x, buf, sr);
                int nChans = chanMap.chansPerBuf[i];
                ears_buffer_set_size_and_numchannels((t_object *)x, buf, duration, MAX(nChans, 1));

                outBuf[i].set((t_object *) x, earsbufobj_get_outlet_buffer_name((t_earsbufobj *)x, i, iterBuf));
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
        
        object_free(setclock);
    }
    
    
    for (int i = x->nBufOutlets - 1; i >= 0 ; i--)
        earsbufobj_outlet_buffer((t_earsbufobj *)x, i);
}

void earsprocess_stop(t_earsprocess *x)
{
    x->stopped = true;
}


void earsprocess_autoclock(t_earsprocess *x, t_patcher *p)
{
    for (t_box *b = jpatcher_get_firstobject(p); b; b = jbox_get_nextobject(b))
    {
        t_object *o = jbox_get_object(b);
        if (object_classname(o) == gensym("ears.process~"))
            continue;
        method c = zgetfn(o, gensym("clock"));
        if (c ||
            object_classname(o) == gensym("pipe"))
            (c)(o, x->clock_name);
        
        
        t_patcher *subpatch;
        long index = 0;
        
        while (b && (subpatch = (t_patcher *)object_subpatcher(o, &index, x))) {
            earsprocess_autoclock(x, subpatch);
        }
    }
}


