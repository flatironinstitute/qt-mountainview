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
#ifndef RENDERABLEWIDGET_H
#define RENDERABLEWIDGET_H

#include <QWidget>

class RenderableWidgetPrivate;
class RenderableWidget : public QWidget {
public:
    friend class RenderableWidgetPrivate;
    RenderableWidget(QWidget* parent = 0);
    virtual ~RenderableWidget();
    void setExportMode(bool val);
    bool exportMode() const;
    virtual QImage renderImage(int W, int H) = 0;

private:
    RenderableWidgetPrivate* d;
};

#endif // RENDERABLEWIDGET_H
