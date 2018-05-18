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

#ifndef PAINTLAYER_H
#define PAINTLAYER_H

#include <QMouseEvent>
#include <QPainter>
#include <QWidget>
#include <renderablewidget.h>

class PaintLayerPrivate;
class PaintLayer : public QObject {
    Q_OBJECT
public:
    friend class PaintLayerPrivate;
    PaintLayer(QObject* parent = 0);
    virtual ~PaintLayer();

    virtual void paint(QPainter* painter) = 0;

    void setExportMode(bool val);
    bool exportMode() const;

    virtual void mousePressEvent(QMouseEvent* evt) { Q_UNUSED(evt) }
    virtual void mouseReleaseEvent(QMouseEvent* evt) { Q_UNUSED(evt) }
    virtual void mouseMoveEvent(QMouseEvent* evt) { Q_UNUSED(evt) }

    //used by the subclass
    QSize windowSize() const;
    QFont font() const;
    void setFont(QFont font);

    //used by the widget/paintlayerstack
    virtual void setWindowSize(QSize size);

protected:
signals:
    void windowSizeChanged();
    void repaintNeeded();

private:
    PaintLayerPrivate* d;
};

class PaintLayerWidgetPrivate;
class PaintLayerWidget : public RenderableWidget {
public:
    friend class PaintLayerWidgetPrivate;
    PaintLayerWidget(PaintLayer* PL);
    virtual ~PaintLayerWidget();
    QImage renderImage(int W, int H) Q_DECL_OVERRIDE;

protected:
    void paintEvent(QPaintEvent* evt);

private:
    PaintLayerWidgetPrivate* d;
};

#endif // PAINTLAYER_H
