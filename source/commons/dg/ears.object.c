#include "ears.commons.h"

#define EARS_ALLOCATIONVERBOSE false

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


t_hashtab *ears_hashtab_get()
{
    return (t_hashtab *)(gensym("ears")->s_thing);
}

void ears_hashtab_setup()
{
    if (!ears_hashtab_get()) {
        t_hashtab *h = hashtab_new(0);
        gensym("ears")->s_thing = (t_object *)h;
    }
}

void earsbufobj_buffer_release_raw(t_earsbufobj *e_ob, t_object *buf, t_symbol *name, char mustfree)
{    
    if (name && buf) {
        t_atom_long count = 0;
        t_hashtab *ht = ears_hashtab_get();
        if (ht) {
            t_max_err err = hashtab_lookuplong(ht, name, &count);
            if (err == MAX_ERR_NONE) {
                if (count > 1) {
                    if (EARS_ALLOCATIONVERBOSE)
                        post("--- ears allocation: Releasing buffer %s: now has count %ld", name->s_name, count-1);
                    hashtab_storelong(ht, name, count-1); // decrease reference count
                } else {
                    if (EARS_ALLOCATIONVERBOSE)
                        post("--- ears allocation: Releasing buffer %s: count is now 0. %s", name->s_name, mustfree ? "Also freeing buffer." : "Not freeing buffer.");
                    hashtab_chuckkey(ht, name);
                    if (mustfree) {
                        object_free_debug(buf); ///< This makes Max crash, for some weird reason... Gotta discover why, highest priority
                    }
                }
            } else {
                if (EARS_ALLOCATIONVERBOSE)
                    post("--- ears allocation: Trying to release buffer %s which was not autoassigned, nothing to do.", name->s_name);
            }
        }
    }
}

