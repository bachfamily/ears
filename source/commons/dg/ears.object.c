#include "ears.object.h"

long ears_check_bach_version()
{
    long min_required_bach_version = 80000;
    bach = (t_bach *) gensym("bach")->s_thing;
    if (!bach) {
        return 1;
    } else if (bach->b_version < min_required_bach_version) {
        return 1;
    }
    return 0;
}

void ears_error_bachcheck()
{
    if (!gensym("bach")->s_thing) {
        error("error: ears needs bach to be installed in order to work (www.bachproject.net).");
    } else {
        char temp[2048];
        snprintf_zero(temp, 2048, "%s", bach_get_current_version_string_verbose());
        if (temp[0]) {
            error("error: ears needs a bach version of at least 0.8.");
            error("   Your bach version is %s. Please upgrade bach.", temp);
        } else {
            error("error: ears needs a bach version of at least 0.8.", temp);
            error("   You have installed an older version. Please upgrade bach.");
        }
    }
}

t_symbol *ears_buffer_name_get_for_polybuffer(t_symbol *polybuffername, long index) {
    char polybuffer_buffername[MAX_SYM_LENGTH+30];
    snprintf_zero(polybuffer_buffername, MAX_SYM_LENGTH+30, "%s.%d", polybuffername->s_name, index);
    return gensym(polybuffer_buffername);
}


t_hashtab *ears_hashtab_get()
{
    return ((t_hashtab **)(gensym("ears")->s_thing))[0];
}

t_hashtab *ears_hashtab_spectrograms_get()
{
    return ((t_hashtab **)(gensym("ears")->s_thing))[1];
}

long ears_hashtab_spectrograms_store(t_symbol *buffername, void *data)
{
                                                                        // will be freed with sysmem_freeptr()
    if (hashtab_storeflags(ears_hashtab_spectrograms_get(), buffername, (t_object *)data, OBJ_FLAG_MEMORY) == MAX_ERR_NONE)
        return 0;
    
    return 1;
}

void *ears_hashtab_spectrograms_retrieve(t_symbol *buffername)
{
    t_object *storeddata = NULL;
    hashtab_lookup(ears_hashtab_spectrograms_get(), buffername, &storeddata);
    if (storeddata) {
        return storeddata;
    } else {
        return NULL;
    }
}

void ears_hashtabs_setup()
{
    if (!gensym("ears")->s_thing || gensym("ears")->s_thing == WHITENULL) {
        t_hashtab *h_main = hashtab_new(0);
        t_hashtab *h_spectrograms = hashtab_new(0);
        t_hashtab **h = (t_hashtab **)bach_newptr(2 * sizeof(t_hashtab *));
        h[0] = h_main;
        h[1] = h_spectrograms;
        gensym("ears")->s_thing = (t_object *)h;
    }
}

long earsbufobj_proxy_getinlet(t_earsbufobj *e_ob)
{
    return e_ob->l_curr_proxy;
}

void earsbufobj_buffer_release(t_earsbufobj *e_ob, e_earsbufobj_in_out where, long store, long bufferidx)
{
    t_object *buf = earsbufobj_get_stored_buffer_obj(e_ob, where, store, bufferidx);
    t_symbol *name = earsbufobj_get_stored_buffer_name(e_ob, where, store, bufferidx);
    
    if (name && buf) {
        if (e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_DYNAMIC) {
            if (EARS_ALLOCATIONVERBOSE)
                post("--- ears allocation: Buffer %s will be kept in memory (dynamic mode)", name->s_name);
        } else {
            ears_buffer_release(buf, name);
        }
    }
}

void earsbufobj_polybuffer_release(t_earsbufobj *e_ob, e_earsbufobj_in_out where, long store)
{
    t_symbol *name = earsbufobj_get_store(e_ob, where, store)->polybuffer_name;
    t_object *obj = ears_polybuffer_getobject(name);

    if (name && obj) {
        if (e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_DYNAMIC) {
            if (EARS_ALLOCATIONVERBOSE)
                post("--- ears allocation: Polybuffer %s will be kept in memory (dynamic mode)", name->s_name);
        } else {
            ears_polybuffer_release(obj, name);
        }
    }
}


void ears_hashtab_inccount(t_symbol *name)
{
    if (name) {
        t_atom_long count = 0;
        t_hashtab *ht = ears_hashtab_get();
        if (ht) {
            t_max_err err = hashtab_lookuplong(ht, name, &count);
            if (err == MAX_ERR_NONE) {
                if (EARS_ALLOCATIONVERBOSE)
                    post("--- ears allocation: Incrementing ears-wide count for buffer %s: now has count %ld", name->s_name, count+1);
                hashtab_storelong(ht, name, count+1); // increase reference count
            }
        }
    }
}

void ears_hashtab_clipcount(t_symbol *name)
{
    if (name) {
        t_atom_long count = 0;
        t_hashtab *ht = ears_hashtab_get();
        if (ht) {
            t_max_err err = hashtab_lookuplong(ht, name, &count);
            if (err == MAX_ERR_NONE) {
                if (count != 0) count = 1;
                if (EARS_ALLOCATIONVERBOSE)
                    post("--- ears allocation: Clipping count for buffer %s: now has ears-wide count %ld", name->s_name, count+1);
                hashtab_storelong(ht, name, count); // increase reference count
            }
        }
    }
}


void ears_hashtab_store(t_symbol *name)
{
    if (name) {
        t_hashtab *ht = ears_hashtab_get();
        if (ht) {
            if (EARS_ALLOCATIONVERBOSE)
                post("--- ears allocation: Storing buffer %s in hash table (with count = 0)", name->s_name);
            hashtab_storelong(ht, name, 0);
        }
    }
}

t_atom_long ears_hashtab_getcount(t_symbol *name)
{
    t_atom_long count = 0;
    if (name) {
        t_hashtab *ht = ears_hashtab_get();
        if (ht) {
            hashtab_lookuplong(ht, name, &count);
        }
    }
    return count;
}



// Link one of the inlets or outlets to a given buffer via the buffer name.
// If no buffer exists with such name, a new buffer is created.

// Important: before calling earsbufobj_buffer_link, the buffer status of the corresponding buffer must be up to date
void earsbufobj_buffer_link(t_earsbufobj *e_ob, e_earsbufobj_in_out where, long store_index, long buffer_index, t_symbol *buf_name)
{
    if (!buf_name)
        return; // to do: handle better
    
    // retrieving existing references
    t_symbol **name = NULL;
    t_object **buf = NULL;
    switch (where) {
        case EARSBUFOBJ_IN:
            if (store_index >= 0 && store_index < e_ob->l_numbufins && buffer_index >= 0 && buffer_index < e_ob->l_instore[store_index].num_stored_bufs) {
                name = &e_ob->l_instore[store_index].stored_buf[buffer_index].l_name;
                buf = &e_ob->l_instore[store_index].stored_buf[buffer_index].l_buf;
            }
            break;

        case EARSBUFOBJ_OUT:
            if (store_index >= 0 && store_index < e_ob->l_numbufouts && buffer_index >= 0 && buffer_index < e_ob->l_outstore[store_index].num_stored_bufs) {
                name = &e_ob->l_outstore[store_index].stored_buf[buffer_index].l_name;
                buf = &e_ob->l_outstore[store_index].stored_buf[buffer_index].l_buf;
            }
            break;

        default:
            break;
    }
    
    
    if (name && *name && buf && *buf) {
        earsbufobj_buffer_release(e_ob, where, store_index, buffer_index);
    }

    // is it a polybuffer buffer?
    if (where == EARSBUFOBJ_OUT && e_ob->l_outstore[store_index].use_polybuffers) {
        t_symbol *polybuffer_name = e_ob->l_outstore[store_index].polybuffer_name;
        t_object *polybuffer_obj = polybuffer_name ? ears_polybuffer_getobject(polybuffer_name) : NULL;
        if (polybuffer_obj) {
            long polybuffer_count = object_attr_getlong(polybuffer_obj, _sym_count);
            for (long i = polybuffer_count; i <= buffer_index; i++) {
                object_method_long(polybuffer_obj, gensym("appendempty"), 1000, NULL);
            }
        }
    }

    
    // does the buffer exists??
    if (!ears_buffer_symbol_is_buffer(buf_name)) {

        if (buf_name->s_thing) {
            // not a buffer, but the name has been used! (e.g. a database? a table?..)
            object_error((t_object *)e_ob, "The symbol %s is already in use for something different than a buffer. Using uniquely generated symbol instead.", buf_name->s_name);
            
            *name = symbol_unique();
            
            // must create an actual buffer with the unique name
            t_object *b = (t_object *)ears_buffer_make(*name, true);
            
            *buf = b;

        } else {
            
            // must create an actual buffer with that name
            t_object *b = (t_object *)ears_buffer_make(buf_name, true);
            
            *buf = b;
            *name = buf_name;

        }
        
    } else {
        // buffer already exists. Cool. Let's just get it.
//        *buf = buffer_ref_getobject(*ref);
        *buf = ears_buffer_getobject(buf_name);
        
        t_llll *generated_outnames = earsbufobj_generated_names_llll_getlist(e_ob->l_generated_outnames, store_index, buffer_index);
        
        ears_buffer_retain(*buf, buf_name, generated_outnames); // we retain the buffer
        
        *name = buf_name;
    }
    
}



void earsbufobj_resize_store(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long new_size, char also_create_unique_buffers)
{
    long i;
    t_earsbufobj_store *store = earsbufobj_get_store(e_ob, type, store_idx);
    if (store->max_num_stored_bufs > 0 && new_size > store->max_num_stored_bufs)
        new_size = store->max_num_stored_bufs;
    
    if (!store->stored_buf && new_size > 0) { // initialized at earsbufobj_setup()
        store->stored_buf = (t_earsbufobj_stored_buffer *)bach_newptrclear(new_size * sizeof(t_earsbufobj_stored_buffer));
        for (i = store->num_stored_bufs; i < new_size; i++) {
            store->stored_buf[i].l_buf = NULL;
            store->stored_buf[i].l_name = NULL;
            store->stored_buf[i].l_status = EARSBUFOBJ_BUFSTATUS_NONE;
        }
        store->num_stored_bufs = new_size;
    } else if (new_size > store->num_stored_bufs) {
        long old_num_bufs = store->num_stored_bufs;
        store->stored_buf = (t_earsbufobj_stored_buffer *)bach_resizeptr(store->stored_buf, new_size * sizeof(t_earsbufobj_stored_buffer));
        for (i = old_num_bufs; i < new_size; i++) {
            store->stored_buf[i].l_buf = NULL;
            store->stored_buf[i].l_name = NULL;
            store->stored_buf[i].l_status = EARSBUFOBJ_BUFSTATUS_COPIED;
        }
        store->num_stored_bufs = new_size;
        if (!(type == EARSBUFOBJ_IN && (!(e_ob->l_flags & EARSBUFOBJ_FLAG_DUPLICATE_INPUT_BUFFERS))))
            if (also_create_unique_buffers) {
                for (i = old_num_bufs; i < new_size; i++) {
                    t_symbol *s = NULL;
                    store->stored_buf[i].l_status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
                    if (type == EARSBUFOBJ_OUT) {
                        s = earsbufobj_output_get_symbol_unique(e_ob, store_idx, i, &store->stored_buf[i].l_status);
                    } else
                        s = symbol_unique();
                    if (s)
                        earsbufobj_buffer_link(e_ob, type, store_idx, i, s);
                }
            }
    } else if (new_size < store->num_stored_bufs) {
        store->stored_buf = (t_earsbufobj_stored_buffer *)bach_resizeptr(store->stored_buf, MAX(1, new_size) * sizeof(t_earsbufobj_stored_buffer));
        store->num_stored_bufs = new_size;
    }
}

long llll_get_num_symbols_root(t_llll *ll)
{
    long count = 0;
    for (t_llllelem *elem = ll->l_head; elem; elem = elem->l_next) {
        if (hatom_gettype(&elem->l_hatom) == H_SYM) {
            count++;
        }
    }
    return count;
}

long earsbufobj_store_buffer_list(t_earsbufobj *e_ob, t_llll *buffers, long store_idx)
{
    long count = 0;
    for (t_llllelem *elem = buffers->l_head; elem; elem = elem->l_next) {
        if (hatom_gettype(&elem->l_hatom) == H_SYM) {
            // storing input buffer
            earsbufobj_store_buffer(e_ob, EARSBUFOBJ_IN, store_idx, count, hatom_getsym(&elem->l_hatom));
            count++;
        } else {
            object_warn((t_object *)e_ob, "Non-symbolic entry found; ignored.");
            /*
            // empty buffer will do.
            char *txtbuf = NULL;
            hatom_to_text_buf(&elem->l_hatom, &txtbuf);
            object_warn((t_object *)e_ob, "No buffer %s found; empty buffer created.", txtbuf);
            earsbufobj_store_empty_buffer(e_ob, EARSBUFOBJ_IN, store_idx, count);
            bach_freeptr(txtbuf);
             */
        }
    }
    return count;
}

long substitute_polybuffers(t_llll *ll)
{
    if (!ll)
        return 1;
    
    t_llll_stack *stack = llll_stack_new();
    t_llllelem *el = ll->l_head;
    while (1) {
        while (el) {
            t_llllelem *this_el = el;
            bool must_delete_el = false;
            
            switch (el->l_hatom.h_type) {
                case H_SYM:
                {
                    t_symbol *s = hatom_getsym(&el->l_hatom);
                    t_object *pbuf = ears_polybuffer_getobject(s);
                    if (pbuf) {
                        long count = object_attr_getlong(pbuf, _sym_count);
                        for (long i = 1; i <= count; i++){
                            llll_insertsym_before(ears_buffer_name_get_for_polybuffer(s, i), el);
                        }
                        must_delete_el = true;
                    }
                    el = el->l_next;
                }
                    break;
                    
                case H_LLLL:
                {
                    t_llll *ll = hatom_getllll(&el->l_hatom);
                    llll_stack_push(stack, el->l_next);
                    el = ll->l_head;
                }
                    break;
                    
                default:
                    el = el->l_next;
                    break;
            }
            
            if (must_delete_el) {
                llll_destroyelem(this_el);
                must_delete_el = false;
            }
        }
        
        if (!stack->s_items)
            break;
        
        el = (t_llllelem *) llll_stack_pop(stack);
    }
    llll_stack_destroy(stack);

    return 0;
}


t_llll *earsbufobj_parse_gimme(t_earsbufobj *e_ob, e_llllobj_obj_types type, t_symbol *msg, long ac, t_atom *av)
{
    t_llll *ll = llllobj_parse_llll((t_object *) e_ob, type, msg, ac, av, LLLL_PARSE_CLONE);

    // checking whether any of the symbols is a POLYbuffer, and in this case substituting it with all its buffers
    substitute_polybuffers(ll);

    return ll;
}

char llll_contains_only_symbols_and_at_least_one(t_llll *ll)
{
    if (!ll->l_head)
        return 0;
    
    for (t_llllelem *el = ll->l_head; el; el = el->l_next)
        if (hatom_gettype(&el->l_hatom) != H_SYM)
            return 0;
    return 1;
}

char llll_is_fit_for_being_names(t_llll *ll)
{
    if (!ll->l_head)
        return 0;
    
    if (ll->l_depth <= 1 && llll_contains_only_symbols_and_at_least_one(ll))
        return 1;
    
    if (ll->l_depth == 2) {
        for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
            if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                if (!llll_contains_only_symbols_and_at_least_one(hatom_getllll(&el->l_hatom)))
                    return 0;
            } else if (hatom_gettype(&el->l_hatom) != H_SYM) {
                return 0;
            }
        }
        return 1;
    }
    
    return 0;
}

// destructive on args
t_llll *earsbufobj_extract_names_from_args(t_earsbufobj *e_ob, t_llll *args, char assign_naming_policy)
{
    t_llll *names = NULL;
    if (args) {
        if (assign_naming_policy) {
            if (args->l_head && hatom_gettype(&args->l_head->l_hatom) == H_SYM) {
                t_symbol *s = hatom_getsym(&args->l_head->l_hatom);
                t_atom av;
                if (s == gensym("=")) {
                    atom_setsym(&av, gensym("copy"));
                    earsbufobj_setattr_naming(e_ob, NULL, 1, &av);
                    llll_behead(args);
                }
                if (s == gensym("!")) {
                    atom_setsym(&av, gensym("dynamic"));
                    earsbufobj_setattr_naming(e_ob, NULL, 1, &av);
                    llll_behead(args);
                }
                if (s == gensym("-")) {
                    atom_setsym(&av, gensym("static"));
                    earsbufobj_setattr_naming(e_ob, NULL, 1, &av);
                    llll_behead(args);
                }
            }
        }
        if (args->l_head) {
            if (hatom_gettype(&args->l_head->l_hatom) == H_LLLL) {
                if (llll_is_fit_for_being_names(hatom_getllll(&args->l_head->l_hatom))) {
                    names = llll_get();
                    llll_appendhatom_clone(names, &args->l_head->l_hatom);
                    llll_behead(args);
                }
            } else if (hatom_gettype(&args->l_head->l_hatom) == H_SYM) {
                names = symbol2llll(hatom_getsym(&args->l_head->l_hatom));
                llll_behead(args);
            }
        }
    }
    
    return names;
}

