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

#include "mvcrosscorrelogramswidget3.h"
#include "histogramview.h"
#include "mvutils.h"
#include "taskprogress.h"
#include "mvmainwindow.h" //to disappear
#include "tabber.h" //to disappear

#include <QAction>
#include <QGridLayout>
#include <QKeyEvent>
#include <QList>
#include <QMessageBox>
#include <QPainter>
#include <QSettings>
#include <math.h>
#include "mlcommon.h"
#include "mvmisc.h"
#include <QFileDialog>
#include <QJsonDocument>

struct Correlogram3 {
    int k1 = 0, k2 = 0;
    QVector<double> data;
};

QVector<double> compute_cc_data3(const QVector<double>& times1_in, const QVector<double>& times2_in, int max_dt, bool exclude_matches, double max_est_data_size);

class MVCrossCorrelogramsWidget3Computer {
public:
    //input
    QString mlproxy_url;
    DiskReadMda firings;
    CrossCorrelogramOptions3 options;
    int max_dt;
    ClusterMerge cluster_merge;
    int pair_mode = false;
    double max_est_data_size = 0;

    //output
    QList<Correlogram3> correlograms;

    void compute();

    bool loaded_from_static_output = false;
    QJsonObject exportStaticOutput();
    void loadStaticOutput(const QJsonObject& X);
};

class MVCrossCorrelogramsWidget3Private {
public:
    MVCrossCorrelogramsWidget3* q;
    MVCrossCorrelogramsWidget3Computer m_computer;
    QList<Correlogram3> m_correlograms;

    QGridLayout* m_grid_layout;

    CrossCorrelogramOptions3 m_options;
    HistogramView::TimeScaleMode m_time_scale_mode = HistogramView::Uniform;

    void update_scale_stuff();
};

MVCrossCorrelogramsWidget3::MVCrossCorrelogramsWidget3(MVAbstractContext* context)
    : MVHistogramGrid(context)
{
    d = new MVCrossCorrelogramsWidget3Private;
    d->q = this;

    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    this->recalculateOn(c, SIGNAL(firingsChanged()), false);
    this->recalculateOn(c, SIGNAL(clusterMergeChanged()), false);
    this->recalculateOn(c, SIGNAL(clusterVisibilityChanged()), false);
    this->recalculateOn(c, SIGNAL(viewMergedChanged()), false);
    this->recalculateOnOptionChanged("cc_max_dt_msec");
    this->recalculateOnOptionChanged("cc_log_time_constant_msec");
    this->recalculateOnOptionChanged("cc_bin_size_msec");
    this->recalculateOnOptionChanged("cc_max_est_data_size");

    {
        QAction* A = new QAction("Log", this);
        A->setProperty("action_type", "toolbar");
        A->setCheckable(true);
        //A->setIcon(QIcon(":/images/log.png"));
        QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(slot_log_time_scale()));
        this->addAction(A);
    }
    {
        QAction* A = new QAction("Warning", this);
        A->setToolTip("When in log time scale mode, there may appear to be a dip near zero even when no such dip exists. This is especially true when the histogram is relatively sparse. Also, the time scale is actually a pseudo log. See the documentation or forum for more details.");
        A->setProperty("action_type", "toolbar");
        QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(slot_warning()));
        this->addAction(A);
    }

    {
        QAction* A = new QAction("Export static view", this);
        A->setProperty("action_type", "");
        QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(slot_export_static_view()));
        this->addAction(A);
    }

    this->recalculate();
}

MVCrossCorrelogramsWidget3::~MVCrossCorrelogramsWidget3()
{
    this->stopCalculation();
    delete d;
}

void MVCrossCorrelogramsWidget3::prepareCalculation()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    d->m_computer.mlproxy_url = c->mlProxyUrl();
    d->m_computer.firings = c->firings();
    d->m_computer.options = d->m_options;
    d->m_computer.max_dt = c->option("cc_max_dt_msec", 100).toDouble() / 1000 * c->sampleRate();
    d->m_computer.cluster_merge.clear();
    if (c->viewMerged()) {
        d->m_computer.cluster_merge = c->clusterMerge();
    }
    d->m_computer.pair_mode = this->pairMode();
    d->m_computer.max_est_data_size = c->option("cc_max_est_data_size", 10000).toDouble();
}

