#include "blaswrap.h"
/*  -- translated by f2c (version 19990503).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct {
    real ops, itcnt;
} latime_;

#define latime_1 latime_

struct {
    real opcnt[6], timng[6];
} lstime_;

#define lstime_1 lstime_

/* Table of constant values */

static integer c__0 = 0;
static real c_b13 = 0.f;
static integer c__2 = 2;
static integer c__1 = 1;
static real c_b49 = 1.f;

/* Subroutine */ int sgelsx_(integer *m, integer *n, integer *nrhs, real *a, 
	integer *lda, real *b, integer *ldb, integer *jpvt, real *rcond, 
	integer *rank, real *work, integer *info)
{
    /* Initialized data */

    static integer gelsx = 1;
    static integer geqpf = 2;
    static integer latzm = 6;
    static integer orm2r = 4;
    static integer trsm = 5;
    static integer tzrqf = 3;

    /* System generated locals */
    integer a_dim1, a_offset, b_dim1, b_offset, i__1, i__2;
    real r__1;

    /* Local variables */
    static real anrm, bnrm, smin, smax;
    static integer i__, j, k, iascl, ibscl, ismin;
    extern doublereal sopla_(char *, integer *, integer *, integer *, integer 
	    *, integer *);
    static integer ismax;
    static real c1, c2, s1, s2, t1, t2;
    extern /* Subroutine */ int strsm_(char *, char *, char *, char *, 
	    integer *, integer *, real *, real *, integer *, real *, integer *
	    ), slaic1_(integer *, integer *, 
	    real *, real *, real *, real *, real *, real *, real *);
    extern doublereal sopbl3_(char *, integer *, integer *, integer *)
	    ;
    extern /* Subroutine */ int sorm2r_(char *, char *, integer *, integer *, 
	    integer *, real *, integer *, real *, real *, integer *, real *, 
	    integer *), slabad_(real *, real *);
    static integer mn;
    extern doublereal slamch_(char *), slange_(char *, integer *, 
	    integer *, real *, integer *, real *), second_(void);
    extern /* Subroutine */ int xerbla_(char *, integer *);
    static real bignum;
    extern /* Subroutine */ int slascl_(char *, integer *, integer *, real *, 
	    real *, integer *, integer *, real *, integer *, integer *), sgeqpf_(integer *, integer *, real *, integer *, integer 
	    *, real *, real *, integer *), slaset_(char *, integer *, integer 
	    *, real *, real *, real *, integer *);
    static real sminpr, smaxpr, smlnum;
    extern /* Subroutine */ int slatzm_(char *, integer *, integer *, real *, 
	    integer *, real *, real *, real *, integer *, real *), 
	    stzrqf_(integer *, integer *, real *, integer *, real *, integer *
	    );
    static real tim1, tim2;


#define a_ref(a_1,a_2) a[(a_2)*a_dim1 + a_1]
#define b_ref(a_1,a_2) b[(a_2)*b_dim1 + a_1]


/*  -- LAPACK driver routine (instrumented to count ops, version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       September 30, 1994   

       Common blocks to return operation counts and timings   

    Purpose   
    =======   

    SGELSX computes the minimum-norm solution to a real linear least   
    squares problem:   
        minimize || A * X - B ||   
    using a complete orthogonal factorization of A.  A is an M-by-N   
    matrix which may be rank-deficient.   

    Several right hand side vectors b and solution vectors x can be   
    handled in a single call; they are stored as the columns of the   
    M-by-NRHS right hand side matrix B and the N-by-NRHS solution   
    matrix X.   

    The routine first computes a QR factorization with column pivoting:   
        A * P = Q * [ R11 R12 ]   
                    [  0  R22 ]   
    with R11 defined as the largest leading submatrix whose estimated   
    condition number is less than 1/RCOND.  The order of R11, RANK,   
    is the effective rank of A.   

    Then, R22 is considered to be negligible, and R12 is annihilated   
    by orthogonal transformations from the right, arriving at the   
    complete orthogonal factorization:   
       A * P = Q * [ T11 0 ] * Z   
                   [  0  0 ]   
    The minimum-norm solution is then   
       X = P * Z' [ inv(T11)*Q1'*B ]   
                  [        0       ]   
    where Q1 consists of the first RANK columns of Q.   

    Arguments   
    =========   

    M       (input) INTEGER   
            The number of rows of the matrix A.  M >= 0.   

    N       (input) INTEGER   
            The number of columns of the matrix A.  N >= 0.   

    NRHS    (input) INTEGER   
            The number of right hand sides, i.e., the number of   
            columns of matrices B and X. NRHS >= 0.   

    A       (input/output) REAL array, dimension (LDA,N)   
            On entry, the M-by-N matrix A.   
            On exit, A has been overwritten by details of its   
            complete orthogonal factorization.   

    LDA     (input) INTEGER   
            The leading dimension of the array A.  LDA >= max(1,M).   

    B       (input/output) REAL array, dimension (LDB,NRHS)   
            On entry, the M-by-NRHS right hand side matrix B.   
            On exit, the N-by-NRHS solution matrix X.   
            If m >= n and RANK = n, the residual sum-of-squares for   
            the solution in the i-th column is given by the sum of   
            squares of elements N+1:M in that column.   

    LDB     (input) INTEGER   
            The leading dimension of the array B. LDB >= max(1,M,N).   

    JPVT    (input/output) INTEGER array, dimension (N)   
            On entry, if JPVT(i) .ne. 0, the i-th column of A is an   
            initial column, otherwise it is a free column.  Before   
            the QR factorization of A, all initial columns are   
            permuted to the leading positions; only the remaining   
            free columns are moved as a result of column pivoting   
            during the factorization.   
            On exit, if JPVT(i) = k, then the i-th column of A*P   
            was the k-th column of A.   

    RCOND   (input) REAL   
            RCOND is used to determine the effective rank of A, which   
            is defined as the order of the largest leading triangular   
            submatrix R11 in the QR factorization with pivoting of A,   
            whose estimated condition number < 1/RCOND.   

    RANK    (output) INTEGER   
            The effective rank of A, i.e., the order of the submatrix   
            R11.  This is the same as the order of the submatrix T11   
            in the complete orthogonal factorization of A.   

    WORK    (workspace) REAL array, dimension   
                        (max( min(M,N)+3*N, 2*min(M,N)+NRHS )),   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value   

    =====================================================================   

       Parameter adjustments */
    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;
    --jpvt;
    --work;

    /* Function Body */

    mn = min(*m,*n);
    ismin = mn + 1;
    ismax = (mn << 1) + 1;

/*     Test the input arguments. */

    *info = 0;
    if (*m < 0) {
	*info = -1;
    } else if (*n < 0) {
	*info = -2;
    } else if (*nrhs < 0) {
	*info = -3;
    } else if (*lda < max(1,*m)) {
	*info = -5;
    } else /* if(complicated condition) */ {
/* Computing MAX */
	i__1 = max(1,*m);
	if (*ldb < max(i__1,*n)) {
	    *info = -7;
	}
    }

    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("SGELSX", &i__1);
	return 0;
    }

