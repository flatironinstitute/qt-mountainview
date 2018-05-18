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
#ifndef MVAMPHISTVIEW3_H
#define MVAMPHISTVIEW3_H

#include "mvabstractview.h"
#include "mvabstractviewfactory.h"
#include "diskreadmda.h"

class MVAmpHistView3Private;
class MVAmpHistView3 : public MVAbstractView {
    Q_OBJECT
public:
    friend class MVAmpHistView3Private;
    MVAmpHistView3(MVAbstractContext* context);
    virtual ~MVAmpHistView3();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    enum AmplitudeMode {
        ReadAmplitudes,
        ComputeAmplitudes
    };

protected:
    void wheelEvent(QWheelEvent* evt);
    void prepareMimeData(QMimeData& mimeData, const QPoint& pos);
    void keyPressEvent(QKeyEvent* evt);

private slots:
    void slot_zoom_in_horizontal(double factor = 1.2);
    void slot_zoom_out_horizontal(double factor = 1.2);
    void slot_pan_left(double units = 0.1);
    void slot_pan_right(double units = 0.1);
    void slot_panel_clicked(int index, Qt::KeyboardModifiers modifiers);
    void slot_update_highlighting_and_captions();
    void slot_current_cluster_changed();
    void slot_update_bins();

private:
    MVAmpHistView3Private* d;
};

class MVAmplitudeHistograms3Factory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVAmplitudeHistograms3Factory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
private slots:
};

DiskReadMda compute_amplitudes(QString timeseries, QString firings, QString mlproxy_url);

#endif // MVAMPHISTVIEW3_H