void MVCrossCorrelogramsWidget3::runCalculation()
{
    d->m_computer.compute();
}

double max2(const QList<Correlogram3>& data0)
{
    double ret = 0;
    for (int i = 0; i < data0.count(); i++) {
        QVector<double> tmp = data0[i].data;
        for (int j = 0; j < tmp.count(); j++) {
            if (tmp[j] > ret)
                ret = tmp[j];
        }
    }
    return ret;
}

void MVCrossCorrelogramsWidget3::onCalculationFinished()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    d->m_correlograms = d->m_computer.correlograms;

    double bin_max = max2(d->m_correlograms);
    double bin_min = -bin_max;
    //int num_bins=100;
    double sample_freq = c->sampleRate();
    int bin_size = c->option("cc_bin_size_msec").toDouble() / 1000 * sample_freq;
    int num_bins = (bin_max - bin_min) / bin_size;
    //if (num_bins < 100)
    //    num_bins = 100;
    if (num_bins > 2000)
        num_bins = 2000;

    double time_width = (bin_max - bin_min) / sample_freq * 1000;
    HorizontalScaleAxisData X;
    X.use_it = true;
    X.label = QString("%1 ms").arg((int)(time_width / 2));
    this->setHorizontalScaleAxis(X);

    QList<HistogramView*> histogram_views;
    for (int ii = 0; ii < d->m_correlograms.count(); ii++) {
        int k1 = d->m_correlograms[ii].k1;
        int k2 = d->m_correlograms[ii].k2;
        if ((c->clusterIsVisible(k1)) && (c->clusterIsVisible(k2))) {
            HistogramView* HV = new HistogramView;
            HV->setData(d->m_correlograms[ii].data);
            HV->setColors(c->colors());
            HV->setBinInfo(bin_min, bin_max, num_bins);
            QString title0;
            QString caption0;
            HV->setProperty("k", d->m_correlograms[ii].k1);
            HV->setProperty("k1", d->m_correlograms[ii].k1);
            HV->setProperty("k2", d->m_correlograms[ii].k2);

            histogram_views << HV;
        }
    }
    this->setHistogramViews(histogram_views);
    d->update_scale_stuff();
}

void MVCrossCorrelogramsWidget3::setOptions(CrossCorrelogramOptions3 opts)
{
    if (opts.mode == All_Auto_Correlograms3)
        this->setPairMode(false);
    else if (opts.mode == Selected_Auto_Correlograms3)
        this->setPairMode(false);
    else if (opts.mode == Cross_Correlograms3)
        this->setPairMode(true);
    else if (opts.mode == Matrix_Of_Cross_Correlograms3) {
        this->setPairMode(true);
        this->setForceSquareMatrix(true);
    }
    else if (opts.mode == Selected_Cross_Correlograms3)
        this->setPairMode(true);
    d->m_options = opts;
    this->recalculate();
}

void MVCrossCorrelogramsWidget3::setTimeScaleMode(HistogramView::TimeScaleMode mode)
{
    if (d->m_time_scale_mode == mode)
        return;
    d->m_time_scale_mode = mode;
    d->update_scale_stuff();
}

HistogramView::TimeScaleMode MVCrossCorrelogramsWidget3::timeScaleMode() const
{
    return d->m_time_scale_mode;
}

QString to_string(HistogramView::TimeScaleMode mode)
{
    if (mode == HistogramView::Log)
        return "Log";
    if (mode == HistogramView::Uniform)
        return "Uniform";
    return "";
}

void from_string(HistogramView::TimeScaleMode& tsm, QString str)
{
    if (str == "Log")
        tsm = HistogramView::Log;
    if (str == "Uniform")
        tsm = HistogramView::Uniform;
}

QJsonObject MVCrossCorrelogramsWidget3::exportStaticView()
{
    QJsonObject ret = MVAbstractView::exportStaticView();
    ret["view-type"] = "MVCrossCorrelogramsWidget";
    ret["version"] = "0.1";
    ret["computer-output"] = d->m_computer.exportStaticOutput();
    ret["options"] = d->m_options.toJsonObject();
    ret["time-scale-mode"] = to_string(d->m_time_scale_mode);
    return ret;
}

