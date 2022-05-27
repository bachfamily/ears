#include "ears.conversions.h"
#include <vector>


std::vector<double> llll_to_vector_double(t_llll *ll)
{
    std::vector<double> res;
    for (t_llllelem *el = ll->l_head; el; el = el->l_next)
        res.push_back(hatom_getdouble(&el->l_hatom));
    return res;
}

const char *ears_ampunit_to_abbrev(e_ears_ampunit u)
{
    switch (u) {
        case EARS_AMPUNIT_LINEAR:
            return "[lin]";
            break;
        case EARS_AMPUNIT_DECIBEL:
            return "[dB]";
            break;
        default:
            return "[]";
            break;
    }
}


const char *ears_timeunit_to_abbrev(e_ears_timeunit u)
{
    switch (u) {
        case EARS_TIMEUNIT_MS:
            return "[ms]";
            break;
        case EARS_TIMEUNIT_SECONDS:
            return "[s]";
            break;
        case EARS_TIMEUNIT_SAMPS:
            return "[samps]";
            break;
        case EARS_TIMEUNIT_DURATION_RATIO:
            return "[durratio]";
            break;
        case EARS_TIMEUNIT_DURATION_DIFFERENCE_MS:
            return "[durdiffms]";
            break;
        case EARS_TIMEUNIT_DURATION_DIFFERENCE_SAMPS:
            return "[durdiffsamps]";
            break;
        default:
            return "[]";
            break;
    }
}


const char *ears_pitchunit_to_abbrev(e_ears_pitchunit pu)
{
    switch (pu) {
        case EARS_PITCHUNIT_FREQRATIO:
            return "[freqratio]";
            break;
        case EARS_PITCHUNIT_HERTZ:
            return "[Hz]";
            break;
        case EARS_PITCHUNIT_MIDI:
            return "[MIDI]";
            break;
        case EARS_PITCHUNIT_CENTS:
            return "[cents]";
            break;
        default:
            return "[]";
            break;
    }
}



const char *ears_frequnit_to_abbrev(e_ears_frequnit u)
{
    switch (u) {
        case EARS_FREQUNIT_HERTZ:
            return "[Hz]";
            break;
        case EARS_FREQUNIT_MIDI:
            return "[MIDI]";
            break;
        case EARS_FREQUNIT_CENTS:
            return "[cents]";
            break;
        case EARS_FREQUNIT_BPM:
            return "[bpm]";
            break;
        default:
            return "[]";
            break;
    }
}

e_ears_timeunit ears_timeunit_from_symbol(t_symbol *s)
{
    if (s == gensym("ms") || s == gensym("millisecond") || s == gensym("milliseconds"))
        return EARS_TIMEUNIT_MS;
    if (s == gensym("s") || s == gensym("sec") || s == gensym("second") || s == gensym("seconds"))
        return EARS_TIMEUNIT_SECONDS;
    if (s == gensym("ratio") || s == gensym("durationratio") || s == gensym("duration ratio") || s == gensym("relative"))
        return EARS_TIMEUNIT_DURATION_RATIO;
    if (s == gensym("deltams") || s == gensym("msdiff") || s == gensym("durationdiffms") || s == gensym("duration difference ms"))
        return EARS_TIMEUNIT_DURATION_DIFFERENCE_MS;
    if (s == gensym("deltasamps") || s == gensym("deltasamples") || s == gensym("samplesdiff") || s == gensym("sampsdiff") || s == gensym("durationdiffsamps") || s == gensym("duration difference samps"))
        return EARS_TIMEUNIT_DURATION_DIFFERENCE_SAMPS;
    if (s == gensym("numdivisions") || s == gensym("intervals") || s == gensym("divisions") || s == gensym("numintervals"))
        return EARS_TIMEUNIT_NUM_INTERVALS;
    if (s == gensym("numpoints") || s == gensym("points") || s == gensym("onsets") || s == gensym("numonsets"))
        return EARS_TIMEUNIT_NUM_ONSETS;
    if (s == gensym("samps") || s == gensym("samples"))
        return EARS_TIMEUNIT_SAMPS;
    return EARS_TIMEUNIT_UNKNOWN;
}

t_symbol *ears_pitchunit_to_symbol(e_ears_pitchunit u)
{
    switch (u) {
        case EARS_PITCHUNIT_CENTS: return gensym("cents"); break;
        case EARS_PITCHUNIT_MIDI: return gensym("midi"); break;
        case EARS_PITCHUNIT_HERTZ: return gensym("hertz"); break;
        case EARS_PITCHUNIT_FREQRATIO: return gensym("freqratio"); break;
        default: return gensym("unknown"); break;
    }
}
e_ears_pitchunit ears_pitchunit_from_symbol(t_symbol *s)
{
    if (s == gensym("Cents") || s == gensym("cents") || s == gensym("midicents") || s == gensym("MIDIcents"))
        return EARS_PITCHUNIT_CENTS;
    if (s == gensym("Midi") || s == gensym("midi") || s == gensym("MIDI") || s == gensym("semitones"))
        return EARS_PITCHUNIT_MIDI;
    if (s == gensym("Hz") || s == gensym("Hertz") || s == gensym("hz") || s == gensym("hertz") || s == gensym("freq") || s == gensym("frequency"))
        return EARS_PITCHUNIT_HERTZ;
    if (s == gensym("Frequency Ratio") || s == gensym("Frequency Ratio") ||
             s == gensym("frequency ratio") || s == gensym("freqratio") || s == gensym("ratio"))
        return EARS_PITCHUNIT_FREQRATIO;
    return EARS_PITCHUNIT_UNKNOWN;
}

