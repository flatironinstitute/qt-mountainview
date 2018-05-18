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
#include "mvamphistview3.h"
#include "actionfactory.h"
#include "histogramlayer.h"
#include "mvpanelwidget2.h"
#include "get_sort_indices.h"
/// TODO get_sort_indices should be put into MLCompute

#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
#include <diskreadmda32.h>
#include <mountainprocessrunner.h>
#include <mvcontext.h>
#include <taskprogress.h>

struct AmpHistogram3 {
    int k;
    QVector<double> data;
};

class MVAmpHistView3Computer {
public:
    //input
    QString mlproxy_url;
    QString firings;
    QString timeseries;
    MVAmpHistView3::AmplitudeMode amplitude_mode;

    //output
    QList<AmpHistogram3> histograms;

    void compute();
};

class MVAmpHistView3Private {
public:
    MVAmpHistView3* q;

    MVAmpHistView3Computer m_computer;
    QList<AmpHistogram3> m_histograms;
    MVPanelWidget2* m_panel_widget;
    QList<HistogramLayer*> m_views;
    QSpinBox* m_num_bins_control;
    MVAmpHistView3::AmplitudeMode m_amplitude_mode = MVAmpHistView3::ComputeAmplitudes;
    double m_zoom_factor = 1;

    void set_views();
};

MVAmpHistView3::MVAmpHistView3(MVAbstractContext* context)
    : MVAbstractView(context)
{
    d = new MVAmpHistView3Private;
    d->q = this;

    QVBoxLayout* layout = new QVBoxLayout;
    this->setLayout(layout);

    d->m_panel_widget = new MVPanelWidget2;
    PanelWidget2Behavior B;
    B.h_scrollable = false;
    B.v_scrollable = true;
    B.preferred_panel_width = 300;
    B.preferred_panel_height = 300;
    B.adjust_layout_to_preferred_size = true;
    d->m_panel_widget->setBehavior(B);
    d->m_panel_widget->setSpacing(14, 14);
    d->m_panel_widget->setMargins(14, 14);
    //d->m_panel_widget->setZoomOnWheel(false);
    layout->addWidget(d->m_panel_widget);

    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomIn, this, d->m_panel_widget, SLOT(zoomIn()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomOut, this, d->m_panel_widget, SLOT(zoomOut()));

    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomInHorizontal, this, SLOT(slot_zoom_in_horizontal()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomOutHorizontal, this, SLOT(slot_zoom_out_horizontal()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::PanLeft, this, SLOT(slot_pan_left()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::PanRight, this, SLOT(slot_pan_right()));

    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    {
        d->m_num_bins_control = new QSpinBox();
        d->m_num_bins_control->setRange(1, 20000);
        d->m_num_bins_control->setSingleStep(20);
        d->m_num_bins_control->setValue(1500);
        this->addToolbarControl(new QLabel(" #bins:"));
        this->addToolbarControl(d->m_num_bins_control);
        QObject::connect(d->m_num_bins_control, SIGNAL(valueChanged(int)), this, SLOT(slot_update_bins()));
    }

    QObject::connect(c, SIGNAL(currentClusterChanged()), this, SLOT(slot_update_highlighting_and_captions()));
    QObject::connect(c, SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_highlighting_and_captions()));
    QObject::connect(c, SIGNAL(currentClusterChanged()), this, SLOT(slot_current_cluster_changed()));

    this->recalculateOn(c, SIGNAL(firingsChanged()), false);
    //this->recalculateOn(c, SIGNAL(clusterMergeChanged()), false);
    this->recalculateOn(c, SIGNAL(clusterVisibilityChanged()), false);
    //this->recalculateOn(c, SIGNAL(viewMergedChanged()), false);
    this->recalculateOn(c, SIGNAL(currentTimeseriesChanged()), true);
    this->recalculateOnOptionChanged("amp_thresh_display", false);

    connect(d->m_panel_widget, SIGNAL(signalPanelClicked(int, Qt::KeyboardModifiers)), this, SLOT(slot_panel_clicked(int, Qt::KeyboardModifiers)));

    this->recalculate();
}

MVAmpHistView3::~MVAmpHistView3()
{
    this->stopCalculation();
    delete d;
}

void MVAmpHistView3::prepareCalculation()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    d->m_computer.mlproxy_url = c->mlProxyUrl();
    d->m_computer.firings = c->firings().makePath();
    d->m_computer.timeseries = c->currentTimeseries().makePath();
    d->m_computer.amplitude_mode = d->m_amplitude_mode;
}

void MVAmpHistView3::runCalculation()
{
    d->m_computer.compute();
}

