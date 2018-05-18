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

#ifndef MVHISTOGRAMGRID_H
#define MVHISTOGRAMGRID_H

#include "mvabstractview.h"
#include "histogramview.h"
#include "mvgridview.h"

#include <mvcontext.h>

struct HorizontalScaleAxisData {
    bool use_it = false;
    QString label;
};

class MVHistogramGridPrivate;
class MVHistogramGrid : public MVGridView {
    Q_OBJECT
public:
    friend class MVHistogramGridPrivate;
    MVHistogramGrid(MVAbstractContext* context);
    virtual ~MVHistogramGrid();

    void paintEvent(QPaintEvent* evt);
    void keyPressEvent(QKeyEvent* evt);
signals:

protected:
    void setHorizontalScaleAxis(HorizontalScaleAxisData X);
    void setHistogramViews(const QList<HistogramView*> views);
    QList<HistogramView*> histogramViews();
    void prepareMimeData(QMimeData& mimeData, const QPoint& pos);
    void setPairMode(bool val);
    bool pairMode() const;

private slots:
    void slot_histogram_view_clicked(Qt::KeyboardModifiers modifiers);
    void slot_cluster_attributes_changed(int cluster_number);
    void slot_cluster_pair_attributes_changed(ClusterPair pair);
    void slot_update_highlighting();
    void slot_context_menu(const QPoint& pt);

private:
    MVHistogramGridPrivate* d;
};

#endif // MVHISTOGRAMGRID_H
