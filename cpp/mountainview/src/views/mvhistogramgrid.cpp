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

#include "mvhistogramgrid.h"
#include "histogramview.h"
#include "mvutils.h"
#include "taskprogress.h"

#include <QAction>
#include <QCoreApplication>
#include <QGridLayout>
#include <QKeyEvent>
#include <QList>
#include <QPainter>
#include <math.h>
#include "mlcommon.h"
#include "mvmisc.h"
#include <QScrollArea>
#include <QScrollBar>
#include "actionfactory.h"

class HorizontalScaleAxis : public QWidget {
public:
    HorizontalScaleAxis();
    HorizontalScaleAxisData m_data;

protected:
    void paintEvent(QPaintEvent* evt);
};

class MVHistogramGridPrivate {
public:
    MVHistogramGrid* q;

    HorizontalScaleAxisData m_horizontal_scale_axis_data;
    HorizontalScaleAxis* m_horizontal_scale_axis;

    bool m_pair_mode = true;

    void do_highlighting_and_captions();
    int find_view_index_for_k(int k);
    void shift_select_clusters_between(int kA, int kB);
    void update_layout();
};

MVHistogramGrid::MVHistogramGrid(MVAbstractContext* context)
    : MVGridView(context)
{
    d = new MVHistogramGridPrivate;
    d->q = this;

    d->m_horizontal_scale_axis = new HorizontalScaleAxis;

    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    QObject::connect(c, SIGNAL(clusterAttributesChanged(int)), this, SLOT(slot_cluster_attributes_changed(int)));
    QObject::connect(c, SIGNAL(clusterPairAttributesChanged(ClusterPair)), this, SLOT(slot_cluster_pair_attributes_changed(ClusterPair)));
    QObject::connect(c, SIGNAL(currentClusterChanged()), this, SLOT(slot_update_highlighting()));
    QObject::connect(c, SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_highlighting()));
    QObject::connect(c, SIGNAL(selectedClusterPairsChanged()), this, SLOT(slot_update_highlighting()));
}

MVHistogramGrid::~MVHistogramGrid()
{
    this->stopCalculation();
    delete d;
}

void MVHistogramGrid::paintEvent(QPaintEvent* evt)
{
    QWidget::paintEvent(evt);

    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    QPainter painter(this);
    if (isCalculating()) {
        //show that something is computing
        painter.fillRect(QRectF(0, 0, width(), height()), c->color("calculation-in-progress"));
    }
}

void MVHistogramGrid::keyPressEvent(QKeyEvent* evt)
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    QList<HistogramView*> histogram_views = this->histogramViews();
    if ((evt->key() == Qt::Key_A) && (evt->modifiers() & Qt::ControlModifier)) {
        if (d->m_pair_mode) {
            QSet<ClusterPair> pairs;
            for (int i = 0; i < histogram_views.count(); i++) {
                int k1 = histogram_views[i]->property("k1").toInt();
                int k2 = histogram_views[i]->property("k2").toInt();
                pairs.insert(ClusterPair(k1, k2));
            }
            c->setSelectedClusterPairs(pairs);
        }
        else {
            QList<int> ks;
            for (int i = 0; i < histogram_views.count(); i++) {
                int k = histogram_views[i]->property("k").toInt();
                if (k) {
                    ks << k;
                }
            }
            c->setSelectedClusters(ks);
        }
    }
    else {
        QWidget::keyPressEvent(evt);
    }
}

void MVHistogramGrid::setHorizontalScaleAxis(HorizontalScaleAxisData X)
{
    d->m_horizontal_scale_axis_data = X;
}