void MVAmpHistView3::onCalculationFinished()
{
    d->m_histograms = d->m_computer.histograms;

    d->set_views();
}

void MVAmpHistView3::wheelEvent(QWheelEvent* evt)
{
    int delta = evt->delta();
    if (delta > 0)
        slot_zoom_in_horizontal();
    else
        slot_zoom_out_horizontal();
}

void MVAmpHistView3::slot_zoom_in_horizontal(double factor)
{
    QList<HistogramLayer*> views = d->m_views;
    for (int i = 0; i < views.count(); i++) {
        views[i]->setXRange(views[i]->xRange() * (1.0 / factor));
        views[i]->autoCenterXRange();
    }
}

void MVAmpHistView3::slot_zoom_out_horizontal(double factor)
{
    slot_zoom_in_horizontal(1 / factor);
}

void MVAmpHistView3::slot_pan_left(double units)
{
    QList<HistogramLayer*> views = d->m_views;
    for (int i = 0; i < views.count(); i++) {
        MVRange range = views[i]->xRange();
        if (range.range()) {
            range = range + units * range.range();
        }
        views[i]->setXRange(range);
    }
}

void MVAmpHistView3::slot_pan_right(double units)
{
    slot_pan_left(-units);
}

void MVAmpHistView3::slot_panel_clicked(int index, Qt::KeyboardModifiers modifiers)
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    /// TODO this is redundant logic with mvtemplatesview2
    if (modifiers & Qt::ShiftModifier) {
        int i1 = d->m_panel_widget->currentPanelIndex();
        int i2 = index;
        int j1 = qMin(i1, i2);
        int j2 = qMax(i1, i2);
        if ((j1 >= 0) && (j2 >= 0)) {
            QSet<int> set = c->selectedClusters().toSet();
            for (int j = j1; j <= j2; j++) {
                if (d->m_views.value(j)) {
                    int k = d->m_views.value(j)->property("k").toInt();
                    if (k > 0)
                        set.insert(k);
                }
            }
            c->setSelectedClusters(set.toList());
        }
    }
    else {
        if (d->m_views.value(index)) {
            int k = d->m_views.value(index)->property("k").toInt();
            if (k > 0) {
                c->clickCluster(k, modifiers);
            }
        }
    }
}

void MVAmpHistView3::slot_update_highlighting_and_captions()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    QList<int> selected_clusters = c->selectedClusters();
    QSet<ClusterPair> selected_cluster_pairs = c->selectedClusterPairs();
    for (int i = 0; i < d->m_views.count(); i++) {
        HistogramLayer* HV = d->m_views[i];
        QString title0, caption0;

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

        HV->setTitle(title0);
        HV->setCaption(caption0);
    }
}

void MVAmpHistView3::slot_current_cluster_changed()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    int k = c->currentCluster();
    for (int i = 0; i < d->m_views.count(); i++) {
        HistogramLayer* HV = d->m_views[i];
        if (HV->property("k").toInt() == k) {
            d->m_panel_widget->setCurrentPanelIndex(i); //for purpose of zooming
        }
    }
}

DiskReadMda compute_amplitudes(QString timeseries, QString firings, QString mlproxy_url)
{
    MountainProcessRunner X;
    QString processor_name = "mv.mv_compute_amplitudes";
    X.setProcessorName(processor_name);

    QMap<QString, QVariant> params;
    params["timeseries"] = timeseries;
    params["firings"] = firings;
    X.setInputParameters(params);
    X.setMLProxyUrl(mlproxy_url);

    QString firings_out_fname = X.makeOutputFilePath("firings_out");

    X.runProcess();
    DiskReadMda ret(firings_out_fname);
    return ret;
}

void MVAmpHistView3Computer::compute()
{
    TaskProgress task(TaskProgress::Calculate, QString("AmplitudeHistograms3"));

    histograms.clear();

    DiskReadMda firings2;
    if (amplitude_mode == MVAmpHistView3::ComputeAmplitudes) {
        firings2 = compute_amplitudes(timeseries, firings, mlproxy_url);
    }
    else {
        firings2 = DiskReadMda(firings);
    }

    QVector<int> labels;
    //QVector<double> amplitudes;
    int L = firings2.N2();

    task.setProgress(0.2);
    for (int n = 0; n < L; n++) {
        labels << (int)firings2.value(2, n);
    }

    int K = MLCompute::max<int>(labels);

    //assemble the histograms index 0 <--> k=1
    for (int k = 1; k <= K; k++) {
        AmpHistogram3 HH;
        HH.k = k;
        this->histograms << HH;
    }

    int row = 3; //for amplitudes
    for (int n = 0; n < L; n++) {
        int label0 = (int)firings2.value(2, n);
        double amp0 = firings2.value(row, n);
        if ((label0 >= 1) && (label0 <= K)) {
            this->histograms[label0 - 1].data << amp0;
        }
    }

    for (int i = 0; i < histograms.count(); i++) {
        if (histograms[i].data.count() == 0) {
            histograms.removeAt(i);
            i--;
        }
    }

    //sort by medium value
    QVector<double> abs_medians(histograms.count());
    for (int i = 0; i < histograms.count(); i++) {
        abs_medians[i] = qAbs(MLCompute::median(histograms[i].data));
    }
    QList<int> inds = get_sort_indices(abs_medians);
    QList<AmpHistogram3> hist_new;
    for (int i = 0; i < inds.count(); i++) {
        hist_new << histograms[inds[i]];
    }
    histograms = hist_new;
}