void earsbufobj_buffer_release(t_earsbufobj *e_ob, e_earsbufobj_in_out where, long store, long bufferidx, bool prevent_from_freeing)
{
    t_object *buf = earsbufobj_get_stored_buffer_obj(e_ob, where, store, bufferidx);
    t_symbol *name = earsbufobj_get_stored_buffer_name(e_ob, where, store, bufferidx);
    bool mustfree = !prevent_from_freeing && earsbufobj_is_buf_autoassigned(e_ob, where, store, bufferidx);
    
    earsbufobj_buffer_release_raw(e_ob, buf, name, mustfree);

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
                    post("--- ears allocation: Incrementing count for buffer %s: now has count %ld", name->s_name, count);
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
                    post("--- ears allocation: Clipping count for buffer %s: now has count %ld", name->s_name, count+1);
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


// THERE ARE ISSUES WITH buffer_ref_getobject() hence this function should NEVER be called
t_symbol *ears_bufferref_to_name(t_buffer_ref *ref)
{
    t_buffer_info info;
    t_object *obj = buffer_ref_getobject(ref);
    info.b_name = NULL;
    if (obj)
        buffer_getinfo(obj, &info);
    return info.b_name;
}



// Link one of the inlets or outlets to a given buffer via the buffer name.
// If no buffer exists with such name, a new buffer is created.

// Important: before calling earsbufobj_buffer_link, the buffer status of the corresponding buffer must be up to date
void earsbufobj_buffer_link(t_earsbufobj *e_ob, e_earsbufobj_in_out where, long store_index, long buffer_index, t_symbol *buf_name)
{
    // retrieving references
    t_buffer_ref **ref = NULL;
    t_symbol **name = NULL;
    t_object **buf = NULL;
    switch (where) {
        case EARSBUFOBJ_IN:
            if (store_index >= 0 && store_index < e_ob->l_numbufins && buffer_index >= 0 && buffer_index < e_ob->l_instore[store_index].num_stored_bufs) {
                ref = &e_ob->l_instore[store_index].stored_buf[buffer_index].l_ref;
                name = &e_ob->l_instore[store_index].stored_buf[buffer_index].l_name;
                buf = &e_ob->l_instore[store_index].stored_buf[buffer_index].l_buf;
            }
            break;

        case EARSBUFOBJ_OUT:
            if (store_index >= 0 && store_index < e_ob->l_numbufouts && buffer_index >= 0 && buffer_index < e_ob->l_outstore[store_index].num_stored_bufs) {
                ref = &e_ob->l_outstore[store_index].stored_buf[buffer_index].l_ref;
                name = &e_ob->l_outstore[store_index].stored_buf[buffer_index].l_name;
                buf = &e_ob->l_outstore[store_index].stored_buf[buffer_index].l_buf;
            }
            break;

        default:
            break;
    }
    
    
    if (!ref) {
        object_error((t_object *)e_ob, "Can't reference to buffer.");
        return;
    }

    if (!*ref) {
        *ref = buffer_ref_new((t_object *)e_ob, buf_name);
    } else {
        buffer_ref_set(*ref, buf_name);

        if (name && *name && buf && *buf) {
            if (e_ob->l_bufouts_naming != EARSBUFOBJ_NAMING_DYNAMIC)
                earsbufobj_buffer_release(e_ob, where, store_index, buffer_index);
        }
        
        // now the old buffer does not exist any more
//        long ex = buffer_ref_exists(buffer_ref_new((t_object *)e_ob, *name));
//        object_post((t_object *)e_ob, "%ld", ex);
    }
    
    if (!*ref) {
        object_error((t_object *)e_ob, "Can't create new buffer reference.");
        return;
    }
    
    // does the buffer reference actually point to an existing buffer??
    if (!buffer_ref_exists(*ref)) {
        // buffer does not exist.

        if (buf_name->s_thing) {
            // not a buffer, but the name has been used! (e.g. a database? a table?..)
            object_error((t_object *)e_ob, "The symbol %s is already in use for something different than a buffer. Using uniquely generated symbol instead.", buf_name->s_name);
            
            *name = symbol_unique();
            
            // must create an actual buffer with the unique name
            t_atom a;
            atom_setsym(&a, *name);
            t_object *b = (t_object *)object_new_typed(CLASS_BOX, gensym("buffer~"), 1, &a);
            buffer_ref_set(*ref, *name);
            *buf = b;
            if (earsbufobj_is_buf_autoassigned(e_ob, where, store_index, buffer_index)) {
                ears_hashtab_store(*name);
                ears_hashtab_inccount(*name);
            }

        } else {
            
            // must create an actual buffer with that name
            t_atom a;
            atom_setsym(&a, buf_name);
            t_object *b = (t_object *)object_new_typed(CLASS_BOX, gensym("buffer~"), 1, &a);
            buffer_ref_set(*ref, buf_name);
            *buf = b;
            *name = buf_name;

            if (earsbufobj_is_buf_autoassigned(e_ob, where, store_index, buffer_index)) {
                ears_hashtab_store(*name);
                ears_hashtab_inccount(*name);
            }
        }
        
    } else {
        // buffer already exists. Cool. Let's just get it.
        *buf = buffer_ref_getobject(*ref);
        *name = buf_name;
        ears_hashtab_inccount(*name);
    }
    
}



void earsbufobj_resize_store(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long new_size, char also_create_unique_buffers)
{
    long i;
    t_earsbufobj_store *store = earsbufobj_get_store(e_ob, type, store_idx);
    if (store->max_num_stored_bufs > 0 && new_size > store->max_num_stored_bufs)
        new_size = store->max_num_stored_bufs;
    
    if (!store->stored_buf && new_size > 0) { // should not happen, should be initialized at earsbufobj_setup()
        store->stored_buf = (t_earsbufobj_stored_buffer *)bach_newptrclear(new_size * sizeof(t_earsbufobj_stored_buffer));
        for (i = store->num_stored_bufs; i < new_size; i++)
            store->stored_buf[i].l_ref = NULL;
        store->num_stored_bufs = new_size;
    } else if (new_size > store->num_stored_bufs) {
        long old_num_bufs = store->num_stored_bufs;
        store->stored_buf = (t_earsbufobj_stored_buffer *)bach_resizeptr(store->stored_buf, new_size * sizeof(t_earsbufobj_stored_buffer));
        for (i = old_num_bufs; i < new_size; i++) {
            store->stored_buf[i].l_ref = NULL;
            store->stored_buf[i].l_buf = NULL;
            store->stored_buf[i].l_name = NULL;
            store->stored_buf[i].l_status = EARSBUFOBJ_BUFSTATUS_COPIED;
        }
        store->num_stored_bufs = new_size;
        if (!(type == EARSBUFOBJ_IN && e_ob->l_flags & EARSBUFOBJ_FLAG_DONT_DUPLICATE_INPUT_BUFFERS))
            if (also_create_unique_buffers)
                for (i = old_num_bufs; i < new_size; i++) {
                    t_symbol *s = NULL;
                    store->stored_buf[i].l_status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
                    if (type == EARSBUFOBJ_OUT)
                        s = earsbufobj_output_get_symbol_unique(e_ob, store_idx, i, &store->stored_buf[i].l_status);
                    else
                        s = symbol_unique();
                    if (s)
                        earsbufobj_buffer_link(e_ob, type, store_idx, i, s);
                }

    } else if (new_size < store->num_stored_bufs) {
        for (i = new_size; i < store->num_stored_bufs; i++)
            object_free_debug(store->stored_buf[i].l_ref);
        store->stored_buf = (t_earsbufobj_stored_buffer *)bach_resizeptr(store->stored_buf, MAX(1, new_size) * sizeof(t_earsbufobj_stored_buffer));
        store->num_stored_bufs = new_size;
    }
}

void earsbufobj_store_buffer_list(t_earsbufobj *e_ob, t_llll *buffers, long store_idx, char copy_format_to_corresponding_output_buffer)
{
    long count = 0;
    for (t_llllelem *elem = buffers->l_head; elem; elem = elem->l_next, count++) {
        if (hatom_gettype(&elem->l_hatom) == H_SYM) {
            // storing input buffer
            earsbufobj_store_buffer(e_ob, EARSBUFOBJ_IN, store_idx, count, hatom_getsym(&elem->l_hatom));
            if (copy_format_to_corresponding_output_buffer && count == 0)
                earsbufobj_store_copy_format(e_ob, EARSBUFOBJ_IN, store_idx, count, EARSBUFOBJ_OUT, 0, count);
            
        } else {
            // empty buffer will do.
            char *txtbuf = NULL;
            hatom_to_text_buf(&elem->l_hatom, &txtbuf);
            object_warn((t_object *)e_ob, "No buffer %s found; empty buffer created.", txtbuf);
            earsbufobj_store_empty_buffer(e_ob, EARSBUFOBJ_IN, store_idx, count);
            bach_freeptr(txtbuf);
        }
    }
}

long substitute_polybuffers(t_llll *ll)
{
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
                            char polybuffer_buffername[MAX_SYM_LENGTH];
                            snprintf_zero(polybuffer_buffername, MAX_SYM_LENGTH, "%s.%d", s->s_name, i);
                            llll_insertsym_before(gensym(polybuffer_buffername), el);
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
    t_llll *ll = llllobj_parse_llll((t_object *) e_ob, LLLL_OBJ_VANILLA, msg, ac, av, LLLL_PARSE_CLONE);

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


// destructive on args
t_llll *earsbufobj_extract_names_from_args(t_earsbufobj *e_ob, t_llll *args)
{
    t_llll *names = NULL;
    if (args && args->l_head) {
        if (hatom_gettype(&args->l_head->l_hatom) == H_LLLL && llll_contains_only_symbols_and_at_least_one(hatom_getllll(&args->l_head->l_hatom))) {
            names = llll_get();
            llll_appendhatom_clone(names, &args->l_head->l_hatom);
            llll_behead(args);
        } else if (hatom_gettype(&args->l_head->l_hatom) == H_SYM) {
            names = symbol2llll(hatom_getsym(&args->l_head->l_hatom));
            llll_behead(args);
        }
    }
    
    return names;
}

void earsbufobj_init(t_earsbufobj *e_ob, long flags)
{
    ears_hashtab_setup();
    
    /// attributes (these must be done before the actual initialization)
    e_ob->l_ampunit = EARSBUFOBJ_AMPUNIT_LINEAR;
    e_ob->l_timeunit = EARSBUFOBJ_TIMEUNIT_MS;
    e_ob->l_envampunit = EARSBUFOBJ_AMPUNIT_LINEAR;
    e_ob->l_envtimeunit = EARSBUFOBJ_TIMEUNIT_DURATION_RATIO;
    e_ob->l_pitchunit = EARSBUFOBJ_PITCHUNIT_CENTS;
    e_ob->l_bufouts_naming = EARSBUFOBJ_NAMING_STATIC;

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
    long i, j, h;

    // INLETS;
    long max_in_len = strlen(in_types);
    e_ob->l_numins = 0;
    e_ob->l_numbufins = 0;
    
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
                earsbufobj_resize_store(e_ob, EARSBUFOBJ_IN, j, 1, false);
                e_ob->l_instore[j].stored_buf[0].l_status = EARSBUFOBJ_BUFSTATUS_COPIED;
                if (!(e_ob->l_flags & EARSBUFOBJ_FLAG_DONT_DUPLICATE_INPUT_BUFFERS)) {
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
    
    llllobj_obj_setup((t_llllobj_object *)e_ob, num_lllls_in, out_types_wk);
    
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
        if (out_types[i] == 'e' || out_types[i] == 'E')
            e_ob->l_numbufouts ++;
    }
    if (e_ob->l_numbufouts) {
        e_ob->l_outstore = (t_earsbufobj_store *) bach_newptrclear(e_ob->l_numbufouts * sizeof(t_earsbufobj_store));
        t_llllelem *elem;
        for (i = 0, j = 0, elem = (outlet_names ? outlet_names->l_head : NULL); i < max_out_len; i++) {
            if (out_types[i] == 'e' || out_types[i] == 'E') {
                long outstoresize = 1;
                if (out_types[i] == 'e') {
                    e_ob->l_outstore[j].max_num_stored_bufs = 1;
                } else if (out_types[i] == 'E') { // 'E' buffer outlets can output a list of buffers from a single outlet
                    e_ob->l_outstore[j].max_num_stored_bufs = 0; // no limit
                    outstoresize = MAX(1, elem && hatom_gettype(&elem->l_hatom) == H_LLLL ? hatom_getllll(&elem->l_hatom)->l_size : 1);
                }
                earsbufobj_resize_store(e_ob, EARSBUFOBJ_OUT, j, outstoresize, false);
                t_llllelem *subelem = (elem && hatom_gettype(&elem->l_hatom) == H_LLLL) ? hatom_getllll(&elem->l_hatom)->l_head : NULL;
                for (h = 0; h < outstoresize; h++) {
                    t_symbol *name = NULL;
                    if (subelem && hatom_gettype(&subelem->l_hatom) == H_SYM) {
                        name = hatom_getsym(&subelem->l_hatom);
                        e_ob->l_outstore[j].stored_buf[h].l_status = EARSBUFOBJ_BUFSTATUS_USERNAMED;
                    } else if (outstoresize == 1 && elem && hatom_gettype(&elem->l_hatom) == H_SYM) {
                        name = hatom_getsym(&elem->l_hatom);
                        e_ob->l_outstore[j].stored_buf[h].l_status = EARSBUFOBJ_BUFSTATUS_USERNAMED;
                    } else {
                        if (e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_COPY)
                                e_ob->l_outstore[j].stored_buf[h].l_status = EARSBUFOBJ_BUFSTATUS_COPIED;
                        else {
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
                    for (h = 0; h < outstoresize; h++)
                        llll_appendsym(ll, e_ob->l_outstore[j].stored_buf[h].l_name);
                    llll_appendllll(e_ob->l_outnames, ll);
                }
                
                elem = (elem ? elem->l_next : NULL);
                j++;
            }
        }
    }
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
                if (elem && hatom_gettype(&elem->l_hatom) == H_SYM) {
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

void earsbufobj_free(t_earsbufobj *e_ob)
{
    long i, j;
    
    e_ob->l_is_freeing = 1;
    
    for (i = e_ob->l_numins - 1; i > 0; i--)
        object_free_debug(e_ob->l_proxy[i]);
    bach_freeptr(e_ob->l_proxy);

    // deleting references
    for (i = 0; i < e_ob->l_numbufins; i++) {
        for (j = 0; j < e_ob->l_instore[i].num_stored_bufs; j++) {
            if (e_ob->l_instore[i].stored_buf[j].l_ref)
                object_free_debug(e_ob->l_instore[i].stored_buf[j].l_ref);
            if (e_ob->l_instore[i].stored_buf[j].l_name && e_ob->l_instore[i].stored_buf[j].l_buf)
                earsbufobj_buffer_release(e_ob, EARSBUFOBJ_IN, i, j);
        }
        bach_freeptr(e_ob->l_instore[i].stored_buf);
    }
    bach_freeptr(e_ob->l_instore);

    for (i = 0; i < e_ob->l_numbufouts; i++) {
        for (j = 0; j < e_ob->l_outstore[i].num_stored_bufs; j++) {
            if (e_ob->l_outstore[i].stored_buf[j].l_ref)
                object_free_debug(e_ob->l_outstore[i].stored_buf[j].l_ref);
            if (e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_DYNAMIC) {
                earsbufobj_release_generated_outnames(e_ob);
            } else {
                if (e_ob->l_outstore[i].stored_buf[j].l_name && e_ob->l_outstore[i].stored_buf[j].l_buf)
                    earsbufobj_buffer_release(e_ob, EARSBUFOBJ_OUT, i, j);
            }
        }
        bach_freeptr(e_ob->l_outstore[i].stored_buf);
    }

    bach_freeptr(e_ob->l_outstore);
//    bach_freeptr(e_ob->l_outlet);
    
    llll_free(e_ob->l_outnames);
    llll_free(e_ob->l_generated_outnames);
    
    if (e_ob->l_mutex)
        systhread_mutex_free_debug(e_ob->l_mutex);

}


t_max_err earsbufobj_notify(t_earsbufobj *e_ob, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    long i, j;
    long res = MAX_ERR_NONE;
    
    if (e_ob->l_is_freeing)
        return res;
    
    for (i = 0; i < e_ob->l_numbufouts; i++)
        for (j = 0; j < e_ob->l_outstore[i].num_stored_bufs; j++) {
            t_buffer_ref *ref = earsbufobj_get_outlet_buffer_ref(e_ob, i, j);
            if (ref) {
                if (buffer_ref_notify(ref, s, msg, sender, data) != MAX_ERR_NONE)
                    res = MAX_ERR_GENERIC;
                
                if (!e_ob->l_is_freeing && msg == gensym("globalsymbol_unbinding")) { // maybe removing some buffer?
                    if (data == earsbufobj_get_outlet_buffer_obj(e_ob, i, j)) {
                        t_buffer_obj *new_obj = buffer_ref_getobject(ref);
                        if (!new_obj) {
                            earsbufobj_buffer_release(e_ob, EARSBUFOBJ_OUT, i, j, true);
                            e_ob->l_outstore[i].stored_buf[j].l_buf = NULL;
                            e_ob->l_outstore[i].stored_buf[j].l_name = NULL;
                            e_ob->l_outstore[i].stored_buf[j].l_ref = NULL;
                        } else {
                            e_ob->l_outstore[i].stored_buf[j].l_buf = new_obj;
                        }
                    }
                }
            }
        }
    
    for (i = 0; i < e_ob->l_numbufins; i++)
        for (j = 0; j < e_ob->l_instore[i].num_stored_bufs; j++) {
            t_buffer_ref *ref = earsbufobj_get_inlet_buffer_ref(e_ob, i, j);
            if (ref) {
                if (buffer_ref_notify(ref, s, msg, sender, data) != MAX_ERR_NONE)
                    res = MAX_ERR_GENERIC;
                
                if (!e_ob->l_is_freeing && msg == gensym("globalsymbol_unbinding")) { // maybe removing some buffer?
                    if (data == earsbufobj_get_inlet_buffer_obj(e_ob, i, j)) {
                        t_buffer_obj *new_obj = buffer_ref_getobject(ref);
                        if (!new_obj) {
                            earsbufobj_buffer_release(e_ob, EARSBUFOBJ_IN, i, j, true);
                            e_ob->l_instore[i].stored_buf[j].l_buf = NULL;
                            e_ob->l_instore[i].stored_buf[j].l_name = NULL;
                            e_ob->l_instore[i].stored_buf[j].l_ref = NULL;
                        } else {
                            e_ob->l_instore[i].stored_buf[j].l_buf = new_obj;
                        }
                    }
                }
            }
        }
    
    return res;
}


/*
t_max_err earsbufobj_notify(t_earsbufobj *e_ob, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    long i, j;
    long res = MAX_ERR_NONE;
    for (i = 0; i < e_ob->l_numbufouts; i++)
        for (j = 0; j < e_ob->l_outstore[i].num_stored_bufs; j++) {
            t_buffer_ref *ref = earsbufobj_get_outlet_buffer_ref(e_ob, i, j);
            if (ref) {
                if (buffer_ref_notify(ref, s, msg, sender, data) != MAX_ERR_NONE)
                    res = MAX_ERR_GENERIC;
                
                if (!e_ob->l_is_freeing && msg == gensym("globalsymbol_unbinding")) { // maybe removing some buffer?
                    if (data == earsbufobj_get_outlet_buffer_obj(e_ob, i, j)) {
// N.B.: buffer_ref_getobject() is unreliable for our instores and outstores
//                    t_buffer_obj *new_obj = buffer_ref_getobject(ref);
//                    if (!new_obj) {
                        
                        t_symbol *name = earsbufobj_get_outlet_buffer_name(e_ob, i, j);
                        if (ears_hashtab_getcount(name) > 0) {
                            // gotta recreate buffer
                            t_atom a;
                            atom_setsym(&a, name);
                            t_object *b = (t_object *)object_new_typed(CLASS_BOX, gensym("buffer~"), 1, &a);
                            buffer_ref_set(ref, name);
                            ears_hashtab_inccount(name);
                            e_ob->l_outstore[i].stored_buf[j].l_buf = b;
                            e_ob->l_outstore[i].stored_buf[j].l_name = name;
                        }
                    }
                }
            }
        }

    
    // checking if some of the stored buffer in the inlets has been undergone unbinding
    // This could happen for the objects that DO NOT keep a cloned copy of the buffer in their instore
    for (i = 0; i < e_ob->l_numbufins; i++)
        for (j = 0; j < e_ob->l_instore[i].num_stored_bufs; j++) {
            t_buffer_ref *ref = earsbufobj_get_inlet_buffer_ref(e_ob, i, j);
            if (ref) {
                if (buffer_ref_notify(ref, s, msg, sender, data) != MAX_ERR_NONE)
                    res = MAX_ERR_GENERIC;
                
                if (!e_ob->l_is_freeing && msg == gensym("globalsymbol_unbinding")) { // maybe removing some buffer?
                    if (data == earsbufobj_get_inlet_buffer_obj(e_ob, i, j)) {
                        t_buffer_obj *new_obj = buffer_ref_getobject(ref);
                        if (!new_obj) {
                            e_ob->l_instore[i].stored_buf[j].l_buf = NULL;
                            e_ob->l_instore[i].stored_buf[j].l_name = NULL;
                            e_ob->l_instore[i].stored_buf[j].l_ref = NULL;
                        }
                    }
                }
            }
        }
    
    return res;
} */

long earsbufobj_get_num_stored_buffers(t_earsbufobj *e_ob, e_earsbufobj_in_out where)
{
    long i, sum = 0;
    switch (where) {
        case EARSBUFOBJ_IN:
            for (i = 0; i < e_ob->l_numbufins; i++)
                sum += e_ob->l_instore[i].num_stored_bufs;
            break;
            
        case EARSBUFOBJ_OUT:
            for (i = 0; i < e_ob->l_numbufouts; i++)
                sum += e_ob->l_outstore[i].num_stored_bufs;
            break;
            
        default:
            break;
    }
    return sum;
}


void earsbufobj_open(t_earsbufobj *e_ob)
{
    long sum = earsbufobj_get_num_stored_buffers(e_ob, EARSBUFOBJ_OUT);
    if (sum > EARS_MAX_BUFFERS_SHOWN_ON_DOUBLECLICK)
        object_warn((t_object *)e_ob, "More than %ld buffers stored as output. Only first %ld buffers shown", EARS_MAX_BUFFERS_SHOWN_ON_DOUBLECLICK, EARS_MAX_BUFFERS_SHOWN_ON_DOUBLECLICK);
    
    long i, j, s = 0;
    for (i = 0; i < e_ob->l_numbufouts && s < EARS_MAX_BUFFERS_SHOWN_ON_DOUBLECLICK; i++) {
        for (j = 0; j < e_ob->l_outstore[i].num_stored_bufs && s < EARS_MAX_BUFFERS_SHOWN_ON_DOUBLECLICK; j++, s++) {
            //            t_symbol *fee = gensym("fee"); // 0x00000001056a5ce0
            buffer_view(earsbufobj_get_outlet_buffer_obj(e_ob, i, j)); // 0x0000608005690dc0
        }
    }
}


// this lets us double-click on index~ to open up the buffer~ it references
void earsbufobj_dblclick(t_earsbufobj *e_ob)
{
    earsbufobj_open(e_ob);
}

// this lets us save the buffer
void earsbufobj_writegeneral(t_earsbufobj *e_ob, t_symbol *msg, long ac, t_atom *av)
{
    t_buffer_obj *buf = NULL;
    long which_buffer = 0;
    
    if (ac && atom_gettype(av) == A_LONG) {
        which_buffer = atom_getlong(av) - 1;
        ac--;
        av++;
    }
    
    if ((buf = earsbufobj_get_outlet_buffer_obj(e_ob, 0, which_buffer)))
        typedmess(buf, msg, ac, av);
}


void earsbufobj_reset(t_earsbufobj *e_ob)
{
    for (long i = 0; i < LLLL_MAX_OUTLETS; i++)
        e_ob->l_generated_outname_count[i] = 0;
}


void earsbufobj_add_common_methods(t_class *c)
{
    class_addmethod(c, (method)earsbufobj_notify, "notify", A_CANT, 0);
    class_addmethod(c, (method)earsbufobj_dblclick, "dblclick", A_CANT, 0);
    class_addmethod(c, (method)earsbufobj_reset, "reset", 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "write", A_GIMME, 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "writeaiff", A_GIMME, 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "writewave", A_GIMME, 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "writeau", A_GIMME, 0);
    class_addmethod(c, (method)earsbufobj_writegeneral, "writeflac", A_GIMME, 0);
    class_addmethod(c, (method)earsbufobj_open, "open", 0);


#ifdef EARS_FROMFILE_NATIVE_MP3_HANDLING
    // TO DO: make this better, only make this ONCE when ears starts, via an extension
    if (mpg123_init() != MPG123_OK)
        error("Error while loading mpg123 library.");
#endif
}

void earsbufobj_class_add_outname_attr(t_class *c)
{
    CLASS_ATTR_LLLL(c, "outname", 0, t_earsbufobj, l_outnames, earsbufobj_getattr_outname, earsbufobj_setattr_outname);
    CLASS_ATTR_STYLE_LABEL(c,"outname",0,"text","Output Buffer Names");
    CLASS_ATTR_BASIC(c, "outname", 0);
    // @description Sets the name for each one of the buffer outlets. Leave blank to auto-assign
    // unique names.
}


t_max_err earsbufobj_setattr_ampunit(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            e_ob->l_ampunit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            if (s == gensym("db") || s == gensym("dB") || s == gensym("decibel") || s == gensym("decibels"))
                e_ob->l_ampunit = EARSBUFOBJ_AMPUNIT_DECIBEL;
            else if (s == gensym("lin") || s == gensym("linear"))
                e_ob->l_ampunit = EARSBUFOBJ_AMPUNIT_LINEAR;
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
            if (s == gensym("db") || s == gensym("decibel"))
                e_ob->l_envampunit = EARSBUFOBJ_AMPUNIT_DECIBEL;
            else if (s == gensym("lin") || s == gensym("linear"))
                e_ob->l_envampunit = EARSBUFOBJ_AMPUNIT_LINEAR;
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
            if (s == gensym("ms") || s == gensym("millisecond") || s == gensym("milliseconds"))
                e_ob->l_timeunit = EARSBUFOBJ_TIMEUNIT_MS;
            else if (s == gensym("perc") || s == gensym("percentage") || s == gensym("ratio") || s == gensym("durationratio") || s == gensym("relative"))
                e_ob->l_timeunit = EARSBUFOBJ_TIMEUNIT_DURATION_RATIO;
            else if (s == gensym("samps") || s == gensym("samples"))
                e_ob->l_timeunit = EARSBUFOBJ_TIMEUNIT_SAMPS;
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
    // @description Sets the unit for time values: Milliseconds (default), Samples or Relative (0. to 1. as a percentage of the buffer length).
}

t_max_err earsbufobj_setattr_pitchunit(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            e_ob->l_pitchunit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            if (s == gensym("cents") || s == gensym("midicents") || s == gensym("MIDIcents"))
                e_ob->l_pitchunit = EARSBUFOBJ_PITCHUNIT_CENTS;
            else if (s == gensym("midi") || s == gensym("MIDI") || s == gensym("semitones"))
                e_ob->l_pitchunit = EARSBUFOBJ_PITCHUNIT_MIDI;
            else if (s == gensym("freqratio") || s == gensym("fr") || s == gensym("ratio"))
                e_ob->l_pitchunit = EARSBUFOBJ_PITCHUNIT_FREQRATIO;
        }
    }
    return MAX_ERR_NONE;
}

                     
void earsbufobj_class_add_pitchunit_attr(t_class *c)
{
    CLASS_ATTR_CHAR(c, "pitchunit", 0, t_earsbufobj, l_pitchunit);
    CLASS_ATTR_STYLE_LABEL(c,"pitchunit",0,"enumindex","Pitch Values Are In");
    CLASS_ATTR_ENUMINDEX(c,"pitchunit", 0, "Cents MIDI Frequency Ratio");
    CLASS_ATTR_ACCESSORS(c, "pitchunit", NULL, earsbufobj_setattr_pitchunit);
    CLASS_ATTR_BASIC(c, "pitchunit", 0);
    CLASS_ATTR_CATEGORY(c, "pitchunit", 0, "Units");
    // @description Sets the unit for pitch values: Cents (default), MIDI, or frequency ratio.
}



t_max_err earsbufobj_setattr_envtimeunit(t_earsbufobj *e_ob, void *attr, long argc, t_atom *argv)
{
    if (argc && argv) {
        if (atom_gettype(argv) == A_LONG)
            e_ob->l_envtimeunit = atom_getlong(argv);
        else if (atom_gettype(argv) == A_SYM) {
            t_symbol *s = atom_getsym(argv);
            if (s == gensym("ms") || s == gensym("millisecond") || s == gensym("milliseconds"))
                e_ob->l_envtimeunit = EARSBUFOBJ_TIMEUNIT_MS;
            else if (s == gensym("perc") || s == gensym("percentage") || s == gensym("ratio") || s == gensym("durationratio") || s == gensym("relative"))
                e_ob->l_envtimeunit = EARSBUFOBJ_TIMEUNIT_DURATION_RATIO;
            else if (s == gensym("samps") || s == gensym("samples"))
                e_ob->l_envtimeunit = EARSBUFOBJ_TIMEUNIT_SAMPS;
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
    // @description Sets the unit for time values inside envelopes: Milliseconds (default), Samples or Relative (0. to 1 as a percentage of the buffer length)
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
                ears_hashtab_clipcount(s);
                earsbufobj_buffer_release_raw(e_ob, buf, s, true);
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
            e_ob->l_bufouts_naming = (e_earsbufobj_namings)atom_getlong(argv);
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
    // @description Chooses the output buffer naming policy
}


t_buffer_ref *earsbufobj_get_inlet_buffer_ref(t_earsbufobj *e_ob, long store_idx, long buffer_idx)
{
    if (store_idx >= 0 && store_idx < e_ob->l_numbufins && buffer_idx >= 0 && buffer_idx <= e_ob->l_instore[store_idx].num_stored_bufs)
        return e_ob->l_instore[store_idx].stored_buf[buffer_idx].l_ref;

    return NULL;
}


t_object *earsbufobj_get_inlet_buffer_obj(t_earsbufobj *e_ob, long store_idx, long buffer_idx)
{
    if (store_idx >= 0 && store_idx < e_ob->l_numbufins && buffer_idx >= 0 && buffer_idx < e_ob->l_instore[store_idx].num_stored_bufs)
        return e_ob->l_instore[store_idx].stored_buf[buffer_idx].l_buf;
    
    return NULL;
}

t_symbol *earsbufobj_get_inlet_buffer_name(t_earsbufobj *e_ob, long store_idx, long buffer_idx)
{
    if (store_idx >= 0 && store_idx < e_ob->l_numbufins && buffer_idx >= 0 && buffer_idx < e_ob->l_instore[store_idx].num_stored_bufs)
        return e_ob->l_instore[store_idx].stored_buf[buffer_idx].l_name;
    
    return NULL;
}

t_buffer_ref *earsbufobj_get_outlet_buffer_ref(t_earsbufobj *e_ob, long store_idx, long buffer_idx)
{
    if (store_idx >= 0 && store_idx < e_ob->l_numbufouts && buffer_idx >= 0 && buffer_idx < e_ob->l_outstore[store_idx].num_stored_bufs)
        return e_ob->l_outstore[store_idx].stored_buf[buffer_idx].l_ref;
    
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


long earsbufobj_outlet_to_bufoutlet(t_earsbufobj *e_ob, long outlet)
{
    return outlet; // TO DO: must be invmap(outnum)
}

// pos are 0-based, differently from llll_getindex()
t_llllelem *earsbufobj_llll_getsymbol(t_llll *ll, long pos1, long pos2, long pos3)
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
void earsbufobj_llll_subssymbol(t_llll *ll, t_symbol *sym, long pos1, long pos2, long pos3)
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
            t_llllelem *el = earsbufobj_llll_getsymbol(e_ob->l_generated_outnames, outstore_idx, buffer_idx, e_ob->l_generated_outname_count[outstore_idx]);
            if (el && hatom_gettype(&el->l_hatom) == H_SYM) {
                sym = hatom_getsym(&el->l_hatom);
                if (status) *status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
            } else {
                sym = symbol_unique();
                earsbufobj_llll_subssymbol(e_ob->l_generated_outnames, sym, outstore_idx, buffer_idx, e_ob->l_generated_outname_count[outstore_idx]);
                if (status) *status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
            }
            e_ob->l_generated_outname_count[outstore_idx]++;
        }
            break;
            
        case EARSBUFOBJ_NAMING_STATIC:
        {
            t_llllelem *el = earsbufobj_llll_getsymbol(e_ob->l_generated_outnames, outstore_idx, buffer_idx, 0);
            if (el && hatom_gettype(&el->l_hatom) == H_SYM) {
                sym = hatom_getsym(&el->l_hatom);
                if (status) *status = EARSBUFOBJ_BUFSTATUS_USERNAMED;
            } else {
                sym = symbol_unique();
                earsbufobj_llll_subssymbol(e_ob->l_generated_outnames, sym, outstore_idx, buffer_idx, 0);
                if (status) *status = EARSBUFOBJ_BUFSTATUS_AUTOASSIGNED;
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
            t_buffer_ref **r = (t_buffer_ref **)bach_newptrclear(num_stored_bufs * sizeof(t_buffer_ref *));
            long j, c = 0;
            for (j = 0; j < num_stored_bufs; j++) {
                t_symbol *name = earsbufobj_get_outlet_buffer_name(e_ob, store, j);
                t_buffer_obj *buf = earsbufobj_get_outlet_buffer_obj(e_ob, store, j);
                t_buffer_ref *ref = earsbufobj_get_outlet_buffer_ref(e_ob, store, j);
                if (name) {
                    s[c] = name;
                    o[c] = buf;
                    r[c] = ref;
                    c++;
                }
            }
            
            // Now we change the outlet names
            for (j = 0; j < num_stored_bufs; j++) { // was: j < c
                if (e_ob->l_outstore[store].stored_buf[j].l_status != EARSBUFOBJ_BUFSTATUS_USERNAMED &&
                    (force_refresh_even_if_static || !(e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_STATIC && (j < c && s[j]))))
                    earsbufobj_buffer_link(e_ob, EARSBUFOBJ_OUT, store, j, earsbufobj_output_get_symbol_unique(e_ob, store, j, &e_ob->l_outstore[store].stored_buf[j].l_status));
            }
            
            // Now we SHOULD delete the "old" buffers - if unused
            // which I don't know how to do...
            // UPDATE: october 2018: NO, I don't think we should..
            
            bach_freeptr(s);
            bach_freeptr(o);
            bach_freeptr(r);
        }
    }
}



void earsbufobj_outlet_anything(t_earsbufobj *e_ob, long outnum, t_symbol *s, long ac, t_atom *av)
{
    llllobj_outlet_anything((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum, s, ac, av);
}

void earsbufobj_outlet_symbol_list(t_earsbufobj *e_ob, long outnum, long numsymbols, t_symbol **s)
{
    if (numsymbols > 0) {
        if (numsymbols == 1)
            llllobj_outlet_anything((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum, s[0], 0, NULL);
        else {
            t_atom *av = (t_atom *)bach_newptr(numsymbols * sizeof(t_atom));
            for (long i = 0; i < numsymbols; i++)
                atom_setsym(av+i, s[i]);
            llllobj_outlet_anything((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum, _sym_list, numsymbols, av);
            bach_freeptr(av);
        }
    }
}

void earsbufobj_outlet_llll(t_earsbufobj *e_ob, long outnum, t_llll *ll)
{
    llllobj_outlet_llll((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum, ll);
}

void earsbufobj_outlet_bang(t_earsbufobj *e_ob, long outnum)
{
    llllobj_outlet_bang((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum);
}

void earsbufobj_outlet_buffer(t_earsbufobj *e_ob, long outnum)
{
    if (outnum >= 0 && outnum < e_ob->l_ob.l_numouts) {
        long store = earsbufobj_outlet_to_bufoutlet(e_ob, outnum);
        if (e_ob->l_outstore[store].num_stored_bufs > 0) {
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
                if (c == 1)
                    llllobj_outlet_anything((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum, atom_getsym(a), 0, NULL);
                else
                    llllobj_outlet_anything((t_object *)e_ob, LLLL_OBJ_VANILLA, outnum, _sym_list, c, a);
            }
            
            bach_freeptr(a);
        }
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




void earsbufobj_store_buffer(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx, t_symbol *buffername)
{
    if (buffername) {
        switch (type) {
            case EARSBUFOBJ_IN:
                if (store_idx >= 0 && store_idx < e_ob->l_numbufins && buffer_idx >= 0 && buffer_idx < e_ob->l_instore[store_idx].num_stored_bufs) {
                    t_earsbufobj_store *store = &e_ob->l_instore[store_idx];
                    if (e_ob->l_flags & EARSBUFOBJ_FLAG_DONT_DUPLICATE_INPUT_BUFFERS) {
                        store->stored_buf[buffer_idx].l_name = buffername;
                        if (!store->stored_buf[buffer_idx].l_ref)
                            store->stored_buf[buffer_idx].l_ref = buffer_ref_new((t_object *)e_ob, buffername);
                        else
                            buffer_ref_set(store->stored_buf[buffer_idx].l_ref, buffername);
                        store->stored_buf[buffer_idx].l_buf = buffer_ref_getobject(store->stored_buf[buffer_idx].l_ref);
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
                        object_error((t_object *)e_ob, EARS_ERROR_BUF_NO_BUFFER_NAMED, buffername ? buffername->s_name : "???");
                        store->stored_buf[buffer_idx].l_ref = NULL;
                        store->stored_buf[buffer_idx].l_name = NULL;
//                        earsbufobj_buffer_link(e_ob, type, store_idx, buffer_idx, symbol_unique()); // storing an empty buffer instead
                    }
                }
                break;
                
            case EARSBUFOBJ_OUT:
                if (store_idx >= 0 && store_idx < e_ob->l_numbufouts && buffer_idx >= 0 && buffer_idx < e_ob->l_outstore[store_idx].num_stored_bufs) {
                    t_earsbufobj_store *store = &e_ob->l_outstore[store_idx];
                    if (e_ob->l_bufouts_naming == EARSBUFOBJ_NAMING_COPY) {
                        store->stored_buf[buffer_idx].l_name = buffername;
                        if (!store->stored_buf[buffer_idx].l_ref)
                            store->stored_buf[buffer_idx].l_ref = buffer_ref_new((t_object *)e_ob, buffername);
                        else
                            buffer_ref_set(store->stored_buf[buffer_idx].l_ref, buffername);
                        store->stored_buf[buffer_idx].l_buf = buffer_ref_getobject(store->stored_buf[buffer_idx].l_ref);
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
                t_buffer_obj *obj = earsbufobj_get_inlet_buffer_obj(e_ob, store_idx, buffer_idx);
                ears_buffer_set_size_and_numchannels((t_object *)e_ob, obj, 0, 1);
                ears_buffer_set_sr((t_object *)e_ob, obj, EARS_DEFAULT_SR);
            }
            break;
            
        case EARSBUFOBJ_OUT:
            if (store_idx >= 0 && store_idx < e_ob->l_numbufouts && buffer_idx >= 0 && buffer_idx < e_ob->l_outstore[store_idx].num_stored_bufs) {
                t_buffer_obj *obj = earsbufobj_get_outlet_buffer_obj(e_ob, store_idx, buffer_idx);
                ears_buffer_set_size_and_numchannels((t_object *)e_ob, obj, 0, 1);
                ears_buffer_set_sr((t_object *)e_ob, obj, EARS_DEFAULT_SR);
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



t_buffer_ref *earsbufobj_get_stored_buffer_ref(t_earsbufobj *e_ob, e_earsbufobj_in_out type, long store_idx, long buffer_idx)
{
    switch (type) {
        case EARSBUFOBJ_IN:
            if (store_idx >= 0 && store_idx < e_ob->l_numbufins && buffer_idx >= 0 && buffer_idx < e_ob->l_instore[store_idx].num_stored_bufs)
                return e_ob->l_instore[store_idx].stored_buf[buffer_idx].l_ref;
            break;
            
        case EARSBUFOBJ_OUT:
            if (store_idx >= 0 && store_idx < e_ob->l_numbufouts && buffer_idx >= 0 && buffer_idx < e_ob->l_outstore[store_idx].num_stored_bufs)
                return e_ob->l_outstore[store_idx].stored_buf[buffer_idx].l_ref;
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

void earsbufobj_store_copy_format(t_earsbufobj *e_ob, e_earsbufobj_in_out source, long source_store_idx, long source_buffer_idx, e_earsbufobj_in_out dest, long dest_store_idx, long dest_buffer_idx)
{
    t_buffer_obj *from = earsbufobj_get_stored_buffer_obj(e_ob, source, source_store_idx, source_buffer_idx);
    t_buffer_obj *to = earsbufobj_get_stored_buffer_obj(e_ob, dest, dest_store_idx, dest_buffer_idx);
    if (from && to)
        ears_buffer_copy_format((t_object *)e_ob, from, to);
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




double earsbufobj_input_to_ms(t_earsbufobj *e_ob, double value, t_buffer_obj *buf)
{
    switch (e_ob->l_timeunit) {
        case EARSBUFOBJ_TIMEUNIT_SAMPS:
            return ears_samps_to_ms(value, buffer_getsamplerate(buf));
            break;
            
        case EARSBUFOBJ_TIMEUNIT_DURATION_RATIO:
            return ears_buffer_get_size_ms((t_object *)e_ob, buf) * value;
            break;
            
        case EARSBUFOBJ_TIMEUNIT_MS:
        default:
            return value;
            break;
    }
}

// TO DO: handle negative values
double earsbufobj_input_to_fsamps(t_earsbufobj *e_ob, double value, t_buffer_obj *buf)
{
    switch (e_ob->l_timeunit) {
        case EARSBUFOBJ_TIMEUNIT_SAMPS:
            return value;
            break;
            
        case EARSBUFOBJ_TIMEUNIT_DURATION_RATIO:
            return ears_buffer_get_size_samps((t_object *)e_ob, buf) * value;
            break;
            
        case EARSBUFOBJ_TIMEUNIT_MS:
        default:
            return ears_ms_to_fsamps(value, buffer_getsamplerate(buf));
            break;
    }
}

long earsbufobj_input_to_samps(t_earsbufobj *e_ob, double value, t_buffer_obj *buf)
{
    return round(earsbufobj_input_to_fsamps(e_ob, value, buf));
}

long earsbufobj_atom_to_samps(t_earsbufobj *e_ob, t_atom *v, t_buffer_obj *buf)
{
    switch (e_ob->l_timeunit) {
        case EARSBUFOBJ_TIMEUNIT_SAMPS:
            return atom_getlong(v);
            break;
            
        case EARSBUFOBJ_TIMEUNIT_DURATION_RATIO:
            return ears_buffer_get_size_samps((t_object *)e_ob, buf) * atom_getfloat(v);
            break;
            
        case EARSBUFOBJ_TIMEUNIT_MS:
        default:
            return ears_ms_to_samps(atom_getfloat(v), buffer_getsamplerate(buf));
            break;
    }
}

void earsbufobj_samps_to_atom(t_earsbufobj *e_ob, long samps, t_buffer_obj *buf, t_atom *a)
{
    switch (e_ob->l_timeunit) {
        case EARSBUFOBJ_TIMEUNIT_SAMPS:
            atom_setlong(a, samps);
            break;
            
        case EARSBUFOBJ_TIMEUNIT_DURATION_RATIO:
            atom_setfloat(a, ((float)samps)/ears_buffer_get_size_samps((t_object *)e_ob, buf));
            break;
            
        case EARSBUFOBJ_TIMEUNIT_MS:
        default:
            atom_setfloat(a, ears_samps_to_ms(samps, buffer_getsamplerate(buf)));
            break;
    }
}



double earsbufobj_input_to_linear(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_ampunit) {
        case EARSBUFOBJ_AMPUNIT_DECIBEL:
            return ears_db_to_linear(value);
            break;
            
        case EARSBUFOBJ_AMPUNIT_LINEAR:
        default:
            return value;
            break;
    }
}


double earsbufobj_input_to_db(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_ampunit) {
        case EARSBUFOBJ_AMPUNIT_LINEAR:
            return ears_linear_to_db(value);
            break;
            
        case EARSBUFOBJ_AMPUNIT_DECIBEL:
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
                case EARSBUFOBJ_AMPUNIT_DECIBEL:
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
                case EARSBUFOBJ_AMPUNIT_DECIBEL:
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
                case EARSBUFOBJ_AMPUNIT_DECIBEL:
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
                case EARSBUFOBJ_TIMEUNIT_MS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, ears_ms_to_fsamps(hatom_getdouble(&sub_ll->l_head->l_hatom), sr));
                }
                    break;
                case EARSBUFOBJ_TIMEUNIT_DURATION_RATIO:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, hatom_getdouble(&sub_ll->l_head->l_hatom) * (dur_samps - 1));
                }
                    break;
                default:
                    break;
            }
        } else {
            switch (e_ob->l_ampunit) {
                case EARSBUFOBJ_AMPUNIT_DECIBEL:
                    hatom_setdouble(&el->l_hatom, ears_db_to_linear(hatom_getdouble(&el->l_hatom)));
                    break;
                default:
                    break;
            }
        }
    }
    
    return out;
}


// llllelem must be an envelope (llll)
t_llll *earsbufobj_llllelem_to_env_samples(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf)
{
    t_llll *out = llll_get();
    llll_appendhatom_clone(out, &elem->l_hatom);
    llll_flatten(out, 1, 0);
    
    double dur_samps = ears_buffer_get_size_samps((t_object *)e_ob, buf);
    double sr = ears_buffer_get_sr((t_object *)e_ob, buf);
    
    for (t_llllelem *el = out->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            switch (e_ob->l_envtimeunit) {
                case EARSBUFOBJ_TIMEUNIT_MS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, ears_ms_to_fsamps(hatom_getdouble(&sub_ll->l_head->l_hatom), sr));
                }
                    break;
                case EARSBUFOBJ_TIMEUNIT_DURATION_RATIO:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, hatom_getdouble(&sub_ll->l_head->l_hatom) * (dur_samps - 1));
                }
                    break;
                default:
                    break;
            }
        }
    }
    
    return out;
}




double earsbufobj_linear_to_output(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_ampunit) {
        case EARSBUFOBJ_AMPUNIT_DECIBEL:
            return ears_linear_to_db(value);
            break;
            
        case EARSBUFOBJ_AMPUNIT_LINEAR:
        default:
            return value;
            break;
    }
}




void earsbufobj_remap_y_to_0_1_and_x_to_samples_do(t_earsbufobj *e_ob, t_llll *out, t_buffer_obj *buf, double orig_from, double orig_to, char convert_from_decibels)
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
    
    
    for (t_llllelem *el = out->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            switch (e_ob->l_envtimeunit) {
                case EARSBUFOBJ_TIMEUNIT_MS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, ears_ms_to_fsamps(hatom_getdouble(&sub_ll->l_head->l_hatom), sr));
                }
                    break;
                case EARSBUFOBJ_TIMEUNIT_DURATION_RATIO:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, hatom_getdouble(&sub_ll->l_head->l_hatom) * (dur_samps - 1));
                }
                    break;
                default:
                    break;
            }
        }
    }
}

