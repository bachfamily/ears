/*							iv.c
 *
 *	Modified Bessel function of noninteger order
 *
 *
 *
 * SYNOPSIS:
 *
 * double v, x, y, iv();
 *
 * y = iv( v, x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns modified Bessel function of order v of the
 * argument.  If x is negative, v must be integer valued.
 *
 * The function is defined as Iv(x) = Jv( ix ).  It is
 * here computed in terms of the confluent hypergeometric
 * function, according to the formula
 *
 *              v  -x
 * Iv(x) = (x/2)  e   hyperg( v+0.5, 2v+1, 2x ) / gamma(v+1)
 *
 * If v is a negative integer, then v is replaced by -v.
 *
 *
 * ACCURACY:
 *
 * Tested at random points (v, x), with v between 0 and
 * 30, x between 0 and 28.
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    DEC       0,30          2000      3.1e-15     5.4e-16
 *    IEEE      0,30         10000      1.7e-14     2.7e-15
 *
 * Accuracy is diminished if v is near a negative integer.
 *
 * See also hyperg.c.
 *
 */
/*							iv.c	*/
/*	Modified Bessel function of noninteger order		*/
/* If x < 0, then v must be an integer. */


/*
Cephes Math Library Release 2.8:  June, 2000
Copyright 1984, 1987, 1988, 2000 by Stephen L. Moshier
*/

#ifdef _WIN64
#define _USE_MATH_DEFINES
#endif

#include <cstddef>
#include "bessel.h"

// #ifdef ANSIPROT
// extern double hyperg ( double, double, double );
// extern double exp ( double );
// extern double gamma ( double );
// extern double log ( double );
// extern double fabs ( double );
// extern double floor ( double );
// #else
// double hyperg(), exp(), gamma(), log(), fabs(), floor();
// #endif
// extern double MACHEP, MAXNUM;

