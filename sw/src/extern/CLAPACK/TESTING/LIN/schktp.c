#include "blaswrap.h"
/*  -- translated by f2c (version 19990503).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Common Block Declarations */

struct {
    integer infot, iounit;
    logical ok, lerr;
} infoc_;

#define infoc_1 infoc_

struct {
    char srnamt[6];
} srnamc_;

#define srnamc_1 srnamc_

/* Table of constant values */

static integer c__1 = 1;
static integer c__0 = 0;
static integer c_n1 = -1;
static integer c__2 = 2;
static integer c__3 = 3;
static integer c__7 = 7;
static integer c__4 = 4;
static real c_b103 = 1.f;
static integer c__8 = 8;
static integer c__9 = 9;

/* Subroutine */ int schktp_(logical *dotype, integer *nn, integer *nval, 
	integer *nns, integer *nsval, real *thresh, logical *tsterr, integer *
	nmax, real *ap, real *ainvp, real *b, real *x, real *xact, real *work,
	 real *rwork, integer *iwork, integer *nout)
{
    /* Initialized data */

    static integer iseedy[4] = { 1988,1989,1990,1991 };
    static char uplos[1*2] = "U" "L";
    static char transs[1*3] = "N" "T" "C";

    /* Format strings */
    static char fmt_9999[] = "(\002 UPLO='\002,a1,\002', DIAG='\002,a1,\002'"
	    ", N=\002,i5,\002, type \002,i2,\002, test(\002,i2,\002)= \002,g1"
	    "2.5)";
    static char fmt_9998[] = "(\002 UPLO='\002,a1,\002', TRANS='\002,a1,\002"
	    "', DIAG='\002,a1,\002', N=\002,i5,\002', NRHS=\002,i5,\002, type "
	    "\002,i2,\002, test(\002,i2,\002)= \002,g12.5)";
    static char fmt_9997[] = "(1x,a6,\002( '\002,a1,\002', '\002,a1,\002', "
	    "'\002,a1,\002',\002,i5,\002, ... ), type \002,i2,\002, test(\002"
	    ",i2,\002)=\002,g12.5)";
    static char fmt_9996[] = "(1x,a6,\002( '\002,a1,\002', '\002,a1,\002', "
	    "'\002,a1,\002', '\002,a1,\002',\002,i5,\002, ... ), type \002,i2,"
	    "\002, test(\002,i2,\002)=\002,g12.5)";

    /* System generated locals */
    address a__1[2], a__2[3], a__3[4];
    integer i__1, i__2[2], i__3, i__4[3], i__5[4];
    char ch__1[2], ch__2[3], ch__3[4];

    /* Builtin functions   
       Subroutine */ int s_copy(char *, char *, ftnlen, ftnlen), s_cat(char *,
	     char **, integer *, integer *, ftnlen);
    integer s_wsfe(cilist *), do_fio(integer *, char *, ftnlen), e_wsfe(void);

    /* Local variables */
    static char diag[1];
    static integer imat, info;
    static char path[3];
    static integer irhs, nrhs;
    static char norm[1], uplo[1];
    static integer nrun, i__;
    extern /* Subroutine */ int alahd_(integer *, char *);
    static integer k, idiag, n;
    static real scale;
    static integer nfail, iseed[4];
    extern logical lsame_(char *, char *);
    static real rcond;
    extern /* Subroutine */ int sget04_(integer *, integer *, real *, integer 
	    *, real *, integer *, real *, real *);
    static real anorm;
    static integer itran;
    static char trans[1];
    static integer iuplo, nerrs;
    extern /* Subroutine */ int stpt01_(char *, char *, integer *, real *, 
	    real *, real *, real *, real *), scopy_(integer *,
	     real *, integer *, real *, integer *), stpt02_(char *, char *, 
	    char *, integer *, integer *, real *, real *, integer *, real *, 
	    integer *, real *, real *), stpt03_(char *
	    , char *, char *, integer *, integer *, real *, real *, real *, 
	    real *, real *, integer *, real *, integer *, real *, real *), stpt05_(char *, char *, char *, integer *
	    , integer *, real *, real *, integer *, real *, integer *, real *,
	     integer *, real *, real *, real *), 
	    stpt06_(real *, real *, char *, char *, integer *, real *, real *,
	     real *);
    static char xtype[1];
    static integer in;
    extern /* Subroutine */ int alaerh_(char *, char *, integer *, integer *, 
	    char *, integer *, integer *, integer *, integer *, integer *, 
	    integer *, integer *, integer *, integer *);
    static real rcondc, rcondi;
    extern /* Subroutine */ int alasum_(char *, integer *, integer *, integer 
	    *, integer *);
    static real rcondo, ainvnm;
    extern /* Subroutine */ int slacpy_(char *, integer *, integer *, real *, 
	    integer *, real *, integer *), slarhs_(char *, char *, 
	    char *, char *, integer *, integer *, integer *, integer *, 
	    integer *, real *, integer *, real *, integer *, real *, integer *
	    , integer *, integer *);
    extern doublereal slantp_(char *, char *, char *, integer *, real *, real 
	    *);
    extern /* Subroutine */ int slatps_(char *, char *, char *, char *, 
	    integer *, real *, real *, real *, real *, integer *), slattp_(integer *, char *, char *, char *
	    , integer *, integer *, real *, real *, real *, integer *), stpcon_(char *, char *, char *, integer *, real 
	    *, real *, real *, integer *, integer *);
    static real result[9];
    extern /* Subroutine */ int serrtr_(char *, integer *), stprfs_(
	    char *, char *, char *, integer *, integer *, real *, real *, 
	    integer *, real *, integer *, real *, real *, real *, integer *, 
	    integer *), stptri_(char *, char *, 
	    integer *, real *, integer *), stptrs_(char *, 
	    char *, char *, integer *, integer *, real *, real *, integer *, 
	    integer *);
    static integer lda, lap;

    /* Fortran I/O blocks */
    static cilist io___26 = { 0, 0, 0, fmt_9999, 0 };
    static cilist io___34 = { 0, 0, 0, fmt_9998, 0 };
    static cilist io___36 = { 0, 0, 0, fmt_9997, 0 };
    static cilist io___38 = { 0, 0, 0, fmt_9996, 0 };
    static cilist io___39 = { 0, 0, 0, fmt_9996, 0 };



/*  -- LAPACK test routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       December 3, 1999   


    Purpose   
    =======   

    SCHKTP tests STPTRI, -TRS, -RFS, and -CON, and SLATPS   

    Arguments   
    =========   

    DOTYPE  (input) LOGICAL array, dimension (NTYPES)   
            The matrix types to be used for testing.  Matrices of type j   
            (for 1 <= j <= NTYPES) are used for testing if DOTYPE(j) =   
            .TRUE.; if DOTYPE(j) = .FALSE., then type j is not used.   

    NN      (input) INTEGER   
            The number of values of N contained in the vector NVAL.   

    NVAL    (input) INTEGER array, dimension (NN)   
            The values of the matrix column dimension N.   

    NNS     (input) INTEGER   
            The number of values of NRHS contained in the vector NSVAL.   

    NSVAL   (input) INTEGER array, dimension (NNS)   
            The values of the number of right hand sides NRHS.   

    THRESH  (input) REAL   
            The threshold value for the test ratios.  A result is   
            included in the output file if RESULT >= THRESH.  To have   
            every test ratio printed, use THRESH = 0.   

    TSTERR  (input) LOGICAL   
            Flag that indicates whether error exits are to be tested.   

    NMAX    (input) INTEGER   
            The leading dimension of the work arrays.  NMAX >= the   
            maximumm value of N in NVAL.   

    AP      (workspace) REAL array, dimension   
                        (NMAX*(NMAX+1)/2)   

    AINVP   (workspace) REAL array, dimension   
                        (NMAX*(NMAX+1)/2)   

    B       (workspace) REAL array, dimension (NMAX*NSMAX)   
            where NSMAX is the largest entry in NSVAL.   

    X       (workspace) REAL array, dimension (NMAX*NSMAX)   

    XACT    (workspace) REAL array, dimension (NMAX*NSMAX)   

    WORK    (workspace) REAL array, dimension   
                        (NMAX*max(3,NSMAX))   

    IWORK   (workspace) INTEGER array, dimension (NMAX)   

    RWORK   (workspace) REAL array, dimension   
                        (max(NMAX,2*NSMAX))   

    NOUT    (input) INTEGER   
            The unit number for output.   

    =====================================================================   

       Parameter adjustments */
    --iwork;
    --rwork;
    --work;
    --xact;
    --x;
    --b;
    --ainvp;
    --ap;
    --nsval;
    --nval;
    --dotype;

    /* Function Body   

       Initialize constants and the random number seed. */

    s_copy(path, "Single precision", (ftnlen)1, (ftnlen)16);
    s_copy(path + 1, "TP", (ftnlen)2, (ftnlen)2);
    nrun = 0;
    nfail = 0;
    nerrs = 0;
    for (i__ = 1; i__ <= 4; ++i__) {
	iseed[i__ - 1] = iseedy[i__ - 1];
/* L10: */
    }

/*     Test the error exits */

    if (*tsterr) {
	serrtr_(path, nout);
    }
    infoc_1.infot = 0;

    i__1 = *nn;
    for (in = 1; in <= i__1; ++in) {

/*        Do for each value of N in NVAL */

	n = nval[in];
	lda = max(1,n);
	lap = lda * (lda + 1) / 2;
	*(unsigned char *)xtype = 'N';

	for (imat = 1; imat <= 10; ++imat) {

/*           Do the tests only if DOTYPE( IMAT ) is true. */

	    if (! dotype[imat]) {
		goto L70;
	    }

	    for (iuplo = 1; iuplo <= 2; ++iuplo) {

/*              Do first for UPLO = 'U', then for UPLO = 'L' */

		*(unsigned char *)uplo = *(unsigned char *)&uplos[iuplo - 1];

/*              Call SLATTP to generate a triangular test matrix. */

		s_copy(srnamc_1.srnamt, "SLATTP", (ftnlen)6, (ftnlen)6);
		slattp_(&imat, uplo, "No transpose", diag, iseed, &n, &ap[1], 
			&x[1], &work[1], &info);

/*              Set IDIAG = 1 for non-unit matrices, 2 for unit. */

		if (lsame_(diag, "N")) {
		    idiag = 1;
		} else {
		    idiag = 2;
		}

/* +    TEST 1   
                Form the inverse of A. */

		if (n > 0) {
		    scopy_(&lap, &ap[1], &c__1, &ainvp[1], &c__1);
		}
		s_copy(srnamc_1.srnamt, "STPTRI", (ftnlen)6, (ftnlen)6);
		stptri_(uplo, diag, &n, &ainvp[1], &info);

/*              Check error code from STPTRI. */

		if (info != 0) {
/* Writing concatenation */
		    i__2[0] = 1, a__1[0] = uplo;
		    i__2[1] = 1, a__1[1] = diag;
		    s_cat(ch__1, a__1, i__2, &c__2, (ftnlen)2);
		    alaerh_(path, "STPTRI", &info, &c__0, ch__1, &n, &n, &
			    c_n1, &c_n1, &c_n1, &imat, &nfail, &nerrs, nout);
		}

/*              Compute the infinity-norm condition number of A. */

		anorm = slantp_("I", uplo, diag, &n, &ap[1], &rwork[1]);
		ainvnm = slantp_("I", uplo, diag, &n, &ainvp[1], &rwork[1]);
		if (anorm <= 0.f || ainvnm <= 0.f) {
		    rcondi = 1.f;
		} else {
		    rcondi = 1.f / anorm / ainvnm;
		}

/*              Compute the residual for the triangular matrix times its   
                inverse.  Also compute the 1-norm condition number of A. */

		stpt01_(uplo, diag, &n, &ap[1], &ainvp[1], &rcondo, &rwork[1],
			 result);

/*              Print the test ratio if it is .GE. THRESH. */

		if (result[0] >= *thresh) {
		    if (nfail == 0 && nerrs == 0) {
			alahd_(nout, path);
		    }
		    io___26.ciunit = *nout;
		    s_wsfe(&io___26);
		    do_fio(&c__1, uplo, (ftnlen)1);
		    do_fio(&c__1, diag, (ftnlen)1);
		    do_fio(&c__1, (char *)&n, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&imat, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&c__1, (ftnlen)sizeof(integer));
		    do_fio(&c__1, (char *)&result[0], (ftnlen)sizeof(real));
		    e_wsfe();
		    ++nfail;
		}
		++nrun;

		i__3 = *nns;
		for (irhs = 1; irhs <= i__3; ++irhs) {
		    nrhs = nsval[irhs];
		    *(unsigned char *)xtype = 'N';

		    for (itran = 1; itran <= 3; ++itran) {

/*                 Do for op(A) = A, A**T, or A**H. */

			*(unsigned char *)trans = *(unsigned char *)&transs[
				itran - 1];
			if (itran == 1) {
			    *(unsigned char *)norm = 'O';
			    rcondc = rcondo;
			} else {
			    *(unsigned char *)norm = 'I';
			    rcondc = rcondi;
			}

/* +    TEST 2   
                   Solve and compute residual for op(A)*x = b. */

			s_copy(srnamc_1.srnamt, "SLARHS", (ftnlen)6, (ftnlen)
				6);
			slarhs_(path, xtype, uplo, trans, &n, &n, &c__0, &
				idiag, &nrhs, &ap[1], &lap, &xact[1], &lda, &
				b[1], &lda, iseed, &info);
			*(unsigned char *)xtype = 'C';
			slacpy_("Full", &n, &nrhs, &b[1], &lda, &x[1], &lda);

			s_copy(srnamc_1.srnamt, "STPTRS", (ftnlen)6, (ftnlen)
				6);
			stptrs_(uplo, trans, diag, &n, &nrhs, &ap[1], &x[1], &
				lda, &info);

/*                 Check error code from STPTRS. */

			if (info != 0) {
/* Writing concatenation */
			    i__4[0] = 1, a__2[0] = uplo;
			    i__4[1] = 1, a__2[1] = trans;
			    i__4[2] = 1, a__2[2] = diag;
			    s_cat(ch__2, a__2, i__4, &c__3, (ftnlen)3);
			    alaerh_(path, "STPTRS", &info, &c__0, ch__2, &n, &
				    n, &c_n1, &c_n1, &c_n1, &imat, &nfail, &
				    nerrs, nout);
			}

			stpt02_(uplo, trans, diag, &n, &nrhs, &ap[1], &x[1], &
				lda, &b[1], &lda, &work[1], &result[1]);

/* +    TEST 3   
                   Check solution from generated exact solution. */

			sget04_(&n, &nrhs, &x[1], &lda, &xact[1], &lda, &
				rcondc, &result[2]);

/* +    TESTS 4, 5, and 6   
                   Use iterative refinement to improve the solution and   
                   compute error bounds. */

			s_copy(srnamc_1.srnamt, "STPRFS", (ftnlen)6, (ftnlen)
				6);
			stprfs_(uplo, trans, diag, &n, &nrhs, &ap[1], &b[1], &
				lda, &x[1], &lda, &rwork[1], &rwork[nrhs + 1],
				 &work[1], &iwork[1], &info);

/*                 Check error code from STPRFS. */

			if (info != 0) {
/* Writing concatenation */
			    i__4[0] = 1, a__2[0] = uplo;
			    i__4[1] = 1, a__2[1] = trans;
			    i__4[2] = 1, a__2[2] = diag;
			    s_cat(ch__2, a__2, i__4, &c__3, (ftnlen)3);
			    alaerh_(path, "STPRFS", &info, &c__0, ch__2, &n, &
				    n, &c_n1, &c_n1, &nrhs, &imat, &nfail, &
				    nerrs, nout);
			}

			sget04_(&n, &nrhs, &x[1], &lda, &xact[1], &lda, &
				rcondc, &result[3]);
			stpt05_(uplo, trans, diag, &n, &nrhs, &ap[1], &b[1], &
				lda, &x[1], &lda, &xact[1], &lda, &rwork[1], &
				rwork[nrhs + 1], &result[4]);

/*                    Print information about the tests that did not pass   
                      the threshold. */

			for (k = 2; k <= 6; ++k) {
			    if (result[k - 1] >= *thresh) {
				if (nfail == 0 && nerrs == 0) {
				    alahd_(nout, path);
				}
				io___34.ciunit = *nout;
				s_wsfe(&io___34);
				do_fio(&c__1, uplo, (ftnlen)1);
				do_fio(&c__1, trans, (ftnlen)1);
				do_fio(&c__1, diag, (ftnlen)1);
				do_fio(&c__1, (char *)&n, (ftnlen)sizeof(
					integer));
				do_fio(&c__1, (char *)&nrhs, (ftnlen)sizeof(
					integer));
				do_fio(&c__1, (char *)&imat, (ftnlen)sizeof(
					integer));
				do_fio(&c__1, (char *)&k, (ftnlen)sizeof(
					integer));
				do_fio(&c__1, (char *)&result[k - 1], (ftnlen)
					sizeof(real));
				e_wsfe();
				++nfail;
			    }
/* L20: */
			}
			nrun += 5;
/* L30: */
		    }
/* L40: */
		}

/* +    TEST 7   
                   Get an estimate of RCOND = 1/CNDNUM. */

		for (itran = 1; itran <= 2; ++itran) {
		    if (itran == 1) {
			*(unsigned char *)norm = 'O';
			rcondc = rcondo;
		    } else {
			*(unsigned char *)norm = 'I';
			rcondc = rcondi;
		    }

		    s_copy(srnamc_1.srnamt, "STPCON", (ftnlen)6, (ftnlen)6);
		    stpcon_(norm, uplo, diag, &n, &ap[1], &rcond, &work[1], &
			    iwork[1], &info);

/*                 Check error code from STPCON. */

		    if (info != 0) {
/* Writing concatenation */
			i__4[0] = 1, a__2[0] = norm;
			i__4[1] = 1, a__2[1] = uplo;
			i__4[2] = 1, a__2[2] = diag;
			s_cat(ch__2, a__2, i__4, &c__3, (ftnlen)3);
			alaerh_(path, "STPCON", &info, &c__0, ch__2, &n, &n, &
				c_n1, &c_n1, &c_n1, &imat, &nfail, &nerrs, 
				nout);
		    }

		    stpt06_(&rcond, &rcondc, uplo, diag, &n, &ap[1], &rwork[1]
			    , &result[6]);

/*                 Print the test ratio if it is .GE. THRESH. */

		    if (result[6] >= *thresh) {
			if (nfail == 0 && nerrs == 0) {
			    alahd_(nout, path);
			}
			io___36.ciunit = *nout;
			s_wsfe(&io___36);
			do_fio(&c__1, "STPCON", (ftnlen)6);
			do_fio(&c__1, norm, (ftnlen)1);
			do_fio(&c__1, uplo, (ftnlen)1);
			do_fio(&c__1, diag, (ftnlen)1);
			do_fio(&c__1, (char *)&n, (ftnlen)sizeof(integer));
			do_fio(&c__1, (char *)&imat, (ftnlen)sizeof(integer));
			do_fio(&c__1, (char *)&c__7, (ftnlen)sizeof(integer));
			do_fio(&c__1, (char *)&result[6], (ftnlen)sizeof(real)
				);
			e_wsfe();
			++nfail;
		    }
		    ++nrun;
/* L50: */
		}
/* L60: */
	    }
L70:
	    ;
	}

/*        Use pathological test matrices to test SLATPS. */

	for (imat = 11; imat <= 18; ++imat) {

/*           Do the tests only if DOTYPE( IMAT ) is true. */

	    if (! dotype[imat]) {
		goto L100;
	    }

	    for (iuplo = 1; iuplo <= 2; ++iuplo) {

/*              Do first for UPLO = 'U', then for UPLO = 'L' */

		*(unsigned char *)uplo = *(unsigned char *)&uplos[iuplo - 1];
		for (itran = 1; itran <= 3; ++itran) {

/*                 Do for op(A) = A, A**T, or A**H. */

		    *(unsigned char *)trans = *(unsigned char *)&transs[itran 
			    - 1];

/*                 Call SLATTP to generate a triangular test matrix. */

		    s_copy(srnamc_1.srnamt, "SLATTP", (ftnlen)6, (ftnlen)6);
		    slattp_(&imat, uplo, trans, diag, iseed, &n, &ap[1], &x[1]
			    , &work[1], &info);

/* +    TEST 8   
                   Solve the system op(A)*x = b. */

		    s_copy(srnamc_1.srnamt, "SLATPS", (ftnlen)6, (ftnlen)6);
		    scopy_(&n, &x[1], &c__1, &b[1], &c__1);
		    slatps_(uplo, trans, diag, "N", &n, &ap[1], &b[1], &scale,
			     &rwork[1], &info);

/*                 Check error code from SLATPS. */

		    if (info != 0) {
/* Writing concatenation */
			i__5[0] = 1, a__3[0] = uplo;
			i__5[1] = 1, a__3[1] = trans;
			i__5[2] = 1, a__3[2] = diag;
			i__5[3] = 1, a__3[3] = "N";
			s_cat(ch__3, a__3, i__5, &c__4, (ftnlen)4);
			alaerh_(path, "SLATPS", &info, &c__0, ch__3, &n, &n, &
				c_n1, &c_n1, &c_n1, &imat, &nfail, &nerrs, 
				nout);
		    }

		    stpt03_(uplo, trans, diag, &n, &c__1, &ap[1], &scale, &
			    rwork[1], &c_b103, &b[1], &lda, &x[1], &lda, &
			    work[1], &result[7]);

/* +    TEST 9   
                   Solve op(A)*x = b again with NORMIN = 'Y'. */

		    scopy_(&n, &x[1], &c__1, &b[n + 1], &c__1);
		    slatps_(uplo, trans, diag, "Y", &n, &ap[1], &b[n + 1], &
			    scale, &rwork[1], &info);

/*                 Check error code from SLATPS. */

		    if (info != 0) {
/* Writing concatenation */
			i__5[0] = 1, a__3[0] = uplo;
			i__5[1] = 1, a__3[1] = trans;
			i__5[2] = 1, a__3[2] = diag;
			i__5[3] = 1, a__3[3] = "Y";
			s_cat(ch__3, a__3, i__5, &c__4, (ftnlen)4);
			alaerh_(path, "SLATPS", &info, &c__0, ch__3, &n, &n, &
				c_n1, &c_n1, &c_n1, &imat, &nfail, &nerrs, 
				nout);
		    }

		    stpt03_(uplo, trans, diag, &n, &c__1, &ap[1], &scale, &
			    rwork[1], &c_b103, &b[n + 1], &lda, &x[1], &lda, &
			    work[1], &result[8]);

/*                 Print information about the tests that did not pass   
                   the threshold. */

		    if (result[7] >= *thresh) {
			if (nfail == 0 && nerrs == 0) {
			    alahd_(nout, path);
			}
			io___38.ciunit = *nout;
			s_wsfe(&io___38);
			do_fio(&c__1, "SLATPS", (ftnlen)6);
			do_fio(&c__1, uplo, (ftnlen)1);
			do_fio(&c__1, trans, (ftnlen)1);
			do_fio(&c__1, diag, (ftnlen)1);
			do_fio(&c__1, "N", (ftnlen)1);
			do_fio(&c__1, (char *)&n, (ftnlen)sizeof(integer));
			do_fio(&c__1, (char *)&imat, (ftnlen)sizeof(integer));
			do_fio(&c__1, (char *)&c__8, (ftnlen)sizeof(integer));
			do_fio(&c__1, (char *)&result[7], (ftnlen)sizeof(real)
				);
			e_wsfe();
			++nfail;
		    }
		    if (result[8] >= *thresh) {
			if (nfail == 0 && nerrs == 0) {
			    alahd_(nout, path);
			}
			io___39.ciunit = *nout;
			s_wsfe(&io___39);
			do_fio(&c__1, "SLATPS", (ftnlen)6);
			do_fio(&c__1, uplo, (ftnlen)1);
			do_fio(&c__1, trans, (ftnlen)1);
			do_fio(&c__1, diag, (ftnlen)1);
			do_fio(&c__1, "Y", (ftnlen)1);
			do_fio(&c__1, (char *)&n, (ftnlen)sizeof(integer));
			do_fio(&c__1, (char *)&imat, (ftnlen)sizeof(integer));
			do_fio(&c__1, (char *)&c__9, (ftnlen)sizeof(integer));
			do_fio(&c__1, (char *)&result[8], (ftnlen)sizeof(real)
				);
			e_wsfe();
			++nfail;
		    }
		    nrun += 2;
/* L80: */
		}
/* L90: */
	    }
L100:
	    ;
	}
/* L110: */
    }

/*     Print a summary of the results. */

    alasum_(path, nout, &nfail, &nrun, &nerrs);

    return 0;

/*     End of SCHKTP */

} /* schktp_ */

