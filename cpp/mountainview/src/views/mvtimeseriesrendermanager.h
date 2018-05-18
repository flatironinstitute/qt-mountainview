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

#ifndef MVTIMESERIESRENDERMANAGER_H
#define MVTIMESERIESRENDERMANAGER_H

#include "multiscaletimeseries.h"
#include "mlcommon.h"

#include <QColor>
#include <QImage>
#include <QRunnable>
#include <QThread>

class MVTimeSeriesRenderManagerPrivate;
class MVTimeSeriesRenderManager : public QObject {
    Q_OBJECT
public:
    friend class MVTimeSeriesRenderManagerPrivate;
    MVTimeSeriesRenderManager();
    virtual ~MVTimeSeriesRenderManager();

    void clear();
    void setMultiScaleTimeSeries(MultiScaleTimeSeries ts);
    void setChannelColors(const QList<QColor>& colors);
    double visibleMinimum() const;
    double visibleMaximum() const;

    QImage getImage(double t1, double t2, double amp_factor, double W, double H);

signals:
    void updated();

private slots:
    void slot_thread_finished();

private:
    MVTimeSeriesRenderManagerPrivate* d;
};

class MVTimeSeriesRenderManagerThread : public QThread {
    Q_OBJECT
public:
    //input
    double amp_factor;
    int ds_factor;
    int panel_width;
    int panel_num_points;
    int index;
    QList<QColor> channel_colors;
    MultiScaleTimeSeries ts;
    QColor get_channel_color(int m);

    //output
    QImage image;
    Mda32 min_data;
    Mda32 max_data;

    void run();
};

class ThreadManager : public QObject {
    Q_OBJECT
public:
    ThreadManager();
    virtual ~ThreadManager();
    void start(QString id, QThread* thread);
    void stop(QString id);
    void clear();
private slots:
    void slot_timer();
    void slot_thread_finished();

private:
    QMap<QString, QThread*> m_queued_threads;
    QMap<QString, QThread*> m_running_threads;
};

#endif // MVTIMESERIESRENDERMANAGER_H