void earsbufobj_init(t_earsbufobj *e_ob, long flags)
{
    ears_hashtabs_setup();

    e_ob->l_is_creating = 1;

    /// attributes (these must be done before the actual initialization)
    e_ob->l_ampunit = EARS_AMPUNIT_LINEAR;
    e_ob->l_timeunit = EARS_TIMEUNIT_MS;
    e_ob->l_antimeunit = EARS_TIMEUNIT_SAMPS;
    e_ob->l_envampunit = EARS_AMPUNIT_LINEAR;
    e_ob->l_envtimeunit = EARS_TIMEUNIT_DURATION_RATIO;
    e_ob->l_pitchunit = EARS_PITCHUNIT_CENTS;
    e_ob->l_frequnit = EARS_FREQUNIT_HERTZ;
    e_ob->l_angleunit = EARS_ANGLEUNIT_RADIANS;
    e_ob->l_bufouts_naming = EARSBUFOBJ_NAMING_STATIC;
    e_ob->l_blocking = EARSBUFOBJ_BLOCKING_MAINTHREAD;
    
    e_ob->l_resamplingpolicy = EARS_RESAMPLINGPOLICY_TOMOSTCOMMONSR;
    e_ob->l_resamplingfilterwidth = EARS_DEFAULT_RESAMPLING_WINDOW_WIDTH;
    
    e_ob->a_framesize = 2048;
    e_ob->a_hopsize = 1024;
    e_ob->a_lastframetoendoffile = 0;
    e_ob->a_winstartfromzero = 0;
    atom_setsym(&e_ob->a_numframes, _llllobj_sym_auto);
    e_ob->a_overlap = 2.;
    e_ob->a_wintype = gensym("hann");
    e_ob->a_winnorm = 1;
    e_ob->a_zeropadding = 0;
    e_ob->a_zerophase = true;

    
    e_ob->l_slopemapping = k_SLOPE_MAPPING_BACH;

    systhread_mutex_new_debug(&e_ob->l_mutex, 0);
    
    e_ob->l_outnames = llll_get();
    e_ob->l_generated_outnames = llll_get();
    e_ob->l_flags = flags;
    
    e_ob->l_numins = 0;
    e_ob->l_numbufins = 0;
    e_ob->l_numbufouts = 0;
}


void earsbufobj_setup(t_earsbufobj *e_ob, const char *in_types, const char *out_types, t_llll *outlet_names)
{
    bool output_polybuffers = e_ob->l_output_polybuffers;
    long i, j, h;

    // INLETS;
    long max_in_len = strlen(in_types);
    e_ob->l_numins = 0;
    e_ob->l_numbufins = 0;
    
    for (long i = 0; i < LLLL_MAX_INLETS; i++)
        e_ob->l_inlet_hot[i] = false;
    e_ob->l_inlet_hot[0] = true;
    
    e_ob->l_proxy = (void **) bach_newptr((max_in_len + 1) * sizeof (void *));
    
    for (i = max_in_len - 1; i > 0; i--)
        e_ob->l_proxy[i] = proxy_new_debug((t_object *) e_ob, i, &e_ob->l_in);
    for (i = 0; i < max_in_len; i++) {
        e_ob->l_numins ++;
        if (in_types[i] == 'e' || in_types[i] == 'E')
            e_ob->l_numbufins++;
    }
    if (e_ob->l_numbufins) {
        e_ob->l_instore = (t_earsbufobj_store *) bach_newptrclear(e_ob->l_numbufins * sizeof(t_earsbufobj_store));
        for (i = 0, j = 0; i < max_in_len; i++) {
            if (in_types[i] == 'e' || in_types[i] == 'E') {
                if (in_types[i] == 'e') {
                    e_ob->l_instore[j].max_num_stored_bufs = 1;
                } else if (in_types[i] == 'E') {
                    e_ob->l_instore[j].max_num_stored_bufs = 0; // no limit
                }
//                earsbufobj_resize_store(e_ob, EARSBUFOBJ_IN, j, 1, false);
//                e_ob->l_instore[j].stored_buf[0].l_status = EARSBUFOBJ_BUFSTATUS_COPIED;
                if (e_ob->l_flags & EARSBUFOBJ_FLAG_DUPLICATE_INPUT_BUFFERS) { // this is currently never used
                    earsbufobj_resize_store(e_ob, EARSBUFOBJ_IN, j, 1, false);
                    e_ob->l_instore[j].stored_buf[0].l_status = EARSBUFOBJ_BUFSTATUS_COPIED;
                    e_ob->l_instore[j].stored_buf[0].l_status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
                    earsbufobj_buffer_link(e_ob, EARSBUFOBJ_IN, j, 0, symbol_unique());
                }
                j++;
            }
        }
    }
    
    // OUTLETS
    long max_out_len = strlen(out_types);
//    e_ob->l_numouts = 0;
    e_ob->l_numbufouts = 0;
    
    // count number of input lllls
    long num_lllls_in = 0;
    for (i = 0; i < max_in_len; i++)
        num_lllls_in += (in_types[i] == '4');

    // count number of output lllls
    long num_lllls_out = 0;
    for (i = 0; i < MIN(LLLL_MAX_OUTLETS, max_out_len); i++)
        num_lllls_out += (out_types[i] == '4');

    char out_types_wk[LLLL_MAX_OUTLETS];
    strncpy_zero(out_types_wk, out_types, MAX(strlen(out_types) + 1, LLLL_MAX_OUTLETS));
    for (i = 0; i < strlen(out_types_wk) && i < LLLL_MAX_OUTLETS; i++)
        if (out_types_wk[i] == 'e' || out_types_wk[i] == 'E')
            out_types_wk[i] = 'z';
    
    long count_a = 0;
    for (int i = 0; i < strlen(out_types_wk); i++)
        if (out_types_wk[i] == 'a') count_a++;
    
    // dirty, just working UP TO 2 'a' outlets (TO BE REFINED)
    if (count_a == 0)
        llllobj_obj_setup((t_llllobj_object *)e_ob, num_lllls_in, out_types_wk);
    else if (count_a == 1)
        llllobj_obj_setup((t_llllobj_object *)e_ob, num_lllls_in, out_types_wk, NULL);
    else if (count_a == 2)
        llllobj_obj_setup((t_llllobj_object *)e_ob, num_lllls_in, out_types_wk, NULL, NULL);
    else
        object_error((t_object *)e_ob, "Unimplemented feature.");

    /*
    e_ob->l_outlet = (void **) bach_newptr((max_out_len + 1) * sizeof (void *));
    for (i = MIN(LLLL_MAX_OUTLETS, max_out_len) - 1; i >= 0; i--) {
        e_ob->l_numouts ++;
        switch (out_types[i]) {
            case 'b':
                e_ob->l_outlet[i] = bangout((t_object *)e_ob);
                break;
            case 'i':
                e_ob->l_outlet[i] = intout((t_object *)e_ob);
                break;
            case 'f':
                e_ob->l_outlet[i] = floatout((t_object *)e_ob);
                break;
            case 'l':
                e_ob->l_outlet[i] = listout((t_object *)e_ob);
                break;
            case 'e':
            case 'E':
            default:
                e_ob->l_outlet[i] = outlet_new((t_object *)e_ob, NULL);
                break;
        }
    } */

    for (i = 0; i < MIN(LLLL_MAX_OUTLETS, max_out_len); i++) {
        e_ob->l_outlet_types[i] = out_types[MIN(LLLL_MAX_OUTLETS, max_out_len) - i - 1];
    }

    for (i = 0; i < MIN(LLLL_MAX_OUTLETS, max_out_len); i++) {
        if (out_types[i] == 'e' || out_types[i] == 'E')
            e_ob->l_numbufouts ++;
    }
    if (e_ob->l_numbufouts) {
        e_ob->l_outstore = (t_earsbufobj_store *) bach_newptrclear(e_ob->l_numbufouts * sizeof(t_earsbufobj_store));
        if (outlet_names && outlet_names->l_depth == 3 && outlet_names->l_size == 1 && hatom_gettype(&outlet_names->l_head->l_hatom) == H_LLLL) // names are assigned to multiple outlets
            outlet_names = hatom_getllll(&outlet_names->l_head->l_hatom);
        t_llllelem *elem;
        for (i = 0, j = 0, elem = (outlet_names ? outlet_names->l_head : NULL); i < max_out_len; i++) {
            if (out_types[i] == 'e' || out_types[i] == 'E') {
                long outstoresize = 1;
                if (out_types[i] == 'e') {
                    e_ob->l_outstore[j].max_num_stored_bufs = 1;
                } else if (out_types[i] == 'E') { // 'E' buffer outlets can output a list of buffers from a single outlet
                    e_ob->l_outstore[j].max_num_stored_bufs = 0; // no limit
                    outstoresize = MAX(1, elem && hatom_gettype(&elem->l_hatom) == H_LLLL ? hatom_getllll(&elem->l_hatom)->l_size : 1);
                    
                    if (output_polybuffers) {
                        e_ob->l_outstore[j].use_polybuffers = true;
                        if (elem && hatom_gettype(&elem->l_hatom) == H_SYM) {
                            t_symbol *s = hatom_getsym(&elem->l_hatom);
                            e_ob->l_outstore[j].polybuffer_name = s;
                            e_ob->l_outstore[j].polybuffer_status = EARSBUFOBJ_BUFSTATUS_USERNAMED;
                            if (!ears_polybuffer_symbol_is_polybuffer(s)) {
                                ears_polybuffer_make(s, true);
                            } else {
                                ears_polybuffer_retain(ears_polybuffer_getobject(s), s);
                            }
                        } else {
                            t_symbol *s = symbol_unique();
                            e_ob->l_outstore[j].polybuffer_status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
                            e_ob->l_outstore[j].polybuffer_name = s;
                            ears_polybuffer_make(s, true);
                        }
                    }
                }
                earsbufobj_resize_store(e_ob, EARSBUFOBJ_OUT, j, outstoresize, false);
                t_llllelem *subelem = (elem && hatom_gettype(&elem->l_hatom) == H_LLLL) ? hatom_getllll(&elem->l_hatom)->l_head : NULL;
                for (h = 0; h < outstoresize; h++) {
                    t_symbol *name = NULL;
                    if (e_ob->l_outstore[j].use_polybuffers) {
                        name = earsbufobj_output_get_symbol_unique(e_ob, j, h, &e_ob->l_outstore[j].stored_buf[h].l_status);
                        e_ob->l_outstore[j].stored_buf[h].l_status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED; // e_ob->l_outstore[j].polybuffer_status;
                    } else if (subelem && hatom_gettype(&subelem->l_hatom) == H_SYM) {
                        name = hatom_getsym(&subelem->l_hatom);
                        e_ob->l_outstore[j].stored_buf[h].l_status = EARSBUFOBJ_BUFSTATUS_USERNAMED;
                    } else if (outstoresize == 1 && elem && hatom_gettype(&elem->l_hatom) == H_SYM) {
                        name = hatom_getsym(&elem->l_hatom);
                        e_ob->l_outstore[j].stored_buf[h].l_status = EARSBUFOBJ_BUFSTATUS_USERNAMED;
                    } else {
                        if (e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_COPY) {
                            e_ob->l_outstore[j].stored_buf[h].l_status = EARSBUFOBJ_BUFSTATUS_COPIED;
                        } else {
                            name = earsbufobj_output_get_symbol_unique(e_ob, j, h, &e_ob->l_outstore[j].stored_buf[h].l_status);
                        }
                    }
                    if (name)
                        earsbufobj_buffer_link(e_ob, EARSBUFOBJ_OUT, j, h, name);
                    subelem = (subelem ? subelem->l_next : NULL);
                }
                
                if (out_types[i] == 'e') {
                    llll_appendsym(e_ob->l_outnames, e_ob->l_outstore[j].stored_buf[0].l_name);
                } else {
                    t_llll *ll = llll_get();
                    for (h = 0; h < outstoresize; h++) {
                        if (e_ob->l_outstore[j].stored_buf[h].l_name)
                            llll_appendsym(ll, e_ob->l_outstore[j].stored_buf[h].l_name);
                        else
                            llll_appendsym(ll, _sym_none);
                    }
                    llll_appendllll(e_ob->l_outnames, ll);
                }
                
                elem = (elem ? elem->l_next : NULL);
                j++;
            }
        }
    }
    if (e_ob->l_output_polybuffers > 0 && e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_DYNAMIC) {
        object_warn((t_object *)e_ob, "Polybuffer output is incompatible with dynamic naming.");
        object_warn((t_object *)e_ob, "    Switching to ordinary buffer output.");
    }
    object_attr_setdisabled((t_object *)e_ob, gensym("blocking"), 1);
    object_attr_setdisabled((t_object *)e_ob, gensym("polyout"), 1);
    e_ob->l_is_creating = 0;
}


void earsbufobj_init_and_setup(t_earsbufobj *e_ob, const char *in_types, const char *out_types, t_llll *outlet_names, long flags)
{
    earsbufobj_init(e_ob, flags);
    earsbufobj_setup(e_ob, in_types, out_types, outlet_names);
}


DEFINE_LLLL_ATTR_DEFAULT_GETTER(t_earsbufobj, l_outnames, earsbufobj_getattr_outname);

t_max_err earsbufobj_setattr_outname(t_earsbufobj *e_ob, t_object *attr, long argc, t_atom *argv)
{
    t_llll *ll = llll_parse(argc, argv);
    t_llllelem *elem;
    long i, j;
    llll_clear(e_ob->l_outnames);
    for (i = 0, elem = (ll ? ll->l_head : NULL); i < e_ob->l_numbufouts; i++, elem = (elem ? elem->l_next : NULL)) {
        if (e_ob->l_outstore[i].max_num_stored_bufs == 1) {
            t_symbol *s = NULL;
            e_ob->l_outstore[i].stored_buf[0].l_status = EARSBUFOBJ_BUFSTATUS_USERNAMED;
            if (elem && hatom_gettype(&elem->l_hatom) == H_SYM)
                s = hatom_getsym(&elem->l_hatom);
            else if (elem && hatom_gettype(&elem->l_hatom) == H_LLLL && hatom_getllll(&elem->l_hatom)->l_head && hatom_gettype(&hatom_getllll(&elem->l_hatom)->l_head->l_hatom) == H_SYM)
                s = hatom_getsym(&hatom_getllll(&elem->l_hatom)->l_head->l_hatom);
            else {
                e_ob->l_outstore[i].stored_buf[0].l_status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
                s = symbol_unique();
            }

            earsbufobj_buffer_link(e_ob, EARSBUFOBJ_OUT, i, 0, s);
            llll_appendsym(e_ob->l_outnames, e_ob->l_outstore[i].stored_buf[0].l_name);
        } else {
            long num_stored_bufs = 1;
            if (elem && hatom_gettype(&elem->l_hatom) == H_LLLL)
                num_stored_bufs = MAX(num_stored_bufs, hatom_getllll(&elem->l_hatom)->l_size);
            earsbufobj_resize_store(e_ob, EARSBUFOBJ_OUT, i, num_stored_bufs, false); // TO BE CHECKED
            
            t_llll *outnames_ll = llll_get();
            t_symbol *s = NULL;
            if (elem && hatom_gettype(&elem->l_hatom) == H_LLLL) {
                if (hatom_getllll(&elem->l_hatom)->l_size >= 1) {
                    t_llllelem *subelem;
                    for (j = 0, subelem = hatom_getllll(&elem->l_hatom)->l_head; subelem && j < e_ob->l_outstore[i].num_stored_bufs; subelem = subelem->l_next, j++) {
                        if (subelem && hatom_gettype(&subelem->l_hatom) == H_SYM) {
                            e_ob->l_outstore[i].stored_buf[j].l_status = EARSBUFOBJ_BUFSTATUS_USERNAMED;
                            s = hatom_getsym(&subelem->l_hatom);
                        } else {
                            e_ob->l_outstore[i].stored_buf[j].l_status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
                            s = symbol_unique();
                        }
                        earsbufobj_buffer_link(e_ob, EARSBUFOBJ_OUT, i, j, s);
                        llll_appendsym(outnames_ll, e_ob->l_outstore[i].stored_buf[j].l_name);
                    }
                } else {
                    e_ob->l_outstore[i].stored_buf[0].l_status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
                    earsbufobj_buffer_link(e_ob, EARSBUFOBJ_OUT, i, 0, symbol_unique());
                    llll_appendsym(outnames_ll, e_ob->l_outstore[i].stored_buf[0].l_name);
                }
            } else {
                if (e_ob->l_outstore[j].use_polybuffers) {
                    if (elem && hatom_gettype(&elem->l_hatom) == H_SYM) {
                        t_symbol *s = hatom_getsym(&elem->l_hatom);
                        e_ob->l_outstore[j].polybuffer_name = s;
                        e_ob->l_outstore[j].polybuffer_status = EARSBUFOBJ_BUFSTATUS_USERNAMED;
                        if (!ears_polybuffer_symbol_is_polybuffer(s)) {
                            ears_polybuffer_make(s, true);
                        } else {
                            ears_polybuffer_retain(ears_polybuffer_getobject(s), s);
                        }
                    } else {
                        t_symbol *s = symbol_unique();
                        e_ob->l_outstore[j].polybuffer_status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
                        e_ob->l_outstore[j].polybuffer_name = s;
                        ears_polybuffer_make(s, true);
                        s = ears_buffer_name_get_for_polybuffer(e_ob->l_outstore[j].polybuffer_name, 1);
                    }
                } else if (elem && hatom_gettype(&elem->l_hatom) == H_SYM) {
                    e_ob->l_outstore[i].stored_buf[0].l_status = EARSBUFOBJ_BUFSTATUS_USERNAMED;
                    s = hatom_getsym(&elem->l_hatom);
                } else {
                    e_ob->l_outstore[i].stored_buf[0].l_status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
                    s = symbol_unique();
                }
                earsbufobj_buffer_link(e_ob, EARSBUFOBJ_OUT, i, 0, s);
                llll_appendsym(outnames_ll, e_ob->l_outstore[i].stored_buf[0].l_name);
            }
            llll_appendllll(e_ob->l_outnames, outnames_ll);
        }
    }
    
    llll_free(ll);
    return MAX_ERR_NONE;
}


