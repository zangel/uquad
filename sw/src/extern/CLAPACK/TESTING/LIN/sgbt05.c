#include "blaswrap.h"
/*  -- translated by f2c (version 19990503).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static integer c__1 = 1;

/* Subroutine */ int sgbt05_(char *trans, integer *n, integer *kl, integer *
	ku, integer *nrhs, real *ab, integer *ldab, real *b, integer *ldb, 
	real *x, integer *ldx, real *xact, integer *ldxact, real *ferr, real *
	berr, real *reslts)
{
    /* System generated locals */
    integer ab_dim1, ab_offset, b_dim1, b_offset, x_dim1, x_offset, xact_dim1,
	     xact_offset, i__1, i__2, i__3, i__4, i__5;
    real r__1, r__2, r__3;

    /* Local variables */
    static real diff, axbi;
    static integer imax;
    static real unfl, ovfl;
    static integer i__, j, k;
    extern logical lsame_(char *, char *);
    static real xnorm;
    extern doublereal slamch_(char *);
    static integer nz;
    static real errbnd;
    extern integer isamax_(integer *, real *, integer *);
    static logical notran;
    static real eps, tmp;


#define xact_ref(a_1,a_2) xact[(a_2)*xact_dim1 + a_1]
#define b_ref(a_1,a_2) b[(a_2)*b_dim1 + a_1]
#define x_ref(a_1,a_2) x[(a_2)*x_dim1 + a_1]
#define ab_ref(a_1,a_2) ab[(a_2)*ab_dim1 + a_1]


/*  -- LAPACK test routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       February 29, 1992   


    Purpose   
    =======   

    SGBT05 tests the error bounds from iterative refinement for the   
    computed solution to a system of equations op(A)*X = B, where A is a   
    general band matrix of order n with kl subdiagonals and ku   
    superdiagonals and op(A) = A or A**T, depending on TRANS.   

    RESLTS(1) = test of the error bound   
              = norm(X - XACT) / ( norm(X) * FERR )   

    A large value is returned if this ratio is not less than one.   

    RESLTS(2) = residual from the iterative refinement routine   
              = the maximum of BERR / ( NZ*EPS + (*) ), where   
                (*) = NZ*UNFL / (min_i (abs(op(A))*abs(X) +abs(b))_i )   
                and NZ = max. number of nonzeros in any row of A, plus 1   

    Arguments   
    =========   

    TRANS   (input) CHARACTER*1   
            Specifies the form of the system of equations.   
            = 'N':  A * X = B     (No transpose)   
            = 'T':  A**T * X = B  (Transpose)   
            = 'C':  A**H * X = B  (Conjugate transpose = Transpose)   

    N       (input) INTEGER   
            The number of rows of the matrices X, B, and XACT, and the   
            order of the matrix A.  N >= 0.   

    KL      (input) INTEGER   
            The number of subdiagonals within the band of A.  KL >= 0.   

    KU      (input) INTEGER   
            The number of superdiagonals within the band of A.  KU >= 0.   

    NRHS    (input) INTEGER   
            The number of columns of the matrices X, B, and XACT.   
            NRHS >= 0.   

    AB      (input) REAL array, dimension (LDAB,N)   
            The original band matrix A, stored in rows 1 to KL+KU+1.   
            The j-th column of A is stored in the j-th column of the   
            array AB as follows:   
            AB(ku+1+i-j,j) = A(i,j) for max(1,j-ku)<=i<=min(n,j+kl).   

    LDAB    (input) INTEGER   
            The leading dimension of the array AB.  LDAB >= KL+KU+1.   

    B       (input) REAL array, dimension (LDB,NRHS)   
            The right hand side vectors for the system of linear   
            equations.   

    LDB     (input) INTEGER   
            The leading dimension of the array B.  LDB >= max(1,N).   

    X       (input) REAL array, dimension (LDX,NRHS)   
            The computed solution vectors.  Each vector is stored as a   
            column of the matrix X.   

    LDX     (input) INTEGER   
            The leading dimension of the array X.  LDX >= max(1,N).   

    XACT    (input) REAL array, dimension (LDX,NRHS)   
            The exact solution vectors.  Each vector is stored as a   
            column of the matrix XACT.   

    LDXACT  (input) INTEGER   
            The leading dimension of the array XACT.  LDXACT >= max(1,N).   

    FERR    (input) REAL array, dimension (NRHS)   
            The estimated forward error bounds for each solution vector   
            X.  If XTRUE is the true solution, FERR bounds the magnitude   
            of the largest entry in (X - XTRUE) divided by the magnitude   
            of the largest entry in X.   

    BERR    (input) REAL array, dimension (NRHS)   
            The componentwise relative backward error of each solution   
            vector (i.e., the smallest relative change in any entry of A   
            or B that makes X an exact solution).   

    RESLTS  (output) REAL array, dimension (2)   
            The maximum over the NRHS solution vectors of the ratios:   
            RESLTS(1) = norm(X - XACT) / ( norm(X) * FERR )   
            RESLTS(2) = BERR / ( NZ*EPS + (*) )   

    =====================================================================   


       Quick exit if N = 0 or NRHS = 0.   

       Parameter adjustments */
    ab_dim1 = *ldab;
    ab_offset = 1 + ab_dim1 * 1;
    ab -= ab_offset;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;
    x_dim1 = *ldx;
    x_offset = 1 + x_dim1 * 1;
    x -= x_offset;
    xact_dim1 = *ldxact;
    xact_offset = 1 + xact_dim1 * 1;
    xact -= xact_offset;
    --ferr;
    --berr;
    --reslts;

    /* Function Body */
    if (*n <= 0 || *nrhs <= 0) {
	reslts[1] = 0.f;
	reslts[2] = 0.f;
	return 0;
    }

    eps = slamch_("Epsilon");
    unfl = slamch_("Safe minimum");
    ovfl = 1.f / unfl;
    notran = lsame_(trans, "N");
/* Computing MIN */
    i__1 = *kl + *ku + 2, i__2 = *n + 1;
    nz = min(i__1,i__2);

/*     Test 1:  Compute the maximum of   
          norm(X - XACT) / ( norm(X) * FERR )   
       over all the vectors X and XACT using the infinity-norm. */

    errbnd = 0.f;
    i__1 = *nrhs;
    for (j = 1; j <= i__1; ++j) {
	imax = isamax_(n, &x_ref(1, j), &c__1);
/* Computing MAX */
	r__2 = (r__1 = x_ref(imax, j), dabs(r__1));
	xnorm = dmax(r__2,unfl);
	diff = 0.f;
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
/* Computing MAX */
	    r__2 = diff, r__3 = (r__1 = x_ref(i__, j) - xact_ref(i__, j), 
		    dabs(r__1));
	    diff = dmax(r__2,r__3);
/* L10: */
	}

	if (xnorm > 1.f) {
	    goto L20;
	} else if (diff <= ovfl * xnorm) {
	    goto L20;
	} else {
	    errbnd = 1.f / eps;
	    goto L30;
	}

L20:
	if (diff / xnorm <= ferr[j]) {
/* Computing MAX */
	    r__1 = errbnd, r__2 = diff / xnorm / ferr[j];
	    errbnd = dmax(r__1,r__2);
	} else {
	    errbnd = 1.f / eps;
	}
L30:
	;
    }
    reslts[1] = errbnd;

