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
#include "mvamphistview2.h"

#include <QGridLayout>
#include <taskprogress.h>
#include "mlcommon.h"
#include "histogramview.h"
#include <QWheelEvent>
#include "actionfactory.h"
#include "get_sort_indices.h"

/// TODO: (MEDIUM) vertical zoom on all histograms

struct AmpHistogram {
    int k;
    QVector<double> data;
};

class MVAmpHistView2Computer {
public:
    //input
    QString mlproxy_url;
    DiskReadMda firings;

    //output
    QList<AmpHistogram> histograms;

    void compute();
};

class MVAmpHistView2Private {
public:
    MVAmpHistView2* q;

    MVAmpHistView2Computer m_computer;
    QList<AmpHistogram> m_histograms;
    double m_zoom_factor = 1;

    void set_views();
};

MVAmpHistView2::MVAmpHistView2(MVAbstractContext* context)
    : MVHistogramGrid(context)
{
    d = new MVAmpHistView2Private;
    d->q = this;

    this->setPairMode(false);

    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomInHorizontal, this, SLOT(slot_zoom_in_horizontal()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomOutHorizontal, this, SLOT(slot_zoom_out_horizontal()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::PanLeft, this, SLOT(slot_pan_left()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::PanRight, this, SLOT(slot_pan_right()));

    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    this->recalculateOn(c, SIGNAL(firingsChanged()), false);
    this->recalculateOn(c, SIGNAL(clusterMergeChanged()), false);
    this->recalculateOn(c, SIGNAL(clusterVisibilityChanged()), false);
    this->recalculateOn(c, SIGNAL(viewMergedChanged()), false);
    this->recalculateOnOptionChanged("amp_thresh_display", false);

    this->recalculate();
}

MVAmpHistView2::~MVAmpHistView2()
{
    this->stopCalculation();
    delete d;
}

void MVAmpHistView2::prepareCalculation()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    d->m_computer.mlproxy_url = c->mlProxyUrl();
    d->m_computer.firings = c->firings();
}

void MVAmpHistView2::runCalculation()
{
    d->m_computer.compute();
}

void MVAmpHistView2::slot_zoom_in_horizontal(double zoom_factor)
{
    QList<HistogramView*> views = this->histogramViews(); //inherited
    for (int i = 0; i < views.count(); i++) {
        views[i]->setXRange(views[i]->xRange() * (1.0 / zoom_factor));
    }
}

void MVAmpHistView2::slot_zoom_out_horizontal(double factor)
{
    slot_zoom_in_horizontal(1 / factor);
}

void MVAmpHistView2::slot_pan_left(double units)
{
    QList<HistogramView*> views = this->histogramViews(); //inherited
    for (int i = 0; i < views.count(); i++) {
        MVRange range = views[i]->xRange();
        if (range.range()) {
            range = range + units * range.range();
        }
        views[i]->setXRange(range);
    }
}

void MVAmpHistView2::slot_pan_right(double units)
{
    slot_pan_left(-units);
}

double compute_min2(const QList<AmpHistogram>& data0)
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

double max2(const QList<AmpHistogram>& data0)
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

void MVAmpHistView2::onCalculationFinished()
{
    d->m_histograms = d->m_computer.histograms;

    d->set_views();
}

void MVAmpHistView2Computer::compute()
{
    TaskProgress task(TaskProgress::Calculate, QString("Amplitude AmpHistograms"));

    histograms.clear();

    QVector<int> labels;
    QVector<double> amplitudes;
    int L = firings.N2();

    task.setProgress(0.2);
    for (int n = 0; n < L; n++) {
        labels << (int)firings.value(2, n);
    }

    int K = MLCompute::max<int>(labels);

    //assemble the histograms index 0 <--> k=1
    for (int k = 1; k <= K; k++) {
        AmpHistogram HH;
        HH.k = k;
        this->histograms << HH;
    }

    int row = 3; //for amplitudes
    for (int n = 0; n < L; n++) {
        int label0 = (int)firings.value(2, n);
        double amp0 = firings.value(row, n);
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

    //sort by abs medium value
    QVector<double> abs_medians(histograms.count());
    for (int i = 0; i < histograms.count(); i++) {
        abs_medians[i] = qAbs(MLCompute::median(histograms[i].data));
    }
    QList<int> inds = get_sort_indices(abs_medians);
    QList<AmpHistogram> hist_new;
    for (int i = 0; i < inds.count(); i++) {
        hist_new << histograms[inds[i]];
    }
    histograms = hist_new;
}

void MVAmpHistView2::wheelEvent(QWheelEvent* evt)
{
    Q_UNUSED(evt)
    /*
    double zoom_factor = 1;
    if (evt->delta() > 0) {
        zoom_factor *= 1.2;
    }
    else if (evt->delta() < 0) {
        zoom_factor /= 1.2;
    }
    QList<HistogramView*> views = this->histogramViews(); //inherited
    for (int i = 0; i < views.count(); i++) {
        views[i]->setXRange(views[i]->xRange() * (1.0 / zoom_factor));
    }
    */
}

void MVAmpHistView2Private::set_views()
{
    double bin_min = compute_min2(m_histograms);
    double bin_max = max2(m_histograms);
    double max00 = qMax(qAbs(bin_min), qAbs(bin_max));

    int num_bins = 200; //how to choose this?

    MVContext* c = qobject_cast<MVContext*>(q->mvContext());
    Q_ASSERT(c);

    double amp_thresh = c->option("amp_thresh_display", 0).toDouble();

    QList<HistogramView*> views;
    for (int ii = 0; ii < m_histograms.count(); ii++) {
        int k0 = m_histograms[ii].k;
        if (c->clusterIsVisible(k0)) {
            HistogramView* HV = new HistogramView;
            HV->setData(m_histograms[ii].data);
            HV->setColors(c->colors());
            //HV->autoSetBins(50);
            HV->setBinInfo(bin_min, bin_max, num_bins);
            HV->setDrawVerticalAxisAtZero(true);
            if (amp_thresh) {
                QList<double> vals;
                vals << -amp_thresh << amp_thresh;
                HV->setVerticalLines(vals);
            }
            HV->setXRange(MVRange(-max00, max00));
            HV->autoCenterXRange();
            HV->setProperty("k", k0);
            views << HV;
        }
    }

    q->setHistogramViews(views); //inherited
}

MVAmplitudeHistogramsFactory::MVAmplitudeHistogramsFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString MVAmplitudeHistogramsFactory::id() const
{
    return QStringLiteral("open-amplitude-histograms");
}

QString MVAmplitudeHistogramsFactory::name() const
{
    return tr("Amplitude Histograms (old)");
}

QString MVAmplitudeHistogramsFactory::title() const
{
    return tr("Amplitudes");
}

MVAbstractView* MVAmplitudeHistogramsFactory::createView(MVAbstractContext* context)
{
    MVAmpHistView2* X = new MVAmpHistView2(context);
    return X;
}

void MVAmplitudeHistogramsFactory::slot_amplitude_histogram_activated()
{
    MVAbstractView* view = qobject_cast<MVAbstractView*>(sender());
    if (!view)
        return;
    //not sure what to do here
}
