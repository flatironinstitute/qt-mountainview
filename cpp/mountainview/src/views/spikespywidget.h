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

#ifndef SPIKESPYWIDGET_H
#define SPIKESPYWIDGET_H

#include "diskreadmda.h"
#include "mvabstractcontext.h"

#include <QWidget>
#include <diskreadmda32.h>

struct SpikeSpyViewData {
    DiskReadMda32 timeseries;
    DiskReadMda firings;
};

class SpikeSpyWidgetPrivate;
class SpikeSpyWidget : public QWidget {
    Q_OBJECT
public:
    friend class SpikeSpyWidgetPrivate;
    SpikeSpyWidget(MVAbstractContext* context);
    virtual ~SpikeSpyWidget();
    void addView(const SpikeSpyViewData& data);

private slots:
    void slot_show_tasks();
    void slot_open_mountainview();
    void slot_view_clicked();

private:
    SpikeSpyWidgetPrivate* d;
};

#endif // SPIKESPYWIDGET_H