namespace cephes {

static double iv_asymptotic(double v, double x);
static void ikv_asymptotic_uniform(double v, double x, double *Iv, double *Kv);
static void ikv_temme(double v, double x, double *Iv, double *Kv);


double iv(double v, double x)
{
int sign;
double t, ax, res;

/* If v is a negative integer, invoke symmetry */
t = floor(v);
if( v < 0.0 )
	{
	if( t == v )
		{
		v = -v;	/* symmetry */
		t = -t;
		}
	}
/* If x is negative, require v to be an integer */
sign = 1;
if( x < 0.0 )
	{
	if( t != v )
		{
		// // mtherr( "iv", DOMAIN );
		return( 0.0 );
		}
	if( v != 2.0 * floor(v/2.0) )
		sign = -1;
	}

/* Avoid logarithm singularity */
if( x == 0.0 )
	{
	if( v == 0.0 )
		return( 1.0 );
	if( v < 0.0 )
		{
		// // mtherr( "iv", OVERFLOW );
		return( MAXNUM );
		}
	else
		return( 0.0 );
	}

ax = fabs(x);
// t = v * log( 0.5 * ax )  -  x;
// t = sign * exp(t) / gamma( v + 1.0 );
// ax = v + 0.5;
// return( t * hyperg( ax,  2.0 * ax,  2.0 * x ) );

if (fabs(v) > 50) {
	/*
	* Uniform asymptotic expansion for large orders.
	*
	* This appears to overflow slightly later than the Boost
	* implementation of Temme's method.
	*/
	ikv_asymptotic_uniform(v, ax, &res, NULL);
}
else {
	/* Otherwise: Temme's method */
	ikv_temme(v, ax, &res, NULL);
}
res *= sign;
return res;
}


/*
 * Compute Iv from (AMS5 9.7.1), asymptotic expansion for large |z|
 * Iv ~ exp(x)/sqrt(2 pi x) ( 1 + (4*v*v-1)/8x + (4*v*v-1)(4*v*v-9)/8x/2! + ...)
 */
static double iv_asymptotic(double v, double x)
{
    double mu;
    double sum, term, prefactor, factor;
    int k;

    prefactor = exp(x) / sqrt(2 * M_PI * x);

    if (prefactor == INFINITY) {
	return prefactor;
    }

    mu = 4 * v * v;
    sum = 1.0;
    term = 1.0;
    k = 1;

    do {
	factor = (mu - (2 * k - 1) * (2 * k - 1)) / (8 * x) / k;
	if (k > 100) {
	    /* didn't converge */
	    // mtherr("iv(iv_asymptotic)", TLOSS);
	    break;
	}
	term *= -factor;
	sum += term;
	++k;
    } while (fabs(term) > MACHEP * fabs(sum));
    return sum * prefactor;
}


/*
 * Uniform asymptotic expansion factors, (AMS5 9.3.9; AMS5 9.3.10)
 *
 * Computed with:
 * --------------------
  import numpy as np
  t = np.poly1d([1,0])
  def up1(p):
  return .5*t*t*(1-t*t)*p.deriv() + 1/8. * ((1-5*t*t)*p).integ()
  us = [np.poly1d([1])]
  for k in range(10):
  us.append(up1(us[-1]))
  n = us[-1].order
  for p in us:
  print "{" + ", ".join(["0"]*(n-p.order) + map(repr, p)) + "},"
  print "N_UFACTORS", len(us)
  print "N_UFACTOR_TERMS", us[-1].order + 1
 * --------------------
 */
#define N_UFACTORS 11
#define N_UFACTOR_TERMS 31
static const double asymptotic_ufactors[N_UFACTORS][N_UFACTOR_TERMS] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, -0.20833333333333334, 0.0, 0.125, 0.0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0.3342013888888889, 0.0, -0.40104166666666669, 0.0, 0.0703125, 0.0,
     0.0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     -1.0258125964506173, 0.0, 1.8464626736111112, 0.0,
     -0.89121093750000002, 0.0, 0.0732421875, 0.0, 0.0, 0.0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     4.6695844234262474, 0.0, -11.207002616222995, 0.0, 8.78912353515625,
     0.0, -2.3640869140624998, 0.0, 0.112152099609375, 0.0, 0.0, 0.0, 0.0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -28.212072558200244, 0.0,
     84.636217674600744, 0.0, -91.818241543240035, 0.0, 42.534998745388457,
     0.0, -7.3687943594796312, 0.0, 0.22710800170898438, 0.0, 0.0, 0.0,
     0.0, 0.0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 212.5701300392171, 0.0,
     -765.25246814118157, 0.0, 1059.9904525279999, 0.0,
     -699.57962737613275, 0.0, 218.19051174421159, 0.0,
     -26.491430486951554, 0.0, 0.57250142097473145, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, -1919.4576623184068, 0.0,
     8061.7221817373083, 0.0, -13586.550006434136, 0.0, 11655.393336864536,
     0.0, -5305.6469786134048, 0.0, 1200.9029132163525, 0.0,
     -108.09091978839464, 0.0, 1.7277275025844574, 0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0},
    {0, 0, 0, 0, 0, 0, 20204.291330966149, 0.0, -96980.598388637503, 0.0,
     192547.0012325315, 0.0, -203400.17728041555, 0.0, 122200.46498301747,
     0.0, -41192.654968897557, 0.0, 7109.5143024893641, 0.0,
     -493.915304773088, 0.0, 6.074042001273483, 0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0},
    {0, 0, 0, -242919.18790055133, 0.0, 1311763.6146629769, 0.0,
     -2998015.9185381061, 0.0, 3763271.2976564039, 0.0,
     -2813563.2265865342, 0.0, 1268365.2733216248, 0.0,
     -331645.17248456361, 0.0, 45218.768981362737, 0.0,
     -2499.8304818112092, 0.0, 24.380529699556064, 0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0},
    {3284469.8530720375, 0.0, -19706819.11843222, 0.0, 50952602.492664628,
     0.0, -74105148.211532637, 0.0, 66344512.274729028, 0.0,
     -37567176.660763353, 0.0, 13288767.166421819, 0.0,
     -2785618.1280864552, 0.0, 308186.40461266245, 0.0,
     -13886.089753717039, 0.0, 110.01714026924674, 0.0, 0.0, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.0}
};


/*
 * Compute Iv, Kv from (AMS5 9.7.7 + 9.7.8), asymptotic expansion for large v
 */
