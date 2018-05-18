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

#include "mvpanelwidget.h"
#include "paintlayerstack.h"
#include <QRectF>
#include <QDebug>

struct MVPanelWidgetPanel {
    int row = 0;
    int col = 0;
    PaintLayer* layer = 0;
    QRectF geom = QRectF(0, 0, 0, 0);
};

class MVPanelWidgetPrivate {
public:
    MVPanelWidget* q;

    QList<MVPanelWidgetPanel> m_panels;

    int m_row_margin = 3;
    int m_row_spacing = 3;
    int m_col_margin = 3;
    int m_col_spacing = 3;

    QRectF m_viewport_geom = QRectF(0, 0, 1, 1);
    int m_current_panel_index = -1;
    bool m_zoom_on_wheel = true;

    QPointF m_press_anchor = QPointF(-1, -1);
    QRectF m_press_anchor_viewport_geom = QRectF(0, 0, 1, 1);
    bool m_is_dragging = false;

    PanelWidgetBehavior m_behavior;

    void correct_viewport_geom();
    void zoom(double factor);
    void update_panel_geometries();
    int panel_index_at_pt(const QPointF& pt);
};

MVPanelWidget::MVPanelWidget()
{
    d = new MVPanelWidgetPrivate;
    d->q = this;

    this->setMouseTracking(true);
}

MVPanelWidget::~MVPanelWidget()
{
    delete d;
}

void MVPanelWidget::setBehavior(const PanelWidgetBehavior& behavior)
{
    d->m_behavior = behavior;
}

void MVPanelWidget::clearPanels(bool delete_layers)
{
    if (delete_layers) {
        foreach (MVPanelWidgetPanel panel, d->m_panels) {
            delete panel.layer;
        }
    }
    d->m_panels.clear();
    update();
}

void MVPanelWidget::addPanel(int row, int col, PaintLayer* layer)
{
    MVPanelWidgetPanel panel;
    panel.row = row;
    panel.col = col;
    panel.layer = layer;
    d->m_panels << panel;
    QObject::connect(layer, SIGNAL(repaintNeeded()), this, SLOT(update()));
    update();
}

int MVPanelWidget::rowCount() const
{
    int ret = 0;
    for (int i = 0; i < d->m_panels.count(); i++) {
        ret = qMax(ret, d->m_panels[i].row + 1);
    }
    return ret;
}

int MVPanelWidget::columnCount() const
{
    int ret = 0;
    for (int i = 0; i < d->m_panels.count(); i++) {
        ret = qMax(ret, d->m_panels[i].col + 1);
    }
    return ret;
}

void MVPanelWidget::setSpacing(int row_spacing, int col_spacing)
{
    d->m_row_spacing = row_spacing;
    d->m_col_spacing = col_spacing;
    update();
}

void MVPanelWidget::setMargins(int row_margin, int col_margin)
{
    d->m_row_margin = row_margin;
    d->m_col_margin = col_margin;
    update();
}

void MVPanelWidget::setZoomOnWheel(bool val)
{
    d->m_zoom_on_wheel = val;
}

int MVPanelWidget::currentPanelIndex() const
{
    return d->m_current_panel_index;
}

void MVPanelWidget::setCurrentPanelIndex(int index)
{
    d->m_current_panel_index = index;
    update();
}

void MVPanelWidget::setViewportGeometry(QRectF geom)
{
    d->m_viewport_geom = geom;
    update();
}

void MVPanelWidget::zoomIn()
{
    d->zoom(1.2);
}

void MVPanelWidget::zoomOut()
{
    d->zoom(1 / 1.2);
}

void MVPanelWidget::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    d->correct_viewport_geom();
    d->update_panel_geometries();

    QPen pen = painter.pen();
    for (int i = 0; i < d->m_panels.count(); i++) {
        QRectF geom = d->m_panels[i].geom;
        d->m_panels[i].layer->setWindowSize(geom.size().toSize());
        double x1 = geom.left();
        double y1 = geom.top();
        painter.translate(QPointF(x1, y1));
        QRegion hold_region = painter.clipRegion();
        painter.setClipRect(0, 0, geom.width(), geom.height());
        d->m_panels[i].layer->paint(&painter);
        painter.translate(QPointF(-x1, -y1));
        painter.setClipRegion(hold_region);
    }
}

void MVPanelWidget::wheelEvent(QWheelEvent* evt)
{
    if (d->m_zoom_on_wheel) {
        int delta = evt->delta();
        double factor = 1;
        if (delta > 0)
            factor = 1.1;
        else
            factor = 1 / 1.1;
        d->zoom(factor);
        evt->accept();
    }
    else {
        QWidget::wheelEvent(evt);
    }
}

void MVPanelWidget::mousePressEvent(QMouseEvent* evt)
{
    d->m_press_anchor = evt->pos();
    d->m_press_anchor_viewport_geom = d->m_viewport_geom;
    d->m_is_dragging = false;

    QWidget::mousePressEvent(evt);
}

void MVPanelWidget::mouseReleaseEvent(QMouseEvent* evt)
{
    d->m_press_anchor = QPointF(-1, -1);
    if (!d->m_is_dragging) {
        int index = d->panel_index_at_pt(evt->pos());
        if (index >= 0) {
            if (evt->button() == Qt::LeftButton)
                emit this->signalPanelClicked(index, evt->modifiers());
        }
    }
    d->m_is_dragging = false;
}

