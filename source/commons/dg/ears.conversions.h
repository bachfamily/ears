/**
	@file
	ears.conversions.h
	Conversions utilities header for the ears library
 
	by Daniele Ghisi
 */

#ifndef _EARS_CONVERSIONS_H_
#define _EARS_CONVERSIONS_H_

typedef enum _ears_timeunit
{
    EARSBUFOBJ_TIMEUNIT_MS = 0,
    EARSBUFOBJ_TIMEUNIT_SAMPS = 1,
    EARSBUFOBJ_TIMEUNIT_DURATION_RATIO = 2
} e_ears_timeunit;


typedef enum _ears_ampunit
{
    EARSBUFOBJ_AMPUNIT_LINEAR = 0,
    EARSBUFOBJ_AMPUNIT_DECIBEL = 1
} e_ears_ampunit;


typedef enum _ears_pitchunit
{
    EARSBUFOBJ_PITCHUNIT_CENTS = 0,
    EARSBUFOBJ_PITCHUNIT_MIDI = 1,
    EARSBUFOBJ_PITCHUNIT_FREQRATIO = 2,
} e_ears_pitchunit;

typedef enum _ears_angleunit
{
    EARSBUFOBJ_ANGLEUNIT_RADIANS = 0,
    EARSBUFOBJ_ANGLEUNIT_DEGREES = 1,
    EARSBUFOBJ_ANGLEUNIT_TURNS = 2
} e_ears_angleunit;


long ears_ms_to_samps(double ms, double sr);
double ears_ms_to_fsamps(double ms, double sr);
double ears_samps_to_ms(double samps, double sr);

double ears_linear_to_db(double amp);
double ears_db_to_linear(double db);

double ears_deg_to_rad(double deg);
double ears_rad_to_deg(double rad);





#endif // _EARS_CONVERSIONS_H_