bool earsbufobj_is_buf_autoassigned(t_earsbufobj *e_ob, e_earsbufobj_in_out where, long store, long bufferidx)
{
    long i = store, j = bufferidx;
    switch (where) {
        case EARSBUFOBJ_IN:
            return e_ob->l_instore[i].stored_buf[j].l_status == EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
            break;

        case EARSBUFOBJ_OUT:
            return e_ob->l_outstore[i].stored_buf[j].l_status == EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
            break;

        default:
            return false;
            break;
    }
}

bool earsbufobj_is_buf_autoassigned_or_copied(t_earsbufobj *e_ob, e_earsbufobj_in_out where, long store, long bufferidx)
{
    long i = store, j = bufferidx;
    switch (where) {
        case EARSBUFOBJ_IN:
            return (e_ob->l_instore[i].stored_buf[j].l_status == EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED ||
                    e_ob->l_instore[i].stored_buf[j].l_status == EARSBUFOBJ_BUFSTATUS_COPIED);
            break;
            
        case EARSBUFOBJ_OUT:
            return (e_ob->l_outstore[i].stored_buf[j].l_status == EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED ||
                    e_ob->l_outstore[i].stored_buf[j].l_status == EARSBUFOBJ_BUFSTATUS_COPIED);
            break;
            
        default:
            return false;
            break;
    }
}

void earsbufobj_free(t_earsbufobj *e_ob)
{
    long i, j;
    
    ears_is_freeing = true;
    
    for (i = e_ob->l_numins - 1; i > 0; i--)
        object_free_debug(e_ob->l_proxy[i]);
    bach_freeptr(e_ob->l_proxy);

    // separate thread running?
    if (e_ob->l_thread) {
        unsigned int rv;
        systhread_join(e_ob->l_thread, &rv);
        // thread is exited
    }
    
    // deleting references
    for (i = 0; i < e_ob->l_numbufins; i++) {
        for (j = 0; j < e_ob->l_instore[i].num_stored_bufs; j++) {
            if (e_ob->l_instore[i].stored_buf[j].l_name && e_ob->l_instore[i].stored_buf[j].l_buf &&
                e_ob->l_instore[i].stored_buf[j].l_status == EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED)
                earsbufobj_buffer_release(e_ob, EARSBUFOBJ_IN, i, j);
        }
        bach_freeptr(e_ob->l_instore[i].stored_buf);
    }
    bach_freeptr(e_ob->l_instore);

    for (i = 0; i < e_ob->l_numbufouts; i++) {
        if (e_ob->l_outstore[i].use_polybuffers) {
            earsbufobj_polybuffer_release(e_ob, EARSBUFOBJ_OUT, i);
        } else {
            for (j = 0; j < e_ob->l_outstore[i].num_stored_bufs; j++) {
                if (e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_DYNAMIC) {
                    earsbufobj_release_generated_outnames(e_ob);
                } else {
                    if (e_ob->l_outstore[i].stored_buf[j].l_name && e_ob->l_outstore[i].stored_buf[j].l_buf)
                        earsbufobj_buffer_release(e_ob, EARSBUFOBJ_OUT, i, j);
                }
            }
        }
        bach_freeptr(e_ob->l_outstore[i].stored_buf);
    }
    
    ears_is_freeing = false;

    bach_freeptr(e_ob->l_outstore);
//    bach_freeptr(e_ob->l_outlet);
    
    llll_free(e_ob->l_outnames);
    llll_free(e_ob->l_generated_outnames);
    
    if (e_ob->l_mutex)
        systhread_mutex_free_debug(e_ob->l_mutex);

}

void bsc(t_earsbufobj *e_ob, t_symbol *s, long ac, t_atom *av)
{
//    post("buffer size changed!");
    e_ob->l_buffer_size_changed = 1;
}

t_max_err earsbufobj_notify(t_earsbufobj *e_ob, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    if (msg == gensym("buffer_modified")) {
//        post("buffer modified");
        switch (e_ob->l_blocking) {
            case EARSBUFOBJ_BLOCKING_CALLINGTHREAD:
                defer(e_ob, (method)bsc, NULL, 0, NULL);
                break;
                
            case EARSBUFOBJ_BLOCKING_OWNTHREAD:
                defer(e_ob, (method)bsc, NULL, 0, NULL);
//                schedule_defer(e_ob, (method)bsc, 1000, NULL, 0, NULL);
                break;

            default:
            case EARSBUFOBJ_BLOCKING_MAINTHREAD:
                break;
        }
    }

    return MAX_ERR_NONE;
}


long earsbufobj_get_num_stored_buffers(t_earsbufobj *e_ob, e_earsbufobj_in_out where, bool ignore_polybuffers)
{
    long i, sum = 0;
    switch (where) {
        case EARSBUFOBJ_IN:
            for (i = 0; i < e_ob->l_numbufins; i++)
                sum += e_ob->l_instore[i].num_stored_bufs;
            break;
            
        case EARSBUFOBJ_OUT:
            for (i = 0; i < e_ob->l_numbufouts; i++) {
                if (!(ignore_polybuffers && e_ob->l_outstore[i].use_polybuffers))
                    sum += e_ob->l_outstore[i].num_stored_bufs;
            }
            break;
            
        default:
            break;
    }
    return sum;
}


void earsbufobj_open(t_earsbufobj *e_ob)
{
    long sum = earsbufobj_get_num_stored_buffers(e_ob, EARSBUFOBJ_OUT, true);
    if (sum > EARS_MAX_BUFFERS_SHOWN_ON_DOUBLECLICK)
        object_warn((t_object *)e_ob, "More than %ld buffers stored as output. Only first %ld buffers shown", EARS_MAX_BUFFERS_SHOWN_ON_DOUBLECLICK, EARS_MAX_BUFFERS_SHOWN_ON_DOUBLECLICK);
    
    long i, j, s = 0;
    for (i = 0; i < e_ob->l_numbufouts && s < EARS_MAX_BUFFERS_SHOWN_ON_DOUBLECLICK; i++) {
        if (e_ob->l_outstore[i].use_polybuffers) {
            t_object *polybuffer = ears_polybuffer_getobject(e_ob->l_outstore[i].polybuffer_name);
            object_method(polybuffer, gensym("open"));
        } else  {
            for (j = 0; j < e_ob->l_outstore[i].num_stored_bufs && s < EARS_MAX_BUFFERS_SHOWN_ON_DOUBLECLICK; j++, s++) {
                //            t_symbol *fee = gensym("fee");
                buffer_view(earsbufobj_get_outlet_buffer_obj(e_ob, i, j));
            }
        }
    }
}


// this lets us double-click on index~ to open up the buffer~ it references
void earsbufobj_dblclick(t_earsbufobj *e_ob)
{
    earsbufobj_open(e_ob);
}



void earsbufobj_write_buffer(t_earsbufobj *e_ob, t_object *buf, t_symbol *msg, t_symbol *exportpath, t_symbol *format)
{
    t_symbol *orig_format = NULL;
    if (format) {
        orig_format = ears_buffer_get_sampleformat((t_object *)e_ob, buf);
        if (orig_format != format) {
            ears_buffer_set_sampleformat((t_object *)e_ob, buf, format);
            // need to plug it back later
        } else
            orig_format = NULL; // don't care later
    }
    
    if (!exportpath) {
        typedmess(buf, msg, 0, NULL);
    } else {
        t_atom av;
        atom_setsym(&av, exportpath);
        typedmess(buf, msg, 1, &av);
    }
    
    if (orig_format)
        ears_buffer_set_sampleformat((t_object *)e_ob, buf, orig_format);
}

void fill_encoding_settings(t_object *ob, t_llll *args, t_ears_encoding_settings *settings)
{
    // parsing args
    t_symbol *vbrmode = gensym("vbr"), *format = NULL;
    long bitrate = 0, bitrate_min = 0, bitrate_max = 0, correction = 0;
    llll_parseattrs(ob, args, true, "siiisi",
                    gensym("vbrmode"), &vbrmode,
                    gensym("bitrate"), &bitrate,
                    gensym("minbitrate"), &bitrate_min,
                    gensym("maxbitrate"), &bitrate_max,
                    gensym("format"), &format,
                    gensym("correction"), &correction);
    
    if (vbrmode == gensym("ABR") || vbrmode == gensym("abr"))
        settings->vbr_type = EARS_MP3_VBRMODE_ABR;
    else if (vbrmode == gensym("CBR") || vbrmode == gensym("cbr"))
        settings->vbr_type = EARS_MP3_VBRMODE_CBR;
    else
        settings->vbr_type = EARS_MP3_VBRMODE_VBR;

    settings->bitrate = bitrate;
    settings->bitrate_min = bitrate_min;
    settings->bitrate_max = bitrate_max;
    settings->format = format;
    settings->use_correction_file = correction;
}



// this lets us save the buffer
void earsbufobj_writegeneral(t_earsbufobj *e_ob, t_symbol *msg, long ac, t_atom *av)
{
    // parse attrs
    t_llll *parsed = llll_parse(ac, av);
    
    t_ears_encoding_settings settings;
    fill_encoding_settings((t_object *)e_ob, parsed, &settings);
    
    t_buffer_obj *buf = NULL;
    long which_buffer = 0;
    
    if (ac && atom_gettype(av) == A_LONG) {
        which_buffer = atom_getlong(av) - 1;
        ac--;
        av++;
    }
    

    
    if ((buf = earsbufobj_get_outlet_buffer_obj(e_ob, 0, which_buffer))) {
        t_symbol *orig_format = NULL;
        if (settings.format) {
            orig_format = ears_buffer_get_sampleformat((t_object *)e_ob, buf);
            if (orig_format != settings.format) {
                ears_buffer_set_sampleformat((t_object *)e_ob, buf, settings.format);
                // need to plug it back later
            } else
                orig_format = NULL; // don't care later
        }
        
        if (msg == gensym("writemp3")) {
            t_fourcc outtype;
            t_fourcc filetype = 'MPEG';
            t_symbol *outfilepath = NULL;
            if (parsed && parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == H_SYM)
                outfilepath = ears_ezresolve_file(hatom_getsym(&parsed->l_head->l_hatom), true, ".mp3");
            else
                ears_saveasdialog((t_object *)e_ob, "Untitled.mp3", &filetype, 1, &outtype, &outfilepath, true);
            
            if (outfilepath)
                ears_buffer_write(buf, outfilepath, (t_object *)e_ob, &settings);
            
        } else if (msg == gensym("writewavpack") || msg == gensym("writewv")) {
            t_fourcc outtype;
//            t_fourcc filetype = 'WAVE';
            t_symbol *outfilepath = NULL;
            if (parsed && parsed->l_head && hatom_gettype(&parsed->l_head->l_hatom) == H_SYM)
                outfilepath = ears_ezresolve_file(hatom_getsym(&parsed->l_head->l_hatom), true, ".wv");
            else
                ears_saveasdialog((t_object *)e_ob, "Untitled.wv", NULL, 0, &outtype, &outfilepath, true);
//                ears_saveasdialog((t_object *)e_ob, "Untitled.wv", &filetype, 1, &outtype, &outfilepath, true);
            
            if (outfilepath) {
                if (!settings.format)
                    settings.format = EARS_DEFAULT_WRITE_FORMAT;
                ears_buffer_write(buf, outfilepath, (t_object *)e_ob, &settings);
            }
            
        } else {
            // all other cases are handled natively via Max API
            if (parsed && parsed->l_size > 0) {
                t_atom *true_av = NULL;
                long true_ac = llll_deparse(parsed, &true_av, 0, 0);
                typedmess(buf, msg, true_ac, true_av);
                bach_freeptr(true_av);
            } else {
                typedmess(buf, msg, 0, NULL);
            }
        }
        
        if (orig_format)
            ears_buffer_set_sampleformat((t_object *)e_ob, buf, orig_format);
    }
    
    llll_free(parsed);
}


void earsbufobj_reset(t_earsbufobj *e_ob)
{
    for (long i = 0; i < LLLL_MAX_OUTLETS; i++)
        e_ob->l_generated_outname_count[i] = 0;
}

void earsbufobj_stop(t_earsbufobj *e_ob)
{
    e_ob->l_must_stop = 1;
}


void earsbufobj_add_common_methods(t_class *c, long flags)
{
    class_addmethod(c, (method)earsbufobj_notify, "notify", A_CANT, 0);
    if (!flags)
        class_addmethod(c, (method)earsbufobj_dblclick, "dblclick", A_CANT, 0);
    class_addmethod(c, (method)earsbufobj_reset, "reset", 0);
    class_addmethod(c, (method)earsbufobj_stop, "stop", 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "write", A_GIMME, 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "writeaiff", A_GIMME, 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "writewave", A_GIMME, 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "writeau", A_GIMME, 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "writeflac", A_GIMME, 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "writemp3", A_GIMME, 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "writewavpack", A_GIMME, 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "writewv", A_GIMME, 0);
    
    class_addmethod(c, (method)earsbufobj_open, "open", 0);

#ifdef EARS_FROMFILE_NATIVE_MP3_HANDLING
    ears_mpg123_init();
#endif
}

void earsbufobj_class_add_outname_attr(t_class *c)
{
    CLASS_ATTR_LLLL(c, "outname", 0, t_earsbufobj, l_outnames, earsbufobj_getattr_outname, earsbufobj_setattr_outname);
    CLASS_ATTR_STYLE_LABEL(c,"outname",0,"text","Output Buffer Names");
    CLASS_ATTR_BASIC(c, "outname", 0);
    CLASS_ATTR_CATEGORY(c, "outname", 0, "Behavior");
    // @description Sets the name for each one of the buffer outlets. Leave blank to auto-assign
    // unique names.
}

t_max_err earsbufobj_setattr_blocking(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (!e_ob->l_is_creating)
            object_error((t_object *)e_ob, "The blocking attribute can only be set in the object box.");
        else if (atom_gettype(argv) == A_LONG)
            e_ob->l_blocking = atom_getlong(argv);
    }
    return MAX_ERR_NONE;
}

void earsbufobj_class_add_blocking_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "blocking", 0, t_earsbufobj, l_blocking);
    CLASS_ATTR_STYLE_LABEL(c,"blocking",0,"enumindex","Blocking Mode");
    CLASS_ATTR_ENUMINDEX(c,"blocking", 0, "Non-Blocking Blocking (Low Priority) Blocking (High Priority)");
    CLASS_ATTR_BASIC(c, "blocking", 0);
    CLASS_ATTR_CATEGORY(c, "blocking", 0, "Behavior");
    CLASS_ATTR_ACCESSORS(c, "blocking", NULL, earsbufobj_setattr_blocking);
    // @description Sets the blocking mode, i.e. the thread to be used for computation: <br />
    // 0: the object uses its own separate thread; <br />
    // 1: the object uses the main thread (default); <br />
    // 2: the object uses its the scheduler thread. <br />
    // The <m>blocking</m> attribute is static: it can only be set in the object box at instantiation.
}


