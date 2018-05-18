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
#include "createtimeseriesdialog.h"

#include "ui_createtimeseriesdialog.h"

class CreateTimeseriesDialogPrivate {
public:
    CreateTimeseriesDialog* q;
    Ui_CreateTimeseriesDialog ui;
};

CreateTimeseriesDialog::CreateTimeseriesDialog()
{
    d = new CreateTimeseriesDialogPrivate;
    d->q = this;
    d->ui.setupUi(this);
    d->ui.samplerate->setReadOnly(true);
}

CreateTimeseriesDialog::~CreateTimeseriesDialog()
{
    delete d;
}

void CreateTimeseriesDialog::setSampleRate(double val)
{
    d->ui.samplerate->setText(QString("%1").arg(val));
}

void CreateTimeseriesDialog::setFreqMax(double val)
{
    d->ui.freq_max->setText(QString("%1").arg(val));
}

void CreateTimeseriesDialog::setFreqMin(double val)
{
    d->ui.freq_min->setText(QString("%1").arg(val));
}

double CreateTimeseriesDialog::sampleRate() const
{
    return d->ui.samplerate->text().toDouble();
}

double CreateTimeseriesDialog::freqMin() const
{
    return d->ui.freq_min->text().toDouble();
}

double CreateTimeseriesDialog::freqMax() const
{
    return d->ui.freq_max->text().toDouble();
}