void MVCrossCorrelogramsWidget3::loadStaticView(const QJsonObject& X)
{
    MVAbstractView::loadStaticView(X);
    QJsonObject computer_output = X["computer-output"].toObject();
    d->m_computer.loadStaticOutput(computer_output);
    CrossCorrelogramOptions3 opts;
    opts.fromJsonObject(X["options"].toObject());
    this->setOptions(opts);
    HistogramView::TimeScaleMode tsm;
    from_string(tsm, X["time-scale-mode"].toString());
    this->setTimeScaleMode(tsm);
    this->recalculate();
}

void MVCrossCorrelogramsWidget3::slot_log_time_scale()
{
    HistogramView::TimeScaleMode mode = timeScaleMode();
    if (mode == HistogramView::Uniform)
        mode = HistogramView::Log;
    //else if (mode==HistogramView::Cubic) mode=HistogramView::Log;
    else if (mode == HistogramView::Log)
        mode = HistogramView::Uniform;
    setTimeScaleMode(mode);
}

void MVCrossCorrelogramsWidget3::slot_warning()
{
    QAction* A = qobject_cast<QAction*>(sender());
    QString str = A->toolTip();
    QMessageBox::information(this, "Warning about log scale", str);
}

void MVCrossCorrelogramsWidget3::slot_export_static_view()
{
    //QSettings settings("SCDA", "MountainView");
    //QString default_dir = settings.value("default_export_dir", "").toString();
    QString default_dir = QDir::currentPath();
    QString fname = QFileDialog::getSaveFileName(this, "Export static cross-correlogram view", default_dir, "*.smv");
    if (fname.isEmpty())
        return;
    //settings.setValue("default_export_dir", QFileInfo(fname).path());
    if (QFileInfo(fname).suffix() != "smv")
        fname = fname + ".smv";
    QJsonObject obj = exportStaticView();
    if (!TextFile::write(fname, QJsonDocument(obj).toJson())) {
        qWarning() << "Unable to write file: " + fname;
    }
}

double pseudorandomnumber(double i)
{
    double ret = sin(i + cos(i));
    ret = (ret + 5) - (int)(ret + 5);
    return ret;
}

QVector<double> pseudorandomsample(const QVector<double>& X, double dsfactor)
{
    QVector<double> ret;
    for (int i = 0; i < X.count(); i++) {
        double randnum = pseudorandomnumber(i);
        if (randnum <= 1 / dsfactor)
            ret << X[i];
    }
    return ret;
}

double estimate_cc_data_size(const QVector<double>& times1, const QVector<double>& times2, int max_dt, bool exclude_matches)
{
    double dsfactor = 10;
    QVector<double> times1b = pseudorandomsample(times1, dsfactor);
    QVector<double> times2b = pseudorandomsample(times2, dsfactor);
    QVector<double> datab = compute_cc_data3(times1b, times2b, max_dt, exclude_matches, 0);
    return datab.count() * dsfactor * dsfactor;
}

bool write_mda(QString fname, const QVector<double>& X)
{
    Mda A(1, X.count());
    for (bigint i = 0; i < X.count(); i++) {
        A.set(X[i], i);
    }
    return A.write64(fname);
}

QVector<double> compute_cc_data3(const QVector<double>& times1_in, const QVector<double>& times2_in, int max_dt, bool exclude_matches, double max_est_data_size)
{
    QVector<double> ret;
    QVector<double> times1 = times1_in;
    QVector<double> times2 = times2_in;
    qSort(times1);
    qSort(times2);

    if ((times1.isEmpty()) || (times2.isEmpty()))
        return ret;

    if (max_est_data_size) {
        double estimated_data_size = estimate_cc_data_size(times1, times2, max_dt, exclude_matches);
        if (estimated_data_size > max_est_data_size) {
            double dsfactor = sqrt(estimated_data_size / max_est_data_size);
            times1 = pseudorandomsample(times1, dsfactor);
            times2 = pseudorandomsample(times2, dsfactor);
        }
    }

    int i1 = 0;
    for (int i2 = 0; i2 < times2.count(); i2++) {
        while ((i1 + 1 < times1.count()) && (times1[i1] < times2[i2] - max_dt))
            i1++;
        int j1 = i1;
        while ((j1 < times1.count()) && (times1[j1] <= times2[i2] + max_dt)) {
            bool ok = true;
            if ((exclude_matches) && (j1 == i2) && (times1[j1] == times2[i2]))
                ok = false;
            if (ok) {
                ret << times1[j1] - times2[i2];
            }
            j1++;
        }
    }
    return ret;
}