t_max_err earsbufobj_setattr_polyout(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (!e_ob->l_is_creating)
            object_error((t_object *)e_ob, "The polyout attribute can only be set in the object box.");
        else if (atom_gettype(argv) == A_LONG)
            e_ob->l_output_polybuffers = atom_getlong(argv);
    }
    return MAX_ERR_NONE;
}


void earsbufobj_class_add_polyout_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "polyout", 0, t_earsbufobj, l_output_polybuffers);
    CLASS_ATTR_STYLE_LABEL(c,"polyout",0,"enumindex","Output Polybuffers");
    CLASS_ATTR_ENUMINDEX(c,"polyout", 0, "Don't Yes (Single Symbol) Yes (Buffer List)");
    CLASS_ATTR_BASIC(c, "polyout", 0);
    CLASS_ATTR_ACCESSORS(c, "polyout", NULL, earsbufobj_setattr_polyout);
    CLASS_ATTR_CATEGORY(c, "polyout", 0, "Behavior");
    // @description Toggles the ability to output a <o>polybuffer~</o> instead of a list of buffers: <br />
    // - 0 (default) means that no polybuffer is created (individual buffers are output); <br />
    // - 1 means that a polybuffer is created and its name is output; <br />
    // - 2 means that a polybuffer is created and the individual names of its buffers are output.
}


t_max_err earsbufobj_setattr_ampunit(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            e_ob->l_ampunit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            e_ears_ampunit new_ampunit = ears_ampunit_from_symbol(s);
            if (new_ampunit != EARS_AMPUNIT_UNKNOWN)
                e_ob->l_ampunit = new_ampunit;
            else
                object_error((t_object *)e_ob, "Unknown amplitude unit!");
        }
    }
    return MAX_ERR_NONE;
}


void earsbufobj_class_add_ampunit_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "ampunit", 0, t_earsbufobj, l_ampunit);
    CLASS_ATTR_STYLE_LABEL(c,"ampunit",0,"enumindex","Amplitude Values Are In");
    CLASS_ATTR_ENUMINDEX(c,"ampunit", 0, "Linear Decibel");
    CLASS_ATTR_ACCESSORS(c, "ampunit", NULL, earsbufobj_setattr_ampunit);
    CLASS_ATTR_BASIC(c, "ampunit", 0);
    CLASS_ATTR_CATEGORY(c, "ampunit", 0, "Units");
    // @description Sets the unit for amplitudes: Linear (default) or Decibel (0dB corresponding to 1. in the linear scale).
}


t_max_err earsbufobj_setattr_envampunit(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            e_ob->l_envampunit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            e_ears_ampunit new_ampunit = ears_ampunit_from_symbol(s);
            if (new_ampunit != EARS_AMPUNIT_UNKNOWN)
                e_ob->l_envampunit = new_ampunit;
            else
                object_error((t_object *)e_ob, "Unknown amplitude unit!");
        }
    }
    return MAX_ERR_NONE;
}


void earsbufobj_class_add_envampunit_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "envampunit", 0, t_earsbufobj, l_envampunit);
    CLASS_ATTR_STYLE_LABEL(c,"envampunit",0,"enumindex","Envelope Amplitude Values Are In");
    CLASS_ATTR_ENUMINDEX(c,"envampunit", 0, "Linear Decibel");
    CLASS_ATTR_ACCESSORS(c, "envampunit", NULL, earsbufobj_setattr_envampunit);
    CLASS_ATTR_BASIC(c, "envampunit", 0);
    CLASS_ATTR_CATEGORY(c, "envampunit", 0, "Units");
    // @description Sets the unit for amplitudes inside envelopes: Linear (default) or Decibel (0dB corresponding to 1. in the linear scale).
}



t_max_err earsbufobj_setattr_timeunit(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            e_ob->l_timeunit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            e_ears_timeunit new_timeunit = ears_timeunit_from_symbol(s);
            if (new_timeunit != EARS_TIMEUNIT_UNKNOWN)
                e_ob->l_timeunit = new_timeunit;
            else
                object_error((t_object *)e_ob, "Unknown time unit!");
        }
    }
    return MAX_ERR_NONE;
}


void earsbufobj_class_add_timeunit_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "timeunit", 0, t_earsbufobj, l_timeunit);
    CLASS_ATTR_STYLE_LABEL(c,"timeunit",0,"enumindex","Time Values Are In");
    CLASS_ATTR_ENUMINDEX(c,"timeunit", 0, "Milliseconds Samples Relative");
    CLASS_ATTR_ACCESSORS(c, "timeunit", NULL, earsbufobj_setattr_timeunit);
    CLASS_ATTR_BASIC(c, "timeunit", 0);
    CLASS_ATTR_CATEGORY(c, "timeunit", 0, "Units");
    // @description Sets the unit for time values: Milliseconds, Samples, Relative (0. to 1. as a percentage of the buffer length).
    // The default is always Milliseconds except for the <o>ears.repeat~</o> module (Relative).
}


t_max_err earsbufobj_setattr_antimeunit(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            e_ob->l_antimeunit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            e_ears_timeunit new_timeunit = ears_timeunit_from_symbol(s);
            if (new_timeunit != EARS_TIMEUNIT_UNKNOWN)
                e_ob->l_antimeunit = new_timeunit;
            else
                object_error((t_object *)e_ob, "Unknown time unit!");
        }
    }
    return MAX_ERR_NONE;
}

void earsbufobj_class_add_antimeunit_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "antimeunit", 0, t_earsbufobj, l_antimeunit);
    CLASS_ATTR_STYLE_LABEL(c,"antimeunit",0,"enumindex","Analysis Time Values Are In");
    CLASS_ATTR_ENUMINDEX(c,"antimeunit", 0, "Milliseconds Samples Relative");
    CLASS_ATTR_ACCESSORS(c, "antimeunit", NULL, earsbufobj_setattr_antimeunit);
    CLASS_ATTR_BASIC(c, "antimeunit", 0);
    CLASS_ATTR_CATEGORY(c, "antimeunit", 0, "Units");
    // @description Sets the unit for time values: Milliseconds, Samples, Relative (0. to 1. as a percentage of the buffer length).
    // The default is always Milliseconds except for the <o>ears.repeat~</o> module (Relative).
}

t_max_err earsbufobj_setattr_pitchunit(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            e_ob->l_pitchunit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            e_ears_pitchunit new_pitchunit = ears_pitchunit_from_symbol(atom_getsym(argv));
            if (new_pitchunit != EARS_PITCHUNIT_UNKNOWN)
                e_ob->l_pitchunit = new_pitchunit;
            else
                object_error((t_object *)e_ob, "Unknown pitch unit!");
        }
    }
    return MAX_ERR_NONE;
}


                     
void earsbufobj_class_add_pitchunit_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "pitchunit", 0, t_earsbufobj, l_pitchunit);
    CLASS_ATTR_STYLE_LABEL(c,"pitchunit",0,"enumindex","Pitch Values Are In");
    CLASS_ATTR_ENUMINDEX(c,"pitchunit", 0, "Cents MIDI Hertz Frequency Ratio");
    CLASS_ATTR_ACCESSORS(c, "pitchunit", NULL, earsbufobj_setattr_pitchunit);
    CLASS_ATTR_BASIC(c, "pitchunit", 0);
    CLASS_ATTR_CATEGORY(c, "pitchunit", 0, "Units");
    // @description Sets the unit for pitch values: Cents (default), MIDI, Hertz (frequency), or frequency ratio.
}

t_max_err earsbufobj_setattr_frequnit(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            e_ob->l_frequnit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            e_ears_frequnit new_frequnit = ears_frequnit_from_symbol(atom_getsym(argv));
            if (new_frequnit != EARS_FREQUNIT_UNKNOWN)
                e_ob->l_frequnit = new_frequnit;
            else
                object_error((t_object *)e_ob, "Unknown frequency unit!");
        }
    }
    return MAX_ERR_NONE;
}



void earsbufobj_class_add_frequnit_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "frequnit", 0, t_earsbufobj, l_frequnit);
    CLASS_ATTR_STYLE_LABEL(c,"frequnit",0,"enumindex","Frequency Values Are In");
    CLASS_ATTR_ENUMINDEX(c,"frequnit", 0, "Hertz BPM Cents MIDI");
    CLASS_ATTR_ACCESSORS(c, "frequnit", NULL, earsbufobj_setattr_frequnit);
    CLASS_ATTR_BASIC(c, "frequnit", 0);
    CLASS_ATTR_CATEGORY(c, "frequnit", 0, "Units");
    // @description Sets the unit for pitch values: Hertz (default), BPM, Cents, MIDI 
}



t_max_err earsbufobj_setattr_angleunit(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            e_ob->l_angleunit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            if (s == gensym("radians") || s == gensym("radian") || s == gensym("rad"))
                e_ob->l_angleunit = EARS_ANGLEUNIT_RADIANS;
            else if (s == gensym("degrees") || s == gensym("degree") || s == gensym("deg"))
                e_ob->l_angleunit = EARS_ANGLEUNIT_DEGREES;
            else if (s == gensym("turns") || s == gensym("turn"))
                e_ob->l_angleunit = EARS_ANGLEUNIT_TURNS;
            else
                object_error((t_object *)e_ob, "Unknown angle unit!");
        }
    }
    return MAX_ERR_NONE;
}

void earsbufobj_class_add_angleunit_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "angleunit", 0, t_earsbufobj, l_angleunit);
    CLASS_ATTR_STYLE_LABEL(c,"angleunit",0,"enumindex","Angle Values Are In");
    CLASS_ATTR_ENUMINDEX(c,"angleunit", 0, "Radians Degrees Turns");
    CLASS_ATTR_ACCESSORS(c, "angleunit", NULL, earsbufobj_setattr_angleunit);
    CLASS_ATTR_BASIC(c, "angleunit", 0);
    CLASS_ATTR_CATEGORY(c, "angleunit", 0, "Units");
    // @description Sets the unit for angles: Radians (default), Degrees, or Turns.
}


void earsbufobj_class_add_slopemapping_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c,"slopemapping",0, t_earsbufobj, l_slopemapping);
    CLASS_ATTR_STYLE_LABEL(c,"slopemapping",0,"enumindex","Slope Mapping");
    CLASS_ATTR_ENUMINDEX(c,"slopemapping", 0, "bach Max");
    // @description Sets the function to be used for slope mapping: either bach (default) or Max.
}

void earsbufobj_class_add_resamplingpolicy_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c,"resamplingpolicy",0, t_earsbufobj, l_resamplingpolicy);
    CLASS_ATTR_STYLE_LABEL(c,"resamplingpolicy",0,"enumindex","Resampling Policy");
    CLASS_ATTR_ENUMINDEX(c,"resamplingpolicy", 0, "Don't To Lowest To Highest To Most Common To Max Current");
    CLASS_ATTR_CATEGORY(c, "resamplingpolicy", 0, "Resampling");
    // @description Sets the resampling policy used when buffers have different sample rates:
    // "Don't" (no resampling - beware: temporality is not preserved!), "To lowest" (buffers are to the lowest sample rate),
    // "To highest" (buffers are converted to the highest sample rate), "To most common" (buffers are to the most common
    // sample rate), "To Max Current" (buffers are converted to the current Max sample rate).
}


void earsbufobj_class_add_resamplingfiltersize_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c,"resamplingfiltersize",0, t_earsbufobj, l_resamplingfilterwidth);
    CLASS_ATTR_STYLE_LABEL(c,"resamplingfiltersize",0,"text","Resampling Filter Size");
    CLASS_ATTR_CATEGORY(c, "resamplingfiltersize", 0, "Resampling");
    // @description Sets the resampling filter size.
}


t_max_err earsbufobj_setattr_envtimeunit(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            e_ob->l_envtimeunit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            e_ears_timeunit new_timeunit = ears_timeunit_from_symbol(s);
            if (new_timeunit != EARS_TIMEUNIT_UNKNOWN)
                e_ob->l_envtimeunit = new_timeunit;
            else
                object_error((t_object *)e_ob, "Unknown time unit!");
        }
    }
    return MAX_ERR_NONE;
}


void earsbufobj_class_add_envtimeunit_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "envtimeunit", 0, t_earsbufobj, l_envtimeunit);
    CLASS_ATTR_STYLE_LABEL(c,"envtimeunit",0,"enumindex","Envelope Time Values Are In");
    CLASS_ATTR_ENUMINDEX(c,"envtimeunit", 0, "Milliseconds Samples Relative");
    CLASS_ATTR_ACCESSORS(c, "envtimeunit", NULL, earsbufobj_setattr_envtimeunit);
    CLASS_ATTR_BASIC(c, "envtimeunit", 0);
    CLASS_ATTR_CATEGORY(c, "envtimeunit", 0, "Units");
    // @description Sets the unit for time values inside envelopes: Milliseconds, Samples, Relative (0. to 1. as a percentage of the buffer length).
    // The default is Relative.
}


void earsbufobj_release_generated_outnames(t_earsbufobj *e_ob)
{
    t_llll *temp = llll_clone(e_ob->l_generated_outnames);
    llll_flatten(temp, 0, 0);
    for (t_llllelem *el = temp->l_head; el; el = el->l_next) {
        t_symbol *s = hatom_getsym(&el->l_hatom);
        if (s) {
            t_object *buf = ears_buffer_getobject(s);
            if (buf) {
//                ears_hashtab_clipcount(s);
                ears_buffer_release(buf, s);
            }
        }
    }
    llll_clear(e_ob->l_generated_outnames);
    llll_free(temp);
}

t_max_err earsbufobj_setattr_naming(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        long old_bufouts_naming = e_ob->l_bufouts_naming;
        
        if (atom_gettype(argv) == A_LONG)
            e_ob->l_bufouts_naming = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            if (s == gensym("copy"))
                e_ob->l_bufouts_naming = EARSBUFOBJ_NAMING_COPY;
            else if (s == gensym("static"))
                e_ob->l_bufouts_naming = EARSBUFOBJ_NAMING_STATIC;
            else if (s == gensym("dynamic") || s == gensym("dyn"))
                e_ob->l_bufouts_naming = EARSBUFOBJ_NAMING_DYNAMIC;
            else {
                object_error((t_object *)e_ob, "Unknown naming mode.");
                e_ob->l_bufouts_naming = EARSBUFOBJ_NAMING_STATIC;
            }
        }

        // in any case:
        earsbufobj_release_generated_outnames(e_ob);
        
        if (e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_COPY) {
            if (!(e_ob->l_flags & EARSBUFOBJ_FLAG_SUPPORTS_COPY_NAMES)) {
                object_warn((t_object *)e_ob, "Object does not support 'Copy' naming mode. Switching to 'Static'.");
                e_ob->l_bufouts_naming = EARSBUFOBJ_NAMING_STATIC;
            }
        }
    
        object_attr_setdisabled((t_object *)e_ob, gensym("outname"), e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_COPY);

        if (old_bufouts_naming == EARSBUFOBJ_NAMING_COPY && e_ob->l_bufouts_naming != EARSBUFOBJ_NAMING_COPY)
            earsbufobj_refresh_outlet_names(e_ob, true);
    }
    return MAX_ERR_NONE;
}


void earsbufobj_class_add_naming_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "naming", 0, t_earsbufobj, l_bufouts_naming);
    CLASS_ATTR_STYLE_LABEL(c,"naming",0,"enumindex","Output Naming Policy");
    CLASS_ATTR_ENUMINDEX(c,"naming", 0, "Copy Static Dynamic");
    CLASS_ATTR_ACCESSORS(c, "naming", NULL, earsbufobj_setattr_naming);
    CLASS_ATTR_BASIC(c, "naming", 0);
    CLASS_ATTR_CATEGORY(c, "naming", 0, "Behavior");
    // @description Chooses the output buffer naming policy
}


t_max_err earsbufobj_setattr_framesize(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (is_atom_number(argv)) {
            double f = atom_getfloat(argv);
            e_ob->a_framesize = f;
            e_ob->a_overlap = e_ob->a_framesize / e_ob->a_hopsize;
        }
    }
    return MAX_ERR_NONE;
}


void earsbufobj_class_add_framesize_attr(t_class *c)
{
    CLASS_ATTR_DOUBLE(c, "framesize", 0, t_earsbufobj, a_framesize);
    CLASS_ATTR_STYLE_LABEL(c,"framesize",0,"text","Frame Size");
    CLASS_ATTR_ACCESSORS(c, "framesize", NULL, earsbufobj_setattr_framesize);
    CLASS_ATTR_BASIC(c, "framesize", 0);
    CLASS_ATTR_CATEGORY(c, "framesize", 0, "Analysis");
    // @description Sets the analysis frame size or window size (the unit depends on the <m>antimeunit</m> attribute)
}


t_max_err earsbufobj_setattr_hopsize(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (is_atom_number(argv)) {
            double hop = atom_getfloat(argv);
            e_ob->a_hopsize = hop;
            e_ob->a_overlap = e_ob->a_framesize / e_ob->a_hopsize;
        }
    }
    return MAX_ERR_NONE;
}

