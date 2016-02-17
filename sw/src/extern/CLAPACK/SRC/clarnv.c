#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int clarnv_(integer *idist, integer *iseed, integer *n, 
	complex *x)
{
/*  -- LAPACK auxiliary routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    CLARNV returns a vector of n random complex numbers from a uniform or   
    normal distribution.   

    Arguments   
    =========   

    IDIST   (input) INTEGER   
            Specifies the distribution of the random numbers:   
            = 1:  real and imaginary parts each uniform (0,1)   
            = 2:  real and imaginary parts each uniform (-1,1)   
            = 3:  real and imaginary parts each normal (0,1)   
            = 4:  uniformly distributed on the disc abs(z) < 1   
            = 5:  uniformly distributed on the circle abs(z) = 1   

    ISEED   (input/output) INTEGER array, dimension (4)   
            On entry, the seed of the random number generator; the array   
            elements must be between 0 and 4095, and ISEED(4) must be   
            odd.   
            On exit, the seed is updated.   

    N       (input) INTEGER   
            The number of random numbers to be generated.   

    X       (output) COMPLEX array, dimension (N)   
            The generated random numbers.   

    Further Details   
    ===============   

    This routine calls the auxiliary routine SLARUV to generate random   
    real numbers from a uniform (0,1) distribution, in batches of up to   
    128 using vectorisable code. The Box-Muller method is used to   
    transform numbers from a uniform to a normal distribution.   

    =====================================================================   


       Parameter adjustments */
    /* System generated locals */
    integer i__1, i__2, i__3, i__4, i__5;
    real r__1, r__2;
    complex q__1, q__2, q__3;
    /* Builtin functions */
    double log(doublereal), sqrt(doublereal);
    void c_exp(complex *, complex *);
    /* Local variables */
    static integer i__;
    static real u[128];
    static integer il, iv;
    extern /* Subroutine */ int slaruv_(integer *, integer *, real *);

    --x;
    --iseed;

    /* Function Body */
    i__1 = *n;
    for (iv = 1; iv <= i__1; iv += 64) {
/* Computing MIN */
	i__2 = 64, i__3 = *n - iv + 1;
	il = min(i__2,i__3);

/*        Call SLARUV to generate 2*IL real numbers from a uniform (0,1)   
          distribution (2*IL <= LV) */

	i__2 = il << 1;
	slaruv_(&iseed[1], &i__2, u);

	if (*idist == 1) {

/*           Copy generated numbers */

	    i__2 = il;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		i__3 = iv + i__ - 1;
		i__4 = (i__ << 1) - 2;
		i__5 = (i__ << 1) - 1;
		q__1.r = u[i__4], q__1.i = u[i__5];
		x[i__3].r = q__1.r, x[i__3].i = q__1.i;
/* L10: */
	    }
	} else if (*idist == 2) {

/*           Convert generated numbers to uniform (-1,1) distribution */

	    i__2 = il;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		i__3 = iv + i__ - 1;
		r__1 = u[(i__ << 1) - 2] * 2.f - 1.f;
		r__2 = u[(i__ << 1) - 1] * 2.f - 1.f;
		q__1.r = r__1, q__1.i = r__2;
		x[i__3].r = q__1.r, x[i__3].i = q__1.i;
/* L20: */
	    }
	} else if (*idist == 3) {

/*           Convert generated numbers to normal (0,1) distribution */

	    i__2 = il;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		i__3 = iv + i__ - 1;
		r__1 = sqrt(log(u[(i__ << 1) - 2]) * -2.f);
		r__2 = u[(i__ << 1) - 1] * 6.2831853071795864769252867663f;
		q__3.r = 0.f, q__3.i = r__2;
		c_exp(&q__2, &q__3);
		q__1.r = r__1 * q__2.r, q__1.i = r__1 * q__2.i;
		x[i__3].r = q__1.r, x[i__3].i = q__1.i;
/* L30: */
	    }
	} else if (*idist == 4) {

/*           Convert generated numbers to complex numbers uniformly   
             distributed on the unit disk */

	    i__2 = il;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		i__3 = iv + i__ - 1;
		r__1 = sqrt(u[(i__ << 1) - 2]);
		r__2 = u[(i__ << 1) - 1] * 6.2831853071795864769252867663f;
		q__3.r = 0.f, q__3.i = r__2;
		c_exp(&q__2, &q__3);
		q__1.r = r__1 * q__2.r, q__1.i = r__1 * q__2.i;
		x[i__3].r = q__1.r, x[i__3].i = q__1.i;
/* L40: */
	    }
	} else if (*idist == 5) {

/*           Convert generated numbers to complex numbers uniformly   
             distributed on the unit circle */

	    i__2 = il;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		i__3 = iv + i__ - 1;
		r__1 = u[(i__ << 1) - 1] * 6.2831853071795864769252867663f;
		q__2.r = 0.f, q__2.i = r__1;
		c_exp(&q__1, &q__2);
		x[i__3].r = q__1.r, x[i__3].i = q__1.i;
/* L50: */
	    }
	}
/* L60: */
    }
    return 0;

/*     End of CLARNV */

} /* clarnv_ */