void MVHistogramGrid::setHistogramViews(const QList<HistogramView*> views)
{
    MVGridView::clearViews();
    foreach (HistogramView* V, views) {
        MVGridView::addView(V);
    }

    for (int jj = 0; jj < MVGridView::viewCount(); jj++) {
        HistogramView* HV = qobject_cast<HistogramView*>(MVGridView::view(jj));
        connect(HV, SIGNAL(clicked(Qt::KeyboardModifiers)), this, SLOT(slot_histogram_view_clicked(Qt::KeyboardModifiers)));
        //connect(HV, SIGNAL(activated()), this, SLOT(slot_histogram_view_activated()));
        connect(HV, SIGNAL(activated(QPoint)), this, SLOT(slot_context_menu(QPoint)));
        //connect(HV, SIGNAL(signalExportHistogramMatrixImage()), this, SLOT(slot_export_image()));
    }
    d->update_layout();

    d->do_highlighting_and_captions();
}

QList<HistogramView*> MVHistogramGrid::histogramViews()
{
    QList<HistogramView*> ret;
    for (int jj = 0; jj < MVGridView::viewCount(); jj++) {
        HistogramView* HV = qobject_cast<HistogramView*>(MVGridView::view(jj));
        ret << HV;
    }
    return ret;
}

QStringList cluster_pair_set_to_string_list(const QSet<ClusterPair>& pairs)
{
    QStringList ret;
    QList<ClusterPair> list = pairs.toList();
    qSort(list);
    foreach (ClusterPair pair, list) {
        ret << pair.toString();
    }
    return ret;
}

void MVHistogramGrid::prepareMimeData(QMimeData& mimeData, const QPoint& pos)
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    if (this->pairMode()) {
        ds << cluster_pair_set_to_string_list(c->selectedClusterPairs());
        mimeData.setData("application/x-msv-cluster-pairs", ba); // selected cluster pairs data
    }
    else {
        ds << c->selectedClusters();
        mimeData.setData("application/x-msv-clusters", ba); // selected cluster data
    }
    MVAbstractView::prepareMimeData(mimeData, pos);
}

void MVHistogramGrid::setPairMode(bool val)
{
    d->m_pair_mode = val;
}

bool MVHistogramGrid::pairMode() const
{
    return d->m_pair_mode;
}

void MVHistogramGrid::slot_histogram_view_clicked(Qt::KeyboardModifiers modifiers)
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    if (d->m_pair_mode) {
        int k1 = sender()->property("k1").toInt();
        int k2 = sender()->property("k2").toInt();
        c->clickClusterPair(ClusterPair(k1, k2), modifiers);
    }
    else {
        int k = sender()->property("k").toInt();
        if (modifiers & Qt::ControlModifier) {
            c->clickCluster(k, Qt::ControlModifier);
        }
        else if (modifiers & Qt::ShiftModifier) {
            int k0 = c->currentCluster();
            d->shift_select_clusters_between(k0, k);
        }
        else {
            c->clickCluster(k, Qt::NoModifier);
        }
    }
}

void MVHistogramGrid::slot_cluster_attributes_changed(int cluster_number)
{
    Q_UNUSED(cluster_number)
    if (!this->pairMode())
        d->do_highlighting_and_captions();
}

void MVHistogramGrid::slot_cluster_pair_attributes_changed(ClusterPair pair)
{
    Q_UNUSED(pair)
    if (this->pairMode())
        d->do_highlighting_and_captions();
}

void MVHistogramGrid::slot_update_highlighting()
{
    d->do_highlighting_and_captions();
}

void MVHistogramGrid::slot_context_menu(const QPoint& pt)
{
    // IMO this code doesn't work correctly as the "activated" signal
    // doesn't "select" the HV thus we're being triggered over a widget
    // that is different than the selected HV

    HistogramView* HV = qobject_cast<HistogramView*>(sender());
    if (!HV)
        return;
    // send yourself a context menu request
    QContextMenuEvent e(QContextMenuEvent::Mouse, mapFromGlobal(HV->mapToGlobal(pt)));
    QCoreApplication::sendEvent(this, &e);
}

