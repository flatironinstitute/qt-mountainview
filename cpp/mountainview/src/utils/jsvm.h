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
#ifndef JSVM_H
#define JSVM_H

#include "svm.h"

#include <QVector>
#include <mda32.h>

//labels should be 1 and 2
bool get_svm_discrim_direction(double& cutoff, QVector<double>& direction, const Mda32& X, const QVector<int>& labels);

#endif // JSVM_H
