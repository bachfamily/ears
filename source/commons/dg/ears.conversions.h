/**
	@file
	ears.conversions.h
	Conversions utilities header for the ears library
 
	by Daniele Ghisi
 */

#ifndef _EARS_CONVERSIONS_H_
#define _EARS_CONVERSIONS_H_

#include "foundation/llllobj.h"
#include "foundation/llll_commons_ext.h"

typedef enum _ears_timeunit
{
    EARS_TIMEUNIT_UNKNOWN = -1,
    EARS_TIMEUNIT_MS = 0,
    EARS_TIMEUNIT_SAMPS = 1,
    EARS_TIMEUNIT_DURATION_RATIO = 2,
    EARS_TIMEUNIT_DURATION_DIFFERENCE_MS = 3,
    EARS_TIMEUNIT_DURATION_DIFFERENCE_SAMPS = 4,

    // these are only relevant for Essentia, never exposed
    EARS_TIMEUNIT_SECONDS,

    // these below are supported in the API but unused in the objects and will not be used in ears
    // they may be removed later on
    EARS_TIMEUNIT_NUM_INTERVALS,
    EARS_TIMEUNIT_NUM_ONSETS
} e_ears_timeunit;


typedef enum _ears_ampunit
{
    EARS_AMPUNIT_UNKNOWN = -1,
    EARS_AMPUNIT_LINEAR = 0,
    EARS_AMPUNIT_DECIBEL = 1
} e_ears_ampunit;

// TO DO : store this as an attribute of earsbufobjs
#define EARS_MIDDLE_A_TUNING 440

typedef enum _ears_pitchunit
{
    EARS_PITCHUNIT_UNKNOWN = -1,
    EARS_PITCHUNIT_CENTS = 0,
    EARS_PITCHUNIT_MIDI = 1,
    EARS_PITCHUNIT_HERTZ = 2,
    EARS_PITCHUNIT_FREQRATIO = 3,
} e_ears_pitchunit;

typedef enum _ears_frequnit
{
    EARS_FREQUNIT_UNKNOWN = -1,
    EARS_FREQUNIT_HERTZ = 0,
    EARS_FREQUNIT_BPM = 1,
    EARS_FREQUNIT_CENTS = 2,
    EARS_FREQUNIT_MIDI = 3,
} e_ears_frequnit;

typedef enum _ears_angleunit
{
    EARS_ANGLEUNIT_UNKNOWN = -1,
    EARS_ANGLEUNIT_RADIANS = 0,
    EARS_ANGLEUNIT_DEGREES = 1,
    EARS_ANGLEUNIT_TURNS = 2
} e_ears_angleunit;


std::vector<double> llll_to_vector_double(t_llll *ll);

const char *ears_ampunit_to_abbrev(e_ears_ampunit u);
const char *ears_timeunit_to_abbrev(e_ears_timeunit u);
const char *ears_pitchunit_to_abbrev(e_ears_pitchunit u);
const char *ears_frequnit_to_abbrev(e_ears_frequnit u);

e_ears_timeunit ears_timeunit_from_symbol(t_symbol *s);
e_ears_ampunit ears_ampunit_from_symbol(t_symbol *s);
e_ears_pitchunit ears_pitchunit_from_symbol(t_symbol *s);
e_ears_frequnit ears_frequnit_from_symbol(t_symbol *s);

t_symbol *ears_pitchunit_to_symbol(e_ears_pitchunit u);
t_symbol *ears_frequnit_to_symbol(e_ears_frequnit u);

long ears_ms_to_samps(double ms, double sr);
double ears_ms_to_fsamps(double ms, double sr);
double ears_samps_to_ms(double samps, double sr);

double ears_linear_to_db(double amp);
double ears_db_to_linear(double db);

double ears_deg_to_rad(double deg);
double ears_rad_to_deg(double rad);

double ears_ratio_to_cents(double ratio);
double ears_cents_to_ratio(double cents);

double ears_hz_to_cents(double freq, double middleAtuning);
double ears_cents_to_hz(double cents, double middleAtuning);

double ears_angle_to_radians(double angle, char angleunit);
double ears_radians_to_angle(double rad, char angleunit);

double ears_principal_phase(double phase); // gets the phase between -pi and pi


// convenience utility
void ears_llll_to_env_samples(t_llll *ll, double dur_samps, double sr, char envtimeunit);
void ears_llll_to_radians(t_llll *ll, char angleunit);



#endif // _EARS_CONVERSIONS_H_
