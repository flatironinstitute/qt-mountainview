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
#include "get_pca_features.h"
#include "mda.h"
#include "eigenvalue_decomposition.h"
#include "get_sort_indices.h"
#include "mlcommon.h"

#include <QTime>

/*
bool get_pca_features(int M, int N, int num_features, double* features_out, double* X_in, int num_representatives)
{
    int increment = 1;
    if (num_representatives)
        increment = qMax(1L, N / num_representatives);
    QTime timer;
    timer.start();
    Mda XXt(M, M);
    double* XXt_ptr = XXt.dataPtr();
    for (int i = 0; i < N; i += increment) {
        double* tmp = &X_in[i * M];
        int aa = 0;
        for (int m2 = 0; m2 < M; m2++) {
            for (int m1 = 0; m1 < M; m1++) {
                XXt_ptr[aa] += tmp[m1] * tmp[m2];
                aa++;
            }
        }
    }

    Mda U;
    Mda S;
    eigenvalue_decomposition_sym(U, S, XXt);
    QVector<double> eigenvals;
    for (int i = 0; i < S.totalSize(); i++)
        eigenvals << S.get(i);
    QList<int> inds = get_sort_indices(eigenvals);

    Mda FF(num_features, N);
    int aa = 0;
    for (int i = 0; i < N; i++) {
        double* tmp = &X_in[i * M];
        for (int j = 0; j < num_features; j++) {
            if (inds.count() - 1 - j >= 0) {
                int k = inds.value(inds.count() - 1 - j);
                double val = 0;
                for (int m = 0; m < M; m++) {
                    val += U.get(m, k) * tmp[m];
                }
                FF.set(val, aa);
            }
            aa++;
        }
    }

    for (int i = 0; i < FF.totalSize(); i++) {
        features_out[i] = FF.get(i);
    }

    return true;
}

bool pca_denoise(int M, int N, int num_features, double* X_out, double* X_in, int num_representatives)
{
    int increment = 1;
    if (num_representatives)
        increment = qMax(1L, N / num_representatives);
    QTime timer;
    timer.start();
    Mda XXt(M, M);
    double* XXt_ptr = XXt.dataPtr();
    for (int i = 0; i < N; i += increment) {
        double* tmp = &X_in[i * M];
        int aa = 0;
        for (int m2 = 0; m2 < M; m2++) {
            for (int m1 = 0; m1 < M; m1++) {
                XXt_ptr[aa] += tmp[m1] * tmp[m2];
                aa++;
            }
        }
    }

    Mda U;
    Mda S;
    eigenvalue_decomposition_sym(U, S, XXt);
    QVector<double> eigenvals;
    for (int i = 0; i < S.totalSize(); i++)
        eigenvals << S.get(i);
    QList<int> inds = get_sort_indices(eigenvals);

    for (int i = 0; i < N; i++) {
        double* tmp_in = &X_in[i * M];
        double* tmp_out = &X_out[i * M];
        for (int m = 0; m < M; m++)
            tmp_out[m] = 0;
        for (int j = 0; j < num_features; j++) {
            if (inds.count() - 1 - j >= 0) {
                int k = inds.value(inds.count() - 1 - j);
                double val = 0;
                for (int m = 0; m < M; m++) {
                    val += U.get(m, k) * tmp_in[m];
                }
                for (int m = 0; m < M; m++) {
                    tmp_out[m] += U.get(m, k) * val;
                }
            }
        }
    }

    return true;
}
*/
