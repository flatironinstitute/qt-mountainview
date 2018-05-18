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

#include "clusterpairmetricsview.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QTreeWidget>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include "actionfactory.h"

class ClusterPairMetricsViewPrivate {
public:
    ClusterPairMetricsView* q;
    QTreeWidget* m_tree;

    void refresh_tree();
};

ClusterPairMetricsView::ClusterPairMetricsView(MVAbstractContext* mvcontext)
    : MVAbstractView(mvcontext)
{
    d = new ClusterPairMetricsViewPrivate;
    d->q = this;

    QHBoxLayout* hlayout = new QHBoxLayout;
    this->setLayout(hlayout);

    d->m_tree = new QTreeWidget;
    d->m_tree->setSortingEnabled(true);
    d->m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    hlayout->addWidget(d->m_tree);

    //this->recalculateOn(mvContext(), SIGNAL(clusterPairAttributesChanged(int)), false);
    //this->recalculateOn(mvContext(), SIGNAL(clusterVisibilityChanged()), false);

    d->refresh_tree();
    this->recalculate();

    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    QObject::connect(d->m_tree, SIGNAL(itemSelectionChanged()), this, SLOT(slot_item_selection_changed()));
    QObject::connect(d->m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(slot_current_item_changed()));
    QObject::connect(c, SIGNAL(currentClusterPairChanged()), this, SLOT(slot_update_current_cluster_pair()));
    QObject::connect(c, SIGNAL(selectedClusterPairsChanged()), this, SLOT(slot_update_selected_cluster_pairs()));
}

ClusterPairMetricsView::~ClusterPairMetricsView()
{
    this->stopCalculation();
    delete d;
}

void ClusterPairMetricsView::prepareCalculation()
{
    if (!mvContext())
        return;
}

void ClusterPairMetricsView::runCalculation()
{
}

void ClusterPairMetricsView::onCalculationFinished()
{
    d->refresh_tree();
}

void ClusterPairMetricsView::keyPressEvent(QKeyEvent* evt)
{
    Q_UNUSED(evt)
}

void ClusterPairMetricsView::prepareMimeData(QMimeData& mimeData, const QPoint& pos)
{
    (void)mimeData;
    (void)pos;
    /*
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << mvContext()->selectedClusterPairs();
    mimeData.setData("application/x-msv-clusters", ba); // selected cluster data

    MVAbstractView::prepareMimeData(mimeData, pos); // call base class implementation
    */
}

void ClusterPairMetricsView::slot_current_item_changed()
{
    QTreeWidgetItem* it = d->m_tree->currentItem();
    if (it) {
        //int k1 = it->data(0, Qt::UserRole).toInt();
        //int k2 = it->data(0, Qt::UserRole + 1).toInt();
        //mvContext()->setCurrentClusterPair(k1,k2);
    }
    else {
        //mvContext()->setCurrentCluster(-1);
    }
}

void ClusterPairMetricsView::slot_item_selection_changed()
{
    //QList<QTreeWidgetItem*> items=d->m_tree->selectedItems();

    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    QSet<ClusterPair> selected = c->selectedClusterPairs();

    for (int i = 0; i < d->m_tree->topLevelItemCount(); i++) {
        QTreeWidgetItem* it = d->m_tree->topLevelItem(i);
        int k1 = it->data(0, Qt::UserRole).toInt();
        int k2 = it->data(0, Qt::UserRole + 1).toInt();
        if (it->isSelected()) {
            selected.insert(ClusterPair(k1, k2));
        }
        else {
            selected.remove(ClusterPair(k1, k2));
        }
    }

    c->setSelectedClusterPairs(selected);
}

void ClusterPairMetricsView::slot_update_current_cluster_pair()
{
    //int current = mvContext()->currentClusterPair();

    /*
    for (int i = 0; i < d->m_tree->topLevelItemCount(); i++) {
        QTreeWidgetItem* it = d->m_tree->topLevelItem(i);
        int k = it->data(0, Qt::UserRole).toInt();
        if (k == current) {
            d->m_tree->setCurrentItem(it);
        }
    }
    */
}

void ClusterPairMetricsView::slot_update_selected_cluster_pairs()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    QSet<ClusterPair> selected = c->selectedClusterPairs();

    for (int i = 0; i < d->m_tree->topLevelItemCount(); i++) {
        QTreeWidgetItem* it = d->m_tree->topLevelItem(i);
        int k1 = it->data(0, Qt::UserRole).toInt();
        int k2 = it->data(0, Qt::UserRole + 1).toInt();
        it->setSelected(selected.contains(ClusterPair(k1, k2)));
    }
}

class NumericSortTreeWidgetItem : public QTreeWidgetItem {
public:
    NumericSortTreeWidgetItem(QTreeWidget* parent)
        : QTreeWidgetItem(parent)
    {
    }

private:
    bool operator<(const QTreeWidgetItem& other) const
    {
        int column = treeWidget()->sortColumn();
        return text(column).toDouble() > other.text(column).toDouble();
    }
};

void ClusterPairMetricsViewPrivate::refresh_tree()
{
    m_tree->clear();

    MVContext* c = qobject_cast<MVContext*>(q->mvContext());
    Q_ASSERT(c);

    QList<ClusterPair> keys = c->clusterPairAttributesKeys();

    QSet<QString> metric_names_set;
    for (int ii = 0; ii < keys.count(); ii++) {
        QJsonObject metrics = c->clusterPairAttributes(keys[ii])["metrics"].toObject();
        QStringList nnn = metrics.keys();
        foreach (QString name, nnn) {
            metric_names_set.insert(name);
        }
    }

    QStringList metric_names = metric_names_set.toList();
    qSort(metric_names);

    QStringList headers;
    headers << "Cluster1"
            << "Cluster2";
    headers.append(metric_names);
    m_tree->setHeaderLabels(headers);

    for (int ii = 0; ii < keys.count(); ii++) {
        ClusterPair pair = keys[ii];
        //if (q->mvContext()->clusterPairIsVisible(pair)) {
        NumericSortTreeWidgetItem* it = new NumericSortTreeWidgetItem(m_tree);
        QJsonObject metrics = c->clusterPairAttributes(pair)["metrics"].toObject();
        it->setText(0, QString("%1").arg(pair.k1()));
        it->setText(1, QString("%2").arg(pair.k2()));
        it->setData(0, Qt::UserRole, pair.k1());
        it->setData(0, Qt::UserRole, pair.k2());
        for (int j = 0; j < metric_names.count(); j++) {
            it->setText(j + 2, QString("%1").arg(metrics[metric_names[j]].toDouble()));
        }
        m_tree->addTopLevelItem(it);
        //}
    }

    for (int c = 0; c < m_tree->columnCount(); c++) {
        m_tree->resizeColumnToContents(c);
    }
}
