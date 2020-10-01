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
