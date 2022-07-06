/**
	@file
	libpeanos.h
	Library containing mathematic functions for peano mapping, analysis and synthesis.
    Relies on MPFR and GMP.
    Main functions are peanos_featuresToPeanos() and peanos_peanosToFeatures().

    by Daniele Ghisi
 */

#ifndef _LIBPEANOS_H_
#define _LIBPEANOS_H_

#include "math.h"
#include <mpfr.h>
#include <gmp.h>
#include <stdlib.h>
#include <math.h>

#define PEANOS_PI_OVER_TWO (1.5707963268)


long peanos_numDigitsToBits(long numDigits)
{
    return (long)ceil(numDigits * log2(10) ) + 1;
}

void peanos_sphericalToCartesian(double *spherical, double *cartesian, long N)
{
    double r = spherical[0];
    double temp = 1.;
    for (long i = 0; i < N-1; i++) {
        cartesian[i] = r * temp * cos(spherical[i+1]);
        temp *= sin(spherical[i+1]);
    }
    cartesian[N-1] = r * temp;
}


void peanos_cartesianToSpherical(double *cartesian, double *spherical, long N)
{
    double temp = 0.;
    for (long i = 0; i < N; i++)
        temp += cartesian[i] * cartesian[i];
    spherical[0] = sqrt(temp);
    
    temp = cartesian[N-1]*cartesian[N-1];
    for (long i = N-2; i >= 0; i--) {
        temp += cartesian[i] * cartesian[i];
        spherical[i+1] = (temp == 0 ? 0 : cartesian[i]/sqrt(temp));
    }
}


// PEANO: prendo un numero tra 0 e 1 e lo butto su [0, 1]^N
// ternary_digits is expected to be sized at N*precision
void peanos_ternaryDigitsToHyperCube(char *ternary_digits, long N, double *output_coord, long precision)
{
    for (long i = 0; i < N; i++)
        output_coord[i] = 0;
    
    // building partial sum array, for optimization
    double *ternary_digits_sum = (double *)malloc((N*precision) * sizeof(double)); // TODO: avoid this allocation
    double *ternary_digits_colsum = (double *)malloc(precision * sizeof(double)); // TODO: avoid this allocation
    double temp = 0;
    for (long i = 0; i < N*precision; i++) {
        temp += ternary_digits[i];
        ternary_digits_sum[i] = temp;
    }

    for (long i = 1; i <= N; i++) {
        
        // building partial sum array, for optimization
        double temp = 0;
        for (long s = 0; s < precision; s++) {
            temp += ternary_digits[i+s*N-1];
            ternary_digits_colsum[s] = temp;
        }
  
        long div = 3;
        for (long j = 0; j < precision; j++) {
            long partialsum = 0;
            long ijn1 = i+j*N-1;
            
// Naive version:
//            long partialsum_naive = 0;
//            for (long s = 1; s <= i+j*N; s++) partialsum_naive += ternary_digits[s-1];
//            for (long s = 0; s <= j; s++) partialsum_naive -= ternary_digits[i+s*N-1];


// Optimized version:
            partialsum = ternary_digits_sum[ijn1];
            partialsum -= ternary_digits_colsum[j];
//            if(partialsum != partialsum_naive) {
//                printf("error!");
//            }

//            char tmp[2048];
            
            if (partialsum % 2 == 0) {
//                sprintf(tmp, "#%d: digit of den %d is %d\n", i, div, ternary_digits[ijn1]);
                output_coord[i-1] += (double)ternary_digits[ijn1]/div;
            } else {
//                sprintf(tmp, "#%d: digit of den %d is %d\n", i, div, 2-ternary_digits[ijn1]);
                output_coord[i-1] += (double)(2 - ternary_digits[ijn1])/div;
            }

//            printf(tmp);

            div *= 3;
        }
    }
    
    free(ternary_digits_sum);
    free(ternary_digits_colsum);
}