typedef QVector<double> DoubleList;
typedef QVector<int> IntList;
void MVCrossCorrelogramsWidget3Computer::compute()
{
    TaskProgress task(TaskProgress::Calculate, QString("Cross Correlograms (%1)").arg(options.mode));
    if (loaded_from_static_output) {
        task.log("Loaded from static output");
        return;
    }

    correlograms.clear();

    QVector<double> times;
    QVector<int> labels;
    bigint L = firings.N2();

    //assemble the times and labels arrays
    task.setProgress(0.2);
    for (bigint n = 0; n < L; n++) {
        times << firings.value(1, n);
        //times << debug.value(1,n);
        labels << (int)firings.value(2, n);
    }

    //compute K (the maximum label)
    int K = MLCompute::max(labels);

    //handle the merge
    QMap<int, int> label_map = cluster_merge.labelMap(K);
    for (int n = 0; n < L; n++) {
        labels[n] = label_map[labels[n]];
    }

    //Assemble the correlogram objects depending on mode
    if (options.mode == All_Auto_Correlograms3) {
        for (int k = 1; k <= K; k++) {
            Correlogram3 CC;
            CC.k1 = k;
            CC.k2 = k;
            this->correlograms << CC;
        }
    }
    else if (options.mode == Selected_Auto_Correlograms3) {
        for (int i = 0; i < options.ks.count(); i++) {
            int k = options.ks[i];
            Correlogram3 CC;
            CC.k1 = k;
            CC.k2 = k;
            this->correlograms << CC;
        }
    }
    else if (options.mode == Cross_Correlograms3) {
        int k0 = options.ks.value(0);
        for (int k = 1; k <= K; k++) {
            Correlogram3 CC;
            CC.k1 = k0;
            CC.k2 = k;
            this->correlograms << CC;
        }
    }
    else if (options.mode == Matrix_Of_Cross_Correlograms3) {
        for (int i = 0; i < options.ks.count(); i++) {
            for (int j = 0; j < options.ks.count(); j++) {
                Correlogram3 CC;
                CC.k1 = options.ks[i];
                CC.k2 = options.ks[j];
                this->correlograms << CC;
            }
        }
    }
    else if (options.mode == Selected_Cross_Correlograms3) {
        for (int i = 0; i < options.pairs.count(); i++) {
            Correlogram3 CC;
            CC.k1 = options.pairs[i].k1();
            CC.k2 = options.pairs[i].k2();
            this->correlograms << CC;
        }
    }

    //assemble the times organized by k
    QList<DoubleList> the_times;
    for (int k = 0; k <= K; k++) {
        the_times << DoubleList();
    }
    for (bigint ii = 0; ii < labels.count(); ii++) {
        int k = labels[ii];
        if (k < the_times.count()) {
            the_times[k] << times[ii];
        }
    }

    //compute the cross-correlograms
    task.setProgress(0.7);
    for (int j = 0; j < correlograms.count(); j++) {
        if (MLUtil::threadInterruptRequested()) {
            return;
        }
        int k1 = correlograms[j].k1;
        int k2 = correlograms[j].k2;
        correlograms[j].data = compute_cc_data3(the_times.value(k1), the_times.value(k2), max_dt, (k1 == k2), max_est_data_size);
        if (correlograms[j].data.count() > max_est_data_size) {
            qWarning() << QString("%1>%2").arg(correlograms[j].data.count()).arg(max_est_data_size);
        }
        if ((correlograms[j].data.isEmpty()) && (!pair_mode)) {
            correlograms.removeAt(j);
            j--;
        }
    }
}

