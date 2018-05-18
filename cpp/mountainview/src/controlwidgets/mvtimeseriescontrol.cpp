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

#include "mvtimeseriescontrol.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QRadioButton>
#include <QThread>
#include <QTimer>
#include <mountainprocessrunner.h>
#include <mvcontext.h>
#include "createtimeseriesdialog.h"

class TimeseriesCalculator : public QThread {
public:
    //inputs
    QString name;
    QString raw;
    double samplerate = 0;
    double freq_min = 0, freq_max = 0;
    bool whiten = false;

    //outputs
    QString output_timeseries;

    //compute
    void run();
};

class MVTimeseriesControlPrivate {
public:
    MVTimeseriesControl* q;
    QStringList m_button_names;

    TimeseriesCalculator m_calculator;
};

MVTimeseriesControl::MVTimeseriesControl(MVAbstractContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVTimeseriesControlPrivate;
    d->q = this;

    QGridLayout* glayout = new QGridLayout;

    d->m_button_names << "Raw Data"
                      << "Filtered Data"
                      << "Preprocessed Data";

    QStringList labels;
    labels << "Raw data "
           << "Filtered data"
           << "Filt + whitened data";

    int row = 0;
    for (int ii = 0; ii < d->m_button_names.count(); ii++) {
        QString name = d->m_button_names.value(ii);
        QRadioButton* B = this->createRadioButtonControl(name, labels.value(ii));
        glayout->addWidget(B, row, 0);
        if (name != "Raw Data") {
            QToolButton* TB = this->createToolButtonControl("Create " + name, "create...");
            glayout->addWidget(TB, row, 1);
            connect(TB, SIGNAL(clicked(bool)), this, SLOT(slot_create()));
            TB->setProperty("timeseries_name", name);
        }
        row++;
    }
    this->setLayout(glayout);

    connect(context, SIGNAL(timeseriesNamesChanged()), this, SLOT(updateControls()));
    connect(context, SIGNAL(currentTimeseriesChanged()), this, SLOT(updateControls()));

    connect(&d->m_calculator, SIGNAL(finished()), this, SLOT(slot_calculator_finished()));

    this->updateControls();
}

MVTimeseriesControl::~MVTimeseriesControl()
{
    delete d;
}

QString MVTimeseriesControl::title() const
{
    return "Timeseries";
}

void MVTimeseriesControl::updateContext()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    foreach (QString name, d->m_button_names) {
        if (this->controlValue(name).toBool()) {
            c->setCurrentTimeseriesName(name);
        }
    }
}

void MVTimeseriesControl::updateControls()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    this->setControlValue(c->currentTimeseriesName(), true);

    foreach (QString name, d->m_button_names) {
        bool val = c->timeseriesNames().contains(name);
        this->setControlEnabled(name, val);
    }
}

void MVTimeseriesControl::slot_create()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    QString name = sender()->property("timeseries_name").toString();
    CreateTimeseriesDialog dlg;
    dlg.setSampleRate(c->sampleRate());
    dlg.setFreqMin(c->option("suggest_freq_min", 300).toDouble());
    dlg.setFreqMax(c->option("suggest_freq_max", 6000).toDouble());
    dlg.setWindowTitle("Create " + name);
    if (dlg.exec() == QDialog::Accepted) {
        //create timeseries
        d->m_calculator.name = name;
        if (d->m_calculator.isRunning()) {
            d->m_calculator.requestInterruption();
            d->m_calculator.wait(1000);
            if (d->m_calculator.isRunning())
                d->m_calculator.terminate();
        }
        d->m_calculator.samplerate = dlg.sampleRate();
        d->m_calculator.freq_min = dlg.freqMin();
        d->m_calculator.freq_max = dlg.freqMax();
        d->m_calculator.raw = c->timeseries("Raw Data").makePath();
        d->m_calculator.whiten = false;
        if (name == "Preprocessed Data")
            d->m_calculator.whiten = true;
        d->m_calculator.start();
    }
}

void MVTimeseriesControl::slot_calculator_finished()
{
    QString output_timeseries = d->m_calculator.output_timeseries;
    if (output_timeseries.isEmpty())
        return;
    MVContext* context = (MVContext*)this->mvContext();
    context->addTimeseries(d->m_calculator.name, output_timeseries);
    context->setCurrentTimeseriesName(d->m_calculator.name);
}

void TimeseriesCalculator::run()
{
    this->output_timeseries = "";

    QString out = "";
    {
        MountainProcessRunner MPR;
        //MPR.setProcessorName("bandpass_filter");
        MPR.setProcessorName("ms3.bandpass_filter");
        QVariantMap params;
        params["timeseries"] = raw;
        params["samplerate"] = samplerate;
        params["freq_min"] = freq_min;
        params["freq_max"] = freq_max;
        MPR.setInputParameters(params);
        out = MPR.makeOutputFilePath("timeseries_out");
        MPR.runProcess();
        if (this->isInterruptionRequested())
            return;
    }

    if (!this->whiten) {
        this->output_timeseries = out;
    }
    else {
        MountainProcessRunner MPR;
        //MPR.setProcessorName("whiten");
        MPR.setProcessorName("ms3.whiten");
        QVariantMap params2;
        params2["timeseries"] = out;
        MPR.setInputParameters(params2);
        QString out2 = MPR.makeOutputFilePath("timeseries_out");
        MPR.runProcess();
        if (this->isInterruptionRequested())
            return;

        this->output_timeseries = out2;
    }
}
