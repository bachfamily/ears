#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef TWOPI
#define TWOPI        6.28318530717958647692
#endif
#ifndef PIOVERTWO
#define PIOVERTWO    1.57079632679489661923
#endif


long ears_get_window(float *win, const char *type, long numframes)
{
    double H = (numframes-1.)/2.;
    double halfF = PI / (numframes-1.);
    double F = 2 * PI / (numframes-1.);
    double twoF = 2 * F;
    double threeF = 3 * F;
    long err = 0;
    if (strcmp(type, "rect") == 0 || strcmp(type, "rectangle") == 0 || strcmp(type, "rectangular") == 0) {
        for (long i = 0; i < numframes; i++)
            win[i] = 1.;
    } else if (strcmp(type, "tri") == 0 || strcmp(type, "triangle") == 0 || strcmp(type, "triangular") == 0) {
        for (long i = 0; i < numframes; i++)
            win[i] = 1 - fabs((i - H)/H); // denominator could be +1 or +2
    } else if (strcmp(type, "sine") == 0 || strcmp(type, "sin") == 0 || strcmp(type, "cos") == 0 || strcmp(type, "cosine") == 0) {
        for (long i = 0; i < numframes; i++)
            win[i] = sin(halfF * i);
    } else if (strcmp(type, "hann") == 0) {
        double a0 = 0.5;
        double a1 = 1. - a0;
        for (long i = 0; i < numframes; i++)
            win[i] = a0  - a1 * cos(F*i);
    } else if (strcmp(type, "hamming") == 0) {
        double a0 = 25./46.;
        double a1 = 1. - a0;
        for (long i = 0; i < numframes; i++)
            win[i] = a0  - a1 * cos(F*i);
    } else if (strcmp(type, "blackman") == 0) {
        double a0 = 0.42, a1 = 0.5, a2 = 0.08;
        for (long i = 0; i < numframes; i++) {
            win[i] = a0 - a1 * cos(F*i) + a2 * cos(twoF * i);
        }
    } else if (strcmp(type, "nuttall") == 0) {
        double a0 = 0.355768, a1 = 0.487396, a2 = 0.144232, a3 = 0.012604;
        for (long i = 0; i < numframes; i++) {
            win[i] = a0 - a1 * cos(F*i) + a2 * cos(twoF * i) - a3 * cos(threeF * i);
        }
    } else if (strcmp(type, "blackmannuttall") == 0) {
        double a0 = 0.3635819, a1 = 0.4891775, a2 = 0.1365995, a3 = 0.0106411;
        for (long i = 0; i < numframes; i++) {
            win[i] = a0 - a1 * cos(F*i) + a2 * cos(twoF * i) - a3 * cos(threeF * i);
        }
    } else if (strcmp(type, "blackmanharris") == 0) {
        double a0 = 0.35875, a1 = 0.48829, a2 = 0.14128, a3 = 0.01168;
        for (long i = 0; i < numframes; i++) {
            win[i] = a0 - a1 * cos(F*i) + a2 * cos(twoF * i) - a3 * cos(threeF * i);
        }
    } else if (strcmp(type, "gaussian") == 0) {
        const double sigma = 0.4;
        double temp;
        double sigmaH = sigma * H;
        for (long i = 0; i < numframes; i++) {
            temp = (i - H)/sigmaH;
            win[i] = exp(-0.5 * temp * temp);
        }
    } else {
        for (long i = 0; i < numframes; i++)
            win[i] = 1.;
        err = 1;
    }
    return err;
}

