/**
 @file
 ears.py.c
 
 @name
 ears.py~
 
 @realname
 ears.py~
 
 @type
 object
 
 @module
 ears
 
 @status
 experimental
 
 @author
 Francesco Bianchi and Andrea Agostini
 
 @digest
 Offline host for patches operating on buffers
 
 @description
 Loads a DSP patch and runs it in non-realtime
 reading from and writing to buffers.
 
 @discussion
 
 @category
 ears py
 
 @keywords
 buffer, offline, patch, patcher, non-realtime
 
 @seealso
 ears.in, ears.in~, ears.mc.in~, ears.out, ears.out~, ears.mc.out~, ears.pyinfo~, ears.tovector~, ears.fromvector~
 
 @owner
 Andrea Agostini
 */

#include <ext.h>
#include <ext_obex.h>
#include <ext_wind.h>
#include <ext_buffer.h>
#include <Python/Python.h>




// Fixes a compiler error with object_method_direct when building the code as cpp
// Shouldn't be necessary with recent versions of the Max SDK, but it won't hurt either
#define object_method_direct_cpp(rt, sig, x, s, ...) ((rt (*)sig)object_method_direct_getmethod((t_object *)x, s))((t_object*)object_method_direct_getobject((t_object *)x, s), __VA_ARGS__)

t_class *earspy_class;

typedef struct _earspy
{
    t_earsbufobj e_ob;
    
    t_symbol* inbuf[EARS_PROCESS_MAX_INPUT_BUFFERS];
    
    t_bool running;
    
    t_symbol *patch_name;
    
    t_bool stopped;
    
    t_atom_long nBufInlets;
    t_atom_long nBufOutlets;
    t_atom_long nOutBufChans[EARS_PROCESS_MAX_OUTPUT_BUFFERS];
    t_atom_long nDataOutlets;
    
    void *dataOutlets[EARS_PROCESS_MAX_DATA_OUTLETS];
    
    // python objects
    t_symbol* scriptPath;
    t_symbol* envPath;
    t_symbol* function;
    t_symbol* home;
    t_symbol* py_executable;
    t_symbol* py_path;
    PyObject* pModule;
    PyObject* pFuncName;
    
} t_earspy;



void *earspy_new(t_symbol *s, long argc, t_atom *argv);
void earspy_free(t_earspy *x);
void earspy_assist(t_earspy *x, void *b, long m, long a, char *s);
void earspy_inletinfo(t_earspy *x, void *b, long a, char *t);


void earspy_deletepatch(t_earspy *x, t_symbol *msg, long argc, t_atom *argv);
void earspy_clear(t_earspy *x);
void earspy_patchername(t_earspy *x, t_symbol *s, long argc, t_atom *argv);


void earspy_bang(t_earspy *x);
void earspy_bang_do(t_earspy *x, t_symbol *s, t_atom_long ac, t_atom *av);

void earspy_stop(t_earspy *x);
void earspy_int(t_earspy *x, t_atom_long n);
void earspy_float(t_earspy *x, t_atom_float f);
void earspy_anything(t_earspy *x, t_symbol *s, t_atom_long argc, t_atom *argv);


void earspy_dblclick(t_earspy *x);

