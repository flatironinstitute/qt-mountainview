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

#include "individualmergedecisionpage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <flowlayout.h>

class IndividualMergeDecisionPagePrivate {
public:
    IndividualMergeDecisionPage* q;
    MVContext* m_context;
    MVMainWindow* m_main_window;

    QLineEdit* m_pair_edit;
    QAbstractButton* m_previous_button;
    QAbstractButton* m_next_button;

    QList<ClusterPair> m_cluster_pairs;
    int m_current_cluster_pair_index = -1;

    void update_controls();
    void update_candidate_pairs();

    QAbstractButton* make_select_and_open_view_button(QString text, QString view_id, QString container_name);
};

IndividualMergeDecisionPage::IndividualMergeDecisionPage(MVContext* context, MVMainWindow* mw)
{
    d = new IndividualMergeDecisionPagePrivate;
    d->q = this;
    d->m_context = context;
    d->m_main_window = mw;

    this->setTitle("Individual merge decisions");

    QVBoxLayout* layout = new QVBoxLayout;

    QLabel* label = new QLabel(TextFile::read(":/guides/guidev2/page_individual_merge_decisions.txt"));
    label->setWordWrap(true);
    layout->addWidget(label);

    {
        QHBoxLayout* hlayout = new QHBoxLayout;
        hlayout->addStretch();
        {
            d->m_pair_edit = new QLineEdit;
            d->m_pair_edit->setReadOnly(true);
            hlayout->addWidget(d->m_pair_edit);
        }
        {
            QPushButton* B = new QPushButton("Update candidates");
            QObject::connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_update_candidates()));
            hlayout->addWidget(B);
        }
        {
            QPushButton* B = new QPushButton("Previous");
            QObject::connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_goto_previous_pair()));
            B->setEnabled(false);
            hlayout->addWidget(B);
            d->m_previous_button = B;
        }
        {
            QPushButton* B = new QPushButton("Next");
            QObject::connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_goto_next_pair()));
            B->setEnabled(false);
            hlayout->addWidget(B);
            d->m_next_button = B;
        }
        hlayout->addStretch();
        layout->addLayout(hlayout);
    }

    FlowLayout* flayout = new FlowLayout;
    {
        flayout->addWidget(d->make_select_and_open_view_button("Spike spray", "open-spike-spray", "north"));
    }
    {
        flayout->addWidget(d->make_select_and_open_view_button("Firing events", "open-firing-events", "south"));
    }
    {
        flayout->addWidget(d->make_select_and_open_view_button("PCA Features", "open-pca-features", ""));
    }
    {
        flayout->addWidget(d->make_select_and_open_view_button("Channel Features", "open-channel-features", ""));
    }
    {
        flayout->addWidget(d->make_select_and_open_view_button("Cross-correlograms", "open-selected-cross-correlograms", ""));
    }
    layout->addLayout(flayout);

    {
        QHBoxLayout* hlayout = new QHBoxLayout;
        hlayout->addStretch();
        {
            QPushButton* B = new QPushButton("Add \"merge_candidate\" tag");
            QObject::connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_add_merge_candidate_tag()));
            hlayout->addWidget(B);
        }
        {
            QPushButton* B = new QPushButton("Remove \"merge_candidate\" tag");
            QObject::connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_remove_merge_candidate_tag()));
            hlayout->addWidget(B);
        }
        {
            QPushButton* B = new QPushButton("Add \"mua\" and \"reject\" tags");
            QObject::connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_add_mua_and_reject_tags()));
            hlayout->addWidget(B);
        }
        hlayout->addStretch();
        layout->addLayout(hlayout);
    }

    this->setLayout(layout);
}

IndividualMergeDecisionPage::~IndividualMergeDecisionPage()
{
    delete d;
}

void IndividualMergeDecisionPage::slot_goto_previous_pair()
{
    if (d->m_current_cluster_pair_index <= 0)
        return;
    d->m_current_cluster_pair_index--;
    d->update_controls();
}