void earsbufobj_class_add_hopsize_attr(t_class *c)
{
    CLASS_ATTR_DOUBLE(c, "hopsize", 0, t_earsbufobj, a_hopsize);
    CLASS_ATTR_STYLE_LABEL(c,"hopsize",0,"text","Hop Size");
    CLASS_ATTR_ACCESSORS(c, "hopsize", NULL, earsbufobj_setattr_hopsize);
    CLASS_ATTR_BASIC(c, "hopsize", 0);
    CLASS_ATTR_CATEGORY(c, "hopsize", 0, "Analysis");
    // @description Sets the analysis hop size (the unit depends on the <m>antimeunit</m> attribute)
    // Floating point values are allowed.
}


t_max_err earsbufobj_setattr_overlap(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (is_atom_number(argv)) {
            double olap = atom_getfloat(argv);
            e_ob->a_overlap = olap;
            object_attr_setfloat(e_ob, gensym("hopsize"), e_ob->a_framesize / olap);
        }
    }
    return MAX_ERR_NONE;
}

void earsbufobj_class_add_overlap_attr(t_class *c)
{
    CLASS_ATTR_DOUBLE(c, "overlap", 0, t_earsbufobj, a_overlap);
    CLASS_ATTR_STYLE_LABEL(c,"overlap",0,"text","Overlap");
    CLASS_ATTR_ACCESSORS(c, "overlap", NULL, earsbufobj_setattr_overlap);
    CLASS_ATTR_CATEGORY(c, "overlap", 0, "Analysis");
    // @description Sets the overlap factor between the analysis window size and the hop size.
}


t_max_err earsbufobj_setattr_numframes(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG) {
            atom_setlong(&e_ob->a_numframes, atom_getlong(argv));
            object_attr_setdisabled((t_object *)e_ob, gensym("hopsize"), true);
            object_attr_setdisabled((t_object *)e_ob, gensym("framesize"), true);
        } else if (atom_gettype(argv) == A_SYM) {
            atom_setsym(&e_ob->a_numframes, _llllobj_sym_auto);
            object_attr_setdisabled((t_object *)e_ob, gensym("hopsize"), false);
            object_attr_setdisabled((t_object *)e_ob, gensym("framesize"), false);
        }
    }
    return MAX_ERR_NONE;
}
void earsbufobj_class_add_numframes_attr(t_class *c)
{
    CLASS_ATTR_ATOM(c, "numframes", 0, t_earsbufobj, a_numframes);
    CLASS_ATTR_STYLE_LABEL(c,"numframes",0,"text","Number of Analysis Frames");
    CLASS_ATTR_ACCESSORS(c, "numframes", NULL, earsbufobj_setattr_numframes);
    CLASS_ATTR_CATEGORY(c, "numframes", 0, "Analysis");
    // @description Sets the number of analysis frames. Defaults to "auto", as this number is a consequence of the
    // <m>framesize</m> and <m>hopsize</m> attributes. If this number is set to a positive integer value, the <m>hopsize</m>
    // is ignored and inferred from <m>numframes</m>.
}

void earsbufobj_class_add_wintype_attr(t_class *c)
{
    CLASS_ATTR_SYM(c, "wintype", 0, t_earsbufobj, a_wintype);
    CLASS_ATTR_STYLE_LABEL(c,"wintype",0,"text","Window Type");
    CLASS_ATTR_ENUM(c,"wintype", 0, "hamming hann hannnsgcq triangular square blackmanharris62 blackmanharris70 blackmanharris74 blackmanharris92");
    CLASS_ATTR_BASIC(c, "wintype", 0);
    CLASS_ATTR_CATEGORY(c, "wintype", 0, "Analysis");
    // @description Sets the window type.
    // Available windows are the ones allowed by the Essentia library:
    // "hamming", "hann", "hannnsgcq", "triangular", "square", "blackmanharris62", "blackmanharris70", "blackmanharris74", "blackmanharris92"
}

void earsbufobj_class_add_winnormalized_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "winnormalized", 0, t_earsbufobj, a_winnorm);
    CLASS_ATTR_STYLE_LABEL(c,"winnormalized",0,"onoff","Windows Are Normalized");
    CLASS_ATTR_CATEGORY(c, "winnormalized", 0, "Analysis");
    // @description Toggles the ability for windows to be normalized to have an area of 1 and then scaled by a factor of 2.
}

void earsbufobj_class_add_zeropadding_attr(t_class *c)
{
    CLASS_ATTR_LONG(c, "zeropadding", 0, t_earsbufobj, a_zeropadding);
    CLASS_ATTR_STYLE_LABEL(c,"zeropadding",0,"text","Zero Padding Amount");
    CLASS_ATTR_CATEGORY(c, "zeropadding", 0, "Analysis");
    // @description Sets the number of samples for zero padding.
}

void earsbufobj_class_add_zerophase_attr(t_class *c)
{
    CLASS_ATTR_LONG(c, "zerophase", 0, t_earsbufobj, a_zerophase);
    CLASS_ATTR_STYLE_LABEL(c,"zerophase",0,"onoff","Zero Phase Windowing");
    CLASS_ATTR_CATEGORY(c, "zerophase", 0, "Analysis");
    // @description Toggles zero-phase windowing.
}


void earsbufobj_class_add_winstartfromzero_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "winstartfromzero", 0, t_earsbufobj, a_winstartfromzero);
    CLASS_ATTR_STYLE_LABEL(c,"winstartfromzero",0,"onoff","First Window Starts At Zero");
    CLASS_ATTR_CATEGORY(c, "winstartfromzero", 0, "Analysis");
    // @description If on, the first window is centered at framesize/2; if off (default), the first window is centered at zero.
}



e_slope_mapping earsbufobj_get_slope_mapping(t_earsbufobj *e_ob)
{
    return (e_slope_mapping)e_ob->l_slopemapping;
}


t_object *earsbufobj_get_inlet_buffer_obj(t_earsbufobj *e_ob, long store_idx, long buffer_idx, bool update_buffer_obj)
{
    if (!update_buffer_obj || e_ob->l_flags & EARSBUFOBJ_FLAG_DUPLICATE_INPUT_BUFFERS) {
        if (store_idx >= 0 && store_idx < e_ob->l_numbufins && buffer_idx >= 0 && buffer_idx < e_ob->l_instore[store_idx].num_stored_bufs)
            return e_ob->l_instore[store_idx].stored_buf[buffer_idx].l_buf;
    } else {
        if (store_idx >= 0 && store_idx < e_ob->l_numbufins && buffer_idx >= 0 && buffer_idx < e_ob->l_instore[store_idx].num_stored_bufs) {
            t_symbol *s = e_ob->l_instore[store_idx].stored_buf[buffer_idx].l_name;
            t_object *ob = ears_buffer_getobject(s);
            if (ob != e_ob->l_instore[store_idx].stored_buf[buffer_idx].l_buf) {
                // update object
                e_ob->l_instore[store_idx].stored_buf[buffer_idx].l_buf = ob;
            }
            return ob;
        }
    }
    return NULL;
}

t_symbol *earsbufobj_get_inlet_buffer_name(t_earsbufobj *e_ob, long store_idx, long buffer_idx)
{
    if (store_idx >= 0 && store_idx < e_ob->l_numbufins && buffer_idx >= 0 && buffer_idx < e_ob->l_instore[store_idx].num_stored_bufs)
        return e_ob->l_instore[store_idx].stored_buf[buffer_idx].l_name;
    
    return NULL;
}



t_object *earsbufobj_get_outlet_buffer_obj(t_earsbufobj *e_ob, long store_idx, long buffer_idx)
{
    if (store_idx >= 0 && store_idx < e_ob->l_numbufouts && buffer_idx >= 0 && buffer_idx < e_ob->l_outstore[store_idx].num_stored_bufs)
        return e_ob->l_outstore[store_idx].stored_buf[buffer_idx].l_buf;
    
    return NULL;
}

t_symbol *earsbufobj_get_outlet_buffer_name(t_earsbufobj *e_ob, long store_idx, long buffer_idx)
{
    if (store_idx >= 0 && store_idx < e_ob->l_numbufouts && buffer_idx >= 0 && buffer_idx < e_ob->l_outstore[store_idx].num_stored_bufs)
        return e_ob->l_outstore[store_idx].stored_buf[buffer_idx].l_name;
    
    return NULL;
}


long earsbufobj_outlet_to_bufstore(t_earsbufobj *e_ob, long outlet)
{
    long curr_num_buf_outlets = 0;
    for (long i = 0; i < outlet; i++) {
        if (e_ob->l_outlet_types[i] == 'E' || e_ob->l_outlet_types[i] == 'e') {
            curr_num_buf_outlets++;
        }
    }
    return curr_num_buf_outlets;
}

// pos are 0-based, differently from llll_getindex()
t_llll *earsbufobj_generated_names_llll_getlist(t_llll *ll, long pos1, long pos2)
{
    t_llllelem *el = llll_getindex(ll, pos1 + 1, I_STANDARD);
    if (el && hatom_gettype(&el->l_hatom) == H_LLLL) {
        t_llllelem *subel = llll_getindex(hatom_getllll(&el->l_hatom), pos2 + 1, I_STANDARD);
        if (subel && hatom_gettype(&subel->l_hatom) == H_LLLL)
            return hatom_getllll(&subel->l_hatom);
    }
    return NULL;
}

// pos are 0-based, differently from llll_getindex()
t_llllelem *earsbufobj_generated_names_llll_getsymbol(t_llll *ll, long pos1, long pos2, long pos3)
{
    t_llllelem *el = llll_getindex(ll, pos1 + 1, I_STANDARD);
    if (el && hatom_gettype(&el->l_hatom) == H_LLLL) {
        t_llllelem *subel = llll_getindex(hatom_getllll(&el->l_hatom), pos2 + 1, I_STANDARD);
        if (subel && hatom_gettype(&subel->l_hatom) == H_LLLL)
            return llll_getindex(hatom_getllll(&subel->l_hatom), pos3 + 1, I_STANDARD);
    }
    return NULL;
}


// pos are 0-based, differently from llll_getindex()
void earsbufobj_generated_names_llll_subssymbol(t_llll *ll, t_symbol *sym, long pos1, long pos2, long pos3)
{
    t_llll *address = llll_get();
    llll_appendlong(address, pos1 + 1);
    llll_appendlong(address, pos2 + 1);
    llll_appendlong(address, pos3 + 1);

    t_llll *subs_model = llll_get();
    llll_appendsym(subs_model, sym);
    
    llll_wrap_once(&address);
    llll_wrap_once(&subs_model);
    
    t_llll *sizes = llll_get();
    llll_appendlong(sizes, 1);

    llll_multisubs(ll, address, subs_model, sizes);

    llll_free(address);
    llll_free(subs_model);
    llll_free(sizes);
}


// generates an unique symbol for a certain output store
t_symbol *earsbufobj_output_get_symbol_unique(t_earsbufobj *e_ob, long outstore_idx, long buffer_idx, e_earsbufobj_bufstatus *status)
{
    t_symbol *sym = NULL;
    
/*    char *buf1 = NULL, *buf2 = NULL;
    llll_to_text_buf(e_ob->l_generated_outnames, &buf1, 0, 6, 0, LLLL_T_NONE, LLLL_TE_SMART, NULL);
  */
    switch (e_ob->l_bufouts_naming) {
        case EARSBUFOBJ_NAMING_DYNAMIC:
        {
            t_llllelem *el = earsbufobj_generated_names_llll_getsymbol(e_ob->l_generated_outnames, outstore_idx, buffer_idx, e_ob->l_generated_outname_count[outstore_idx]);
            if (el && hatom_gettype(&el->l_hatom) == H_SYM) {
                sym = hatom_getsym(&el->l_hatom);
                if (status) *status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
            } else {
                sym = symbol_unique();
                earsbufobj_generated_names_llll_subssymbol(e_ob->l_generated_outnames, sym, outstore_idx, buffer_idx, e_ob->l_generated_outname_count[outstore_idx]);
                if (status) *status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
            }
            e_ob->l_generated_outname_count[outstore_idx]++;
        }
            break;
            
        case EARSBUFOBJ_NAMING_STATIC:
        {
            if (e_ob->l_outstore[outstore_idx].use_polybuffers) {
                sym = ears_buffer_name_get_for_polybuffer(e_ob->l_outstore[outstore_idx].polybuffer_name, buffer_idx+1);
                earsbufobj_generated_names_llll_subssymbol(e_ob->l_generated_outnames, sym, outstore_idx, buffer_idx, 0);
                if (status) *status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
            } else {
                t_llllelem *el = earsbufobj_generated_names_llll_getsymbol(e_ob->l_generated_outnames, outstore_idx, buffer_idx, 0);
                if (el && hatom_gettype(&el->l_hatom) == H_SYM) {
                    sym = hatom_getsym(&el->l_hatom);
                    if (status) *status = EARSBUFOBJ_BUFSTATUS_USERNAMED;
                } else {
                    sym = symbol_unique();
                    earsbufobj_generated_names_llll_subssymbol(e_ob->l_generated_outnames, sym, outstore_idx, buffer_idx, 0);
                    if (status) *status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
                }
            }
        }
            break;
            
        case EARSBUFOBJ_NAMING_COPY:
        default:
            sym = earsbufobj_get_inlet_buffer_name(e_ob, outstore_idx, buffer_idx);
//            if (!sym)
//                sym = symbol_unique();
            if (status) *status = EARSBUFOBJ_BUFSTATUS_COPIED;
            break;
    }
/*
    llll_to_text_buf(e_ob->l_generated_outnames, &buf1, 0, 6, 0, LLLL_T_NONE, LLLL_TE_SMART, NULL);

    bach_freeptr(buf1);
    bach_freeptr(buf2);
  */
    return sym;
}


long earsbufobj_get_num_inlet_stored_buffers(t_earsbufobj *e_ob, long store_idx, char remove_empty_buffers)
{
    long num_stored_buffers = e_ob->l_instore[store_idx].num_stored_bufs;
    
    if (remove_empty_buffers) {
        while (num_stored_buffers > 0) {
            if (ears_buffer_get_size_samps((t_object *)e_ob, earsbufobj_get_inlet_buffer_obj(e_ob, store_idx, num_stored_buffers - 1)) == 0) {
                num_stored_buffers--;
            } else
                break;
        }
    }
    return num_stored_buffers;
}

long earsbufobj_get_num_outlet_stored_buffers(t_earsbufobj *e_ob, long store_idx, char remove_empty_buffers)
{
    long num_stored_buffers = e_ob->l_outstore[store_idx].num_stored_bufs;
    
    if (remove_empty_buffers) {
        while (num_stored_buffers > 0) {
            if (ears_buffer_get_size_samps((t_object *)e_ob, earsbufobj_get_outlet_buffer_obj(e_ob, store_idx, num_stored_buffers - 1)) == 0) {
                num_stored_buffers--;
            } else
                break;
        }
    }
    return num_stored_buffers;
}

void earsbufobj_refresh_outlet_names(t_earsbufobj *e_ob, char force_refresh_even_if_static)
{
    long store;
    for (store = 0; store < e_ob->l_numbufouts; store++) {
        long num_stored_bufs = e_ob->l_outstore[store].num_stored_bufs;
        if (num_stored_bufs > 0) {
            t_symbol **s = (t_symbol **)bach_newptrclear(num_stored_bufs * sizeof(t_symbol *));
            t_buffer_obj **o = (t_buffer_obj **)bach_newptrclear(num_stored_bufs * sizeof(t_buffer_obj *));
            long j, c = 0;
            for (j = 0; j < num_stored_bufs; j++) {
                t_symbol *name = earsbufobj_get_outlet_buffer_name(e_ob, store, j);
                t_buffer_obj *buf = earsbufobj_get_outlet_buffer_obj(e_ob, store, j);
                if (name) {
                    s[c] = name;
                    o[c] = buf;
                    c++;
                }
            }
            
            // Now we change the outlet names
            for (j = 0; j < num_stored_bufs; j++) { // was: j < c
                if (e_ob->l_outstore[store].stored_buf[j].l_status != EARSBUFOBJ_BUFSTATUS_USERNAMED &&
                    (force_refresh_even_if_static || !(e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_STATIC && (j < c && s[j]))))
                    earsbufobj_buffer_link(e_ob, EARSBUFOBJ_OUT, store, j, earsbufobj_output_get_symbol_unique(e_ob, store, j, &e_ob->l_outstore[store].stored_buf[j].l_status));
            }
            
            bach_freeptr(s);
            bach_freeptr(o);
        }
    }
}