/*     Quick return if possible   

   Computing MIN */
    i__1 = min(*m,*n);
    if (min(i__1,*nrhs) == 0) {
	*rank = 0;
	return 0;
    }

/*     Get machine parameters */

    lstime_1.opcnt[gelsx - 1] += 2.f;
    smlnum = slamch_("S") / slamch_("P");
    bignum = 1.f / smlnum;
    slabad_(&smlnum, &bignum);

/*     Scale A, B if max elements outside range [SMLNUM,BIGNUM] */

    anrm = slange_("M", m, n, &a[a_offset], lda, &work[1]);
    iascl = 0;
    if (anrm > 0.f && anrm < smlnum) {

/*        Scale matrix norm up to SMLNUM */

	lstime_1.opcnt[gelsx - 1] += (real) (*m * *n);
	slascl_("G", &c__0, &c__0, &anrm, &smlnum, m, n, &a[a_offset], lda, 
		info);
	iascl = 1;
    } else if (anrm > bignum) {

/*        Scale matrix norm down to BIGNUM */

	lstime_1.opcnt[gelsx - 1] += (real) (*m * *n);
	slascl_("G", &c__0, &c__0, &anrm, &bignum, m, n, &a[a_offset], lda, 
		info);
	iascl = 2;
    } else if (anrm == 0.f) {

/*        Matrix all zero. Return zero solution. */

	i__1 = max(*m,*n);
	slaset_("F", &i__1, nrhs, &c_b13, &c_b13, &b[b_offset], ldb);
	*rank = 0;
	goto L100;
    }

    bnrm = slange_("M", m, nrhs, &b[b_offset], ldb, &work[1]);
    ibscl = 0;
    if (bnrm > 0.f && bnrm < smlnum) {

/*        Scale matrix norm up to SMLNUM */

	lstime_1.opcnt[gelsx - 1] += (real) (*m * *nrhs);
	slascl_("G", &c__0, &c__0, &bnrm, &smlnum, m, nrhs, &b[b_offset], ldb,
		 info);
	ibscl = 1;
    } else if (bnrm > bignum) {

/*        Scale matrix norm down to BIGNUM */

	lstime_1.opcnt[gelsx - 1] += (real) (*m * *nrhs);
	slascl_("G", &c__0, &c__0, &bnrm, &bignum, m, nrhs, &b[b_offset], ldb,
		 info);
	ibscl = 2;
    }