void IndividualMergeDecisionPage::slot_goto_next_pair()
{
    if (d->m_current_cluster_pair_index >= d->m_cluster_pairs.count())
        return;
    d->m_current_cluster_pair_index++;
    d->update_controls();
}

void IndividualMergeDecisionPage::slot_button_clicked()
{
    QString action = sender()->property("action").toString();
    if (action == "open_view") {
        d->m_main_window->setCurrentContainerName(sender()->property("container-name").toString());
        d->m_main_window->openView(sender()->property("view-id").toString());
    }
    else if (action == "select_and_open_view") {
        d->m_context->clickClusterPair(d->m_cluster_pairs.value(d->m_current_cluster_pair_index), Qt::NoModifier);
        d->m_main_window->setCurrentContainerName(sender()->property("container-name").toString());
        d->m_main_window->openView(sender()->property("view-id").toString());
    }
}

void IndividualMergeDecisionPage::slot_add_merge_candidate_tag()
{
    ClusterPair pair = d->m_cluster_pairs.value(d->m_current_cluster_pair_index);
    QSet<QString> tags = d->m_context->clusterPairTags(pair);
    if ((pair.k1()) && (pair.k2())) {
        tags.insert("merge_candidate");
    }
    d->m_context->setClusterPairTags(pair, tags);
}

void IndividualMergeDecisionPage::slot_remove_merge_candidate_tag()
{
    ClusterPair pair = d->m_cluster_pairs.value(d->m_current_cluster_pair_index);
    QSet<QString> tags = d->m_context->clusterPairTags(pair);
    if ((pair.k1()) && (pair.k2())) {
        tags.remove("merge_candidate");
    }
    d->m_context->setClusterPairTags(pair, tags);
}

void IndividualMergeDecisionPage::slot_add_mua_and_reject_tags()
{
    ClusterPair pair = d->m_cluster_pairs.value(d->m_current_cluster_pair_index);
    QList<int> ks;
    ks << pair.k1() << pair.k2();
    foreach (int k, ks) {
        QSet<QString> tags = d->m_context->clusterTags(k);
        tags.insert("mua");
        tags.insert("reject");
        d->m_context->setClusterTags(k, tags);
    }
}

void IndividualMergeDecisionPage::slot_update_candidates()
{
    d->update_candidate_pairs();
}

void IndividualMergeDecisionPagePrivate::update_controls()
{
    ClusterPair pair = m_cluster_pairs.value(m_current_cluster_pair_index);
    if ((pair.k1()) && (pair.k2())) {
        m_pair_edit->setText(QString("%1/%2").arg(pair.k1()).arg(pair.k2()));
    }
    else {
        m_pair_edit->setText("");
    }
    m_previous_button->setEnabled(m_current_cluster_pair_index - 1 >= 0);
    m_next_button->setEnabled(m_current_cluster_pair_index + 1 < m_cluster_pairs.count());
}

void IndividualMergeDecisionPagePrivate::update_candidate_pairs()
{
    QList<ClusterPair> pairs;
    QList<ClusterPair> keys = m_context->clusterPairAttributesKeys();
    foreach (ClusterPair key, keys) {
        if (m_context->clusterPairTags(key).contains("merge_candidate")) {
            pairs << key;
        }
    }
    qSort(pairs);
    m_cluster_pairs = pairs;
    m_current_cluster_pair_index = 0;
    update_controls();
}

QAbstractButton* IndividualMergeDecisionPagePrivate::make_select_and_open_view_button(QString text, QString view_id, QString container_name)
{
    QPushButton* B = new QPushButton(text);
    B->setProperty("action", "select_and_open_view");
    B->setProperty("view-id", view_id);
    B->setProperty("container-name", container_name);
    QObject::connect(B, SIGNAL(clicked(bool)), q, SLOT(slot_button_clicked()));
    return B;
}