t_symbol *ears_frequnit_to_symbol(e_ears_frequnit u)
{
    switch (u) {
        case EARS_FREQUNIT_CENTS: return gensym("cents"); break;
        case EARS_FREQUNIT_MIDI: return gensym("midi"); break;
        case EARS_FREQUNIT_HERTZ: return gensym("hertz"); break;
        default: return gensym("unknown"); break;
    }
}

e_ears_frequnit ears_frequnit_from_symbol(t_symbol *s)
{
    if (s == gensym("Cents") || s == gensym("cents") || s == gensym("midicents") || s == gensym("MIDIcents"))
        return EARS_FREQUNIT_CENTS;
    if (s == gensym("Midi") || s == gensym("midi") || s == gensym("MIDI") || s == gensym("semitones"))
        return EARS_FREQUNIT_MIDI;
    if (s == gensym("Hz") || s == gensym("Hertz") || s == gensym("hz") || s == gensym("hertz") || s == gensym("freq") || s == gensym("frequency"))
        return EARS_FREQUNIT_HERTZ;
    return EARS_FREQUNIT_UNKNOWN;
}

e_ears_ampunit ears_ampunit_from_symbol(t_symbol *s)
{
    if (s == gensym("db") || s == gensym("dB") || s == gensym("decibel") || s == gensym("decibels"))
        return EARS_AMPUNIT_DECIBEL;
    if (s == gensym("lin") || s == gensym("linear"))
        return EARS_AMPUNIT_LINEAR;
    return EARS_AMPUNIT_UNKNOWN;
}



double ears_ms_to_fsamps(double ms, double sr)
{
    return ms * sr / 1000.;
}

double ears_samps_to_ms(double samps, double sr)
{
    return samps * 1000. / sr;
}

long ears_ms_to_samps(double ms, double sr)
{
    return round(ears_ms_to_fsamps(ms, sr));
}

double ears_db_to_linear(double db)
{
    return pow(10, db/20.);
}

double ears_linear_to_db(double amp)
{
    return 20 * log10(amp);
}

double ears_deg_to_rad(double deg)
{
    return deg * 0.01745329252;
}

double ears_rad_to_deg(double rad)
{
    return rad / 0.01745329252;
}


double ears_ratio_to_cents(double ratio)
{
    return 1200 * log2(ratio);
}

double ears_cents_to_ratio(double cents)
{
    return pow(2, cents/1200.);
}


double ears_hz_to_cents(double freq, double middleAtuning)
{
    return 6900 + 1200 * log2(freq/middleAtuning);
}

double ears_cents_to_hz(double cents, double middleAtuning)
{
    return middleAtuning * pow(2, (cents-6900.)/1200.);
}


double ears_angle_to_radians(double angle, char angleunit)
{
    switch (angleunit) {
        case EARS_ANGLEUNIT_DEGREES:
            return ears_deg_to_rad(angle);
            break;
        case EARS_ANGLEUNIT_TURNS:
            return ears_deg_to_rad(TWOPI * angle);
            break;
        default:
            return angle;
            break;
    }
}

double ears_radians_to_angle(double rad, char angleunit)
{
    switch (angleunit) {
        case EARS_ANGLEUNIT_DEGREES:
            return ears_rad_to_deg(rad);
            break;
        case EARS_ANGLEUNIT_TURNS:
            return rad/TWOPI;
            break;
        default:
            return rad;
            break;
    }
}





void ears_llll_to_radians(t_llll *out, char angleunit)
{
    for (t_llllelem *el = out->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            // envelopes values
            switch (angleunit) {
                case EARS_ANGLEUNIT_DEGREES:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && sub_ll->l_head->l_next && is_hatom_number(&sub_ll->l_head->l_next->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_next->l_hatom, ears_deg_to_rad(hatom_getdouble(&sub_ll->l_head->l_next->l_hatom)));
                }
                    break;
                case EARS_ANGLEUNIT_TURNS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && sub_ll->l_head->l_next && is_hatom_number(&sub_ll->l_head->l_next->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_next->l_hatom, TWOPI * hatom_getdouble(&sub_ll->l_head->l_next->l_hatom));
                }
                    break;
                default:
                    break;
            }
        } else if (is_hatom_number(&el->l_hatom)) {
            // single values
            switch (angleunit) {
                case EARS_ANGLEUNIT_DEGREES:
                    hatom_setdouble(&el->l_hatom, ears_deg_to_rad(hatom_getdouble(&el->l_hatom)));
                    break;
                case EARS_ANGLEUNIT_TURNS:
                    hatom_setdouble(&el->l_hatom, hatom_getdouble(&el->l_hatom)*TWOPI);
                    break;
                default:
                    break;
            }
        }
    }
}

// inplace, destructive! will alter ll
void ears_llll_to_env_samples(t_llll *ll, double dur_samps, double sr, char envtimeunit)
{
    if (!ll)
        return;
    
    for (t_llllelem *el = ll->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            switch (envtimeunit) {
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
                case EARS_TIMEUNIT_DURATION_DIFFERENCE_MS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, dur_samps - ears_ms_to_fsamps(hatom_getdouble(&sub_ll->l_head->l_hatom), sr));
                }
                    break;
                case EARS_TIMEUNIT_DURATION_DIFFERENCE_SAMPS:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && is_hatom_number(&sub_ll->l_head->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_hatom, dur_samps - hatom_getdouble(&sub_ll->l_head->l_hatom));
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
                        hatom_setdouble(&sub_ll->l_head->l_hatom, (1./(hatom_getdouble(&sub_ll->l_head->l_hatom)-1)) * (dur_samps - 1));
                }
                    break;
                default:
                    break;
            }
        }
    }
}

