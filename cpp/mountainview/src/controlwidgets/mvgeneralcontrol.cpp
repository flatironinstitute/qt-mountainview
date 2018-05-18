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

#include "mvgeneralcontrol.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QTimer>
#include <mvcontext.h>

class MVGeneralControlPrivate {
public:
    MVGeneralControl* q;
};

MVGeneralControl::MVGeneralControl(MVAbstractContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVGeneralControlPrivate;
    d->q = this;

    QGridLayout* glayout = new QGridLayout;
    int row = 0;
    {
        QWidget* X = this->createChoicesControl("timeseries");
        X->setToolTip("The default timeseries used for all the views");
        connect(context, SIGNAL(timeseriesNamesChanged()), this, SLOT(updateControls()));
        connect(context, SIGNAL(currentTimeseriesChanged()), this, SLOT(updateControls()));
        glayout->addWidget(new QLabel("Timeseries:"), row, 0);
        glayout->addWidget(X, row, 1);
        row++;
    }
    this->setLayout(glayout);

    updateControls();
}

MVGeneralControl::~MVGeneralControl()
{
    delete d;
}

QString MVGeneralControl::title() const
{
    return "General Options";
}

void MVGeneralControl::updateContext()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    c->setCurrentTimeseriesName(this->controlValue("timeseries").toString());
}

void MVGeneralControl::updateControls()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    this->setChoices("timeseries", c->timeseriesNames());
    this->setControlValue("timeseries", c->currentTimeseriesName());
}