static void ikv_asymptotic_uniform(double v, double x,
			    double *i_value, double *k_value)
{
    double i_prefactor, k_prefactor;
    double t, t2, eta, z;
    double i_sum, k_sum, term, divisor;
    int k, n;
    int sign = 1;

    if (v < 0) {
	/* Negative v; compute I_{-v} and K_{-v} and use (AMS 9.6.2) */
	sign = -1;
	v = -v;
    }

    z = x / v;
    t = 1 / sqrt(1 + z * z);
    t2 = t * t;
    eta = sqrt(1 + z * z) + log(z / (1 + 1 / t));

    i_prefactor = sqrt(t / (2 * M_PI * v)) * exp(v * eta);
    i_sum = 1.0;

    k_prefactor = sqrt(M_PI * t / (2 * v)) * exp(-v * eta);
    k_sum = 1.0;

    divisor = v;
    for (n = 1; n < N_UFACTORS; ++n) {
	/*
	 * Evaluate u_k(t) with Horner's scheme;
	 * (using the knowledge about which coefficients are zero)
	 */
	term = 0;
	for (k = N_UFACTOR_TERMS - 1 - 3 * n;
	     k < N_UFACTOR_TERMS - n; k += 2) {
	    term *= t2;
	    term += asymptotic_ufactors[n][k];
	}
	for (k = 1; k < n; k += 2) {
	    term *= t2;
	}
	if (n % 2 == 1) {
	    term *= t;
	}

	/* Sum terms */
	term /= divisor;
	i_sum += term;
	k_sum += (n % 2 == 0) ? term : -term;

	/* Check convergence */
	if (fabs(term) < MACHEP) {
	    break;
	}

	divisor *= v;
    }

    if (fabs(term) > 1e-3 * fabs(i_sum)) {
	/* Didn't converge */
	// mtherr("ikv_asymptotic_uniform", TLOSS);
    }
    if (fabs(term) > MACHEP * fabs(i_sum)) {
	/* Some precision lost */
	// mtherr("ikv_asymptotic_uniform", PLOSS);
    }

    if (k_value != NULL) {
	/* symmetric in v */
	*k_value = k_prefactor * k_sum;
    }

    if (i_value != NULL) {
	if (sign == 1) {
	    *i_value = i_prefactor * i_sum;
	}
	else {
	    /* (AMS 9.6.2) */
	    *i_value = (i_prefactor * i_sum
			+ (2 / M_PI) * sin(M_PI * v) * k_prefactor * k_sum);
	}
    }
}


/*
 * The following code originates from the Boost C++ library,
 * from file `boost/math/special_functions/detail/bessel_ik.hpp`,
 * converted from C++ to C.
 */

#ifdef DEBUG
#define BOOST_ASSERT(a) assert(a)
#else
#define BOOST_ASSERT(a)
#endif

/*
 * Modified Bessel functions of the first and second kind of fractional order
 *
 * Calculate K(v, x) and K(v+1, x) by method analogous to
 * Temme, Journal of Computational Physics, vol 21, 343 (1976)
 */
static int temme_ik_series(double v, double x, double *K, double *K1)
{
    double f, h, p, q, coef, sum, sum1, tolerance;
    double a, b, c, d, sigma, gamma1, gamma2;
    unsigned long k;
    double gp;
    double gm;


    /*
     * |x| <= 2, Temme series converge rapidly
     * |x| > 2, the larger the |x|, the slower the convergence
     */
    BOOST_ASSERT(fabs(x) <= 2);
    BOOST_ASSERT(fabs(v) <= 0.5f);

    gp = gamma(v + 1) - 1;
    gm = gamma(-v + 1) - 1;

    a = log(x / 2);
    b = exp(v * a);
    sigma = -a * v;
    c = fabs(v) < MACHEP ? 1 : sin(M_PI * v) / (v * M_PI);
    d = fabs(sigma) < MACHEP ? 1 : sinh(sigma) / sigma;
    gamma1 = fabs(v) < MACHEP ? -EULER : (0.5f / v) * (gp - gm) * c;
    gamma2 = (2 + gp + gm) * c / 2;

    /* initial values */
    p = (gp + 1) / (2 * b);
    q = (1 + gm) * b / 2;
    f = (cosh(sigma) * gamma1 + d * (-a) * gamma2) / c;
    h = p;
    coef = 1;
    sum = coef * f;
    sum1 = coef * h;

    /* series summation */
    tolerance = MACHEP;
    for (k = 1; k < MAXITER; k++) {
	f = (k * f + p + q) / (k * k - v * v);
	p /= k - v;
	q /= k + v;
	h = p - k * f;
	coef *= x * x / (4 * k);
	sum += coef * f;
	sum1 += coef * h;
	if (fabs(coef * f) < fabs(sum) * tolerance) {
	    break;
	}
    }
    if (k == MAXITER) {
	// mtherr("ikv_temme(temme_ik_series)", TLOSS);
    }

    *K = sum;
    *K1 = 2 * sum1 / x;

    return 0;
}