t_llll *earsbufobj_llll_remap_y_to_0_1_and_x_to_samples(t_earsbufobj *e_ob, t_llll *ll, t_buffer_obj *buf, double orig_from, double orig_to, char convert_from_decibels)
{
    if (!ll)
        return NULL;
    
    t_llll *out = llll_get();
    llll_appendllll_clone(out, ll);
    llll_flatten(out, 1, 0);
    
    earsbufobj_remap_y_to_0_1_and_x_to_samples_do(e_ob, out, buf, orig_from, orig_to, convert_from_decibels);
    
    return out;
}



// llllelem can be either a number or a t_pts
t_llll *earsbufobj_llllelem_remap_y_to_0_1_and_x_to_samples(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf, double orig_from, double orig_to, char convert_from_decibels)
{
    t_llll *out = llll_get();
    llll_appendhatom_clone(out, &elem->l_hatom);
    llll_flatten(out, 1, 0);
    
    earsbufobj_remap_y_to_0_1_and_x_to_samples_do(e_ob, out, buf, orig_from, orig_to, convert_from_decibels);
    
    return out;
}


double ears_ratio_to_cents(double ratio)
{
    return 1200 * log2(ratio);
}

double ears_cents_to_ratio(double cents)
{
    return pow(2, cents/1200.);
}

