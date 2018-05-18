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
#ifndef FTELECTRODEARRAYVIEW_H
#define FTELECTRODEARRAYVIEW_H

#include <QWidget>
#include "mda.h"
#include "diskreadmda.h"

class FTElectrodeArrayViewPrivate;

class FTElectrodeArrayView : public QWidget {
    Q_OBJECT
public:
    friend class FTElectrodeArrayViewPrivate;
    explicit FTElectrodeArrayView(QWidget* parent = 0);

    void setElectrodeLocations(const Mda& L);
    void setWaveform(const Mda& X);
    Mda* waveform();

    int timepoint();
    void setTimepoint(int val);

    void animate();
    void setLoopAnimation(bool val);
    void pauseAnimation();
    void stopAnimation();
    bool isAnimating();
    void setAnimationSpeed(float hz);

    void setShowChannelNumbers(bool val);
    void setAutoSelectChannels(bool val);

    void setGlobalAbsMax(float val);
    void setNormalizeIntensity(bool val);
    void setBrightness(float val);

    QList<int> selectedElectrodeIndices();
    void setSelectedElectrodeIndices(const QList<int>& X);

signals:
    void signalTimepointChanged();
    void signalSelectedElectrodesChanged();
    void signalElectrodeLeftClicked(int);
    void signalLoop();

protected:
    virtual void paintEvent(QPaintEvent* evt);
    virtual void mouseMoveEvent(QMouseEvent* evt);
    virtual void mousePressEvent(QMouseEvent* evt);

private slots:
    void slot_timer();

private:
    FTElectrodeArrayViewPrivate* d;

    ~FTElectrodeArrayView();

signals:

public slots:
};

#endif // FTELECTRODEARRAYVIEW_H
