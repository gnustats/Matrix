//
//              LAPACK++ 1.1 Linear Algebra Package 1.1
//               University of Tennessee, Knoxvilee, TN.
//            Oak Ridge National Laboratory, Oak Ridge, TN.
//        Authors: J. J. Dongarra, E. Greaser, R. Pozo, D. Walker
//                 (C) 1992-1996 All Rights Reserved
//
//                             NOTICE
//
// Permission to use, copy, modify, and distribute this software and
// its documentation for any purpose and without fee is hereby granted
// provided that the above copyright notice appear in all copies and
// that both the copyright notice and this permission notice appear in
// supporting documentation.
//
// Neither the Institutions (University of Tennessee, and Oak Ridge National
// Laboratory) nor the Authors make any representations about the suitability 
// of this software for any purpose.  This software is provided ``as is'' 
// without express or implied warranty.
//
// LAPACK++ was funded in part by the U.S. Department of Energy, the
// National Science Foundation and the State of Tennessee.
//
// Modifications Copyright (C) 2000-2000 the R Development Core Team
//

#include "lafnames.h"
#include LA_UPPER_TRIANG_MAT_DOUBLE_H
#include "blas3.h"
#include "vi.h"

double LaUpperTriangMatDouble::outofbounds_ = 0; // initialize outofbounds.

LaUpperTriangMatDouble& LaUpperTriangMatDouble::copy(const LaMatDouble &ob)
{
    int M = ob.size(0);
    int N = ob.size(1);

    resize(ob);
    for (int i = 0; i < M; i++)
	for (int j = i; j < N; j++)
	    (*this)(i,j) = ob(i,j);

    return *this;
}

LaMatDouble& LaUpperTriangMatDouble::operator=(double s)
{
    int M = size(0);
    int N = size(1);

    for (int i = 0; i < M; i++)
        for (int j = i; j < N; j++)
            (*this)(i,j) = s;

    return *this;
}

ostream& LaUpperTriangMatDouble::printMatrix(ostream &s) const
{
    if (*info_)     // print out only matrix info, not actual values
    {
	*info_ = 0; // reset the flag
	s << "(" << size(0) << "x" << size(1) << ") " ;
	s << "Indices: " << index(0) << " " << index(1);
	s << " #ref: " << ref_count() ;
	s << " sa:" << shallow();
    } else {
	int M = size(0);
	int N = size(1);

	for (int i = 0; i < M; i++) {
	    for (int j = 0; j < N; j++) {
		if (j >= i)
		    s << (*this)(i,j) << "  ";
	    }
	    s << endl;
	}
    }
    return s;
}

LaUpperTriangMatDouble* LaUpperTriangMatDouble::solve() const
{				// inverse
    LaUpperTriangMatDouble *inv; //create a copy to return
    inv = new LaUpperTriangMatDouble(*this); 

    int info;
    F77_CALL(dtrtri)('U', 'N', inv->size(1), &(*inv)(0,0), inv->gdim(0), info);
    if (info != 0)
	throw(LaException("LaUpperTriangMatDouble::solve()",
			  "Non-zero return code from dtrtri"));
    return inv;
}

LaMatDouble& LaUpperTriangMatDouble::solve(LaMatDouble& B) const
{				// in-place solution
    F77_CALL(dtrsm)('L', 'U', 'N', 'N', size(1), B.size(1), 1.0,
		    &data_(0,0), gdim(0), &B(0,0), B.gdim(0));
    return B;
}

LaMatDouble& LaUpperTriangMatDouble::solve(LaMatDouble& X, const LaMatDouble& B) const
{
    X.inject(B);
    return solve(X);
}

double LaUpperTriangMatDouble::norm(char which) const
{
    VectorDouble work(size(0));	// only needed for Infinity norm
    return F77_CALL(dlantr)(which, 'U', 'N', size(0), size(1),
			    &(*this)(0,0), gdim(0), &work(0));
}

double LaUpperTriangMatDouble::rcond(char which) const
{
    double val;
    VectorDouble work(3 * size(0));
    int info;
    VectorInt iwork(size(0));
    F77_CALL(dtrcon)(which, 'U', 'N', size(0), &(*this)(0,0),
		     gdim(0), val, &work(0), &iwork(0), info);
    return val;
}

SEXP LaUpperTriangMatDouble::asSEXP() const
{
    SEXP val = PROTECT(allocMatrix(REALSXP, size(0), size(1)));
    F77_CALL(dlacpy)('U', size(0), size(1), &(*this)(0,0), gdim(0),
		     REAL(val), size(0));
    SEXP classes = PROTECT(allocVector(STRSXP, 2));
    STRING(classes)[0] = mkChar("UpperTriangular");
    STRING(classes)[1] = mkChar("Matrix");
    setAttrib(val, R_ClassSymbol, classes);
    UNPROTECT(2);
    return val;
}

