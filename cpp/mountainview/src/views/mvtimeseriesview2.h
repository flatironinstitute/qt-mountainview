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

#ifndef MVTIMESERIESVIEW2_H
#define MVTIMESERIESVIEW2_H

#include <QWidget>
#include <diskreadmda.h>
#include <mvutils.h>
#include "mvabstractviewfactory.h"
#include "mvtimeseriesviewbase.h"

class MVTimeSeriesView2Private;
class MVTimeSeriesView2 : public MVTimeSeriesViewBase {
    Q_OBJECT
public:
    friend class MVTimeSeriesView2Private;
    MVTimeSeriesView2(MVAbstractContext* context);
    virtual ~MVTimeSeriesView2();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void paintContent(QPainter* painter);

    void setAmplitudeFactor(double factor); // display range will be between -1/factor and 1/factor, but not clipped (thus channel plots may overlap)
    void autoSetAmplitudeFactor();
    void autoSetAmplitudeFactorWithinTimeRange();

    double amplitudeFactor() const;

    void resizeEvent(QResizeEvent* evt);
    void keyPressEvent(QKeyEvent* evt);

private slots:
    void slot_vertical_zoom_in();
    void slot_vertical_zoom_out();

private:
    MVTimeSeriesView2Private* d;
};

class MVTimeSeriesDataFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVTimeSeriesDataFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
};

#endif // MVTIMESERIESVIEW2_H