/*     Compute QR factorization with column pivoting of A:   
          A * P = Q * R */

    lstime_1.opcnt[geqpf - 1] += sopla_("SGEQPF", m, n, &c__0, &c__0, &c__0);
    tim1 = second_();
    sgeqpf_(m, n, &a[a_offset], lda, &jpvt[1], &work[1], &work[mn + 1], info);
    tim2 = second_();
    lstime_1.timng[geqpf - 1] += tim2 - tim1;

/*     workspace 3*N. Details of Householder rotations stored   
       in WORK(1:MN).   

       Determine RANK using incremental condition estimation */

    work[ismin] = 1.f;
    work[ismax] = 1.f;
    smax = (r__1 = a_ref(1, 1), dabs(r__1));
    smin = smax;
    if ((r__1 = a_ref(1, 1), dabs(r__1)) == 0.f) {
	*rank = 0;
	i__1 = max(*m,*n);
	slaset_("F", &i__1, nrhs, &c_b13, &c_b13, &b[b_offset], ldb);
	goto L100;
    } else {
	*rank = 1;
    }

L10:
    if (*rank < mn) {
	i__ = *rank + 1;
	latime_1.ops = 0.f;
	slaic1_(&c__2, rank, &work[ismin], &smin, &a_ref(1, i__), &a_ref(i__, 
		i__), &sminpr, &s1, &c1);
	slaic1_(&c__1, rank, &work[ismax], &smax, &a_ref(1, i__), &a_ref(i__, 
		i__), &smaxpr, &s2, &c2);
	lstime_1.opcnt[gelsx - 1] = lstime_1.opcnt[gelsx - 1] + latime_1.ops 
		+ 1.f;

	if (smaxpr * *rcond <= sminpr) {
	    lstime_1.opcnt[gelsx - 1] += (real) (*rank << 1);
	    i__1 = *rank;
	    for (i__ = 1; i__ <= i__1; ++i__) {
		work[ismin + i__ - 1] = s1 * work[ismin + i__ - 1];
		work[ismax + i__ - 1] = s2 * work[ismax + i__ - 1];
/* L20: */
	    }
	    work[ismin + *rank] = c1;
	    work[ismax + *rank] = c2;
	    smin = sminpr;
	    smax = smaxpr;
	    ++(*rank);
	    goto L10;
	}
    }

/*     Logically partition R = [ R11 R12 ]   
                               [  0  R22 ]   
       where R11 = R(1:RANK,1:RANK)   

       [R11,R12] = [ T11, 0 ] * Y */

    if (*rank < *n) {
	lstime_1.opcnt[tzrqf - 1] += sopla_("STZRQF", rank, n, &c__0, &c__0, &
		c__0);
	tim1 = second_();
	stzrqf_(rank, n, &a[a_offset], lda, &work[mn + 1], info);
	tim2 = second_();
	lstime_1.timng[tzrqf - 1] += tim2 - tim1;
    }

/*     Details of Householder rotations stored in WORK(MN+1:2*MN)   

       B(1:M,1:NRHS) := Q' * B(1:M,1:NRHS) */

    lstime_1.opcnt[orm2r - 1] += sopla_("SORMQR", m, nrhs, &mn, &c__0, &c__0);
    tim1 = second_();
    sorm2r_("Left", "Transpose", m, nrhs, &mn, &a[a_offset], lda, &work[1], &
	    b[b_offset], ldb, &work[(mn << 1) + 1], info);
    tim2 = second_();
    lstime_1.timng[orm2r - 1] += tim2 - tim1;

