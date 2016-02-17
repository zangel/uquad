#include "blaswrap.h"
/*  -- translated by f2c (version 19990503).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct {
    integer selopt, seldim;
    logical selval[20];
    real selwr[20], selwi[20];
} sslct_;

#define sslct_1 sslct_

logical cslect_(complex *z__)
{
    /* System generated locals */
    integer i__1, i__2, i__3;
    complex q__1, q__2;
    logical ret_val;

    /* Builtin functions */
    double c_abs(complex *);

    /* Local variables */
    static real rmin;
    static integer i__;
    static real x;


/*  -- LAPACK test routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    CSLECT returns .TRUE. if the eigenvalue Z is to be selected,   
    otherwise it returns .FALSE.   
    It is used by CCHK41 to test if CGEES succesfully sorts eigenvalues,   
    and by CCHK43 to test if CGEESX succesfully sorts eigenvalues.   

    The common block /SSLCT/ controls how eigenvalues are selected.   
    If SELOPT = 0, then CSLECT return .TRUE. when real(Z) is less than   
    zero, and .FALSE. otherwise.   
    If SELOPT is at least 1, CSLECT returns SELVAL(SELOPT) and adds 1   
    to SELOPT, cycling back to 1 at SELMAX.   

    Arguments   
    =========   

    Z       (input) COMPLEX   
            The eigenvalue Z.   

    ===================================================================== */


    if (sslct_1.selopt == 0) {
	ret_val = z__->r < 0.f;
    } else {
	rmin = 0.f;
	i__1 = sslct_1.seldim;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    i__2 = i__ - 1;
	    i__3 = i__ - 1;
	    q__2.r = sslct_1.selwr[i__2], q__2.i = sslct_1.selwi[i__3];
	    q__1.r = z__->r - q__2.r, q__1.i = z__->i - q__2.i;
	    x = c_abs(&q__1);
	    if (x <= rmin) {
		rmin = x;
		ret_val = sslct_1.selval[i__ - 1];
	    }
/* L10: */
	}
    }
    return ret_val;

/*     End of CSLECT */

} /* cslect_ */