/*     Test 2:  Compute the maximum of BERR / ( NZ*EPS + (*) ), where   
       (*) = NZ*UNFL / (min_i (abs(op(A))*abs(X) +abs(b))_i ) */

    i__1 = *nrhs;
    for (k = 1; k <= i__1; ++k) {
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    tmp = (r__1 = b_ref(i__, k), dabs(r__1));
	    if (notran) {
/* Computing MAX */
		i__3 = i__ - *kl;
/* Computing MIN */
		i__5 = i__ + *ku;
		i__4 = min(i__5,*n);
		for (j = max(i__3,1); j <= i__4; ++j) {
		    tmp += (r__1 = ab_ref(*ku + 1 + i__ - j, j), dabs(r__1)) *
			     (r__2 = x_ref(j, k), dabs(r__2));
/* L40: */
		}
	    } else {
/* Computing MAX */
		i__4 = i__ - *ku;
/* Computing MIN */
		i__5 = i__ + *kl;
		i__3 = min(i__5,*n);
		for (j = max(i__4,1); j <= i__3; ++j) {
		    tmp += (r__1 = ab_ref(*ku + 1 + j - i__, i__), dabs(r__1))
			     * (r__2 = x_ref(j, k), dabs(r__2));
/* L50: */
		}
	    }
	    if (i__ == 1) {
		axbi = tmp;
	    } else {
		axbi = dmin(axbi,tmp);
	    }
/* L60: */
	}
/* Computing MAX */
	r__1 = axbi, r__2 = nz * unfl;
	tmp = berr[k] / (nz * eps + nz * unfl / dmax(r__1,r__2));
	if (k == 1) {
	    reslts[2] = tmp;
	} else {
	    reslts[2] = dmax(reslts[2],tmp);
	}
/* L70: */
    }

    return 0;

/*     End of SGBT05 */

} /* sgbt05_ */

#undef ab_ref
#undef x_ref
#undef b_ref
#undef xact_ref