double earsbufobj_input_to_cents(t_earsbufobj *e_ob, double value)
{
    switch (e_ob->l_pitchunit) {
        case EARSBUFOBJ_PITCHUNIT_CENTS:
            return value;
            break;
            
        case EARSBUFOBJ_PITCHUNIT_MIDI:
            return value*100.;
            break;
            
        case EARSBUFOBJ_PITCHUNIT_FREQRATIO:
            return ears_ratio_to_cents(value);
            break;

        default:
            return value;
            break;
    }
}



// llllelem can be either a number or a t_pts
t_llll *earsbufobj_llllelem_to_cents_and_samples(t_earsbufobj *e_ob, t_llllelem *elem, t_buffer_obj *buf)
{
    t_llll *out = llll_get();
    llll_appendhatom_clone(out, &elem->l_hatom);
    llll_flatten(out, 1, 0);
    
    double dur_samps = ears_buffer_get_size_samps((t_object *)e_ob, buf);
    double sr = ears_buffer_get_sr((t_object *)e_ob, buf);
    
    for (t_llllelem *el = out->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            switch (e_ob->l_pitchunit) {
                case EARSBUFOBJ_PITCHUNIT_FREQRATIO:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && sub_ll->l_head->l_next && is_hatom_number(&sub_ll->l_head->l_next->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_next->l_hatom, ears_ratio_to_cents(hatom_getdouble(&sub_ll->l_head->l_next->l_hatom)));
                }
                    break;
                case EARSBUFOBJ_PITCHUNIT_MIDI:
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
                case EARSBUFOBJ_TIMEUNIT_MS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, ears_ms_to_fsamps(hatom_getdouble(&sub_ll->l_head->l_hatom), sr));
                }
                    break;
                case EARSBUFOBJ_TIMEUNIT_DURATION_RATIO:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, hatom_getdouble(&sub_ll->l_head->l_hatom) * (dur_samps - 1));
                }
                    break;
                default:
                    break;
            }
        } else {
            switch (e_ob->l_pitchunit) {
                case EARSBUFOBJ_PITCHUNIT_FREQRATIO:
                    hatom_setdouble(&el->l_hatom, ears_ratio_to_cents(hatom_getdouble(&el->l_hatom)));
                    break;
                case EARSBUFOBJ_PITCHUNIT_MIDI:
                    hatom_setdouble(&el->l_hatom, 100*hatom_getdouble(&el->l_hatom));
                    break;
                default:
                    break;
            }
        }
    }
    
    return out;
}
