#include "blaswrap.h"
/*  -- translated by f2c (version 19990503).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static doublecomplex c_b1 = {1.,0.};

/* Subroutine */ int zgeqrs_(integer *m, integer *n, integer *nrhs, 
	doublecomplex *a, integer *lda, doublecomplex *tau, doublecomplex *b, 
	integer *ldb, doublecomplex *work, integer *lwork, integer *info)
{
    /* System generated locals */
    integer a_dim1, a_offset, b_dim1, b_offset, i__1;

    /* Local variables */
    extern /* Subroutine */ int ztrsm_(char *, char *, char *, char *, 
	    integer *, integer *, doublecomplex *, doublecomplex *, integer *,
	     doublecomplex *, integer *), 
	    xerbla_(char *, integer *), zunmqr_(char *, char *, 
	    integer *, integer *, integer *, doublecomplex *, integer *, 
	    doublecomplex *, doublecomplex *, integer *, doublecomplex *, 
	    integer *, integer *);


/*  -- LAPACK routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   


    Purpose   
    =======   

    Solve the least squares problem   
        min || A*X - B ||   
    using the QR factorization   
        A = Q*R   
    computed by ZGEQRF.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  M >= N >= 0.   

    NRHS    (input) INTEGER   
            The number of columns of B.  NRHS >= 0.   

    A       (input) COMPLEX*16 array, dimension (LDA,N)   
            Details of the QR factorization of the original matrix A as   
            returned by ZGEQRF.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= M.   

    TAU     (input) COMPLEX*16 array, dimension (N)   
            Details of the orthogonal matrix Q.   

    B       (input/output) COMPLEX*16 array, dimension (LDB,NRHS)   
            On entry, the m-by-nrhs right hand side matrix B.   
            On exit, the n-by-nrhs solution matrix X.   

    LDB     (input) INTEGER   
            The leading dimension of the array B. LDB >= M.   

    WORK    (workspace) COMPLEX*16 array, dimension (LWORK)   

    LWORK   (input) INTEGER   
            The length of the array WORK.  LWORK must be at least NRHS,   
            and should be at least NRHS*NB, where NB is the block size   
            for this environment.   

    INFO    (output) INTEGER   
            = 0: successful exit   
            < 0: if INFO = -i, the i-th argument had an illegal value   

    =====================================================================   


       Test the input arguments.   

       Parameter adjustments */
    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    --tau;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;
    --work;

    /* Function Body */
    *info = 0;
    if (*m < 0) {
	*info = -1;
    } else if (*n < 0 || *n > *m) {
	*info = -2;
    } else if (*nrhs < 0) {
	*info = -3;
    } else if (*lda < max(1,*m)) {
	*info = -5;
    } else if (*ldb < max(1,*m)) {
	*info = -8;
    } else if (*lwork < 1 || *lwork < *nrhs && *m > 0 && *n > 0) {
	*info = -10;
    }
    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("ZGEQRS", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0 || *nrhs == 0 || *m == 0) {
	return 0;
    }

/*     B := Q' * B */

    zunmqr_("Left", "Conjugate transpose", m, nrhs, n, &a[a_offset], lda, &
	    tau[1], &b[b_offset], ldb, &work[1], lwork, info);

/*     Solve R*X = B(1:n,:) */

    ztrsm_("Left", "Upper", "No transpose", "Non-unit", n, nrhs, &c_b1, &a[
	    a_offset], lda, &b[b_offset], ldb);

    return 0;

/*     End of ZGEQRS */

} /* zgeqrs_ */