void earspy_setupOutObjects(t_earspy *x);

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
        return;
    }
    
    CLASS_NEW_CHECK_SIZE(earspy_class, "ears.py~", (method)earspy_new, (method)earspy_free, sizeof(t_earspy), NULL, A_GIMME, 0);
    
    
    class_addmethod(earspy_class, (method)earspy_assist, "assist", A_CANT, 0);
    
    // @method open @digest Open patcher
    // @description The <m>open</m> message
    // opens the patcher window loaded in the object, if any.
    class_addmethod(earspy_class, (method)earspy_open, "open", 0);
    
    
    // @method (doubleclick) @digest Open patcher
    // @description Double-clicking on the object
    // opens the patcher window loaded in the object, if any.
    class_addmethod(earspy_class, (method)earspy_open, "dblclick", A_CANT, 0);
    
    // @method bang @digest Run py
    // @description
    // A <m>bang</m> in any of the buffer inlets causes the object
    // to perform the py with the latest received
    // buffer names and parameters. <br/>
    // A <m>bang</m> in any other inlet is routed
    // to the client patch through its <o>ears.in</o> objects.
    class_addmethod(earspy_class, (method)earspy_bang, "bang", 0);
    
    // @method stop @digest Stop py
    // @description
    // The <m>stop</m> message in any of the buffer inlets causes the object
    // to stop immediately the py it is performing;
    // in any other inlet, it is routed
    // to the client patch through its <o>ears.in</o> objects.
    class_addmethod(earspy_class, (method)earspy_stop, "stop", 0);
    
    
    // @method patchername @digest Load patch
    // @description
    // The <m>patchername</m> message in any of the buffer inlets causes the object
    // to stop immediately the py it is performing;
    // in any other inlet, it is routed
    // to the client patch through its <o>ears.in</o> objects.
    class_addmethod(earspy_class, (method)earspy_patchername, "patchername", A_DEFER, 0);
    
    
    class_addmethod(earspy_class, (method)earspy_int, "int", A_LONG, 0);
    class_addmethod(earspy_class, (method)earspy_float, "float", A_FLOAT, 0);
    class_addmethod(earspy_class, (method)earspy_anything, "list", A_GIMME, 0);
    
    // @method list/llll @digest Function depends on inlet
    // @description
    // If a list or llll with buffer names is passed to one of the buffer inlets,
    // The contents of the corresponding buffers will be passed
    // to the loaded patcher through the patcher's
    // <o>ears.in~</o> and <o>ears.mc.in~</o> objects,
    // for being pyed by the DSP chain of the patcher itself.
    // The names of the buffers containing the pyed audio,
    // as output by the patcher's <o>ears.out~</o> and <o>ears.mc.out~</o> objects,
    // will be subsequently output.<br/>
    // Lists and llll received in non-buffer inlets
    // will be passed as they are to the loaded patcher,
    // through its <o>ears.in</o> objects.<br/>
    // If the list or llll is received in the first inlet,
    // The pying will be triggered.
    class_addmethod(earspy_class, (method)earspy_anything, "anything", A_GIMME, 0);
    
    

    
    class_addmethod(earspy_class, (method)earspy_assist, "assist", A_CANT, 0);
    class_addmethod(earspy_class, (method)earspy_inletinfo, "inletinfo", A_CANT, 0);
    
    earsbufobj_class_add_outname_attr(earspy_class);
    earsbufobj_class_add_alloc_attr(earspy_class);

    CLASS_ATTR_SYM(c, "envpath", 0, t_earspy, envPath);
    CLASS_ATTR_SYM(c, "scriptpath", 0, t_earspy, scriptPath);
    CLASS_ATTR_SYM(c, "function", 0, t_earspy, function);

    class_register(CLASS_BOX, earspy_class);

    return 0;
}

// 1 arg: patch name
// 2 args: buffer name (llll), patch name: [ foo bar ] patchname
void *earspy_new(t_symbol *s, long argc, t_atom *argv)
{
    // @arg 0 @name outnames @optional 1 @type symbol
    // @digest Output buffer names
    // @description @copy EARS_DOC_OUTNAME_ATTR
    
    // @arg 1 @name patcher name @optional 1 @type symbol
    // @digest Patcher name
    // @description Sets the name of the patch to be loaded
    
    t_earspy *x = (t_earspy *) object_alloc(earspy_class);
    t_symbol *patchname = nullptr;
    
    earsbufobj_init((t_earsbufobj*) x, EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES);
    
    x->autoclock = 1;
    
    x->function = gensym("process");
    long true_ac = attr_args_offset(argc, argv);
    attr_args_process(x, argc - true_ac, argv + true_ac);
    
    t_llll* args = llll_parse(true_ac, argv);
    
    x->nBufInlets = 1;
    
    
    // e = buffer; E = buffer list;
    char intypes[2048];
    int i;
    for (i = 0; i < x->nBufInlets; i++) {
        intypes[i] = 'E';
    }
    /*
    for (i = 0; i < x->theInOutlets->maxIdx; i++) {
        intypes[i + x->nBufInlets] = 'a';
    }
     */
    intypes[i + x->nBufInlets] = 0;
    
    char outtypes[2048];
    /*
    for (i = x->nDataOutlets - 1; i >= 0; i--) {
        x->dataOutlets[i] = outlet_new(x, "anything");
    }
     */
    x->nBufOutlets = 1;
    for (i = 0; i < x->nBufOutlets; i++) {
        outtypes[i] = 'E';
    }
    outtypes[i] = 0;
    
    earsbufobj_setup((t_earsbufobj *) x, intypes, outtypes, args);
    llll_free(args);

    x->running = true;
    return x;
}

