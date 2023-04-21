/**
	@file	ears_doc_commons_dg.h
	@brief	Common documentation about some basic concepts and syntaxes of ears.
	
	by Daniele Ghisi
*/

	
#define EARS_DOC_OUTNAME_ATTR
// The <o>outname</o> attribute sets a name (or a wrapped list of names) for each of the buffer outles
// of the objects in ears. These names are only accounted for if the <m>naming</m> mode is 'Static'. <br />

#define EARS_DOC_COORDINATE_CONVENTION
// Azimuth is considered 0 at the front, and increases clockwise.
// X is the left-right axis (positive to the right) while Y is front-back axis (positive to the front). <br />

#define EARS_DOC_ACCEPTED_SAMPLETYPES
// Accepted sampletype symbols are : 'int8' (8-bit integer), 'int16' (16-bit integer, default),
// 'int24' (24-bit integer), 'int32' (32-bit integer), 'float32' (32-bit floating-point),
// 'float64' (64-bit floating-point), 'mulaw' (8-bit mu-law encoding), 'alaw' (8-bit a-law encoding) <br />