void earsbufobj_outlet_anything(t_earsbufobj *e_ob, long outnum, t_symbol *s, long ac, t_atom *av)
{
    llllobj_outlet_anything((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum, s, ac, av);
}

void earsbufobj_outlet_symbol_list_do(t_earsbufobj *e_ob, t_symbol *s, long ac, t_atom *av)
{
    long outnum = atom_getlong(av);
    long numsymbols = atom_getlong(av+1);
    if (numsymbols > 0) {
        if (numsymbols == 1)
            llllobj_outlet_anything((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum, atom_getsym(av+2), 0, NULL);
        else {
            llllobj_outlet_anything((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum, _sym_list, numsymbols, av+2);
        }
    }
    bach_freeptr(av);
}

void earsbufobj_outlet_symbol_list(t_earsbufobj *e_ob, long outnum, long numsymbols, t_symbol **s)
{
    if (numsymbols > 0) {
        t_atom *av = (t_atom *)bach_newptr((numsymbols + 2) * sizeof(t_atom));
        atom_setlong(av, outnum);
        atom_setlong(av+1, numsymbols);
        for (long i = 0; i < numsymbols; i++)
            atom_setsym(av+i+2, s[i]);
        if (e_ob->l_blocking == 0) {
            earsbufobj_updateprogress(e_ob, 1.);
            defer(e_ob, (method)earsbufobj_outlet_symbol_list_do, NULL, numsymbols+2, av);
        } else {
            earsbufobj_outlet_symbol_list_do(e_ob, NULL, numsymbols+2, av);
        }
    }
}


void earsbufobj_outlet_llll_do(t_earsbufobj *e_ob, t_symbol *s, long ac, t_atom *av)
{
    long outnum = atom_getlong(av);
    t_llll *ll = (t_llll *)atom_getobj(av+1);
    llllobj_outlet_llll((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum, ll);
    llll_release(ll);
}

void earsbufobj_outlet_llll(t_earsbufobj *e_ob, long outnum, t_llll *ll)
{
    t_atom av[2];
    atom_setlong(av, outnum);
    atom_setobj(av+1, ll);
    llll_retain(ll);
    if (e_ob->l_blocking == 0) {
        earsbufobj_updateprogress(e_ob, 1.);
        defer(e_ob, (method)earsbufobj_outlet_llll_do, NULL, 2, av);
    } else {
        earsbufobj_outlet_llll_do(e_ob, NULL, 2, av);
    }
}


void earsbufobj_gunload_llll(t_earsbufobj *e_ob, long outnum, t_llll *ll)
{
    llllobj_gunload_llll((t_object *)e_ob, LLLL_OBJ_VANILLA, ll, outnum);
}

void earsbufobj_shoot_llll_do(t_earsbufobj *e_ob, t_symbol *s, long ac, t_atom *av)
{
    long outnum = atom_getlong(av);
    llllobj_shoot_llll((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum);
}

void earsbufobj_shoot_llll(t_earsbufobj *e_ob, long outnum)
{
    t_atom av;
    atom_setlong(&av, outnum);
    if (e_ob->l_blocking == 0) {
        earsbufobj_updateprogress(e_ob, 1.);
        defer(e_ob, (method)earsbufobj_shoot_llll_do, NULL, 1, &av);
    } else {
        earsbufobj_shoot_llll_do(e_ob, NULL, 1, &av);
    }
}


void earsbufobj_outlet_bang_do(t_earsbufobj *e_ob, t_symbol *s, long ac, t_atom *av)
{
    long outnum = atom_getlong(av);
    llllobj_outlet_bang((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum);
}

void earsbufobj_outlet_bang(t_earsbufobj *e_ob, long outnum)
{
    t_atom av;
    atom_setlong(&av, outnum);
    if (e_ob->l_blocking == 0) {
        earsbufobj_updateprogress(e_ob, 1.);
        defer(e_ob, (method)earsbufobj_outlet_bang_do, NULL, 1, &av);
    } else {
        earsbufobj_outlet_bang_do(e_ob, NULL, 1, &av);
    }
}

void earsbufobj_outlet_buffer_do(t_earsbufobj *e_ob, t_symbol *s, long ac, t_atom *av)
{
    long outnum = atom_getlong(av);
    if (outnum >= 0 && outnum < e_ob->l_ob.l_numouts) {
        long store = earsbufobj_outlet_to_bufstore(e_ob, outnum);
        if (e_ob->l_outstore[store].num_stored_bufs > 0) {
            if (e_ob->l_outstore[store].use_polybuffers && e_ob->l_output_polybuffers == 1) {
                t_symbol *name = e_ob->l_outstore[store].polybuffer_name;
                llllobj_outlet_anything((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum, name, 0, NULL);
            } else {
                t_atom *a = (t_atom *)bach_newptr(e_ob->l_outstore[store].num_stored_bufs * sizeof(t_atom));
                long j, c = 0;
                for (j = 0; j < e_ob->l_outstore[store].num_stored_bufs; j++) {
                    t_symbol *name = earsbufobj_get_outlet_buffer_name(e_ob, store, j);
                    if (name) {
                        atom_setsym(a+c, name);
                        c++;
                    }
                }
                
                if (c > 0) {
                    llllobj_outlet_anything((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum, atom_getsym(a), c - 1, a + 1);
                }
                
                bach_freeptr(a);
            }
        }
    }
}

void earsbufobj_outlet_buffer(t_earsbufobj *e_ob, long outnum)
{
    t_atom av;
    atom_setlong(&av, outnum);
    if (e_ob->l_blocking == 0) {
        earsbufobj_updateprogress(e_ob, 1.);
        defer(e_ob, (method)earsbufobj_outlet_buffer_do, NULL, 1, &av);
    } else {
        earsbufobj_outlet_buffer_do(e_ob, NULL, 1, &av);
    }
}

void earsbufobj_importreplace_buffer(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx, t_symbol *filename)
{
    switch (type) {
        case EARSBUFOBJ_IN:
            if (store_idx >= 0 && store_idx < e_ob->l_numbufins && buffer_idx >= 0 && buffer_idx < e_ob->l_instore[store_idx].num_stored_bufs) {
                t_earsbufobj_store *store = &e_ob->l_instore[store_idx];
                t_atom a;
                atom_setsym(&a, filename);
                typedmess(store->stored_buf[buffer_idx].l_buf, gensym("importreplace"), 1, &a);
            }
            break;
            
        case EARSBUFOBJ_OUT:
            if (store_idx >= 0 && store_idx < e_ob->l_numbufouts && buffer_idx >= 0 && buffer_idx < e_ob->l_outstore[store_idx].num_stored_bufs) {
                t_earsbufobj_store *store = &e_ob->l_outstore[store_idx];
                t_atom a;
                atom_setsym(&a, filename);
                typedmess(store->stored_buf[buffer_idx].l_buf, gensym("importreplace"), 1, &a);
            }
            break;
    }
}


long earsbufobj_get_instore_size(t_earsbufobj *e_ob, long store_idx)
{
    if (store_idx >= 0 && store_idx < e_ob->l_numbufins)
        return e_ob->l_instore[store_idx].num_stored_bufs;
    return 0;
}


long earsbufobj_get_outstore_size(t_earsbufobj *e_ob, long store_idx)
{
    if (store_idx >= 0 && store_idx < e_ob->l_numbufouts)
        return e_ob->l_outstore[store_idx].num_stored_bufs;
    return 0;
}

void earsbufobj_store_buffer(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx, t_symbol *buffername)
{
    if (buffername) {
        switch (type) {
            case EARSBUFOBJ_IN:
                if (store_idx >= 0 && store_idx < e_ob->l_numbufins &&
                    buffer_idx >= 0 && buffer_idx < earsbufobj_get_instore_size(e_ob, store_idx)) {
                    t_earsbufobj_store *store = &e_ob->l_instore[store_idx];
                    if (!(e_ob->l_flags & EARSBUFOBJ_FLAG_DUPLICATE_INPUT_BUFFERS)) {
                        store->stored_buf[buffer_idx].l_name = buffername;
                        store->stored_buf[buffer_idx].l_buf = ears_buffer_getobject(buffername);
                    } else {
                        /*                    t_atom a;
                         ears_buffer_copy_format((t_object *)e_ob, temp_obj, store->stored_buf[buffer_idx].l_buf);
                         atom_setsym(&a, buffername);
                         typedmess(store->stored_buf[buffer_idx].l_buf, gensym("duplicate"), 1, &a); */
                        t_buffer_ref *temp_ref = buffer_ref_new((t_object *)e_ob, buffername);
                        t_buffer_obj *temp_obj = buffer_ref_getobject(temp_ref);
                        ears_buffer_clone((t_object *)e_ob, temp_obj, store->stored_buf[buffer_idx].l_buf);
                        object_free(temp_ref);
                    }
                    
                    if (!store->stored_buf[buffer_idx].l_buf) {
                        store->stored_buf[buffer_idx].l_name = NULL;
                        object_error((t_object *)e_ob, EARS_ERROR_BUF_NO_BUFFER_NAMED, buffername ? buffername->s_name : "???");
                    }
                }
                break;
                
            case EARSBUFOBJ_OUT:
                if (store_idx >= 0 && store_idx < e_ob->l_numbufouts && buffer_idx >= 0 && buffer_idx < e_ob->l_outstore[store_idx].num_stored_bufs) {
                    t_earsbufobj_store *store = &e_ob->l_outstore[store_idx];
                    if (e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_COPY) {
                        store->stored_buf[buffer_idx].l_name = buffername;
                        store->stored_buf[buffer_idx].l_buf = ears_buffer_getobject(buffername);
                    } else {
                        /*                    t_atom a;
                         atom_setsym(&a, buffername);
                         typedmess(store->stored_buf[buffer_idx].l_buf, gensym("duplicate"), 1, &a); */
                        t_buffer_ref *temp_ref = buffer_ref_new((t_object *)e_ob, buffername);
                        t_buffer_obj *temp_obj = buffer_ref_getobject(temp_ref);
                        ears_buffer_clone((t_object *)e_ob, temp_obj, store->stored_buf[buffer_idx].l_buf);
                        object_free(temp_ref);
                    }
                }
                break;
        }
    } else {
        object_error((t_object *)e_ob, "Buffer has no name!");
    }
}


void earsbufobj_store_empty_buffer(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx)
{
    switch (type) {
        case EARSBUFOBJ_IN:
            if (store_idx >= 0 && store_idx < e_ob->l_numbufins && buffer_idx >= 0 && buffer_idx < e_ob->l_instore[store_idx].num_stored_bufs) {
                if (e_ob->l_flags & EARSBUFOBJ_FLAG_DUPLICATE_INPUT_BUFFERS) {
                    t_buffer_obj *obj = earsbufobj_get_inlet_buffer_obj(e_ob, store_idx, buffer_idx);
                    ears_buffer_set_size_and_numchannels((t_object *)e_ob, obj, 0, 1);
                    ears_buffer_set_sr((t_object *)e_ob, obj, sys_getsr());
                } else {
                    
                }
            }
            break;
            
        case EARSBUFOBJ_OUT:
            if (store_idx >= 0 && store_idx < e_ob->l_numbufouts && buffer_idx >= 0 && buffer_idx < e_ob->l_outstore[store_idx].num_stored_bufs) {
                t_buffer_obj *obj = earsbufobj_get_outlet_buffer_obj(e_ob, store_idx, buffer_idx);
                ears_buffer_set_size_and_numchannels((t_object *)e_ob, obj, 0, 1);
                ears_buffer_set_sr((t_object *)e_ob, obj, sys_getsr());
            }
            break;
    }
}

t_earsbufobj_store *earsbufobj_get_store(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long index)
{
    switch (type) {
        case EARSBUFOBJ_IN:
            if (index >= 0 && index < e_ob->l_numbufins)
                return &e_ob->l_instore[index];
            break;
            
        case EARSBUFOBJ_OUT:
            if (index >= 0 && index < e_ob->l_numbufouts)
                return &e_ob->l_outstore[index];
            break;

        default:
            break;
    }
    return NULL;
}

t_buffer_obj *earsbufobj_get_stored_buffer_obj(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx)
{
    switch (type) {
        case EARSBUFOBJ_IN:
            if (store_idx >= 0 && store_idx < e_ob->l_numbufins && buffer_idx >= 0 && buffer_idx < e_ob->l_instore[store_idx].num_stored_bufs)
                return e_ob->l_instore[store_idx].stored_buf[buffer_idx].l_buf;
            break;
            
        case EARSBUFOBJ_OUT:
            if (store_idx >= 0 && store_idx < e_ob->l_numbufouts && buffer_idx >= 0 && buffer_idx < e_ob->l_outstore[store_idx].num_stored_bufs)
                return e_ob->l_outstore[store_idx].stored_buf[buffer_idx].l_buf;
            break;
            
        default:
            break;
    }
    return NULL;
}



t_symbol *earsbufobj_get_stored_buffer_name(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx)
{
    switch (type) {
        case EARSBUFOBJ_IN:
            if (store_idx >= 0 && store_idx < e_ob->l_numbufins && buffer_idx >= 0 && buffer_idx < e_ob->l_instore[store_idx].num_stored_bufs)
                return e_ob->l_instore[store_idx].stored_buf[buffer_idx].l_name;
            break;
            
        case EARSBUFOBJ_OUT:
            if (store_idx >= 0 && store_idx < e_ob->l_numbufouts && buffer_idx >= 0 && buffer_idx < e_ob->l_outstore[store_idx].num_stored_bufs)
                return e_ob->l_outstore[store_idx].stored_buf[buffer_idx].l_name;
            break;
            
        default:
            break;
    }
    return NULL;
}

// TODO
// t_earsobj_object
// t_earsobj_pxobject
// earsbufobj -> earsobj
void earsbufobj_store_copy_format(t_earsbufobj *e_ob, e_earsbufobj_in_out source, long source_store_idx, long source_buffer_idx, e_earsbufobj_in_out dest, long dest_store_idx, long dest_buffer_idx)
{
    t_buffer_obj *from = earsbufobj_get_stored_buffer_obj(e_ob, source, source_store_idx, source_buffer_idx);
    t_buffer_obj *to = earsbufobj_get_stored_buffer_obj(e_ob, dest, dest_store_idx, dest_buffer_idx);
    if (from && to)
        ears_buffer_copy_format((t_object *)e_ob, from, to, true); // don't change size/numchannels, though!, we'll do that later.
}



void earsbufobj_mutex_lock(t_earsbufobj *e_ob)
{
    systhread_mutex_lock(e_ob->l_mutex);
}

void earsbufobj_mutex_unlock(t_earsbufobj *e_ob)
{
    systhread_mutex_unlock(e_ob->l_mutex);
}






////// UNIT CONVERSIONS



double earsbufobj_time_to_durationratio(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, bool is_envelope, bool is_analysis)
{
    double size_ms = ears_buffer_get_size_ms((t_object *)e_ob, buf);
    switch (is_envelope ? e_ob->l_envtimeunit : (is_analysis ? e_ob->l_antimeunit : e_ob->l_timeunit)) {
        case EARS_TIMEUNIT_SAMPS:
            return (1000. * value / buffer_getsamplerate(buf)) / size_ms;
            break;
            
        case EARS_TIMEUNIT_DURATION_RATIO:
            return value;
            break;

        case EARS_TIMEUNIT_NUM_INTERVALS:
            return 1./value;
            break;

        case EARS_TIMEUNIT_NUM_ONSETS:
            return 1./(value-1);
            break;

        case EARS_TIMEUNIT_SECONDS:
            return (value * 1000. / size_ms);
            break;

        case EARS_TIMEUNIT_MS:
        default:
            return value / size_ms;
            break;
    }
}



double earsbufobj_time_to_ms(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, bool is_envelope, bool is_analysis)
{
    switch (is_envelope ? e_ob->l_envtimeunit : (is_analysis ? e_ob->l_antimeunit : e_ob->l_timeunit)) {
        case EARS_TIMEUNIT_SAMPS:
            return ears_samps_to_ms(value, buffer_getsamplerate(buf));
            break;
            
        case EARS_TIMEUNIT_DURATION_RATIO:
            return ears_buffer_get_size_ms((t_object *)e_ob, buf) * value;
            break;

        case EARS_TIMEUNIT_NUM_INTERVALS:
            return ears_buffer_get_size_ms((t_object *)e_ob, buf) * (1./value);
            break;

        case EARS_TIMEUNIT_NUM_ONSETS:
            return ears_buffer_get_size_ms((t_object *)e_ob, buf) * (1./(value-1));
            break;

        case EARS_TIMEUNIT_SECONDS:
            return value*1000.;
            break;

        case EARS_TIMEUNIT_MS:
        default:
            return value;
            break;
    }
}

// TO DO: handle negative values
double earsbufobj_time_to_fsamps(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, bool is_envelope, bool is_analysis)
{
    double res = 0;
    switch (is_envelope ? e_ob->l_envtimeunit : (is_analysis ? e_ob->l_antimeunit : e_ob->l_timeunit)) {
        case EARS_TIMEUNIT_SAMPS:
            res = value;
            break;
            
        case EARS_TIMEUNIT_DURATION_RATIO:
            res = ears_buffer_get_size_samps((t_object *)e_ob, buf) * value;
            break;

        case EARS_TIMEUNIT_NUM_INTERVALS:
            res = ears_buffer_get_size_samps((t_object *)e_ob, buf) * (1./value);
            break;

        case EARS_TIMEUNIT_NUM_ONSETS:
            res = ears_buffer_get_size_samps((t_object *)e_ob, buf) * (1./(value-1));
            break;

        case EARS_TIMEUNIT_SECONDS:
            res = ears_ms_to_fsamps(value*1000., buffer_getsamplerate(buf));
            break;

        case EARS_TIMEUNIT_MS:
        default:
            res = ears_ms_to_fsamps(value, buffer_getsamplerate(buf));
            break;
    }
    return res;
}

long earsbufobj_time_to_samps(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, bool is_envelope, bool is_analysis)
{
    return round(earsbufobj_time_to_fsamps(e_ob, value, buf, is_envelope, is_analysis));
}


double earsbufobj_convert_timeunit(t_earsbufobj *e_ob, double value, t_buffer_obj *buf, e_ears_timeunit new_timeunit, bool is_envelope, bool is_analysis)
{
    switch (new_timeunit) {
        case EARS_TIMEUNIT_SAMPS:
            return earsbufobj_time_to_fsamps(e_ob, value, buf, is_envelope, is_analysis);
            break;
            
        case EARS_TIMEUNIT_DURATION_RATIO:
            return earsbufobj_time_to_durationratio(e_ob, value, buf, is_envelope, is_analysis);
            break;
            
        case EARS_TIMEUNIT_NUM_INTERVALS:
            return 1./earsbufobj_time_to_durationratio(e_ob, value, buf, is_envelope, is_analysis);
            break;

        case EARS_TIMEUNIT_NUM_ONSETS:
            return 1 + (1./earsbufobj_time_to_durationratio(e_ob, value, buf, is_envelope, is_analysis));
            break;

        case EARS_TIMEUNIT_SECONDS:
            return earsbufobj_time_to_ms(e_ob, value, buf, is_envelope, is_analysis)/1000.;
            break;

        case EARS_TIMEUNIT_MS:
        default:
            return earsbufobj_time_to_ms(e_ob, value, buf, is_envelope, is_analysis);
            break;
    }
}



double ears_convert_ampunit(double value, t_buffer_obj *buf, e_ears_ampunit from, e_ears_ampunit to)
{
    t_earsbufobj e_ob;
    e_ob.l_ampunit = from;
    switch (to) {
        case EARS_AMPUNIT_LINEAR:
            return earsbufobj_amplitude_to_linear(&e_ob, value);
            break;
            
        case EARS_AMPUNIT_DECIBEL:
        default:
            return earsbufobj_amplitude_to_db(&e_ob, value);
            break;
    }
}


// TO DO: can be optimized
void ears_convert_ampunit(std::vector<float> &vec, t_buffer_obj *buf, e_ears_ampunit from, e_ears_ampunit to)
{
    if (from == to)
        return;
    
    for (long i = 0; i < vec.size(); i++)
        vec[i] = ears_convert_ampunit(vec[i], buf, from, to);
}




double ears_convert_frequnit(double value, t_buffer_obj *buf, e_ears_frequnit from, e_ears_frequnit to)
{
    t_earsbufobj e_ob;
    e_ob.l_frequnit = from;
    switch (to) {
        case EARS_FREQUNIT_CENTS:
            return earsbufobj_freq_to_cents(&e_ob, value);
            break;
            
        case EARS_FREQUNIT_MIDI:
            return earsbufobj_freq_to_midi(&e_ob, value);
            break;

        case EARS_FREQUNIT_BPM:
            return earsbufobj_freq_to_hz(&e_ob, value)*60.;
            break;

        case EARS_FREQUNIT_HERTZ:
        default:
            return earsbufobj_freq_to_hz(&e_ob, value);
            break;
            
    }
}

// TO DO: can be optimized
void ears_convert_frequnit(std::vector<float> &vec, t_buffer_obj *buf, e_ears_frequnit from, e_ears_frequnit to)
{
    if (from == to)
        return;
    
    for (long i = 0; i < vec.size(); i++)
        vec[i] = ears_convert_frequnit(vec[i], buf, from, to);
}


double ears_convert_angleunit(double value, t_buffer_obj *buf, e_ears_angleunit from, e_ears_angleunit to)
{
    t_earsbufobj e_ob;
    e_ob.l_angleunit = from;
    switch (to) {
        case EARS_ANGLEUNIT_DEGREES:
            return earsbufobj_angle_to_degrees(&e_ob, value);
            break;

        case EARS_ANGLEUNIT_TURNS:
            return earsbufobj_angle_to_radians(&e_ob, value)/TWOPI;
            break;

        default:
        case EARS_ANGLEUNIT_RADIANS:
            return earsbufobj_angle_to_radians(&e_ob, value);
            break;
    }
}



// TO DO: can be optimized
void ears_convert_angleunit(std::vector<float> &vec, t_buffer_obj *buf, e_ears_angleunit from, e_ears_angleunit to)
{
    if (from == to)
        return;
    
    for (long i= 0; i < vec.size(); i++)
        vec[i] = ears_convert_angleunit(vec[i], buf, from, to);
}



double ears_convert_pitchunit(double value, t_buffer_obj *buf, e_ears_pitchunit from, e_ears_pitchunit to)
{
    t_earsbufobj e_ob;
    e_ob.l_pitchunit = from;
    switch (to) {
            
        case EARS_PITCHUNIT_MIDI:
            return earsbufobj_pitch_to_cents(&e_ob, value)/100.;
            break;
            
        case EARS_PITCHUNIT_FREQRATIO:
            return ears_cents_to_ratio(earsbufobj_pitch_to_cents(&e_ob, value));
            break;
            
        case EARS_PITCHUNIT_HERTZ:
            return earsbufobj_pitch_to_hz(&e_ob, value);
            break;

        default:
        case EARS_PITCHUNIT_CENTS:
            return earsbufobj_pitch_to_cents(&e_ob, value);
            break;

    }
}


// TO DO: can be optimized
void ears_convert_pitchunit(std::vector<float> &vec, t_buffer_obj *buf, e_ears_pitchunit from, e_ears_pitchunit to)
{
    if (from == to)
        return;
    
    for (long i= 0; i < vec.size(); i++)
        vec[i] = ears_convert_pitchunit(vec[i], buf, from, to);
}

double ears_convert_timeunit(double value, t_buffer_obj *buf, e_ears_timeunit from, e_ears_timeunit to)
{
    t_earsbufobj e_ob;
    e_ob.l_timeunit = from;
    switch (to) {
        case EARS_TIMEUNIT_SAMPS:
            return earsbufobj_time_to_fsamps(&e_ob, value, buf, false);
            break;
            
        case EARS_TIMEUNIT_DURATION_RATIO:
            return earsbufobj_time_to_durationratio(&e_ob, value, buf, false);
            break;
            
        case EARS_TIMEUNIT_NUM_INTERVALS:
            return 1./earsbufobj_time_to_durationratio(&e_ob, value, buf, false);
            break;
            
        case EARS_TIMEUNIT_NUM_ONSETS:
            return 1 + (1./earsbufobj_time_to_durationratio(&e_ob, value, buf, false));
            break;
            
        case EARS_TIMEUNIT_SECONDS:
            return earsbufobj_time_to_ms(&e_ob, value, buf, false)/1000.;
            break;
            
        case EARS_TIMEUNIT_MS:
        default:
            return earsbufobj_time_to_ms(&e_ob, value, buf, false);
            break;
    }
}

// TO DO: can be optimized
void ears_convert_timeunit(std::vector<float> &vec, t_buffer_obj *buf, e_ears_timeunit from, e_ears_timeunit to)
{
    if (from == to)
        return;
    
    for (long i= 0; i < vec.size(); i++)
        vec[i] = ears_convert_timeunit(vec[i], buf, from, to);
}



long earsbufobj_atom_to_samps(t_earsbufobj *e_ob, t_atom *v, t_buffer_obj *buf)
{
    switch (e_ob->l_timeunit) {
        case EARS_TIMEUNIT_SAMPS:
            return atom_getlong(v);
            break;
            
        case EARS_TIMEUNIT_DURATION_RATIO:
            return ears_buffer_get_size_samps((t_object *)e_ob, buf) * atom_getfloat(v);
            break;

        case EARS_TIMEUNIT_NUM_INTERVALS:
            return ears_buffer_get_size_samps((t_object *)e_ob, buf) * (1./atom_getfloat(v));
            break;

        case EARS_TIMEUNIT_NUM_ONSETS:
            return ears_buffer_get_size_samps((t_object *)e_ob, buf) * (1./(atom_getfloat(v)-1));
            break;

        case EARS_TIMEUNIT_SECONDS:
            return ears_ms_to_samps(atom_getfloat(v)*1000., buffer_getsamplerate(buf));
            break;

        case EARS_TIMEUNIT_MS:
        default:
            return ears_ms_to_samps(atom_getfloat(v), buffer_getsamplerate(buf));
            break;
    }
}

void earsbufobj_samps_to_atom(t_earsbufobj *e_ob, long samps, t_buffer_obj *buf, t_atom *a)
{
    switch (e_ob->l_timeunit) {
        case EARS_TIMEUNIT_SAMPS:
            atom_setlong(a, samps);
            break;
            
        case EARS_TIMEUNIT_DURATION_RATIO:
            atom_setfloat(a, ((float)samps)/ears_buffer_get_size_samps((t_object *)e_ob, buf));
            break;

        case EARS_TIMEUNIT_NUM_INTERVALS:
            atom_setfloat(a, 1./(((float)samps)/ears_buffer_get_size_samps((t_object *)e_ob, buf)));
            break;
            
        case EARS_TIMEUNIT_NUM_ONSETS:
            atom_setfloat(a, 1 + (1./(((float)samps)/ears_buffer_get_size_samps((t_object *)e_ob, buf))));
            break;

        case EARS_TIMEUNIT_SECONDS:
            atom_setfloat(a, ears_samps_to_ms(samps, buffer_getsamplerate(buf))/1000.);
            break;

        case EARS_TIMEUNIT_MS:
        default:
            atom_setfloat(a, ears_samps_to_ms(samps, buffer_getsamplerate(buf)));
            break;
    }
}



double earsbufobj_amplitude_to_linear(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_ampunit) {
        case EARS_AMPUNIT_DECIBEL:
            return ears_db_to_linear(value);
            break;
            
        case EARS_AMPUNIT_LINEAR:
        default:
            return value;
            break;
    }
}


double earsbufobj_amplitude_to_db(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_ampunit) {
        case EARS_AMPUNIT_LINEAR:
            return ears_linear_to_db(value);
            break;
            
        case EARS_AMPUNIT_DECIBEL:
        default:
            return value;
            break;
    }
}

double earsbufobj_angle_to_degrees(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_ampunit) {
        case EARS_ANGLEUNIT_DEGREES:
            return value;
            break;

        case EARS_ANGLEUNIT_TURNS:
            return ears_rad_to_deg(value * TWOPI);
            break;

        case EARS_ANGLEUNIT_RADIANS:
        default:
            return ears_rad_to_deg(value);
            break;
    }
}


double earsbufobj_angle_to_radians(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_ampunit) {
        case EARS_ANGLEUNIT_DEGREES:
            return ears_deg_to_rad(value);
            break;
            
        case EARS_ANGLEUNIT_TURNS:
            return value * TWOPI;
            break;
            
        case EARS_ANGLEUNIT_RADIANS:
        default:
            return value;
            break;
    }
}


// llllelem can be either a number or a t_pts
t_llll *earsbufobj_llllelem_to_linear(t_earsbufobj *e_ob, t_llllelem *elem)
{
    t_llll *out = llll_get();
    llll_appendhatom_clone(out, &elem->l_hatom);
    llll_flatten(out, 1, 0);
    
    for (t_llllelem *el = out->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            switch (e_ob->l_envampunit) {
                case EARS_AMPUNIT_DECIBEL:
                {
                    
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && sub_ll->l_head->l_next && is_hatom_number(&sub_ll->l_head->l_next->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_next->l_hatom, ears_db_to_linear(hatom_getdouble(&sub_ll->l_head->l_next->l_hatom)));
                }
                    break;
                default:
                    break;
            }
        } else {
            switch (e_ob->l_ampunit) {
                case EARS_AMPUNIT_DECIBEL:
                    hatom_setdouble(&el->l_hatom, ears_db_to_linear(hatom_getdouble(&el->l_hatom)));
                    break;
                default:
                    break;
            }
        }
    }
    
    return out;
}

// llllelem can be either a number or a t_pts
t_llll *earsbufobj_llllelem_to_linear_and_samples(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf)
{
    t_llll *out = llll_get();
    llll_appendhatom_clone(out, &elem->l_hatom);
    llll_flatten(out, 1, 0);

    double dur_samps = ears_buffer_get_size_samps((t_object *)e_ob, buf);
    double sr = ears_buffer_get_sr((t_object *)e_ob, buf);

    for (t_llllelem *el = out->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            switch (e_ob->l_envampunit) {
                case EARS_AMPUNIT_DECIBEL:
                {
                    
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && sub_ll->l_head->l_next && is_hatom_number(&sub_ll->l_head->l_next->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_next->l_hatom, ears_db_to_linear(hatom_getdouble(&sub_ll->l_head->l_next->l_hatom)));
                }
                    break;
                default:
                    break;
            }
            switch (e_ob->l_envtimeunit) {
                case EARS_TIMEUNIT_MS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, ears_ms_to_fsamps(hatom_getdouble(&sub_ll->l_head->l_hatom), sr));
                }
                    break;
                case EARS_TIMEUNIT_SECONDS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, ears_ms_to_fsamps(hatom_getdouble(&sub_ll->l_head->l_hatom)*1000., sr));
                }
                    break;
                case EARS_TIMEUNIT_DURATION_RATIO:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, hatom_getdouble(&sub_ll->l_head->l_hatom) * (dur_samps - 1));
                }
                    break;
                case EARS_TIMEUNIT_NUM_INTERVALS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, (1./hatom_getdouble(&sub_ll->l_head->l_hatom)) * (dur_samps - 1));
                }
                    break;
                case EARS_TIMEUNIT_NUM_ONSETS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, (1 + 1./hatom_getdouble(&sub_ll->l_head->l_hatom)) * (dur_samps - 1));
                }
                    break;
                default:
                    break;
            }
        } else {
            switch (e_ob->l_ampunit) {
                case EARS_AMPUNIT_DECIBEL:
                    hatom_setdouble(&el->l_hatom, ears_db_to_linear(hatom_getdouble(&el->l_hatom)));
                    break;
                default:
                    break;
            }
        }
    }
    
    return out;
}




