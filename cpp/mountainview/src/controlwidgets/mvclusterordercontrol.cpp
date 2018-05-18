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

#include "flowlayout.h"
#include "mvclusterordercontrol.h"
#include "mlcommon.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QPushButton>
#include <QTimer>
#include <QFileDialog>
#include <QSettings>
#include <QJsonDocument>
#include <mvmainwindow.h>
#include <QMessageBox>
#include <QTextBrowser>
#include <mountainprocessrunner.h>
#include <computationthread.h>
#include <mvcontext.h>
#include "taskprogress.h"

class MVClusterOrderComputationThread : public ComputationThread {
public:
    //input
    QVariantMap params;
    //output
    QString cluster_scores_path;
    QString cluster_pair_scores_path;
    void compute();
};

class MVClusterOrderControlPrivate {
public:
    MVClusterOrderControl* q;
    MVClusterOrderComputationThread m_computation_thread;
};

MVClusterOrderControl::MVClusterOrderControl(MVAbstractContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVClusterOrderControlPrivate;
    d->q = this;

    QObject::connect(&d->m_computation_thread, SIGNAL(computationFinished()), this, SLOT(slot_computation_finished()));

    FlowLayout* flayout = new FlowLayout;
    this->setLayout(flayout);
    {
        QPushButton* B = new QPushButton("Order by detectability");
        connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_order_by_detectability()));
        flayout->addWidget(B);
    }

    updateControls();
}

MVClusterOrderControl::~MVClusterOrderControl()
{
    delete d;
}

QString MVClusterOrderControl::title() const
{
    return "Cluster Order";
}

void MVClusterOrderControl::updateContext()
{
}

void MVClusterOrderControl::updateControls()
{
}

void MVClusterOrderControl::slot_order_by_detectability()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    if (d->m_computation_thread.isRunning()) {
        d->m_computation_thread.stopComputation();
    }

    QVariantMap params;
    params["timeseries"] = c->currentTimeseries().makePath();
    params["firings"] = c->firings().makePath();
    params["clip_size"] = 80;
    params["detect_threshold"] = c->option("amp_thresh_display", 0).toDouble();
    params["add_noise_level"] = 1;
    params["cluster_scores_only"] = 1;
    d->m_computation_thread.params = params;
    d->m_computation_thread.startComputation();
}

void MVClusterOrderControl::slot_computation_finished()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    d->m_computation_thread.stopComputation(); //paranoid
    DiskReadMda cluster_scores(d->m_computation_thread.cluster_scores_path);
    QList<double> scores0;
    for (int i = 0; i < cluster_scores.N2(); i++) {
        scores0 << cluster_scores.value(1, i); //are we sure it will be in the right order?
    }
    c->setClusterOrderScores("detectability", scores0);
}

void MVClusterOrderComputationThread::compute()
{
    TaskProgress task("Cluster order");
    MountainProcessRunner MPR;
    MPR.setProcessorName("cluster_scores");
    MPR.setInputParameters(params);
    cluster_scores_path = MPR.makeOutputFilePath("cluster_scores");
    cluster_pair_scores_path = MPR.makeOutputFilePath("cluster_pair_scores");
    MPR.runProcess();
}