QJsonObject MVCrossCorrelogramsWidget3Computer::exportStaticOutput()
{
    QJsonObject ret;
    ret["version"] = "MVCrossCorrelogramsWidget3Computer-0.1";
    QJsonArray cc;
    for (int i = 0; i < correlograms.count(); i++) {
        QJsonObject oo;
        oo["data"] = MLUtil::toJsonValue(correlograms[i].data);
        oo["k1"] = correlograms[i].k1;
        oo["k2"] = correlograms[i].k2;
        cc.append(oo);
    }
    ret["correlograms"] = cc;
    return ret;
}

void MVCrossCorrelogramsWidget3Computer::loadStaticOutput(const QJsonObject& X)
{
    QJsonArray cc = X["correlograms"].toArray();
    correlograms.clear();
    for (int ii = 0; ii < cc.count(); ii++) {
        QJsonObject oo = cc[ii].toObject();
        Correlogram3 CC;
        MLUtil::fromJsonValue(CC.data, oo["data"]);
        CC.k1 = oo["k1"].toInt();
        CC.k2 = oo["k2"].toInt();
        correlograms << CC;
    }
    loaded_from_static_output = true;
}

MVAutoCorrelogramsFactory::MVAutoCorrelogramsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString MVAutoCorrelogramsFactory::id() const
{
    return QStringLiteral("open-auto-correlograms");
}

QString MVAutoCorrelogramsFactory::name() const
{
    return tr("All auto-correlograms");
}

QString MVAutoCorrelogramsFactory::title() const
{
    return tr("All auto-Correlograms");
}

MVAbstractView* MVAutoCorrelogramsFactory::createView(MVAbstractContext* context)
{
    MVCrossCorrelogramsWidget3* X = new MVCrossCorrelogramsWidget3(context);
    CrossCorrelogramOptions3 opts;
    opts.mode = All_Auto_Correlograms3;
    X->setOptions(opts);
    return X;
}

bool MVAutoCorrelogramsFactory::isEnabled(MVAbstractContext* context) const
{
    Q_UNUSED(context)
    return true;
}

/*
QList<QAction*> MVAutoCorrelogramsFactory::actions(const QMimeData& md)
{
    QList<QAction*> actions;
    if (!md.data("x-mv-main").isEmpty()) {
        QAction* action = new QAction("All auto-correlograms", 0);
        MVMainWindow* mw = this->mainWindow();
        connect(action, &QAction::triggered, [mw]() {
            mw->openView("open-auto-correlograms");
        });
        actions << action;
    }
    return actions;
}
*/

MVSelectedAutoCorrelogramsFactory::MVSelectedAutoCorrelogramsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString MVSelectedAutoCorrelogramsFactory::id() const
{
    return QStringLiteral("open-selected-auto-correlograms");
}

QString MVSelectedAutoCorrelogramsFactory::name() const
{
    return tr("Selected auto-correlograms");
}

QString MVSelectedAutoCorrelogramsFactory::title() const
{
    return tr("Selected auto-correlograms");
}

MVAbstractView* MVSelectedAutoCorrelogramsFactory::createView(MVAbstractContext* context)
{
    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    MVCrossCorrelogramsWidget3* X = new MVCrossCorrelogramsWidget3(context);
    QList<int> ks = c->selectedClusters();
    X->setTitle(title()+" ("+c->selectedClustersRangeText()+")");
    if (ks.isEmpty())
        ks = c->clusterVisibilityRule().subset.toList();
    qSort(ks);
    if (ks.isEmpty())
        return X;
    CrossCorrelogramOptions3 opts;
    opts.mode = Selected_Auto_Correlograms3;
    opts.ks = ks;
    X->setOptions(opts);
    return X;
}

bool MVSelectedAutoCorrelogramsFactory::isEnabled(MVAbstractContext* context) const
{
    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    return (c->selectedClusters().count() >= 1);
}

/*
QList<QAction*> MVSelectedAutoCorrelogramsFactory::actions(const QMimeData& md)
{
    QList<QAction*> actions;
    QSet<int> clusters;
    QDataStream ds(md.data("application/x-msv-clusters"));
    ds >> clusters;
    if (!clusters.isEmpty()) {
        QAction* action = new QAction("Selected auto-correlograms", 0);
        MVMainWindow* mw = this->mainWindow();
        connect(action, &QAction::triggered, [mw]() {
            mw->openView("open-selected-auto-correlograms");
        });
        actions << action;
    }
    return actions;
}
*/