t_llll *earsbufobj_llllelem_to_env_samples(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf)
{
    t_llll *out = llll_get();
    llll_appendhatom_clone(out, &elem->l_hatom);
    llll_flatten(out, 1, 0);
    
    double dur_samps = ears_buffer_get_size_samps((t_object *)e_ob, buf);
    double sr = ears_buffer_get_sr((t_object *)e_ob, buf);
    
    ears_llll_to_env_samples(out, dur_samps, sr, e_ob->l_envtimeunit);
    return out;
}




double earsbufobj_linear_to_output(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_ampunit) {
        case EARS_AMPUNIT_DECIBEL:
            return ears_linear_to_db(value);
            break;
            
        case EARS_AMPUNIT_LINEAR:
        default:
            return value;
            break;
    }
}




void earsbufobj_llll_convert_envtimeunit_and_normalize_range_do(t_earsbufobj *e_ob, t_llll *out, t_buffer_obj *buf,
                                                                e_ears_timeunit dest_envtimeunit,
                                                                double orig_from, double orig_to, char convert_from_decibels)
{
    double dur_samps = ears_buffer_get_size_samps((t_object *)e_ob, buf);
    double sr = ears_buffer_get_sr((t_object *)e_ob, buf);
    
    if (orig_from != 0 || orig_to != 1 || convert_from_decibels) {
        for (t_llllelem *el = out->l_head; el; el = el->l_next) {
            if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                if (sub_ll && sub_ll->l_head && sub_ll->l_head->l_next && is_hatom_number(&sub_ll->l_head->l_next->l_hatom)) {
                    if (convert_from_decibels)
                        hatom_setdouble(&sub_ll->l_head->l_next->l_hatom, ears_db_to_linear(hatom_getdouble(&sub_ll->l_head->l_next->l_hatom)));
                    else
                        hatom_setdouble(&sub_ll->l_head->l_next->l_hatom, rescale(hatom_getdouble(&sub_ll->l_head->l_next->l_hatom), orig_from, orig_to, 0., 1.));
                }
            } else {
                if (convert_from_decibels)
                    hatom_setdouble(&el->l_hatom, ears_db_to_linear(hatom_getdouble(&el->l_hatom)));
                else
                    hatom_setdouble(&el->l_hatom, rescale(hatom_getdouble(&el->l_hatom), orig_from, orig_to, 0., 1.));
            }
        }
    }
    
    
    if (e_ob->l_timeunit != dest_envtimeunit) {
        for (t_llllelem *el = out->l_head; el; el = el->l_next) {
            if (hatom_gettype(&el->l_hatom) == H_LLLL) {
                t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom)) {
                    double ctu = earsbufobj_convert_timeunit(e_ob, hatom_getdouble(&sub_ll->l_head->l_hatom), buf, dest_envtimeunit, true);
                    hatom_setdouble(&sub_ll->l_head->l_hatom, ctu);
                }
            }
        }
    }
}