/*     workspace NRHS   

       B(1:RANK,1:NRHS) := inv(T11) * B(1:RANK,1:NRHS) */

    lstime_1.opcnt[trsm - 1] += sopbl3_("STRSM ", rank, nrhs, &c__0);
    tim1 = second_();
    strsm_("Left", "Upper", "No transpose", "Non-unit", rank, nrhs, &c_b49, &
	    a[a_offset], lda, &b[b_offset], ldb);
    tim2 = second_();
    lstime_1.timng[trsm - 1] += tim2 - tim1;

    i__1 = *n;
    for (i__ = *rank + 1; i__ <= i__1; ++i__) {
	i__2 = *nrhs;
	for (j = 1; j <= i__2; ++j) {
	    b_ref(i__, j) = 0.f;
/* L30: */
	}
/* L40: */
    }

/*     B(1:N,1:NRHS) := Y' * B(1:N,1:NRHS) */

    if (*rank < *n) {
	lstime_1.opcnt[latzm - 1] += (real) (((*n - *rank) * *nrhs + *nrhs + (
		*n - *rank) * *nrhs << 1) * *rank);
	tim1 = second_();
	i__1 = *rank;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    i__2 = *n - *rank + 1;
	    slatzm_("Left", &i__2, nrhs, &a_ref(i__, *rank + 1), lda, &work[
		    mn + i__], &b_ref(i__, 1), &b_ref(*rank + 1, 1), ldb, &
		    work[(mn << 1) + 1]);
/* L50: */
	}
	tim2 = second_();
	lstime_1.timng[latzm - 1] += tim2 - tim1;
    }

/*     workspace NRHS   

       B(1:N,1:NRHS) := P * B(1:N,1:NRHS) */

    i__1 = *nrhs;
    for (j = 1; j <= i__1; ++j) {
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    work[(mn << 1) + i__] = 1.f;
/* L60: */
	}
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    if (work[(mn << 1) + i__] == 1.f) {
		if (jpvt[i__] != i__) {
		    k = i__;
		    t1 = b_ref(k, j);
		    t2 = b_ref(jpvt[k], j);
L70:
		    b_ref(jpvt[k], j) = t1;
		    work[(mn << 1) + k] = 0.f;
		    t1 = t2;
		    k = jpvt[k];
		    t2 = b_ref(jpvt[k], j);
		    if (jpvt[k] != i__) {
			goto L70;
		    }
		    b_ref(i__, j) = t1;
		    work[(mn << 1) + k] = 0.f;
		}
	    }
/* L80: */
	}
/* L90: */
    }

/*     Undo scaling */

    if (iascl == 1) {
	lstime_1.opcnt[gelsx - 1] += (real) (*n * *nrhs + *rank * *rank);
	slascl_("G", &c__0, &c__0, &anrm, &smlnum, n, nrhs, &b[b_offset], ldb,
		 info);
	slascl_("U", &c__0, &c__0, &smlnum, &anrm, rank, rank, &a[a_offset], 
		lda, info);
    } else if (iascl == 2) {
	lstime_1.opcnt[gelsx - 1] += (real) (*n * *nrhs + *rank * *rank);
	slascl_("G", &c__0, &c__0, &anrm, &bignum, n, nrhs, &b[b_offset], ldb,
		 info);
	slascl_("U", &c__0, &c__0, &bignum, &anrm, rank, rank, &a[a_offset], 
		lda, info);
    }
    if (ibscl == 1) {
	lstime_1.opcnt[gelsx - 1] += (real) (*n * *nrhs);
	slascl_("G", &c__0, &c__0, &smlnum, &bnrm, n, nrhs, &b[b_offset], ldb,
		 info);
    } else if (ibscl == 2) {
	lstime_1.opcnt[gelsx - 1] += (real) (*n * *nrhs);
	slascl_("G", &c__0, &c__0, &bignum, &bnrm, n, nrhs, &b[b_offset], ldb,
		 info);
    }

L100:

    return 0;

/*     End of SGELSX */

} /* sgelsx_ */

#undef b_ref
#undef a_ref