MVCrossCorrelogramsFactory::MVCrossCorrelogramsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString MVCrossCorrelogramsFactory::id() const
{
    return QStringLiteral("open-cross-correlograms");
}

QString MVCrossCorrelogramsFactory::name() const
{
    return tr("Cross-Correlograms");
}

QString MVCrossCorrelogramsFactory::title() const
{
    return tr("Cross-Correlograms");
}

MVAbstractView* MVCrossCorrelogramsFactory::createView(MVAbstractContext* context)
{
    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    MVCrossCorrelogramsWidget3* X = new MVCrossCorrelogramsWidget3(context);
    QList<int> ks = c->selectedClusters();
    if (ks.count() != 1)
        return X;

    CrossCorrelogramOptions3 opts;
    opts.mode = Cross_Correlograms3;
    opts.ks = ks;

    X->setOptions(opts);
    return X;
}

bool MVCrossCorrelogramsFactory::isEnabled(MVAbstractContext* context) const
{
    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    return (c->selectedClusters().count() == 1);
}

/*
QList<QAction*> MVCrossCorrelogramsFactory::actions(const QMimeData& md)
{
    QList<QAction*> actions;
    QSet<int> clusters;
    QDataStream ds(md.data("application/x-msv-clusters"));
    ds >> clusters;
    if (!clusters.isEmpty()) {
        QAction* action = new QAction("Cross-correlograms", 0);
        MVMainWindow* mw = this->mainWindow();
        connect(action, &QAction::triggered, [mw]() {
            mw->openView("open-cross-correlograms");
        });
        if (clusters.count() != 1)
            action->setEnabled(false);
        actions << action;
    }
    return actions;
}
*/

MVMatrixOfCrossCorrelogramsFactory::MVMatrixOfCrossCorrelogramsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString MVMatrixOfCrossCorrelogramsFactory::id() const
{
    return QStringLiteral("open-matrix-of-cross-correlograms");
}

QString MVMatrixOfCrossCorrelogramsFactory::name() const
{
    return tr("Matrix of Cross-Correlograms");
}

QString MVMatrixOfCrossCorrelogramsFactory::title() const
{
    return tr("CC Matrix");
}

MVAbstractView* MVMatrixOfCrossCorrelogramsFactory::createView(MVAbstractContext* context)
{
    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    MVCrossCorrelogramsWidget3* X = new MVCrossCorrelogramsWidget3(context);
    X->setTitle(title()+" ("+c->selectedClustersRangeText()+")");
    QList<int> ks = c->selectedClusters();
    if (ks.isEmpty())
        ks = c->clusterVisibilityRule().subset.toList();
    qSort(ks);
    if (ks.isEmpty())
        return X;
    CrossCorrelogramOptions3 opts;
    opts.mode = Matrix_Of_Cross_Correlograms3;
    opts.ks = ks;
    X->setOptions(opts);
    return X;
}

bool MVMatrixOfCrossCorrelogramsFactory::isEnabled(MVAbstractContext* context) const
{
    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    return (!c->selectedClusters().isEmpty());
}

/*
QList<QAction*> MVMatrixOfCrossCorrelogramsFactory::actions(const QMimeData& md)
{
    QList<QAction*> actions;
    QSet<int> clusters;
    QDataStream ds(md.data("application/x-msv-clusters"));
    ds >> clusters;
    if (!clusters.isEmpty()) {
        QAction* action = new QAction("Matrix of cross-correlograms", 0);
        MVMainWindow* mw = this->mainWindow();
        connect(action, &QAction::triggered, [mw]() {
            mw->openView("open-matrix-of-cross-correlograms");
        });
        actions << action;
    }
    return actions;
}
*/

MVSelectedCrossCorrelogramsFactory::MVSelectedCrossCorrelogramsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString MVSelectedCrossCorrelogramsFactory::id() const
{
    return QStringLiteral("open-selected-cross-correlograms");
}

QString MVSelectedCrossCorrelogramsFactory::name() const
{
    return tr("Selected Cross-Correlograms");
}