void peanos_unitIntervalToTernaryRepresentation(mpfr_t input, long numdigits, char *ternary_digits)
{
    if (mpfr_cmp_d(input, 1.) >= 0) {
        for (long i = 0; i < numdigits; i++)
            ternary_digits[i] = 2;
    } else {
        mpz_t div;
        mpz_init(div);
        mpz_set_ui(div, 3);

        mpfr_t temp, d;
        mpfr_init2(d, mpfr_get_prec(input) * 2); // TODO: why * 2 ?
        mpfr_init2(temp, mpfr_get_prec(input) * 2);
        mpfr_set(temp, input, MPFR_RNDN);
        for (long i = 0; i < numdigits; i++) {
            mpfr_mul_z(d, temp, div, MPFR_RNDN);
            mpfr_floor(d, d);
            long digit = mpfr_get_ui(d, MPFR_RNDN);
            mpfr_div_z(d, d, div, MPFR_RNDN);
            mpfr_sub(temp, temp, d, MPFR_RNDN);
            mpz_mul_ui(div, div, 3);
            ternary_digits[i] = (digit < 0 ? 0 : (digit > 2 ? 2 : digit));
        }
        mpfr_clear(temp);
        mpfr_clear(d);
        mpz_clear(div);
    }
}

void peanos_unitIntervalToTernaryRepresentation_d(double input, long numdigits, char *ternary_digits)
{
    if (input >= 1.) {
        for (long i = 0; i < numdigits; i++)
            ternary_digits[i] = 2;
    } else {
        long div = 3;
        double temp = input;
        for (long i = 0; i < numdigits; i++) {
            long digit = (int)(temp*div);
            temp -= (double)digit/div;
            div *= 3;
            ternary_digits[i] = (digit < 0 ? 0 : (digit > 2 ? 2 : digit));
        }
    }
}

void peanos_ternaryRepresentationToUnitInterval(char *ternary_digits, long numdigits, mpfr_t output)
{
    mpz_t div;
    mpz_init(div);
    mpz_set_ui(div, 3);

    mpfr_t d;
    mpfr_init2(d, peanos_numDigitsToBits(numdigits) * 2); // TODO: why * 2 ? should not be needed
    mpfr_init2(output, peanos_numDigitsToBits(numdigits) * 2);
    mpfr_set_ui(output, 0, MPFR_RNDN);
    mpfr_set_ui(d, 1, MPFR_RNDN);

    
    for (long i = 0; i < numdigits; i++) {
        mpfr_set_ui(d, 1, MPFR_RNDN);
        mpfr_div_z(d, d, div, MPFR_RNDN);
        if (ternary_digits[i] == 1) {
            mpfr_add (output, output, d, MPFR_RNDN);
        } else if (ternary_digits[i] == 2) {
            mpfr_add (output, output, d, MPFR_RNDN);
            mpfr_add (output, output, d, MPFR_RNDN); // yes, twice!
        }
        mpz_mul_ui(div, div, 3);
    }
    
    mpfr_clear(d);
    mpz_clear(div);
}

// Map a number between [0, 1] in a number in [0,1]^n via a Peano curve
void peanos_unitIntervalToHyperCube(mpfr_t input, long N, double *output_coord, long precision)
{
    char *ternary_digits = (char *)malloc((N*precision) * sizeof(char)); // TODO: avoid this allocation
    peanos_unitIntervalToTernaryRepresentation(input, N*precision, ternary_digits);
    peanos_ternaryDigitsToHyperCube(ternary_digits, N, output_coord, precision);
    free(ternary_digits);
}

void peanos_unitIntervalToHyperCube_d(double input, long N, double *output_coord, long precision)
{
    char *ternary_digits = (char *)malloc((N*precision) * sizeof(char)); // TODO: avoid this allocation
    peanos_unitIntervalToTernaryRepresentation_d(input, N*precision, ternary_digits);
    peanos_ternaryDigitsToHyperCube(ternary_digits, N, output_coord, precision);
    free(ternary_digits);
}



