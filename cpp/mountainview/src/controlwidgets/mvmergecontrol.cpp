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

#include "mvmergecontrol.h"

#include <QToolButton>
#include <flowlayout.h>
#include <mvcontext.h>

class MVMergeControlPrivate {
public:
    MVMergeControl* q;

    void do_merge_or_unmerge(bool merge);
};

MVMergeControl::MVMergeControl(MVAbstractContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVMergeControlPrivate;
    d->q = this;

    QHBoxLayout* layout1 = new QHBoxLayout;
    {
        QToolButton* B = this->createToolButtonControl("merge_selected", "Merge selected", this, SLOT(slot_merge_selected()));
        layout1->addWidget(B);
    }
    {
        QToolButton* B = this->createToolButtonControl("unmerge_selected", "Unmerge selected", this, SLOT(slot_unmerge_selected()));
        layout1->addWidget(B);
    }
    layout1->addStretch();

    QHBoxLayout* layout2 = new QHBoxLayout;
    {
        QCheckBox* CB = this->createCheckBoxControl("view_merged", "View merged");
        layout2->addWidget(CB);
    }
    layout2->addStretch();

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addLayout(layout1);
    vlayout->addLayout(layout2);

    this->setLayout(vlayout);

    this->updateControlsOn(context, SIGNAL(viewMergedChanged()));
    this->updateControlsOn(context, SIGNAL(selectedClusterPairsChanged()));
    this->updateControlsOn(context, SIGNAL(selectedClustersChanged()));
}

MVMergeControl::~MVMergeControl()
{
    delete d;
}

QString MVMergeControl::title() const
{
    return "Merge";
}

void MVMergeControl::updateContext()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    c->setViewMerged(this->controlValue("view_merged").toBool());
}

void MVMergeControl::updateControls()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    bool can_merge_or_unmerge = false;
    if (!c->selectedClusterPairs().isEmpty()) {
        can_merge_or_unmerge = true;
    }
    if (c->selectedClusters().count() >= 2) {
        can_merge_or_unmerge = true;
    }
    this->setControlEnabled("merge_selected", can_merge_or_unmerge);
    this->setControlEnabled("unmerge_selected", can_merge_or_unmerge);

    this->setControlValue("view_merged", c->viewMerged());
}

void MVMergeControl::slot_merge_selected()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    d->do_merge_or_unmerge(true);
    c->setViewMerged(true);
}

void MVMergeControl::slot_unmerge_selected()
{
    d->do_merge_or_unmerge(false);
}

void MVMergeControlPrivate::do_merge_or_unmerge(bool merge)
{
    MVContext* c = qobject_cast<MVContext*>(q->mvContext());
    Q_ASSERT(c);

    QSet<ClusterPair> selected_cluster_pairs = c->selectedClusterPairs();
    if (selected_cluster_pairs.count() >= 1) {
        foreach (ClusterPair pair, selected_cluster_pairs) {
            QSet<QString> tags = c->clusterPairTags(pair);
            if (merge)
                tags.insert("merged");
            else
                tags.remove("merged");
            c->setClusterPairTags(pair, tags);
        }
    }
    else {
        QList<int> selected_clusters = c->selectedClusters();
        //we need to do something special since it is overkill to merge every pair -- and expensive for large # clusters
        if (merge) {
            ClusterMerge CM = c->clusterMerge();
            for (int i1 = 0; i1 < selected_clusters.count() - 1; i1++) {
                int i2 = i1 + 1;
                ClusterPair pair(selected_clusters[i1], selected_clusters[i2]);
                if (CM.representativeLabel(pair.k1()) != CM.representativeLabel(pair.k2())) {
                    //not already merged
                    QSet<QString> tags = c->clusterPairTags(pair);
                    tags.insert("merged");
                    c->setClusterPairTags(pair, tags);
                    QSet<int> tmp;
                    tmp.insert(pair.k1());
                    tmp.insert(pair.k2());
                    CM.merge(tmp);
                }
            }
        }
        else {
            QSet<int> selected_clusters_set = selected_clusters.toSet();
            QList<ClusterPair> keys = c->clusterPairAttributesKeys();
            foreach (ClusterPair pair, keys) {
                if ((selected_clusters_set.contains(pair.k1())) || (selected_clusters_set.contains(pair.k2()))) {
                    QSet<QString> tags = c->clusterPairTags(pair);
                    tags.remove("merged");
                    c->setClusterPairTags(pair, tags);
                }
            }
        }
    }
}