void MVPanelWidget::mouseDoubleClickEvent(QMouseEvent* evt)
{
    int index = d->panel_index_at_pt(evt->pos());
    if (index >= 0) {
        emit this->signalPanelActivated(index);
    }
}

void MVPanelWidget::mouseMoveEvent(QMouseEvent* evt)
{
    if (d->m_press_anchor.x() >= 0) {
        double dx = evt->pos().x() - d->m_press_anchor.x();
        double dy = evt->pos().y() - d->m_press_anchor.y();
        if ((qAbs(dx) >= 4) || (qAbs(dy) >= 4)) {
            d->m_is_dragging = true;
        }
        if (d->m_is_dragging) {
            d->m_viewport_geom = d->m_press_anchor_viewport_geom;
            if (this->columnCount() > 1) {
                d->m_viewport_geom.translate(dx / this->width(), 0);
            }
            if (this->rowCount() > 1) {
                d->m_viewport_geom.translate(0, dy / this->height());
            }
            this->update();
        }
    }
}

void MVPanelWidgetPrivate::correct_viewport_geom()
{
    QRectF G = m_viewport_geom;

    //respect the minimum panel width/height
    if ((m_behavior.h_scrollable) && (q->columnCount() > 1) && (!m_panels.isEmpty())) {
        if (G.width() * q->width() / m_panels.count() < m_behavior.minimum_panel_width) {
            G.setWidth(m_behavior.minimum_panel_width / q->width() * m_panels.count());
        }
    }
    if ((m_behavior.v_scrollable) && (q->rowCount() > 1) && (!m_panels.isEmpty())) {
        if (G.height() * q->height() / m_panels.count() < m_behavior.minimum_panel_height) {
            G.setHeight(m_behavior.minimum_panel_height / q->height() * m_panels.count());
        }
    }

    //don't let it be smaller than it could be
    if (G.width() < 1) {
        G = QRectF(0, G.top(), 1, G.height());
    }
    if (G.height() < 1) {
        G = QRectF(G.left(), 0, G.width(), 1);
    }

    //don't allow scroll too far to right or left, or down or up
    if (G.left() + G.width() < 1)
        G.translate(1 - G.left() - G.width(), 0);
    if (G.left() > 0)
        G.translate(-G.left(), 0);
    if (G.top() + G.height() < 1)
        G.translate(0, 1 - G.top() - G.height());
    if (G.top() > 0)
        G.translate(0, -G.top());

    m_viewport_geom = G;
}

void MVPanelWidgetPrivate::zoom(double factor)
{
    QRectF current_panel_geom = QRectF(0, 0, 1, 1);
    if ((m_current_panel_index >= 0) && (m_current_panel_index < m_panels.count())) {
        current_panel_geom = m_panels[m_current_panel_index].geom;
    }
    if ((q->rowCount() > 1) && (m_behavior.v_scrollable)) {
        m_viewport_geom.setHeight(m_viewport_geom.height() * factor);
    }
    if ((q->columnCount() > 1) && (m_behavior.h_scrollable)) {
        m_viewport_geom.setWidth(m_viewport_geom.width() * factor);
    }
    correct_viewport_geom();
    update_panel_geometries();
    QRectF new_current_panel_geom = QRectF(0, 0, 1, 1);
    if ((m_current_panel_index >= 0) && (m_current_panel_index < m_panels.count())) {
        new_current_panel_geom = m_panels[m_current_panel_index].geom;
    }
    //double dx = new_current_panel_geom.center().x() - current_panel_geom.center().x();
    //double dy = new_current_panel_geom.center().y() - current_panel_geom.center().y();
    double dx = new_current_panel_geom.left() - current_panel_geom.left();
    double dy = new_current_panel_geom.top() - current_panel_geom.top();
    if ((dx) || (dy)) {
        m_viewport_geom.translate(-dx / q->width(), 0);
        m_viewport_geom.translate(0, -dy / q->height());
    }
    correct_viewport_geom();
    update_panel_geometries();
    q->update();
}

void MVPanelWidgetPrivate::update_panel_geometries()
{
    int num_rows = q->rowCount();
    int num_cols = q->columnCount();
    if (!num_rows)
        return;
    if (!num_cols)
        return;
    double window_W = q->width();
    double window_H = q->height();
    QRectF VG = m_viewport_geom;
    QRectF vgeom = QRectF(VG.left() * window_W, VG.top() * window_H, VG.width() * window_W, VG.height() * window_H);
    double W0 = (vgeom.width() - (num_cols - 1) * m_col_spacing - m_col_margin * 2) / num_cols;
    double H0 = (vgeom.height() - (num_rows - 1) * m_row_spacing - m_row_margin * 2) / num_rows;
    for (int i = 0; i < m_panels.count(); i++) {
        double x1 = vgeom.x() + m_col_margin + m_panels[i].col * (W0 + m_col_spacing);
        double y1 = vgeom.y() + m_row_margin + m_panels[i].row * (H0 + m_row_spacing);
        m_panels[i].geom = QRectF(x1, y1, W0, H0);
    }
}

int MVPanelWidgetPrivate::panel_index_at_pt(const QPointF& pt)
{
    for (int i = 0; i < m_panels.count(); i++) {
        if (m_panels[i].geom.contains(pt))
            return i;
    }
    return -1;
}