/* Evaluate continued fraction fv = I_(v+1) / I_v, derived from
 * Abramowitz and Stegun, Handbook of Mathematical Functions, 1972, 9.1.73 */
static int CF1_ik(double v, double x, double *fv)
{
    double C, D, f, a, b, delta, tiny, tolerance;
    unsigned long k;


    /*
     * |x| <= |v|, CF1_ik converges rapidly
     * |x| > |v|, CF1_ik needs O(|x|) iterations to converge
     */

    /*
     * modified Lentz's method, see
     * Lentz, Applied Optics, vol 15, 668 (1976)
     */
    tolerance = 2 * MACHEP;
    tiny = 1 / sqrt(MAXNUM);
    C = f = tiny;		/* b0 = 0, replace with tiny */
    D = 0;
    for (k = 1; k < MAXITER; k++) {
	a = 1;
	b = 2 * (v + k) / x;
	C = b + a / C;
	D = b + a * D;
	if (C == 0) {
	    C = tiny;
	}
	if (D == 0) {
	    D = tiny;
	}
	D = 1 / D;
	delta = C * D;
	f *= delta;
	if (fabs(delta - 1) <= tolerance) {
	    break;
	}
    }
    if (k == MAXITER) {
	// mtherr("ikv_temme(CF1_ik)", TLOSS);
    }

    *fv = f;

    return 0;
}

/*
 * Calculate K(v, x) and K(v+1, x) by evaluating continued fraction
 * z1 / z0 = U(v+1.5, 2v+1, 2x) / U(v+0.5, 2v+1, 2x), see
 * Thompson and Barnett, Computer Physics Communications, vol 47, 245 (1987)
 */
static int CF2_ik(double v, double x, double *Kv, double *Kv1)
{

    double S, C, Q, D, f, a, b, q, delta, tolerance, current, prev;
    unsigned long k;

    /*
     * |x| >= |v|, CF2_ik converges rapidly
     * |x| -> 0, CF2_ik fails to converge
     */

    BOOST_ASSERT(fabs(x) > 1);

    /*
     * Steed's algorithm, see Thompson and Barnett,
     * Journal of Computational Physics, vol 64, 490 (1986)
     */
    tolerance = MACHEP;
    a = v * v - 0.25f;
    b = 2 * (x + 1);		/* b1 */
    D = 1 / b;			/* D1 = 1 / b1 */
    f = delta = D;		/* f1 = delta1 = D1, coincidence */
    prev = 0;			/* q0 */
    current = 1;		/* q1 */
    Q = C = -a;			/* Q1 = C1 because q1 = 1 */
    S = 1 + Q * delta;		/* S1 */
    for (k = 2; k < MAXITER; k++) {	/* starting from 2 */
	/* continued fraction f = z1 / z0 */
	a -= 2 * (k - 1);
	b += 2;
	D = 1 / (b + a * D);
	delta *= b * D - 1;
	f += delta;

	/* series summation S = 1 + \sum_{n=1}^{\infty} C_n * z_n / z_0 */
	q = (prev - (b - 2) * current) / a;
	prev = current;
	current = q;		/* forward recurrence for q */
	C *= -a / k;
	Q += C * q;
	S += Q * delta;

	/* S converges slower than f */
	if (fabs(Q * delta) < fabs(S) * tolerance) {
	    break;
	}
    }
    if (k == MAXITER) {
	// mtherr("ikv_temme(CF2_ik)", TLOSS);
    }

    *Kv = sqrt(M_PI / (2 * x)) * exp(-x) / S;
    *Kv1 = *Kv * (0.5f + v + x + (v * v - 0.25f) * f) / x;

    return 0;
}

