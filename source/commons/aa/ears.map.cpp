//
//  ears.map.cpp
//  lib_ears
//
//  Created by andreaagostini on 07/04/2021.
//

#include <ears.map.h>


t_object *getParentEarsMap(t_object *x)
{
    t_symbol *s = gensym(EARSMAP_SPECIALSYM);
    t_object *t = s->s_thing;
    if (t && !NOGOOD(t) && object_classname(t) == gensym("ears.map~"))
        return gensym(EARSMAP_SPECIALSYM)->s_thing;
    
    t_object *box = x;
    t_symbol *containerName;
    do {
        t_object *patcher = nullptr;
        object_obex_lookup(box, gensym("#P"), (t_object **) &patcher);
        if (!(box = object_attr_getobj(patcher, gensym("box"))))
            object_method(patcher, gensym("getassoc"), &box);
        containerName = box ? object_classname(box) : nullptr;
    } while (containerName && containerName != gensym("ears.map~"));
    
    return box;

}
