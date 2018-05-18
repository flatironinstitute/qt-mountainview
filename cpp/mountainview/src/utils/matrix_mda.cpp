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
#include "matrix_mda.h"

Mda matrix_transpose(const Mda& A)
{
    Mda ret(A.N2(), A.N1());
    for (int i = 0; i < A.N1(); i++) {
        for (int j = 0; j < A.N2(); j++) {
            ret.set(A.get(i, j), j, i);
        }
    }
    return ret;
}

Mda matrix_multiply(const Mda& A, const Mda& B)
{
    int N1 = A.N1();
    int N2 = A.N2();
    int N2B = B.N1();
    int N3 = B.N2();
    if (N2 != N2B) {
        qWarning() << "Unexpected problem in matrix_multiply" << N1 << N2 << N2B << N3;
        exit(-1);
    }
    Mda ret(N1, N3);
    for (int i = 0; i < N1; i++) {
        for (int j = 0; j < N3; j++) {
            double val = 0;
            for (int k = 0; k < N2; k++) {
                val += A.get(i, k) * B.get(k, j);
            }
            ret.set(val, i, j);
        }
    }
    return ret;
}

void matrix_print(const Mda& A)
{
    for (int r = 0; r < A.N1(); r++) {
        for (int c = 0; c < A.N2(); c++) {
            printf("%g ", A.get(r, c));
        }
        printf("\n");
    }
}
