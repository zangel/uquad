#include "blaswrap.h"
/*  -- translated by f2c (version 19990503).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static integer c__1 = 1;
static complex c_b12 = {-1.f,0.f};

/* Subroutine */ int cgbt01_(integer *m, integer *n, integer *kl, integer *ku,
	 complex *a, integer *lda, complex *afac, integer *ldafac, integer *
	ipiv, complex *work, real *resid)
{
    /* System generated locals */
    integer a_dim1, a_offset, afac_dim1, afac_offset, i__1, i__2, i__3, i__4;
    real r__1, r__2;

    /* Local variables */
    static integer lenj, i__, j;
    static complex t;
    static real anorm;
    extern /* Subroutine */ int ccopy_(integer *, complex *, integer *, 
	    complex *, integer *), caxpy_(integer *, complex *, complex *, 
	    integer *, complex *, integer *);
    static integer i1, i2, kd, il, jl, ip, ju, iw;
    extern doublereal slamch_(char *), scasum_(integer *, complex *, 
	    integer *);
    static integer jua;
    static real eps;


#define a_subscr(a_1,a_2) (a_2)*a_dim1 + a_1
#define a_ref(a_1,a_2) a[a_subscr(a_1,a_2)]
#define afac_subscr(a_1,a_2) (a_2)*afac_dim1 + a_1
#define afac_ref(a_1,a_2) afac[afac_subscr(a_1,a_2)]


/*  -- LAPACK test routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    CGBT01 reconstructs a band matrix  A  from its L*U factorization and   
    computes the residual:   
       norm(L*U - A) / ( N * norm(A) * EPS ),   
    where EPS is the machine epsilon.   

    The expression L*U - A is computed one column at a time, so A and   
    AFAC are not modified.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= 0.   

    KL      (input) INTEGER   
            The number of subdiagonals within the band of A.  KL >= 0.   

    KU      (input) INTEGER   
            The number of superdiagonals within the band of A.  KU >= 0.   

    A       (input/output) COMPLEX array, dimension (LDA,N)   
            The original matrix A in band storage, stored in rows 1 to   
            KL+KU+1.   

    LDA     (input) INTEGER.   
            The leading dimension of the array A.  LDA >= max(1,KL+KU+1).   

    AFAC    (input) COMPLEX array, dimension (LDAFAC,N)   
            The factored form of the matrix A.  AFAC contains the banded   
            factors L and U from the L*U factorization, as computed by   
            CGBTRF.  U is stored as an upper triangular band matrix with   
            KL+KU superdiagonals in rows 1 to KL+KU+1, and the   
            multipliers used during the factorization are stored in rows   
            KL+KU+2 to 2*KL+KU+1.  See CGBTRF for further details.   

    LDAFAC  (input) INTEGER   
            The leading dimension of the array AFAC.   
            LDAFAC >= max(1,2*KL*KU+1).   

    IPIV    (input) INTEGER array, dimension (min(M,N))   
            The pivot indices from CGBTRF.   

    WORK    (workspace) COMPLEX array, dimension (2*KL+KU+1)   

    RESID   (output) REAL   
            norm(L*U - A) / ( N * norm(A) * EPS )   

    =====================================================================   


       Quick exit if M = 0 or N = 0.   

       Parameter adjustments */
    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    afac_dim1 = *ldafac;
    afac_offset = 1 + afac_dim1 * 1;
    afac -= afac_offset;
    --ipiv;
    --work;

    /* Function Body */
    *resid = 0.f;
    if (*m <= 0 || *n <= 0) {
	return 0;
    }

/*     Determine EPS and the norm of A. */

    eps = slamch_("Epsilon");
    kd = *ku + 1;
    anorm = 0.f;
    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
/* Computing MAX */
	i__2 = kd + 1 - j;
	i1 = max(i__2,1);
/* Computing MIN */
	i__2 = kd + *m - j, i__3 = *kl + kd;
	i2 = min(i__2,i__3);
	if (i2 >= i1) {
/* Computing MAX */
	    i__2 = i2 - i1 + 1;
	    r__1 = anorm, r__2 = scasum_(&i__2, &a_ref(i1, j), &c__1);
	    anorm = dmax(r__1,r__2);
	}
/* L10: */
    }

/*     Compute one column at a time of L*U - A. */

    kd = *kl + *ku + 1;
    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {

/*        Copy the J-th column of U to WORK.   

   Computing MIN */
	i__2 = *kl + *ku, i__3 = j - 1;
	ju = min(i__2,i__3);
/* Computing MIN */
	i__2 = *kl, i__3 = *m - j;
	jl = min(i__2,i__3);
	lenj = min(*m,j) - j + ju + 1;
	if (lenj > 0) {
	    ccopy_(&lenj, &afac_ref(kd - ju, j), &c__1, &work[1], &c__1);
	    i__2 = ju + jl + 1;
	    for (i__ = lenj + 1; i__ <= i__2; ++i__) {
		i__3 = i__;
		work[i__3].r = 0.f, work[i__3].i = 0.f;
/* L20: */
	    }

/*           Multiply by the unit lower triangular matrix L.  Note that L   
             is stored as a product of transformations and permutations.   

   Computing MIN */
	    i__2 = *m - 1;
	    i__3 = j - ju;
	    for (i__ = min(i__2,j); i__ >= i__3; --i__) {
/* Computing MIN */
		i__2 = *kl, i__4 = *m - i__;
		il = min(i__2,i__4);
		if (il > 0) {
		    iw = i__ - j + ju + 1;
		    i__2 = iw;
		    t.r = work[i__2].r, t.i = work[i__2].i;
		    caxpy_(&il, &t, &afac_ref(kd + 1, i__), &c__1, &work[iw + 
			    1], &c__1);
		    ip = ipiv[i__];
		    if (i__ != ip) {
			ip = ip - j + ju + 1;
			i__2 = iw;
			i__4 = ip;
			work[i__2].r = work[i__4].r, work[i__2].i = work[i__4]
				.i;
			i__2 = ip;
			work[i__2].r = t.r, work[i__2].i = t.i;
		    }
		}
/* L30: */
	    }

/*           Subtract the corresponding column of A. */

	    jua = min(ju,*ku);
	    if (jua + jl + 1 > 0) {
		i__3 = jua + jl + 1;
		caxpy_(&i__3, &c_b12, &a_ref(*ku + 1 - jua, j), &c__1, &work[
			ju + 1 - jua], &c__1);
	    }

/*           Compute the 1-norm of the column.   

   Computing MAX */
	    i__3 = ju + jl + 1;
	    r__1 = *resid, r__2 = scasum_(&i__3, &work[1], &c__1);
	    *resid = dmax(r__1,r__2);
	}
/* L40: */
    }

/*     Compute norm( L*U - A ) / ( N * norm(A) * EPS ) */

    if (anorm <= 0.f) {
	if (*resid != 0.f) {
	    *resid = 1.f / eps;
	}
    } else {
	*resid = *resid / (real) (*n) / anorm / eps;
    }

    return 0;

/*     End of CGBT01 */

} /* cgbt01_ */

#undef afac_ref
#undef afac_subscr
#undef a_ref
#undef a_subscr


