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

#include "clustermetricsplugin.h"

#include "clustermetricsview.h"

#include "curationprogramview.h"

#include <QThread>
#include <clusterpairmetricsview.h>
#include <mountainprocessrunner.h>

class ClusterMetricsPluginPrivate {
public:
    ClusterMetricsPlugin* q;
};

ClusterMetricsPlugin::ClusterMetricsPlugin()
{
    d = new ClusterMetricsPluginPrivate;
    d->q = this;
}

ClusterMetricsPlugin::~ClusterMetricsPlugin()
{
    delete d;
}

QString ClusterMetricsPlugin::name()
{
    return "ClusterMetrics";
}

QString ClusterMetricsPlugin::description()
{
    return "";
}

void compute_basic_metrics(MVAbstractContext* mv_context);
void ClusterMetricsPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new ClusterMetricsFactory(mw));
    //mw->registerViewFactory(new ClusterPairMetricsFactory(mw));
    //compute_basic_metrics(mw->mvContext());
}

ClusterMetricsFactory::ClusterMetricsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString ClusterMetricsFactory::id() const
{
    return QStringLiteral("open-cluster-metrics");
}

QString ClusterMetricsFactory::name() const
{
    return tr("Cluster Metrics");
}

QString ClusterMetricsFactory::title() const
{
    return tr("Cluster Metrics");
}

MVAbstractView* ClusterMetricsFactory::createView(MVAbstractContext* context)
{
    ClusterMetricsView* X = new ClusterMetricsView(context);
    return X;
}

ClusterPairMetricsFactory::ClusterPairMetricsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString ClusterPairMetricsFactory::id() const
{
    return QStringLiteral("open-cluster-pair-metrics");
}

QString ClusterPairMetricsFactory::name() const
{
    return tr("Cluster Pair Metrics");
}

QString ClusterPairMetricsFactory::title() const
{
    return tr("Cluster Metrics");
}

MVAbstractView* ClusterPairMetricsFactory::createView(MVAbstractContext* context)
{
    ClusterPairMetricsView* X = new ClusterPairMetricsView(context);
    return X;
}

void compute_basic_metrics(MVAbstractContext* mv_context)
{
    MVContext* c = qobject_cast<MVContext*>(mv_context);
    Q_ASSERT(c);
    basic_metrics_calculator* thread = new basic_metrics_calculator;
    thread->timeseries = c->currentTimeseries().makePath();
    thread->firings = c->firings().makePath();
    thread->samplerate = c->sampleRate();
    thread->mv_context = c;
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(slot_on_finished()));
    thread->start();
}

void basic_metrics_calculator::run()
{
    MountainProcessRunner MPR;
    MPR.setProcessorName("basic_metrics");
    QMap<QString, QVariant> params;
    params["timeseries"] = timeseries;
    params["firings"] = firings;
    params["samplerate"] = samplerate;
    MPR.setInputParameters(params);
    cluster_metrics_path = MPR.makeOutputFilePath("cluster_metrics");
    cluster_pair_metrics_path = MPR.makeOutputFilePath("cluster_pair_metrics");
    MPR.runProcess();
}

void basic_metrics_calculator::slot_on_finished()
{
    mv_context->loadClusterMetricsFromFile(cluster_metrics_path);
    mv_context->loadClusterPairMetricsFromFile(cluster_pair_metrics_path);
    QString output = CurationProgramView::applyCurationProgram(mv_context);
    //printf("%s\n", output.toUtf8().data());
}
