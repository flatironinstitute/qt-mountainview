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

#include "paintlayerstack.h"
#include <QDebug>

class PaintLayerStackPrivate {
public:
    PaintLayerStack* q;
    QList<PaintLayer*> m_layers;
};

PaintLayerStack::PaintLayerStack(QObject* parent)
    : PaintLayer(parent)
{
    d = new PaintLayerStackPrivate;
    d->q = this;
}

PaintLayerStack::~PaintLayerStack()
{
    delete d;
}

void PaintLayerStack::addLayer(PaintLayer* layer)
{
    d->m_layers << layer;

    /// Witold, is this the right thing to do?
    layer->setParent(this);

    layer->setWindowSize(this->windowSize());

    QObject::connect(layer, SIGNAL(repaintNeeded()), this, SIGNAL(repaintNeeded()));
}

void PaintLayerStack::paint(QPainter* painter)
{
    for (int i = 0; i < d->m_layers.count(); i++) {
        bool hold_export_mode = d->m_layers[i]->exportMode();
        d->m_layers[i]->setExportMode(this->exportMode());
        d->m_layers[i]->paint(painter);
        d->m_layers[i]->setExportMode(hold_export_mode);
    }
}

void PaintLayerStack::mousePressEvent(QMouseEvent* evt)
{
    evt->setAccepted(false);
    for (int i = d->m_layers.count() - 1; i >= 0; i--) {
        if (evt->isAccepted())
            break;
        d->m_layers[i]->mousePressEvent(evt);
    }
}

void PaintLayerStack::mouseReleaseEvent(QMouseEvent* evt)
{
    evt->setAccepted(false);
    for (int i = d->m_layers.count() - 1; i >= 0; i--) {
        if (evt->isAccepted())
            break;
        d->m_layers[i]->mouseReleaseEvent(evt);
    }
}

void PaintLayerStack::mouseMoveEvent(QMouseEvent* evt)
{
    evt->setAccepted(false);
    for (int i = d->m_layers.count() - 1; i >= 0; i--) {
        if (evt->isAccepted())
            break;
        d->m_layers[i]->mouseMoveEvent(evt);
    }
}

void PaintLayerStack::setWindowSize(QSize size)
{
    for (int i = 0; i < d->m_layers.count(); i++) {
        d->m_layers[i]->setWindowSize(size);
    }
    PaintLayer::setWindowSize(size);
}