void earspy_free(t_earspy *x)
{
    if (x->pModule != NULL)
        Py_DECREF(x->pModule);
    Py_FinalizeEx();
    earsbufobj_free((t_earsbufobj*) x);
}



void earspy_open(t_earspy *x)
{
    //long inlet = proxy_getinlet((t_object *) x);
    
    //if (inlet >= x->nBufInlets) {
    earspy_anything(x, _sym_open, 0, NULL);
    //} else {
    //    object_method((t_object *) x->client_patch, _sym_open);
    //}
}




void earspy_int(t_earspy *x, t_atom_long i)
{
    t_atom a[1];
    atom_setlong(a, i);
    earspy_anything(x, _sym_int, 1, a);
}

void earspy_float(t_earspy *x, t_atom_float f)
{
    t_atom a[1];
    atom_setfloat(a, f);
    earspy_anything(x, _sym_float, 1, a);
}

void earspy_anything(t_earspy *x, t_symbol *s, t_atom_long ac, t_atom* av)
{
    long inlet = proxy_getinlet((t_object *) x);
    
    if (inlet > 0 && inlet >= x->nBufInlets) {
        // TODO: send to ears.in <<<--- SEEMS TO BE OK...
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
        earspy_bang(x);
    
    llll_free(parsed);
}

void earspy_bang(t_earspy *x)
{
    if (!x->client_patch)
        return;
    
    long inlet = proxy_getinlet((t_object *) x);
    
    if (inlet >= x->nBufInlets) {
        earspy_anything(x, _sym_bang, 0, NULL);
    } else {
        defer(x, (method) earspy_bang_do, NULL, 0, NULL);
    }
    
}

void earspy_bang_do(t_earspy *x, t_symbol *s, t_atom_long ac, t_atom *av)
{
    double sr;
    
    if (x->reload)
        earspy_patchername((t_earspy*) x, x->patch_name, x->client_argc, x->client_argv);
    
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
            earspy_autoclock(x, x->client_patch);
            for (t_object* o : *x->earspyinfoObjects) {
                object_method(o, gensym("prepare"), &scarg);
            }
        }
        
        for (t_object* o : *x->earspyinfoObjects) {
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
        
        for (const auto &i : *x->earspyinfoObjects) {
            object_method(i, gensym("end"));
        }
        
        object_free(setclock);
    }
    
    for (int i = x->nBufOutlets - 1; i >= 0 ; i--)
        earsbufobj_outlet_buffer((t_earsbufobj *)x, i);
}






void earspy_assist(t_earspy *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        if (a < x->nBufInlets) {
            sprintf(s, "symbol/list/llll: Incoming buffer Name"); // @in 0 @loop 1 @type symbol/llll @digest Incoming buffer name
        } else {
            sprintf(s, "symbol/list/llll: Incoming data"); // @in 1 @loop 1 @type symbol/llll @digest Incoming data for <o>ears.in</o>.
        }
    } else {
        if (a < x->nBufOutlets) {
            sprintf(s, "symbol/list/llll: Output buffer"); // @out 0 @loop 1 @type symbol/llll @digest Output buffer
        } else {
            sprintf(s, "symbol/list/llll: Output data"); // @out 1 @loop 1 @type symbol/llll @digest Output data from <o>ears.out</o>.
        }
    }
}


void earspy_inletinfo(t_earspy *x, void *b, long a, char *t)
{
    if (a != 0)
        *t = 1;
}




