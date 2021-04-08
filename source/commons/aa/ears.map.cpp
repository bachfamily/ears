//
//  ears.map.cpp
//  lib_ears
//
//  Created by andreaagostini on 07/04/2021.
//

#include <ears.map.h>


t_object *getParentEarsMap(t_object *x)
{
    // TODO: what if we're not in ears.map~'s constructor?
    return gensym(EARSMAP_SPECIALSYM)->s_thing;
}
