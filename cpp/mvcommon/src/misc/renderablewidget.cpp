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

#include "renderablewidget.h"

class RenderableWidgetPrivate {
public:
    RenderableWidget* q;
    bool m_export_mode = false;
};

RenderableWidget::RenderableWidget(QWidget* parent)
    : QWidget(parent)
{
    d = new RenderableWidgetPrivate;
    d->q = this;
}

RenderableWidget::~RenderableWidget()
{
    delete d;
}

void RenderableWidget::setExportMode(bool val)
{
    d->m_export_mode = val;
}

bool RenderableWidget::exportMode() const
{
    return d->m_export_mode;
}