t_llll *earsbufobj_llll_convert_envtimeunit_and_normalize_range(t_earsbufobj *e_ob, t_llll *ll, t_buffer_obj *buf,
                                                                e_ears_timeunit dest_envtimeunit,
                                                                double orig_from, double orig_to, char convert_from_decibels)
{
    if (!ll)
        return NULL;
    
    t_llll *out = llll_get();
    llll_appendllll_clone(out, ll);
    llll_flatten(out, 1, 0);
    
    earsbufobj_llll_convert_envtimeunit_and_normalize_range_do(e_ob, out, buf, dest_envtimeunit, orig_from, orig_to, convert_from_decibels);
    
    return out;
}



// llllelem can be either a number or a t_pts
t_llll *earsbufobj_llllelem_convert_envtimeunit_and_normalize_range(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf,
                                                                    e_ears_timeunit dest_envtimeunit,
                                                                    double orig_from, double orig_to, char convert_from_decibels)
{
    t_llll *out = llll_get();
    llll_appendhatom_clone(out, &elem->l_hatom);
    llll_flatten(out, 1, 0);
    
    earsbufobj_llll_convert_envtimeunit_and_normalize_range_do(e_ob, out, buf, dest_envtimeunit, orig_from, orig_to, convert_from_decibels);
    
    return out;
}

double earsbufobj_pitch_to_cents(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_pitchunit) {
        case EARS_PITCHUNIT_CENTS:
            return value;
            break;
            
        case EARS_PITCHUNIT_MIDI:
            return value*100.;
            break;
            
        case EARS_PITCHUNIT_HERTZ:
            return ears_hz_to_cents(value, EARS_MIDDLE_A_TUNING);
            break;
            
        case EARS_PITCHUNIT_FREQRATIO:
            return ears_ratio_to_cents(value);
            break;

        default:
            return value;
            break;
    }
}

double earsbufobj_pitch_to_hz(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_pitchunit) {
        case EARS_PITCHUNIT_CENTS:
            return ears_cents_to_hz(value, EARS_MIDDLE_A_TUNING);
            break;
            
        case EARS_PITCHUNIT_MIDI:
            return ears_cents_to_hz(value*100., EARS_MIDDLE_A_TUNING);
            break;
            
        case EARS_PITCHUNIT_HERTZ:
            return value;
            break;
            
        case EARS_PITCHUNIT_FREQRATIO:
            return ears_cents_to_hz(ears_ratio_to_cents(value), EARS_MIDDLE_A_TUNING);
            break;
            
        default:
            return value;
            break;
    }
}



double earsbufobj_freq_to_hz(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_frequnit) {
        case EARS_FREQUNIT_CENTS:
            return ears_cents_to_hz(value, EARS_MIDDLE_A_TUNING);
            break;
            
        case EARS_FREQUNIT_MIDI:
            return ears_cents_to_hz(value*100., EARS_MIDDLE_A_TUNING);
            break;

        case EARS_FREQUNIT_BPM:
            return value/60.;
            break;

        case EARS_FREQUNIT_HERTZ:
            return value;
            break;
            
        default:
            return value;
            break;
    }
}

double earsbufobj_freq_to_cents(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_frequnit) {
        case EARS_FREQUNIT_CENTS:
            return value;
            break;
            
        case EARS_FREQUNIT_MIDI:
            return value * 100;
            break;

        case EARS_FREQUNIT_BPM:
            return ears_hz_to_cents(value/60., EARS_MIDDLE_A_TUNING);
            break;

        case EARS_FREQUNIT_HERTZ:
            return ears_hz_to_cents(value, EARS_MIDDLE_A_TUNING);
            break;
            
        default:
            return value;
            break;
    }
}

double earsbufobj_freq_to_midi(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_frequnit) {
        case EARS_FREQUNIT_CENTS:
            return value/100.;
            break;
            
        case EARS_FREQUNIT_MIDI:
            return value;
            break;

        case EARS_FREQUNIT_BPM:
            return ears_hz_to_cents(value/60., EARS_MIDDLE_A_TUNING)/100.;
            break;
            
        case EARS_FREQUNIT_HERTZ:
            return ears_hz_to_cents(value, EARS_MIDDLE_A_TUNING)/100.;
            break;
            
        default:
            return value;
            break;
    }
}


// llllelem can be either a number or a t_pts
t_llll *earsbufobj_pitch_llllelem_to_cents_and_samples(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf)
{
    t_llll *out = llll_get();
    llll_appendhatom_clone(out, &elem->l_hatom);
    llll_flatten(out, 1, 0);
    
    double dur_samps = ears_buffer_get_size_samps((t_object *)e_ob, buf);
    double sr = ears_buffer_get_sr((t_object *)e_ob, buf);
    
    for (t_llllelem *el = out->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            switch (e_ob->l_pitchunit) {
                case EARS_PITCHUNIT_FREQRATIO:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && sub_ll->l_head->l_next && is_hatom_number(&sub_ll->l_head->l_next->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_next->l_hatom, ears_ratio_to_cents(hatom_getdouble(&sub_ll->l_head->l_next->l_hatom)));
                }
                    break;
                case EARS_PITCHUNIT_HERTZ:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && sub_ll->l_head->l_next && is_hatom_number(&sub_ll->l_head->l_next->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_next->l_hatom, ears_hz_to_cents(hatom_getdouble(&sub_ll->l_head->l_next->l_hatom), EARS_MIDDLE_A_TUNING));
                }
                    break;
                case EARS_PITCHUNIT_MIDI:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && sub_ll->l_head->l_next && is_hatom_number(&sub_ll->l_head->l_next->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_next->l_hatom, 100* hatom_getdouble(&sub_ll->l_head->l_next->l_hatom));
                }
                    break;
                default:
                    break;
            }
            switch (e_ob->l_envtimeunit) {
                case EARS_TIMEUNIT_MS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, ears_ms_to_fsamps(hatom_getdouble(&sub_ll->l_head->l_hatom), sr));
                }
                    break;
                case EARS_TIMEUNIT_SECONDS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, ears_ms_to_fsamps(hatom_getdouble(&sub_ll->l_head->l_hatom)*1000., sr));
                }
                    break;
                case EARS_TIMEUNIT_DURATION_RATIO:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, hatom_getdouble(&sub_ll->l_head->l_hatom) * (dur_samps - 1));
                }
                    break;
                case EARS_TIMEUNIT_NUM_INTERVALS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, (1./hatom_getdouble(&sub_ll->l_head->l_hatom)) * (dur_samps - 1));
                }
                    break;
                case EARS_TIMEUNIT_NUM_ONSETS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, (1. + (1./hatom_getdouble(&sub_ll->l_head->l_hatom))) * (dur_samps - 1));
                }
                    break;
                default:
                    break;
            }
        } else {
            switch (e_ob->l_pitchunit) {
                case EARS_PITCHUNIT_FREQRATIO:
                    hatom_setdouble(&el->l_hatom, ears_ratio_to_cents(hatom_getdouble(&el->l_hatom)));
                    break;
                case EARS_PITCHUNIT_HERTZ:
                    hatom_setdouble(&el->l_hatom, ears_hz_to_cents(hatom_getdouble(&el->l_hatom), EARS_MIDDLE_A_TUNING));
                    break;
                case EARS_PITCHUNIT_MIDI:
                    hatom_setdouble(&el->l_hatom, 100*hatom_getdouble(&el->l_hatom));
                    break;
                default:
                    break;
            }
        }
    }
    
    return out;
}


t_bool earsbufobj_is_sym_naming_mech(t_symbol *s)
{
    return s == gensym("!") || s == gensym("=") || s == gensym("_");
}



void earsbufobj_stopprogress_do(t_earsbufobj *e_ob, t_symbol *sym, short argc, t_atom *argv)
{
    t_object *b = NULL;
    e_ob->l_current_progress = 0;
    auto err = object_obex_lookup((t_object *)e_ob, _sym_pound_B, &b);
    if (err == MAX_ERR_NONE) {
        object_method(b, gensym("stopprogress"));
    }
}

void earsbufobj_startprogress_do(t_earsbufobj *e_ob, t_symbol *sym, short argc, t_atom *argv)
{
    t_object *b = NULL;
    e_ob->l_current_progress = 0;
    auto err = object_obex_lookup((t_object *)e_ob, _sym_pound_B, &b);
    if (err == MAX_ERR_NONE) {
        object_method(b, gensym("startprogress"), &e_ob->l_current_progress);
    }
}

void earsbufobj_updateprogress_do(t_earsbufobj *e_ob, t_symbol *sym, short argc, t_atom *argv)
{
    e_ob->l_current_progress = atom_getfloat(argv);
}

void earsbufobj_startprogress(t_earsbufobj *e_ob)
{
    defer(e_ob, (method)earsbufobj_startprogress_do, NULL, 0, NULL);
}

void earsbufobj_stopprogress(t_earsbufobj *e_ob)
{
    defer(e_ob, (method)earsbufobj_stopprogress_do, NULL, 0, NULL);
}


void earsbufobj_updateprogress(t_earsbufobj *e_ob, t_atom_float progress)
{
    if (e_ob->l_blocking == 0) {
        t_atom av;
        atom_setfloat(&av, progress);
        defer(e_ob, (method)earsbufobj_updateprogress_do, NULL, 1, &av);
    }
}

void earsbufobj_init_progress(t_earsbufobj *e_ob, long num_buffers)
{
    // we start in any case with a whole-object progress bar
    // this is due to the fact that we want the user to recognize straight away
    // that some processing is going on, even if the object is only processing
    // a single buffer.
    if (num_buffers == 1) {
        earsbufobj_updateprogress(e_ob, 1.);
    } else {
        earsbufobj_updateprogress(e_ob, 0.);
    }
}

long earsbufobj_iter_progress(t_earsbufobj *e_ob, long count, long num_buffers)
{
    earsbufobj_updateprogress(e_ob, (float)(1+count)/num_buffers);
    if (e_ob->l_must_stop) {
        e_ob->l_must_stop = 0;
        return 1;
    }
    return 0;
}



t_max_err earsbufobj_store_buffer_in_dictionary(t_earsbufobj *e_ob, t_buffer_obj *buf, t_dictionary *dict)
{
    t_max_err err = MAX_ERR_NONE;
    char entryname[2048];
    t_atom_long count = 0, block_size;
    
    earsbufobj_mutex_lock(e_ob);
    t_atom_long num_channels = ears_buffer_get_numchannels((t_object *)e_ob, buf);
    t_atom_float sr = ears_buffer_get_sr((t_object *)e_ob, buf);
    t_atom_long num_frames = ears_buffer_get_size_samps((t_object *)e_ob, buf);
    
    dictionary_appendlong(dict, gensym("numchannels"), num_channels);
    dictionary_appendlong(dict, gensym("numframes"), num_frames);
    dictionary_appendfloat(dict, gensym("sr"), sr);
    dictionary_appendsym(dict, gensym("name"), ears_buffer_get_name((t_object *)e_ob, buf));

    if (num_frames > 0 && num_channels > 0) {
        long num_atoms = num_frames * num_channels;
        t_atom *av = (t_atom *)bach_newptr(EARS_EMBED_BLOCK_SIZE * sizeof(t_atom));
        
        float *sample = buffer_locksamples(buf);
        if (!sample) {
            err = MAX_ERR_GENERIC;
            object_error((t_object *)e_ob, EARS_ERROR_BUF_CANT_READ);
        } else {
            t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
            t_atom_long    framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
            
            if (channelcount != num_channels || framecount != num_frames) {
                err = MAX_ERR_GENERIC;
                object_error((t_object *)e_ob, "Mismatch in number of samples.");
            } else {
                float *cursor = sample;
                
                while (num_atoms > 0) {
                    block_size = num_atoms > EARS_EMBED_BLOCK_SIZE ? EARS_EMBED_BLOCK_SIZE : num_atoms;
                    sprintf(entryname, "block_%010" ATOM_LONG_FMT_MODIFIER "d", count++);
                    
                    for (long i = 0; i < block_size; i++) {
//                        atom_setfloat(av + i, cursor[i]);
                        atom_setlong(av + i, *((t_int32 *)&(cursor[i])));
                    }
                    
                    dictionary_appendatoms(dict, gensym(entryname), block_size, av);
                    cursor += block_size;
                    num_atoms -= block_size;
                }
                
                dictionary_appendlong(dict, gensym("block_count"), count);
            }
            buffer_unlocksamples(buf);
            
            bach_freeptr(av);
        }
    } else {
        dictionary_appendlong(dict, gensym("block_count"), count);

    }

    earsbufobj_mutex_unlock(e_ob);

    return err;
}

t_max_err earsbufobj_retrieve_buffer_from_dictionary(t_earsbufobj *e_ob, t_dictionary *dict, t_buffer_obj *buf)
{
    char entryname[2048];
    t_atom_long num_channels = 0, num_frames = 0, block_count = 0;
    t_atom_float sr = 0;
    t_symbol *name = NULL;
    t_max_err err = MAX_ERR_NONE;

    long ac = 0;
    t_atom *av = NULL;
    long whole_numsamps = 0;
    t_float *whole_samps, *this_whole_samps;

    earsbufobj_mutex_lock(e_ob);

    err |= dictionary_getlong(dict, gensym("numchannels"), &num_channels);
    err |= dictionary_getlong(dict, gensym("numframes"), &num_frames);
    err |= dictionary_getfloat(dict, gensym("sr"), &sr);
    err |= dictionary_getsym(dict, gensym("name"), &name);
    err |= dictionary_getlong(dict, gensym("block_count"), &block_count);

    if (err) {
        earsbufobj_mutex_unlock(e_ob);
        object_error((t_object *)e_ob, "Error while retrieving saved buffer.");
        return MAX_ERR_GENERIC;
    } else if (num_channels == 0 || sr == 0) {
        earsbufobj_mutex_unlock(e_ob);
        object_error((t_object *)e_ob, "Mismatch in number of samples.");
        return MAX_ERR_GENERIC;
    } else if (name != ears_buffer_get_name((t_object *)e_ob, buf) && e_ob->l_bufouts_naming != EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED) {
        earsbufobj_mutex_unlock(e_ob);
        object_error((t_object *)e_ob, "Mismatch in buffer name.");
        return MAX_ERR_GENERIC;
    }
    
    ears_buffer_set_sr((t_object *)e_ob, buf, sr);
    ears_buffer_set_size_and_numchannels((t_object *)e_ob, buf, num_frames, num_channels);
    
    if (block_count == 0) {
        ears_buffer_clear((t_object *)e_ob, buf);
    } else {
        this_whole_samps = whole_samps = (t_float *) bach_newptr(block_count * EARS_EMBED_BLOCK_SIZE * sizeof(t_float));
        for (t_atom_long i = 0; i < block_count; i++) {
            sprintf(entryname, "block_%010ld", i);
            dictionary_getatoms(dict, gensym(entryname), &ac, &av);
            for (long j = 0; j < ac; j++) {
                long val = (t_int32)atom_getlong(av+j);
                this_whole_samps[j] = *((float *)(&val));
//                this_whole_samps[j] = (float)(t_atom_float)atom_getfloat(av+j);
            }
            whole_numsamps += ac;
            this_whole_samps += ac;
        }
        
        if (whole_numsamps != num_channels * num_frames) {
            object_error((t_object *)e_ob, "Wrong saved information about number of samples!");
        }
        
        float *sample = buffer_locksamples(buf);
        
        if (!sample) {
            err = EARS_ERR_CANT_READ;
            object_error((t_object *)e_ob, EARS_ERROR_BUF_CANT_READ);
        } else {
            t_atom_long    channelcount = buffer_getchannelcount(buf);        // number of floats in a frame
            t_atom_long    framecount   = buffer_getframecount(buf);            // number of floats long the buffer is for a single channel
            
            sysmem_copyptr(whole_samps, sample, MIN(whole_numsamps, channelcount * framecount) * sizeof(float));
            buffer_setdirty(buf);
            buffer_unlocksamples(buf);
        }
        
        bach_freeptr(whole_samps);
    }

    earsbufobj_mutex_unlock(e_ob);
    return EARS_ERR_NONE;
}