double compute_min3(const QList<AmpHistogram3>& data0)
{
    double ret = 0;
    for (int i = 0; i < data0.count(); i++) {
        QVector<double> tmp = data0[i].data;
        for (int j = 0; j < tmp.count(); j++) {
            if (tmp[j] < ret)
                ret = tmp[j];
        }
    }
    return ret;
}

double max3(const QList<AmpHistogram3>& data0)
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

void MVAmpHistView3::slot_update_bins()
{
    double bin_min = compute_min3(d->m_histograms);
    double bin_max = max3(d->m_histograms);
    double max00 = qMax(qAbs(bin_min), qAbs(bin_max));
    int num_bins = d->m_num_bins_control->value();
    for (int i = 0; i < d->m_views.count(); i++) {
        d->m_views[i]->setBinInfo(bin_min, bin_max, num_bins);
        d->m_views[i]->setXRange(MVRange(-max00 * 0.5, max00 * 0.5));
        d->m_views[i]->autoCenterXRange();
    }
}

void MVAmpHistView3Private::set_views()
{
    m_panel_widget->clearPanels(true);
    m_views.clear();

    MVContext* c = qobject_cast<MVContext*>(q->mvContext());
    Q_ASSERT(c);

    double amp_thresh = c->option("amp_thresh_display", 0).toDouble();

    QList<HistogramLayer*> views;
    for (int ii = 0; ii < m_histograms.count(); ii++) {
        int k0 = m_histograms[ii].k;
        if (c->clusterIsVisible(k0)) {
            HistogramLayer* HV = new HistogramLayer;
            HV->setData(m_histograms[ii].data);
            HV->setColors(c->colors());
            //HV->autoSetBins(50);
            HV->setDrawVerticalAxisAtZero(true);
            if (amp_thresh) {
                QList<double> vals;
                for (int i = 1; i <= 1; i++) {
                    vals << -amp_thresh* i << amp_thresh* i;
                }
                HV->setVerticalLines(vals);
            }
            HV->setProperty("k", k0);
            views << HV;
        }
    }

    int num_rows = qMax(1.0, sqrt(views.count()));
    int num_cols = ceil(views.count() * 1.0 / num_rows);
    m_views = views;
    for (int i = 0; i < views.count(); i++) {
        m_panel_widget->addPanel(i / num_cols, i % num_cols, views[i]);
    }

    q->slot_update_highlighting_and_captions();
    q->slot_update_bins();
}

void MVAmpHistView3::prepareMimeData(QMimeData& mimeData, const QPoint& pos)
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << c->selectedClusters();
    mimeData.setData("application/x-msv-clusters", ba); // selected cluster data
    MVAbstractView::prepareMimeData(mimeData, pos);
}

void MVAmpHistView3::keyPressEvent(QKeyEvent* evt)
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    if (evt->matches(QKeySequence::SelectAll)) {
        QList<int> all_ks;
        for (int i = 0; i < d->m_views.count(); i++) {
            all_ks << d->m_views[i]->property("k").toInt();
        }
        c->setSelectedClusters(all_ks);
    }
}

MVAmplitudeHistograms3Factory::MVAmplitudeHistograms3Factory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString MVAmplitudeHistograms3Factory::id() const
{
    return QStringLiteral("open-amplitude-histograms-3");
}

QString MVAmplitudeHistograms3Factory::name() const
{
    return tr("Amplitude Histograms");
}

QString MVAmplitudeHistograms3Factory::title() const
{
    return tr("Amplitudes");
}

MVAbstractView* MVAmplitudeHistograms3Factory::createView(MVAbstractContext* context)
{
    MVAmpHistView3* X = new MVAmpHistView3(context);
    return X;
}
