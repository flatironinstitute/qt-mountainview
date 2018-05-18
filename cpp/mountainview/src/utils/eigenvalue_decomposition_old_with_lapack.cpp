/*
 * Copyright 2016-2017 Flatiron Institute, Simons Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "eigenvalue_decomposition.h"
#include <QDebug>
#include <QTime>

#ifdef USE_LAPACK

#define HAVE_LAPACK_CONFIG_H
#define LAPACK_COMPLEX_CPP
bool eigenvalue_decomposition_sym(Mda& U, Mda& S, Mda& X)
{
    //X=U*diag(S)*U'
    //X is MxM, U is MxM, S is 1xM
    //X must be real and symmetric

    int M = X.N1();
    if (M != X.N2()) {
        qWarning() << "Unexpected problem in eigenvalue_decomposition_sym" << X.N1() << X.N2();
        exit(-1);
    }

    U.allocate(M, M);
    S.allocate(1, M);
    double* Uptr = U.dataPtr();
    double* Sptr = S.dataPtr();
    double* Xptr = X.dataPtr();

    for (int ii = 0; ii < M * M; ii++) {
        Uptr[ii] = Xptr[ii];
    }

    //'V' means compute eigenvalues and eigenvectors (use 'N' for eigenvalues only)
    //'U' means upper triangle of A is stored.
    //QTime timer; timer.start();
    int info = LAPACKE_dsyevd(LAPACK_COL_MAJOR, 'V', 'U', M, Uptr, M, Sptr);
    //printf("Time for call to LAPACKE_dsyev: %g sec\n",timer.elapsed()*1.0/1000);
    if (info != 0) {
        qWarning() << "Error in LAPACKE_dsyev" << info;
        return false;
    }
    return true;
}

#endif
