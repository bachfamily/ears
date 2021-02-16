#include "ears.conversions.h"

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


double ears_freq_to_cents(double freq, double middleAtuning)
{
    return 6900 + 1200 * log2(freq/middleAtuning);
}

double ears_cents_to_freq(double cents, double middleAtuning)
{
    return middleAtuning * pow(2, (cents-6900.)/1200.);
}




void ears_llll_to_radians(t_llll *out, char angleunit)
{
    for (t_llllelem *el = out->l_head; el; el = el->l_next) {
        if (hatom_gettype(&el->l_hatom) == H_LLLL) {
            // envelopes values
            switch (angleunit) {
                case EARSBUFOBJ_ANGLEUNIT_DEGREES:
                {
                    t_llll *sub_ll = hatom_getllll(&el->l_hatom);
                    if (sub_ll && sub_ll->l_head && sub_ll->l_head->l_next && is_hatom_number(&sub_ll->l_head->l_next->l_hatom))
                        hatom_setdouble(&sub_ll->l_head->l_next->l_hatom, ears_deg_to_rad(hatom_getdouble(&sub_ll->l_head->l_next->l_hatom)));
                }
                    break;
                case EARSBUFOBJ_ANGLEUNIT_TURNS:
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
                case EARSBUFOBJ_ANGLEUNIT_DEGREES:
                    hatom_setdouble(&el->l_hatom, ears_deg_to_rad(hatom_getdouble(&el->l_hatom)));
                    break;
                case EARSBUFOBJ_ANGLEUNIT_TURNS:
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

