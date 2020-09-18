/**
	@file
	ears.ambisonic.h
	Ambisonic bridge with HoaLibrary
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_AMBISONIC_H_
#define _EARS_BUF_AMBISONIC_H_

#include "ears.commons.h"

#define EARS_AMBISONIC_MAX_LOUDSPEAKERS 512
#define EARS_AMBISONIC_BINAURAL_SADIE /// < if defined the Sadie transfer function are used, otherwise the Listen ones (see HOALibrary)

// azimuth always computed in navigational coordinates: 0 is at the front, angles increase clockwise
typedef enum _ears_coordinate_type
{
    EARS_COORDINATES_AED = 0, // spherical (azimuth, elevation, distance)
    EARS_COORDINATES_XYZ = 1, // cartesian (x, y, z)
    EARS_COORDINATES_AZR = 2, // cylindrical (azimuth, z, axial radius)
} e_ears_coordinate_type;

t_ears_err ears_buffer_hoa_encode(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, int dimension, long order, e_ears_coordinate_type coord_type, t_llll *coord1, t_llll *coord2, t_llll *coord3);

// order is inferred by number of source channels
t_ears_err ears_buffer_hoa_decode(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, int dimension,
                                  long num_out_channels, double *out_channels_azimuth, double *out_channels_elevation);

t_ears_err ears_buffer_hoa_decode_binaural(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, int dimension, long blockSize);

#endif // _EARS_BUF_AMBISONIC_H_