void MVHistogramGridPrivate::do_highlighting_and_captions()
{
    MVContext* c = qobject_cast<MVContext*>(q->mvContext());
    Q_ASSERT(c);

    QList<int> selected_clusters = c->selectedClusters();
    QSet<ClusterPair> selected_cluster_pairs = c->selectedClusterPairs();
    QList<HistogramView*> histogram_views = q->histogramViews();
    for (int i = 0; i < histogram_views.count(); i++) {
        HistogramView* HV = histogram_views[i];
        QString title0, caption0;
        if (m_pair_mode) {
            int k1 = HV->property("k1").toInt();
            int k2 = HV->property("k2").toInt();
            if ((k1) && (k2)) {
                HV->setSelected(selected_cluster_pairs.contains(ClusterPair(k1, k2)));
                //HV->setSelected((selected_clusters.contains(k1)) && (selected_clusters.contains(k2)));
            }
            title0 = QString("%1/%2").arg(k1).arg(k2);
            caption0 = c->clusterPairTagsList(ClusterPair(k1, k2)).join(", ");
        }
        else {
            int k = HV->property("k").toInt();
            if (k == c->currentCluster()) {
                HV->setCurrent(true);
            }
            else {
                HV->setCurrent(false);
            }
            if (selected_clusters.contains(k)) {
                HV->setSelected(true);
            }
            else {
                HV->setSelected(false);
            }
            title0 = QString("%1").arg(k);
            caption0 = c->clusterTagsList(k).join(", ");
        }
        HV->setTitle(title0);
        HV->setCaption(caption0);
    }
}

void MVHistogramGridPrivate::shift_select_clusters_between(int kA, int kB)
{
    MVContext* c = qobject_cast<MVContext*>(q->mvContext());
    Q_ASSERT(c);

    QList<HistogramView*> histogram_views = q->histogramViews();
    if (!m_pair_mode) {
        /// TODO (low) handle pair mode case
        QSet<int> selected_clusters = c->selectedClusters().toSet();
        int ind1 = find_view_index_for_k(kA);
        int ind2 = find_view_index_for_k(kB);
        if ((ind1 >= 0) && (ind2 >= 0)) {
            for (int ii = qMin(ind1, ind2); ii <= qMax(ind1, ind2); ii++) {
                selected_clusters.insert(histogram_views[ii]->property("k2").toInt());
            }
        }
        else if (ind1 >= 0) {
            selected_clusters.insert(histogram_views[ind1]->property("k2").toInt());
        }
        else if (ind2 >= 0) {
            selected_clusters.insert(histogram_views[ind2]->property("k2").toInt());
        }
        c->setSelectedClusters(QList<int>::fromSet(selected_clusters));
    }
}

void MVHistogramGridPrivate::update_layout()
{
    ((MVGridView*)q)->updateLayout();

    if (m_horizontal_scale_axis_data.use_it) {
        HorizontalScaleAxis* HSA = m_horizontal_scale_axis;
        HSA->m_data = m_horizontal_scale_axis_data;
        ((MVGridView*)q)->setHorizontalScaleWidget(HSA);
    }
}

int MVHistogramGridPrivate::find_view_index_for_k(int k)
{
    QList<HistogramView*> histogram_views = q->histogramViews();
    for (int i = 0; i < histogram_views.count(); i++) {
        if (histogram_views[i]->property("k").toInt() == k)
            return i;
    }
    return -1;
}

HorizontalScaleAxis::HorizontalScaleAxis()
{
    setFixedHeight(25);
}

void HorizontalScaleAxis::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)
    QPainter painter(this);
    QPen pen = painter.pen();
    pen.setColor(Qt::black);
    painter.setPen(pen);

    int W0 = width();
    //int H0=height();
    int H1 = 8;
    int margin1 = 6;
    int len1 = 6;
    QPointF pt1(W0 / 2, H1);
    QPointF pt2(W0 - margin1, H1);
    QPointF pt3(W0 / 2, H1 - len1);
    QPointF pt4(W0 - margin1, H1 - len1);
    painter.drawLine(pt1, pt2);
    painter.drawLine(pt1, pt3);
    painter.drawLine(pt2, pt4);

    QFont font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);
    QRect text_box(W0 / 2, H1 + 3, W0 / 2, H1 + 3);
    QString txt = QString("%1").arg(m_data.label);
    painter.drawText(text_box, txt, Qt::AlignCenter | Qt::AlignTop);
}
