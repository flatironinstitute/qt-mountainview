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

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <flowlayout.h>
#include <QLineEdit>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include <QThread>
#include "guidev2.h"

#include "mv_compute_templates.h"
#include "individualmergedecisionpage.h"

class GuideV2Private {
public:
    GuideV2* q;

    MVContext* m_context;
    MVMainWindow* m_main_window;

    QWizardPage* make_page_1();
    QWizardPage* make_page_2();
    QWizardPage* make_page_3();
    QWizardPage* make_page_4();
    QWizardPage* make_page_5();
    QWizardPage* make_page_7();

    QAbstractButton* make_instructions_button(QString text, QString instructions);
    QAbstractButton* make_open_view_button(QString text, QString view_id, QString container_name);

    void show_instructions(QString title, QString instructions);
};

GuideV2::GuideV2(MVContext* mvcontext, MVMainWindow* mw)
{
    d = new GuideV2Private;
    d->q = this;
    d->m_context = mvcontext;
    d->m_main_window = mw;

    this->addPage(d->make_page_1());
    this->addPage(d->make_page_2());
    this->addPage(d->make_page_3());
    this->addPage(d->make_page_4());
    this->addPage(d->make_page_5());
    this->addPage(new IndividualMergeDecisionPage(mvcontext, mw));
    this->addPage(d->make_page_7());

    this->resize(800, 600);

    this->setWindowTitle("Guide Version 2");
}

GuideV2::~GuideV2()
{
    delete d;
}

void GuideV2::slot_button_clicked()
{
    QString action = sender()->property("action").toString();
    if (action == "open_view") {
        d->m_main_window->setCurrentContainerName(sender()->property("container-name").toString());
        d->m_main_window->openView(sender()->property("view-id").toString());
    }
    else if (action == "show_instructions") {
        d->show_instructions(sender()->property("title").toString(), sender()->property("instructions").toString());
    }
}

void GuideV2::slot_select_merge_candidates()
{
    QSet<ClusterPair> pairs;
    QList<ClusterPair> keys = d->m_context->clusterPairAttributesKeys();
    foreach (ClusterPair key, keys) {
        if (d->m_context->clusterPairTags(key).contains("merge_candidate")) {
            pairs.insert(key);
        }
    }
    d->m_context->setSelectedClusterPairs(pairs);
}

void GuideV2::slot_merge_all_merge_candidates()
{
    QSet<ClusterPair> pairs;
    QList<ClusterPair> keys = d->m_context->clusterPairAttributesKeys();
    foreach (ClusterPair key, keys) {
        if (d->m_context->clusterPairTags(key).contains("merge_candidate")) {
            pairs.insert(key);
        }
    }
    foreach (ClusterPair pair, pairs) {
        QSet<QString> tags = d->m_context->clusterPairTags(pair);
        tags.remove("merge_candidate");
        tags.insert("merged");
        d->m_context->setClusterPairTags(pair, tags);
    }
    d->m_context->setViewMerged(true);
}

QWizardPage* GuideV2Private::make_page_1()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Cluster detail view");

    QLabel* label = new QLabel(TextFile::read(":/guides/guidev2/page_cluster_details.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    FlowLayout* flayout = new FlowLayout;
    flayout->addWidget(make_open_view_button("Cluster details", "open-cluster-details", "north"));
    layout->addLayout(flayout);

    return page;
}

QWizardPage* GuideV2Private::make_page_2()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Event Filter");

    QLabel* label = new QLabel(TextFile::read(":/guides/guidev2/page_event_filter.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    //FlowLayout* flayout = new FlowLayout;
    //flayout->addWidget(make_open_view_button("Cluster details", "open-cluster-details", "north"));
    //layout->addLayout(flayout);

    return page;
}

QWizardPage* GuideV2Private::make_page_3()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Cluster detail view");

    QLabel* label = new QLabel(TextFile::read(":/guides/guidev2/page_auto_correlograms.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    FlowLayout* flayout = new FlowLayout;
    flayout->addWidget(make_open_view_button("Auto-correlograms", "open-auto-correlograms", "south"));
    layout->addLayout(flayout);

    return page;
}

QWizardPage* GuideV2Private::make_page_4()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Discrimination histograms");

    QLabel* label = new QLabel(TextFile::read(":/guides/guidev2/page_discrim_hist.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    FlowLayout* flayout = new FlowLayout;
    flayout->addWidget(make_open_view_button("Discrimination histograms", "open-discrim-histograms-guide", "south"));
    layout->addLayout(flayout);

    return page;
}

QWizardPage* GuideV2Private::make_page_5()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Cross-correlograms");

    QLabel* label = new QLabel(TextFile::read(":/guides/guidev2/page_cross_correlograms.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    FlowLayout* flayout = new FlowLayout;
    {
        QPushButton* B = new QPushButton("Select merge candidates");
        QObject::connect(B, SIGNAL(clicked(bool)), q, SLOT(slot_select_merge_candidates()));
        flayout->addWidget(B);
    }
    flayout->addWidget(make_open_view_button("Cross-correlograms", "open-selected-cross-correlograms", "south"));
    layout->addLayout(flayout);

    return page;
}

QWizardPage* GuideV2Private::make_page_7()
{
    QWizardPage* page = new QWizardPage;
    page->setTitle("Merge");

    QLabel* label = new QLabel(TextFile::read(":/guides/guidev2/merge.txt"));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(label);
    page->setLayout(layout);

    FlowLayout* flayout = new FlowLayout;
    {
        QPushButton* B = new QPushButton("Merge all merge candidates");
        QObject::connect(B, SIGNAL(clicked(bool)), q, SLOT(slot_merge_all_merge_candidates()));
        flayout->addWidget(B);
    }

    layout->addLayout(flayout);

    return page;
}

QAbstractButton* GuideV2Private::make_instructions_button(QString text, QString instructions)
{
    QPushButton* B = new QPushButton(text);
    B->setProperty("action", "show_instructions");
    B->setProperty("instructions", instructions);
    QObject::connect(B, SIGNAL(clicked(bool)), q, SLOT(slot_button_clicked()));
    return B;
}

QAbstractButton* GuideV2Private::make_open_view_button(QString text, QString view_id, QString container_name)
{
    QPushButton* B = new QPushButton(text);
    B->setProperty("action", "open_view");
    B->setProperty("view-id", view_id);
    B->setProperty("container-name", container_name);
    QObject::connect(B, SIGNAL(clicked(bool)), q, SLOT(slot_button_clicked()));
    return B;
}

void GuideV2Private::show_instructions(QString title, QString instructions)
{
    QMessageBox::information(q, title, instructions);
}
