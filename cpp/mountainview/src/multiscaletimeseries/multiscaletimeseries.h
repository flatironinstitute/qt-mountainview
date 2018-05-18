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

#ifndef MULTISCALEMDA_H
#define MULTISCALEMDA_H

#include <diskreadmda32.h>

class MultiScaleTimeSeriesPrivate;
class MultiScaleTimeSeries {
public:
    friend class MultiScaleTimeSeriesPrivate;
    MultiScaleTimeSeries();
    MultiScaleTimeSeries(const MultiScaleTimeSeries& other);
    virtual ~MultiScaleTimeSeries();
    void operator=(const MultiScaleTimeSeries& other);
    void setData(const DiskReadMda32& X);
    void setMLProxyUrl(const QString& url);
    void initialize();

    int N1();
    int N2();
    bool getData(Mda32& min, Mda32& max, int t1, int t2, int ds_factor); //returns values at timepoints i1*ds_factor:ds_factor:i2*ds_factor
    double minimum(); //return the global minimum value
    double maximum(); //return the global maximum value

    static int smallest_power_of_3_larger_than(int N);

private:
    MultiScaleTimeSeriesPrivate* d;
};

#endif // MULTISCALEMDA_H
