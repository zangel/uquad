#include "blaswrap.h"
/*  -- translated by f2c (version 19990503).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static integer c__1 = 1;

/* Subroutine */ int ztrt01_(char *uplo, char *diag, integer *n, 
	doublecomplex *a, integer *lda, doublecomplex *ainv, integer *ldainv, 
	doublereal *rcond, doublereal *rwork, doublereal *resid)
{
    /* System generated locals */
    integer a_dim1, a_offset, ainv_dim1, ainv_offset, i__1, i__2, i__3;
    doublecomplex z__1;

    /* Local variables */
    static integer j;
    extern logical lsame_(char *, char *);
    static doublereal anorm;
    extern /* Subroutine */ int ztrmv_(char *, char *, char *, integer *, 
	    doublecomplex *, integer *, doublecomplex *, integer *);
    extern doublereal dlamch_(char *);
    static doublereal ainvnm;
    extern doublereal zlantr_(char *, char *, char *, integer *, integer *, 
	    doublecomplex *, integer *, doublereal *);
    static doublereal eps;


#define ainv_subscr(a_1,a_2) (a_2)*ainv_dim1 + a_1
#define ainv_ref(a_1,a_2) ainv[ainv_subscr(a_1,a_2)]
#define a_subscr(a_1,a_2) (a_2)*a_dim1 + a_1
#define a_ref(a_1,a_2) a[a_subscr(a_1,a_2)]


/*  -- LAPACK test routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    ZTRT01 computes the residual for a triangular matrix A times its   
    inverse:   
       RESID = norm( A*AINV - I ) / ( N * norm(A) * norm(AINV) * EPS ),   
    where EPS is the machine epsilon.   

    Arguments   
    ==========   

    UPLO    (input) CHARACTER*1   
            Specifies whether the matrix A is upper or lower triangular.   
            = 'U':  Upper triangular   
            = 'L':  Lower triangular   

    DIAG    (input) CHARACTER*1   
            Specifies whether or not the matrix A is unit triangular.   
            = 'N':  Non-unit triangular   
            = 'U':  Unit triangular   

    N       (input) INTEGER   
            The order of the matrix A.  N >= 0.   

    A       (input) COMPLEX*16 array, dimension (LDA,N)   
            The triangular matrix A.  If UPLO = 'U', the leading n by n   
            upper triangular part of the array A contains the upper   
            triangular matrix, and the strictly lower triangular part of   
            A is not referenced.  If UPLO = 'L', the leading n by n lower   
            triangular part of the array A contains the lower triangular   
            matrix, and the strictly upper triangular part of A is not   
            referenced.  If DIAG = 'U', the diagonal elements of A are   
            also not referenced and are assumed to be 1.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,N).   

    AINV    (input) COMPLEX*16 array, dimension (LDAINV,N)   
            On entry, the (triangular) inverse of the matrix A, in the   
            same storage format as A.   
            On exit, the contents of AINV are destroyed.   

    LDAINV  (input) INTEGER   
            The leading dimension of the array AINV.  LDAINV >= max(1,N).   

    RCOND   (output) DOUBLE PRECISION   
            The reciprocal condition number of A, computed as   
            1/(norm(A) * norm(AINV)).   

    RWORK   (workspace) DOUBLE PRECISION array, dimension (N)   

    RESID   (output) DOUBLE PRECISION   
            norm(A*AINV - I) / ( N * norm(A) * norm(AINV) * EPS )   

    =====================================================================   


       Quick exit if N = 0   

       Parameter adjustments */
    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    ainv_dim1 = *ldainv;
    ainv_offset = 1 + ainv_dim1 * 1;
    ainv -= ainv_offset;
    --rwork;

    /* Function Body */
    if (*n <= 0) {
	*rcond = 1.;
	*resid = 0.;
	return 0;
    }

/*     Exit with RESID = 1/EPS if ANORM = 0 or AINVNM = 0. */

    eps = dlamch_("Epsilon");
    anorm = zlantr_("1", uplo, diag, n, n, &a[a_offset], lda, &rwork[1]);
    ainvnm = zlantr_("1", uplo, diag, n, n, &ainv[ainv_offset], ldainv, &
	    rwork[1]);
    if (anorm <= 0. || ainvnm <= 0.) {
	*rcond = 0.;
	*resid = 1. / eps;
	return 0;
    }
    *rcond = 1. / anorm / ainvnm;

/*     Set the diagonal of AINV to 1 if AINV has unit diagonal. */

    if (lsame_(diag, "U")) {
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = ainv_subscr(j, j);
	    ainv[i__2].r = 1., ainv[i__2].i = 0.;
/* L10: */
	}
    }

/*     Compute A * AINV, overwriting AINV. */

    if (lsame_(uplo, "U")) {
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    ztrmv_("Upper", "No transpose", diag, &j, &a[a_offset], lda, &
		    ainv_ref(1, j), &c__1);
/* L20: */
	}
    } else {
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = *n - j + 1;
	    ztrmv_("Lower", "No transpose", diag, &i__2, &a_ref(j, j), lda, &
		    ainv_ref(j, j), &c__1);
/* L30: */
	}
    }

/*     Subtract 1 from each diagonal element to form A*AINV - I. */

    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
	i__2 = ainv_subscr(j, j);
	i__3 = ainv_subscr(j, j);
	z__1.r = ainv[i__3].r - 1., z__1.i = ainv[i__3].i;
	ainv[i__2].r = z__1.r, ainv[i__2].i = z__1.i;
/* L40: */
    }

/*     Compute norm(A*AINV - I) / (N * norm(A) * norm(AINV) * EPS) */

    *resid = zlantr_("1", uplo, "Non-unit", n, n, &ainv[ainv_offset], ldainv, 
	    &rwork[1]);

    *resid = *resid * *rcond / (doublereal) (*n) / eps;

    return 0;

/*     End of ZTRT01 */

} /* ztrt01_ */

#undef a_ref
#undef a_subscr
#undef ainv_ref
#undef ainv_subscr


