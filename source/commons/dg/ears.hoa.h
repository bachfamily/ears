/**
	@file
	ears.hoa.h
	Higher-Order Ambisonic bridge with HoaLibrary
 
	by Daniele Ghisi
 */

#ifndef _EARS_BUF_HOA_H_
#define _EARS_BUF_HOA_H_

#include "ears.commons.h"

#define EARS_HOA_MAX_LOUDSPEAKERS 512
#define EARS_HOA_BINAURAL_SADIE /// < if defined the Sadie transfer function are used, otherwise the Listen ones (see HOALibrary)

// azimuth always computed in navigational coordinates: 0 is at the front, angles increase clockwise
typedef enum _ears_coordinate_type
{
    EARS_COORDINATES_AED = 0, // spherical (azimuth, elevation, distance)
    EARS_COORDINATES_XYZ = 1, // cartesian (x, y, z)
    EARS_COORDINATES_AZR = 2, // cylindrical (azimuth, z, axial radius)
} e_ears_coordinate_type;


long ears_hoa_get_dimension_as_long(t_symbol *s);


t_ears_err ears_buffer_hoa_encode(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, int dimension, long order, e_ears_coordinate_type coord_type, t_llll *coord1, t_llll *coord2, t_llll *coord3);

// In the following functions, order is always inferred by number of source channels
t_ears_err ears_buffer_hoa_decode(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, int dimension,
                                  long num_out_channels, double *out_channels_azimuth, double *out_channels_elevation);
t_ears_err ears_buffer_hoa_decode_binaural(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, int dimension, long blockSize);
t_ears_err ears_buffer_hoa_rotate(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, int dimension, t_llll *yaw, t_llll *pitch, t_llll *roll);
t_ears_err ears_buffer_hoa_shift(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, int dimension, t_llll *delta_x, t_llll *delta_y, t_llll *delta_z);
t_ears_err ears_buffer_hoa_mirror(t_object *ob, t_buffer_obj *source, t_buffer_obj *dest, int dimension, int axis);

void quaternion_to_yawpitchroll(double w, double x, double y, double z, double *yaw, double *pitch, double *roll);

#endif // _EARS_BUF_HOA_H_
