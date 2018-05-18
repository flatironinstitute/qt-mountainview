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
#include "mlcommon.h"
#include <math.h>
#include <QDateTime>
#include <QDir>
#include <QCryptographicHash>
#include <QThread>
#include "mlcommon.h"
#include "mda.h"
#include "msmisc.h"

#include <QCoreApplication>
#ifdef QT_GUI_LIB
#include <QMessageBox>
#endif

Mda compute_mean_clip(const Mda& clips)
{
    int M = clips.N1();
    int T = clips.N2();
    int L = clips.N3();
    Mda ret;
    ret.allocate(M, T);
    int aaa = 0;
    for (int i = 0; i < L; i++) {
        int bbb = 0;
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                ret.set(ret.get(bbb) + clips.get(aaa), bbb);
                aaa++;
                bbb++;
            }
        }
    }
    if (L) {
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                ret.set(ret.get(m, t) / L, m, t);
            }
        }
    }
    return ret;
}

Mda32 compute_mean_clip(const Mda32& clips)
{
    int M = clips.N1();
    int T = clips.N2();
    int L = clips.N3();
    Mda32 ret;
    ret.allocate(M, T);
    int aaa = 0;
    for (int i = 0; i < L; i++) {
        int bbb = 0;
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                ret.set(ret.get(bbb) + clips.get(aaa), bbb);
                aaa++;
                bbb++;
            }
        }
    }
    if (L) {
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                ret.set(ret.get(m, t) / L, m, t);
            }
        }
    }
    return ret;
}

Mda32 compute_stdev_clip(const Mda32& clips)
{
    int M = clips.N1();
    int T = clips.N2();
    int L = clips.N3();

    Mda32 stdevs(M, T);
    float* stdevs_ptr = stdevs.dataPtr();

    const float* clips_ptr = clips.constDataPtr();
    Mda sums(M, T);
    Mda sumsqrs(M, T);
    double* sums_ptr = sums.dataPtr();
    double* sumsqrs_ptr = sumsqrs.dataPtr();
    double count = 0;
    for (int i = 0; i < L; i++) {
        const float* Xptr = &clips_ptr[M * T * i];
        for (int i = 0; i < M * T; i++) {
            sums_ptr[i] += Xptr[i];
            sumsqrs_ptr[i] += Xptr[i] * Xptr[i];
        }
        count++;
    }

    for (int i = 0; i < M * T; i++) {
        double sum0 = sums_ptr[i];
        double sumsqr0 = sumsqrs_ptr[i];
        if (count) {
            stdevs_ptr[i] = sqrt(sumsqr0 / count - (sum0 * sum0) / (count * count));
        }
    }
    return stdevs;
}

/*
bool eigenvalue_decomposition_sym_isosplit(Mda& U, Mda& S, Mda& X)
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
    //double* Sptr = S.dataPtr();
    double* Xptr = X.dataPtr();

    for (int ii = 0; ii < M * M; ii++) {
        Uptr[ii] = Xptr[ii];
    }

    //'V' means compute eigenvalues and eigenvectors (use 'N' for eigenvalues only)
    //'U' means upper triangle of A is stored.
    //QTime timer; timer.start();

    //int info = LAPACKE_dsyev(LAPACK_COL_MAJOR, 'V', 'U', M, Uptr, M, Sptr);
    int info = 0;

    //printf("Time for call to LAPACKE_dsyev: %g sec\n",timer.elapsed()*1.0/1000);
    if (info != 0) {
        qWarning() << "Error in LAPACKE_dsyev" << info;
        return false;
    }
    return true;
}
*/

Mda grab_clips_subset(const Mda& clips, const QVector<int>& inds)
{
    int M = clips.N1();
    int T = clips.N2();
    int LLL = inds.count();
    Mda ret;
    ret.allocate(M, T, LLL);
    for (int i = 0; i < LLL; i++) {
        int aaa = i * M * T;
        int bbb = inds[i] * M * T;
        for (int k = 0; k < M * T; k++) {
            ret.set(clips.get(bbb), aaa);
            aaa++;
            bbb++;
        }
    }
    return ret;
}

Mda32 grab_clips_subset(const Mda32& clips, const QVector<int>& inds)
{
    int M = clips.N1();
    int T = clips.N2();
    int LLL = inds.count();
    Mda32 ret;
    ret.allocate(M, T, LLL);
    for (int i = 0; i < LLL; i++) {
        int aaa = i * M * T;
        int bbb = inds[i] * M * T;
        for (int k = 0; k < M * T; k++) {
            ret.set(clips.get(bbb), aaa);
            aaa++;
            bbb++;
        }
    }
    return ret;
}
