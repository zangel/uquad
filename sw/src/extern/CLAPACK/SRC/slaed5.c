#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int slaed5_(integer *i__, real *d__, real *z__, real *delta, 
	real *rho, real *dlam)
{
/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Oak Ridge National Lab, Argonne National Lab,   
       Courant Institute, NAG Ltd., and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    This subroutine computes the I-th eigenvalue of a symmetric rank-one   
    modification of a 2-by-2 diagonal matrix   

               diag( D )  +  RHO *  Z * transpose(Z) .   

    The diagonal elements in the array D are assumed to satisfy   

               D(i) < D(j)  for  i < j .   

    We also assume RHO > 0 and that the Euclidean norm of the vector   
    Z is one.   

    Arguments   
    =========   

    I      (input) INTEGER   
           The index of the eigenvalue to be computed.  I = 1 or I = 2.   

    D      (input) REAL array, dimension (2)   
           The original eigenvalues.  We assume D(1) < D(2).   

    Z      (input) REAL array, dimension (2)   
           The components of the updating vector.   

    DELTA  (output) REAL array, dimension (2)   
           The vector DELTA contains the information necessary   
           to construct the eigenvectors.   

    RHO    (input) REAL   
           The scalar in the symmetric updating formula.   

    DLAM   (output) REAL   
           The computed lambda_I, the I-th updated eigenvalue.   

    Further Details   
    ===============   

    Based on contributions by   
       Ren-Cang Li, Computer Science Division, University of California   
       at Berkeley, USA   

    =====================================================================   


       Parameter adjustments */
    /* System generated locals */
    real r__1;
    /* Builtin functions */
    double sqrt(doublereal);
    /* Local variables */
    static real temp, b, c__, w, del, tau;

    --delta;
    --z__;
    --d__;

    /* Function Body */
    del = d__[2] - d__[1];
    if (*i__ == 1) {
	w = *rho * 2.f * (z__[2] * z__[2] - z__[1] * z__[1]) / del + 1.f;
	if (w > 0.f) {
	    b = del + *rho * (z__[1] * z__[1] + z__[2] * z__[2]);
	    c__ = *rho * z__[1] * z__[1] * del;

/*           B > ZERO, always */

	    tau = c__ * 2.f / (b + sqrt((r__1 = b * b - c__ * 4.f, dabs(r__1))
		    ));
	    *dlam = d__[1] + tau;
	    delta[1] = -z__[1] / tau;
	    delta[2] = z__[2] / (del - tau);
	} else {
	    b = -del + *rho * (z__[1] * z__[1] + z__[2] * z__[2]);
	    c__ = *rho * z__[2] * z__[2] * del;
	    if (b > 0.f) {
		tau = c__ * -2.f / (b + sqrt(b * b + c__ * 4.f));
	    } else {
		tau = (b - sqrt(b * b + c__ * 4.f)) / 2.f;
	    }
	    *dlam = d__[2] + tau;
	    delta[1] = -z__[1] / (del + tau);
	    delta[2] = -z__[2] / tau;
	}
	temp = sqrt(delta[1] * delta[1] + delta[2] * delta[2]);
	delta[1] /= temp;
	delta[2] /= temp;
    } else {

/*     Now I=2 */

	b = -del + *rho * (z__[1] * z__[1] + z__[2] * z__[2]);
	c__ = *rho * z__[2] * z__[2] * del;
	if (b > 0.f) {
	    tau = (b + sqrt(b * b + c__ * 4.f)) / 2.f;
	} else {
	    tau = c__ * 2.f / (-b + sqrt(b * b + c__ * 4.f));
	}
	*dlam = d__[2] + tau;
	delta[1] = -z__[1] / (del + tau);
	delta[2] = -z__[2] / tau;
	temp = sqrt(delta[1] * delta[1] + delta[2] * delta[2]);
	delta[1] /= temp;
	delta[2] /= temp;
    }
    return 0;

/*     End OF SLAED5 */

} /* slaed5_ */