QString MVSelectedCrossCorrelogramsFactory::title() const
{
    return tr("CC");
}

MVAbstractView* MVSelectedCrossCorrelogramsFactory::createView(MVAbstractContext* context)
{
    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    MVCrossCorrelogramsWidget3* X = new MVCrossCorrelogramsWidget3(context);
    QList<ClusterPair> pairs = c->selectedClusterPairs().toList();
    qSort(pairs);
    if (pairs.isEmpty())
        return X;
    CrossCorrelogramOptions3 opts;
    opts.mode = Selected_Cross_Correlograms3;
    opts.pairs = pairs;
    X->setOptions(opts);
    return X;
}

bool MVSelectedCrossCorrelogramsFactory::isEnabled(MVAbstractContext* context) const
{
    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    return (!c->selectedClusterPairs().isEmpty());
}

/*
QList<QAction*> MVSelectedCrossCorrelogramsFactory::actions(const QMimeData& md)
{
    Q_UNUSED(md)
    QList<QAction*> actions;
    return actions;
}
*/

void MVCrossCorrelogramsWidget3Private::update_scale_stuff()
{
    MVContext* c = qobject_cast<MVContext*>(q->mvContext());
    Q_ASSERT(c);

    QList<HistogramView*> views = q->histogramViews();
    foreach (HistogramView* HV, views) {
        HV->setTimeScaleMode(m_time_scale_mode);
        double time_constant_sec = c->option("cc_log_time_constant_msec", 1).toDouble() / 1000;
        HV->setTimeConstant(c->sampleRate() * time_constant_sec);
        QList<double> tickvals;
        if (m_time_scale_mode == HistogramView::Uniform) {
        }
        else if (m_time_scale_mode == HistogramView::Log) {
            for (int sign = -1; sign <= 1; sign += 2) {
                tickvals << 30 * sign << 300 * sign << 3000 * sign << 30000 * sign;
            }
        }
        HV->setTickMarks(tickvals);
    }
}

QString to_string(CrossCorrelogramMode3 mode)
{
    if (mode == Undefined3)
        return "Undefined";
    if (mode == All_Auto_Correlograms3)
        return "All_Auto_Correlograms";
    if (mode == Selected_Auto_Correlograms3)
        return "Selected_Auto_Correlograms";
    if (mode == Cross_Correlograms3)
        return "Cross_Correlograms";
    if (mode == Matrix_Of_Cross_Correlograms3)
        return "Matrix_Of_Cross_Correlograms";
    if (mode == Selected_Cross_Correlograms3)
        return "Selected_Cross_Correlograms";
    return "";
}

void from_string(CrossCorrelogramMode3& mode, QString str)
{
    if (str == "Undefined")
        mode = Undefined3;
    if (str == "All_Auto_Correlograms")
        mode = All_Auto_Correlograms3;
    if (str == "Selected_Auto_Correlograms")
        mode = Selected_Auto_Correlograms3;
    if (str == "Cross_Correlograms")
        mode = Cross_Correlograms3;
    if (str == "Matrix_Of_Cross_Correlograms")
        mode = Matrix_Of_Cross_Correlograms3;
    if (str == "Selected_Cross_Correlograms")
        mode = Selected_Cross_Correlograms3;
}

QJsonObject CrossCorrelogramOptions3::toJsonObject()
{
    QJsonObject ret;
    ret["mode"] = to_string(mode);
    ret["ks"] = MLUtil::toJsonValue(ks);
    QList<int> tmp;
    for (int i = 0; i < pairs.count(); i++) {
        tmp << pairs[i].k1();
        tmp << pairs[i].k2();
    }
    ret["pairs"] = MLUtil::toJsonValue(tmp);
    return ret;
}

void CrossCorrelogramOptions3::fromJsonObject(const QJsonObject& X)
{
    from_string(mode, X["mode"].toString());
    MLUtil::fromJsonValue(ks, X["ks"]);
    QList<int> tmp;
    MLUtil::fromJsonValue(tmp, X["pairs"]);
    pairs.clear();
    for (int i = 0; i < tmp.count(); i += 2) {
        ClusterPair pair(tmp.value(i), tmp.value(i + 1));
        pairs << pair;
    }
}
