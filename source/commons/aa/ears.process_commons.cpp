//
//  ears.process.cpp
//  lib_ears
//
//  Created by andreaagostini on 07/04/2021.
//

#include <ears.process_commons.h>


void ears_inouttilde_get_inlet_index(t_ears_inouttilde *x, t_llllelem *el)
{
    x->ioNum = hatom_getlong(&el->l_hatom);
    if (x->ioNum < 1) {
        object_error((t_object *) x, "Wrong inlet number, setting to 1");
        x->ioNum = 1;
    }
}

long ears_inouttilde_get_channel_indices(t_ears_inouttilde *x, t_llllelem *el)
{
    int i = 0;
    for (i = 0;
         el && i < EARS_INTILDE_MAX_CHANS;
         el = el->l_next, i++) {
        t_atom_long v = hatom_getlong(&el->l_hatom);
        if (v < 1) {
            object_error((t_object *) x, "Wrong channel index, setting to 1");
            v = 1;
        }
        x->chan[i] = v;
    }
    return i;
}

long ears_inouttilde_int(t_ears_inouttilde *x, long v)
{
    t_atom outatom;
    atom_setlong(&outatom, v);
    return ears_inouttilde_anything(x, _sym_int, 1, &outatom);
}

long ears_inouttilde_float(t_ears_inouttilde *x, double v)
{
    t_atom outatom;
    atom_setfloat(&outatom, v);
    return ears_inouttilde_anything(x, _sym_float, 1, &outatom);
}

long ears_inouttilde_anything(t_ears_inouttilde *x, t_symbol *s, long ac, t_atom *av)
{
    long nChans = -1;
    t_llll *ll = llllobj_parse_llll((t_object *) x, LLLL_OBJ_MSP, nullptr, ac, av, LLLL_PARSE_RETAIN);
    if (!ll) {
        return nChans;
    }
    if (ll->l_size == 0) {
        x->ioNum = 1;
        x->chan[0] = 1;
        nChans = 1;
        goto ears_inouttilde_exit;
    }
    switch (ll->l_depth) {
        case 1: {
            if (t_llllelem *el; el = ll->l_head) {
                ears_inouttilde_get_inlet_index(x, el);
                el = el->l_next;
                nChans = ears_inouttilde_get_channel_indices(x, el);
            }
            break;
        }
        case 2: {
            switch (ll->l_size) {
                case 1: {
                    t_llllelem *el = hatom_getllll(&ll->l_head->l_hatom)->l_head;
                    nChans = ears_inouttilde_get_channel_indices(x, el);
                    break;
                }
                case 2: {
                    if (hatom_gettype(&ll->l_head->l_hatom) == H_LLLL ||
                        hatom_gettype(&ll->l_tail->l_hatom) != H_LLLL) {
                        object_error((t_object *) x, "Incorrect inlet / channel indices");
                        goto ears_inouttilde_exit;
                    }
                    ears_inouttilde_get_inlet_index(x, ll->l_head);
                    nChans = ears_inouttilde_get_channel_indices(x, hatom_getllll(&ll->l_tail->l_hatom)->l_head);
                    break;
                }
                default:
                    object_error((t_object *) x, "Incorrect inlet / channel indices");
                    goto ears_inouttilde_exit;
            }
            break;
        }
        default:
            object_error((t_object *) x, "Incorrect inlet / channel indices");
            goto ears_inouttilde_exit;
    }
    
ears_inouttilde_exit:
    llll_release(ll);
    return nChans;
}
