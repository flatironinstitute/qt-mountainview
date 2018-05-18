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

#ifndef MVCLUSTERVIEW_H
#define MVCLUSTERVIEW_H

#include <QWidget>
#include <mvabstractcontext.h>
#include "mda.h"
#include "mvutils.h"
#include "affinetransformation.h"
#include "paintlayer.h"

#define MVCV_MODE_HEAT_DENSITY 1
#define MVCV_MODE_LABEL_COLORS 2
#define MVCV_MODE_TIME_COLORS 3
#define MVCV_MODE_AMPLITUDE_COLORS 4

/** \class MVClusterView
 *  \brief A rotatable view of datapoints in 3D space
 *
 *  Several modes are available
 */

class MVClusterViewPrivate;
class MVClusterView : public QWidget {
    Q_OBJECT
public:
    friend class MVClusterViewPrivate;
    MVClusterView(MVAbstractContext* context, QWidget* parent = 0);
    virtual ~MVClusterView();
    void setData(const Mda& X);
    bool hasData();
    void setTimes(const QVector<double>& times);
    void setLabels(const QVector<int>& labels);
    void setAmplitudes(const QVector<double>& amps);

    void setMode(int mode);
    void setCurrentEvent(MVEvent evt, bool do_emit = false);
    MVEvent currentEvent();
    int currentEventIndex();
    AffineTransformation transformation();
    void setTransformation(const AffineTransformation& T);

    QSet<int> activeClusterNumbers() const;
    void setActiveClusterNumbers(const QSet<int>& A);

    QImage renderImage(int W, int H);
    void scaleView(double factor);
signals:
    void currentEventChanged();
    void transformationChanged();
    void activeClusterNumbersChanged();

private slots:
    void slot_context_menu(const QPoint& pos);
    void slot_active_cluster_numbers_changed();
    void slot_hovered_cluster_number_changed();
    void slot_update_colors();

protected:
    void paintEvent(QPaintEvent* evt);
    void mouseMoveEvent(QMouseEvent* evt);
    void mousePressEvent(QMouseEvent* evt);
    void mouseReleaseEvent(QMouseEvent* evt);
    void wheelEvent(QWheelEvent* evt);
    void keyPressEvent(QKeyEvent *evt);
    void resizeEvent(QResizeEvent* evt);

private:
    MVClusterViewPrivate* d;
};

#endif // MVCLUSTERVIEW_H
