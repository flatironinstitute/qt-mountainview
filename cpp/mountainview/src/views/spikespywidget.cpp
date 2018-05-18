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

#include "mvmainwindow.h"
#include "mvtimeseriesview2.h"
#include "spikespywidget.h"
#include "taskprogressview.h"
#include "mvcontext.h"
#include <QHBoxLayout>
#include <QSplitter>
#include <QMenuBar>

class SpikeSpyWidgetPrivate {
public:
    SpikeSpyWidget* q;
    QList<SpikeSpyViewData> m_datas;
    QList<MVTimeSeriesView2*> m_views;
    MVContext* m_context;
    int m_current_view_index;

    QSplitter* m_splitter;
    TaskProgressView* m_task_progress_view;

    QMenuBar* m_menu_bar;

    void update_activated();
};

SpikeSpyWidget::SpikeSpyWidget(MVAbstractContext* context)
{
    d = new SpikeSpyWidgetPrivate;
    d->q = this;

    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    d->m_context = c;
    d->m_current_view_index = 0;

    d->m_splitter = new QSplitter(Qt::Vertical);
    d->m_task_progress_view = new TaskProgressView;
    d->m_task_progress_view->setWindowFlags(Qt::Tool);

    QMenuBar* menubar = new QMenuBar(this);
    d->m_menu_bar = menubar;
    { //Tools
        QMenu* menu = new QMenu("Tools", menubar);
        d->m_menu_bar->addMenu(menu);
        {
            QAction* A = new QAction("Open MountainView", menu);
            menu->addAction(A);
            A->setObjectName("open_mountainview");
            A->setShortcut(QKeySequence("Ctrl+M"));
            connect(A, SIGNAL(triggered()), this, SLOT(slot_open_mountainview()));
        }
        {
            QAction* A = new QAction("Show tasks (for debugging)", menu);
            menu->addAction(A);
            A->setObjectName("show_tasks");
            A->setShortcut(QKeySequence("Ctrl+D"));
            connect(A, SIGNAL(triggered()), this, SLOT(slot_show_tasks()));
        }
    }

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(d->m_menu_bar);
    layout->addWidget(d->m_splitter);
    d->m_splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setLayout(layout);
}

SpikeSpyWidget::~SpikeSpyWidget()
{
    delete d->m_task_progress_view;
    delete d;
}

void SpikeSpyWidget::addView(const SpikeSpyViewData& data)
{
    Q_UNUSED(data)
    MVTimeSeriesView2* W = new MVTimeSeriesView2(d->m_context);
    /// TODO restore functionality of spikespywidget
    /*
    W->setTimeseries(data.timeseries);
    QVector<double> times;
    QVector<int> labels;
    for (int i = 0; i < data.firings.N2(); i++) {
        times << data.firings.value(1, i);
        labels << (int)data.firings.value(2, i);
    }
    W->setTimesLabels(times, labels);
    d->m_splitter->addWidget(W);
    d->m_views << W;
    d->m_datas << data;
    */

    connect(W, SIGNAL(clicked()), this, SLOT(slot_view_clicked()));

    d->update_activated();
}

void SpikeSpyWidget::slot_show_tasks()
{
    d->m_task_progress_view->show();
    d->m_task_progress_view->raise();
}

void SpikeSpyWidget::slot_open_mountainview()
{
    int current_view_index = d->m_current_view_index;
    if (current_view_index >= d->m_datas.count())
        return;
    SpikeSpyViewData data = d->m_datas[current_view_index];

    /// TODO restore functionality of spikespywidget
    /*
    MVMainWindow* W = new MVMainWindow(d->m_context);
    MVFile ff;
    ff.addTimeseriesPath("Timeseries", data.timeseries.makePath());
    ff.setFiringsPath(data.firings.makePath());
    W->setMVFile(ff);
    W->setDefaultInitialization();
    W->show();
    W->setGeometry(this->geometry().adjusted(50, 50, 50, 50));
    */
}

void SpikeSpyWidget::slot_view_clicked()
{
    for (int i = 0; i < d->m_views.count(); i++) {
        if (d->m_views[i] == sender()) {
            d->m_current_view_index = i;
        }
    }
    d->update_activated();
}

void SpikeSpyWidgetPrivate::update_activated()
{
    for (int i = 0; i < m_views.count(); i++) {
        m_views[i]->setActivated(i == m_current_view_index);
    }
}