// PEANO INVERSO: prendo coordinate in [0, 1]^N e trovo un numero in [0, 1].
// ternary_digits must be allocated at N*precision
void peanos_hyperCubeToTernaryDigits(double *coords, long N, char *ternary_digits, long precision)
{
    for (long i = 0; i < N*precision; i++)
        ternary_digits[i] = 0;

    char *ternary_digits_coords = (char *)malloc((N * precision) * sizeof(char)); // TODO: avoid this allocation
    char *ternary_digits_coord_temp = (char *)malloc((precision) * sizeof(char)); // TODO: avoid this allocation
    for (long i = 0; i < N; i++) {
        peanos_unitIntervalToTernaryRepresentation_d(coords[i], precision, ternary_digits_coord_temp);
        for (long j = 0; j < precision; j++)
            ternary_digits_coords[i+j*N] = ternary_digits_coord_temp[j];
    }
    
    for (long j = 0; j < precision; j++) {
        for (long i = 1; i <= N; i++) {
            // compute partial sums
            // TODO: naive version, optimize
            long partialsum_naive = 0;

            //            for (long s = 1; s <= i+j*N; s++) partialsum_naive += ternary_digits[s-1];
            //            for (long s = 0; s <= j; s++) partialsum_naive -= ternary_digits[i+s*N-1];

            for (long s = 1; s < i+j*N; s++)
                partialsum_naive += ternary_digits[s-1];
            for (long s = 0; s < j; s++)
                partialsum_naive -= ternary_digits[i+s*N-1];
            
            if (partialsum_naive % 2 == 0)
                ternary_digits[i + j * N - 1] = ternary_digits_coords[i + j*N - 1];
            else
                ternary_digits[i + j * N - 1] = (2 - ternary_digits_coords[i + j*N - 1]);
        }
    }

    free(ternary_digits_coords);
    free(ternary_digits_coord_temp);
}

void peanos_hyperCubeToUnitInterval(double *coords, long N,  mpfr_t output, long precision) {
    char *ternary_digits = (char *)malloc((N*precision) * sizeof(char)); // TODO: avoid this allocation
    peanos_hyperCubeToTernaryDigits(coords, N, ternary_digits, precision);
    peanos_ternaryRepresentationToUnitInterval(ternary_digits, N*precision, output);
    free(ternary_digits);
}


// N is the size of the allocated arrays
// input is assumed to be between 0 and 1
void peanos_peanosToFeatures(mpfr_t alpha, mpfr_t sigma, mpfr_t kappa, double *amps, double *freqratios, double *noisinesses, long N, long precision, char use_padded_model)
{
    
    if (!use_padded_model) { // Standard model
        // Amplitudes
//        double sph[DADA_PEANOS_MAXPARTIALS+2];
        double *sph = malloc((N+2) * sizeof(double));
        sph[0] = 1;
        peanos_unitIntervalToHyperCube(alpha, N-1, sph+1, precision);
        for (long i = 1; i < N; i++)
            sph[i] *= PEANOS_PI_OVER_TWO; // pi/2
        peanos_sphericalToCartesian(sph, amps, N);
        
        // Freqs
        peanos_unitIntervalToHyperCube(sigma, N, freqratios, precision);
        for (long i = 0; i < N; i++)
            freqratios[i] = (1 + i + freqratios[i]);
        
        // Noisiness
        peanos_unitIntervalToHyperCube(kappa, N, noisinesses, precision);

        free(sph);
    } else { // Model padded for continuity
        
        // Amplitudes
        long Npad = N+2;
        double *sph = malloc((N+2+2) * sizeof(double));
        double *valpha = malloc((N+2+2) * sizeof(double));
//        double sph[DADA_PEANOS_MAXPARTIALS+2+2], valpha[DADA_PEANOS_MAXPARTIALS+2+2];
        sph[0] = 1;
        peanos_unitIntervalToHyperCube(alpha, N+1, sph+1, precision);
        for (long i = 1; i < N+2; i++)
            sph[i] *= PEANOS_PI_OVER_TWO; // pi/2
        peanos_sphericalToCartesian(sph, valpha, N+2);
        
        // additional infrastructure to preserve continuity (see paper)
        double den = sqrt(valpha[Npad-2]*valpha[Npad-2] + valpha[Npad-1] * valpha[Npad-1]);
        double phinminonealpha = 0;
        if (den != 0)
            phinminonealpha = acos(valpha[Npad-2]/den);
        double phionebeta = PEANOS_PI_OVER_TWO * (2./3. + (1/PEANOS_PI_OVER_TWO) * phinminonealpha/3.);
        double S = sin(phionebeta);
        double C = cos(phionebeta);
        double sigma_d = mpfr_get_d(sigma, MPFR_RNDN);
        for (long i = 0; i < N; i++)
            amps[i] = pow(S, sigma_d) * valpha[i];
        amps[N] = pow(S, sigma_d) * sqrt(1 - sigma_d) * valpha[N];
        amps[N+1] = C * sqrt(sigma_d);
        double norm_amps = 0;
        for (long i = 0; i < Npad; i++)
            norm_amps += amps[i]*amps[i];
        norm_amps = sqrt(norm_amps);
        for (long i = 0; i < Npad; i++)
            amps[i] /= norm_amps;
        
        // Freqs
        peanos_unitIntervalToHyperCube(sigma, N+2, freqratios, precision);
        for (long i = 0; i < N+1; i++)
            freqratios[i] = (1 + i + freqratios[i]);
//        freqratios[N+1] = model_freqs[N+1] * x->fzero;
        
        // Noisiness
        peanos_unitIntervalToHyperCube(kappa, N+2, noisinesses, precision);
        
        free(sph);
        free(valpha);
    }
}



