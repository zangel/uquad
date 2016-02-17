#include "blaswrap.h"
/*  -- translated by f2c (version 19990503).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static integer c__1 = 1;

/* Subroutine */ int sget04_(integer *n, integer *nrhs, real *x, integer *ldx,
	 real *xact, integer *ldxact, real *rcond, real *resid)
{
    /* System generated locals */
    integer x_dim1, x_offset, xact_dim1, xact_offset, i__1, i__2;
    real r__1, r__2, r__3;

    /* Local variables */
    static integer i__, j;
    static real xnorm;
    static integer ix;
    static real diffnm;
    extern doublereal slamch_(char *);
    extern integer isamax_(integer *, real *, integer *);
    static real eps;


#define xact_ref(a_1,a_2) xact[(a_2)*xact_dim1 + a_1]
#define x_ref(a_1,a_2) x[(a_2)*x_dim1 + a_1]


/*  -- LAPACK test routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    SGET04 computes the difference between a computed solution and the   
    true solution to a system of linear equations.   

    RESID =  ( norm(X-XACT) * RCOND ) / ( norm(XACT) * EPS ),   
    where RCOND is the reciprocal of the condition number and EPS is the   
    machine epsilon.   

    Arguments   
    =========   

    N       (input) INTEGER   
            The number of rows of the matrices X and XACT.  N >= 0.   

    NRHS    (input) INTEGER   
            The number of columns of the matrices X and XACT.  NRHS >= 0.   

    X       (input) REAL array, dimension (LDX,NRHS)   
            The computed solution vectors.  Each vector is stored as a   
            column of the matrix X.   

    LDX     (input) INTEGER   
            The leading dimension of the array X.  LDX >= max(1,N).   

    XACT    (input) REAL array, dimension( LDX, NRHS )   
            The exact solution vectors.  Each vector is stored as a   
            column of the matrix XACT.   

    LDXACT  (input) INTEGER   
            The leading dimension of the array XACT.  LDXACT >= max(1,N).   

    RCOND   (input) REAL   
            The reciprocal of the condition number of the coefficient   
            matrix in the system of equations.   

    RESID   (output) REAL   
            The maximum over the NRHS solution vectors of   
            ( norm(X-XACT) * RCOND ) / ( norm(XACT) * EPS )   

    =====================================================================   


       Quick exit if N = 0 or NRHS = 0.   

       Parameter adjustments */
    x_dim1 = *ldx;
    x_offset = 1 + x_dim1 * 1;
    x -= x_offset;
    xact_dim1 = *ldxact;
    xact_offset = 1 + xact_dim1 * 1;
    xact -= xact_offset;

    /* Function Body */
    if (*n <= 0 || *nrhs <= 0) {
	*resid = 0.f;
	return 0;
    }

/*     Exit with RESID = 1/EPS if RCOND is invalid. */

    eps = slamch_("Epsilon");
    if (*rcond < 0.f) {
	*resid = 1.f / eps;
	return 0;
    }

/*     Compute the maximum of   
          norm(X - XACT) / ( norm(XACT) * EPS )   
       over all the vectors X and XACT . */

    *resid = 0.f;
    i__1 = *nrhs;
    for (j = 1; j <= i__1; ++j) {
	ix = isamax_(n, &xact_ref(1, j), &c__1);
	xnorm = (r__1 = xact_ref(ix, j), dabs(r__1));
	diffnm = 0.f;
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
/* Computing MAX */
	    r__2 = diffnm, r__3 = (r__1 = x_ref(i__, j) - xact_ref(i__, j), 
		    dabs(r__1));
	    diffnm = dmax(r__2,r__3);
/* L10: */
	}
	if (xnorm <= 0.f) {
	    if (diffnm > 0.f) {
		*resid = 1.f / eps;
	    }
	} else {
/* Computing MAX */
	    r__1 = *resid, r__2 = diffnm / xnorm * *rcond;
	    *resid = dmax(r__1,r__2);
	}
/* L20: */
    }
    if (*resid * eps < 1.f) {
	*resid /= eps;
    }

    return 0;

/*     End of SGET04 */

} /* sget04_ */

#undef x_ref
#undef xact_ref