/* Flags for what to compute */
enum {
    need_i = 0x1,
    need_k = 0x2
};

/*
 * Compute I(v, x) and K(v, x) simultaneously by Temme's method, see
 * Temme, Journal of Computational Physics, vol 19, 324 (1975)
 */
static void ikv_temme(double v, double x, double *Iv_p, double *Kv_p)
{
    /* Kv1 = K_(v+1), fv = I_(v+1) / I_v */
    /* Ku1 = K_(u+1), fu = I_(u+1) / I_u */
    double u, Iv, Kv, Kv1, Ku, Ku1, fv;
    double W, current, prev, next;
    int reflect = 0;
    unsigned n, k;
    int kind;

    kind = 0;
    if (Iv_p != NULL) {
	kind |= need_i;
    }
    if (Kv_p != NULL) {
	kind |= need_k;
    }

    if (v < 0) {
	reflect = 1;
	v = -v;			/* v is non-negative from here */
	kind |= need_k;
    }
    n = round(v);
    u = v - n;			/* -1/2 <= u < 1/2 */

    if (x < 0) {
	if (Iv_p != NULL)
	    *Iv_p = NAN;
	if (Kv_p != NULL)
	    *Kv_p = NAN;
	// mtherr("ikv_temme", DOMAIN);
	return;
    }
    if (x == 0) {
	Iv = (v == 0) ? 1 : 0;
	if (kind & need_k) {
	    // mtherr("ikv_temme", OVERFLOW);
	    Kv = INFINITY;
	}
	else {
	    Kv = NAN;	/* any value will do */
	}

	if (reflect && (kind & need_i)) {
	    double z = (u + n % 2);

	    Iv = sin(M_PI * z) == 0 ? Iv : INFINITY;
	    if (Iv == INFINITY || Iv == -INFINITY) {
		// mtherr("ikv_temme", OVERFLOW);
	    }
	}

	if (Iv_p != NULL) {
	    *Iv_p = Iv;
	}
	if (Kv_p != NULL) {
	    *Kv_p = Kv;
	}
	return;
    }
    /* x is positive until reflection */
    W = 1 / x;			/* Wronskian */
    if (x <= 2) {		/* x in (0, 2] */
	temme_ik_series(u, x, &Ku, &Ku1);	/* Temme series */
    }
    else {			/* x in (2, \infty) */
	CF2_ik(u, x, &Ku, &Ku1);	/* continued fraction CF2_ik */
    }
    prev = Ku;
    current = Ku1;
    for (k = 1; k <= n; k++) {	/* forward recurrence for K */
	next = 2 * (u + k) * current / x + prev;
	prev = current;
	current = next;
    }
    Kv = prev;
    Kv1 = current;
    if (kind & need_i) {
	double lim = (4 * v * v + 10) / (8 * x);

	lim *= lim;
	lim *= lim;
	lim /= 24;
	if ((lim < MACHEP * 10) && (x > 100)) {
	    /*
	     * x is huge compared to v, CF1 may be very slow 
	     * to converge so use asymptotic expansion for large
	     * x case instead.  Note that the asymptotic expansion
	     * isn't very accurate - so it's deliberately very hard
	     * to get here - probably we're going to overflow:
	     */
	    Iv = iv_asymptotic(v, x);
	}
	else {
	    CF1_ik(v, x, &fv);	/* continued fraction CF1_ik */
	    Iv = W / (Kv * fv + Kv1);	/* Wronskian relation */
	}
    }
    else {
	Iv = NAN;		/* any value will do */
    }

    if (reflect) {
	double z = (u + n % 2);

	if (Iv_p != NULL) {
	    *Iv_p = Iv + (2 / M_PI) * sin(M_PI * z) * Kv;	/* reflection formula */
	}
	if (Kv_p != NULL) {
	    *Kv_p = Kv;
	}
    }
    else {
	if (Iv_p != NULL) {
	    *Iv_p = Iv;
	}
	if (Kv_p != NULL) {
	    *Kv_p = Kv;
	}
    }
    return;
}

} // namespace cephes