// N is the size of the allocated arrays
void peanos_featuresToPeanos(double *amps, double *freqratios, double *noisinesses, long N, mpfr_t alpha, mpfr_t sigma, mpfr_t kappa, long precision, char use_padded_model)
{
    if (!use_padded_model) { // Standard model
        // Amplitudes
//        double sph[DADA_PEANOS_MAXPARTIALS+2];
        double *sph = malloc((N+2) * sizeof(double));
        peanos_cartesianToSpherical(amps, sph, N);
        // ignoring sph[0] which contains the gain
        for (long i = 1; i < N; i++)
            sph[i] /= PEANOS_PI_OVER_TWO;
        peanos_hyperCubeToUnitInterval(sph+1, N-1, alpha, precision);

        // Freqs
        double *fr = malloc((N+2) * sizeof(double));
        for (long i = 0; i < N; i++)
            fr[i] = (freqratios[i] - (i + 1));
        peanos_hyperCubeToUnitInterval(fr, N, sigma, precision);
        
        // Noisiness
        peanos_hyperCubeToUnitInterval(noisinesses, N, kappa, precision);

        free(sph);
        free(fr);
    } else { // Model padded for continuity
        /*
        // Amplitudes
        long Npad = N+2;
        double *sph = malloc((N+2+2) * sizeof(double));
        double *valpha = malloc((N+2+2) * sizeof(double));
//        double sph[DADA_PEANOS_MAXPARTIALS+2+2], valpha[DADA_PEANOS_MAXPARTIALS+2+2];
        sph[0] = 1;
        unitIntervalToHyperCube(x->coord_hp[0], N+1, precision, sph+1);
        for (long i = 1; i < N+2; i++)
            sph[i] *= 1.5707963268; // pi/2
        sphericalToCartesian(sph, valpha, N+2);
        
        // additional infrastructure to preserve continuity (see paper)
        double den = sqrt(valpha[Npad-2]*valpha[Npad-2] + valpha[Npad-1] * valpha[Npad-1]);
        double phinminonealpha = 0;
        if (den != 0)
            phinminonealpha = acos(valpha[Npad-2]/den);
        double phionebeta = PIOVERTWO * (2./3. + (1/PIOVERTWO) * phinminonealpha/3.);
        double S = sin(phionebeta);
        double C = cos(phionebeta);
        double sigma_d = mpfr_get_d(x->coord_hp[1], DADA_PEANOS_MPFR_RND);
        for (long i = 0; i < N; i++)
            model_amps[i] = pow(S, sigma_d) * valpha[i];
        model_amps[N] = pow(S, sigma_d) * sqrt(1 - sigma_d) * valpha[N];
        model_amps[N+1] = C * sqrt(sigma_d);
        double norm_amps = 0;
        for (long i = 0; i < Npad; i++)
            norm_amps += model_amps[i]*model_amps[i];
        norm_amps = sqrt(norm_amps);
        for (long i = 0; i < Npad; i++)
            model_amps[i] /= norm_amps;
        
        // Freqs
        unitIntervalToHyperCube(x->coord_hp[1], N+2, precision, model_freqs);
        for (long i = 0; i < N+1; i++)
            model_freqs[i] = (1 + i + model_freqs[i]);
        model_freqs[N+1] = model_freqs[N+1] * x->fzero;
        
        // Noisiness
        unitIntervalToHyperCube(x->coord_hp[2], N+2, precision, model_noisinesses);
        
        free(sph);
        free(valpha);
    }
         */
    }
}
    

#endif // _LIBPEANOS_H_

