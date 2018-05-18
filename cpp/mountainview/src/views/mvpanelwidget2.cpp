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

#include "mvpanelwidget2.h"
#include "paintlayerstack.h"
#include <QRectF>
#include <QDebug>
#include <math.h>

struct MVPanelWidget2Panel {
    int row = 0;
    int col = 0;
    QRectF geometry = QRectF(0, 0, 0, 0);
    PaintLayer* layer = 0;
};

class MVPanelWidget2Private {
public:
    MVPanelWidget2* q;

    QList<MVPanelWidget2Panel> m_panels;

    int m_row_margin = 3;
    int m_row_spacing = 3;
    int m_col_margin = 3;
    int m_col_spacing = 3;

    double m_zoom_factor = 1;

    QPointF m_scroll_offset = QPointF(0, 0);
    int m_current_panel_index = -1;
    bool m_zoom_on_wheel = true;

    QPointF m_press_anchor = QPointF(-1, -1);
    QPointF m_press_anchor_scroll_offset = QPointF(0, 0);
    bool m_is_dragging = false;

    PanelWidget2Behavior m_behavior;

    void zoom(double by_factor);
    int panel_index_at_pt(const QPointF& pt);
    void update_panel_geometries();
};

MVPanelWidget2::MVPanelWidget2()
{
    d = new MVPanelWidget2Private;
    d->q = this;

    this->setMouseTracking(true);
}

MVPanelWidget2::~MVPanelWidget2()
{
    delete d;
}

void MVPanelWidget2::setBehavior(const PanelWidget2Behavior& behavior)
{
    d->m_behavior = behavior;
}

void MVPanelWidget2::clearPanels(bool delete_layers)
{
    if (delete_layers) {
        foreach (MVPanelWidget2Panel panel, d->m_panels) {
            delete panel.layer;
        }
    }
    d->m_panels.clear();
    update();
}

void MVPanelWidget2::addPanel(int row, int col, PaintLayer* layer)
{
    MVPanelWidget2Panel panel;
    panel.row = row;
    panel.col = col;
    panel.layer = layer;
    d->m_panels << panel;
    QObject::connect(layer, SIGNAL(repaintNeeded()), this, SLOT(update()));
    update();
}

int MVPanelWidget2::rowCount() const
{
    int ret = 0;
    for (int i = 0; i < d->m_panels.count(); i++) {
        ret = qMax(ret, d->m_panels[i].row + 1);
    }
    return ret;
}

int MVPanelWidget2::columnCount() const
{
    int ret = 0;
    for (int i = 0; i < d->m_panels.count(); i++) {
        ret = qMax(ret, d->m_panels[i].col + 1);
    }
    return ret;
}

void MVPanelWidget2::setSpacing(int row_spacing, int col_spacing)
{
    d->m_row_spacing = row_spacing;
    d->m_col_spacing = col_spacing;
    update();
}

void MVPanelWidget2::setMargins(int row_margin, int col_margin)
{
    d->m_row_margin = row_margin;
    d->m_col_margin = col_margin;
    update();
}

void MVPanelWidget2::setZoomOnWheel(bool val)
{
    d->m_zoom_on_wheel = val;
}

int MVPanelWidget2::currentPanelIndex() const
{
    return d->m_current_panel_index;
}

void MVPanelWidget2::setCurrentPanelIndex(int index)
{
    d->m_current_panel_index = index;
    update();
}

void MVPanelWidget2::zoomIn()
{
    d->zoom(1.2);
}

void MVPanelWidget2::zoomOut()
{
    d->zoom(1 / 1.2);
}

void MVPanelWidget2::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    d->update_panel_geometries();

    for (int i = 0; i < d->m_panels.count(); i++) {
        QRectF geom = d->m_panels[i].geometry;
        d->m_panels[i].layer->setWindowSize(geom.size().toSize());
        double x1 = geom.left() - d->m_scroll_offset.x();
        double y1 = geom.top() - d->m_scroll_offset.y();
        QRegion hold_region = painter.clipRegion();
        painter.translate(QPointF(x1, y1));
        painter.setClipRect(0, 0, geom.width(), geom.height());
        d->m_panels[i].layer->paint(&painter);
        painter.translate(QPointF(-x1, -y1));
        painter.setClipRegion(hold_region);
    }
}

void MVPanelWidget2::wheelEvent(QWheelEvent* evt)
{
    PanelWidget2Behavior B = d->m_behavior;
    int delta = evt->delta();

    if ((!B.h_scrollable) && (B.v_scrollable)) {
        d->m_scroll_offset.setY(d->m_scroll_offset.y() - delta);
        update();
        evt->accept();
    }
    else {
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
}

void MVPanelWidget2::mousePressEvent(QMouseEvent* evt)
{
    if (evt->button() == Qt::LeftButton) {
        d->m_press_anchor = evt->pos();
        d->m_press_anchor_scroll_offset = d->m_scroll_offset;
        d->m_is_dragging = false;
    }

    QWidget::mousePressEvent(evt);
}

void MVPanelWidget2::mouseReleaseEvent(QMouseEvent* evt)
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

void MVPanelWidget2::mouseDoubleClickEvent(QMouseEvent* evt)
{
    int index = d->panel_index_at_pt(evt->pos());
    if (index >= 0) {
        emit this->signalPanelActivated(index);
    }
}

void MVPanelWidget2::mouseMoveEvent(QMouseEvent* evt)
{
    if (d->m_press_anchor.x() >= 0) {
        double dx = evt->pos().x() - d->m_press_anchor.x();
        double dy = evt->pos().y() - d->m_press_anchor.y();
        if ((qAbs(dx) >= 4) || (qAbs(dy) >= 4)) {
            d->m_is_dragging = true;
        }
        if (d->m_is_dragging) {
            d->m_scroll_offset = d->m_press_anchor_scroll_offset - QPointF(dx, dy);
            this->update();
        }
    }
}

void MVPanelWidget2Private::zoom(double by_factor)
{
    m_zoom_factor *= by_factor;
    q->update();
    /*
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
    */
}

void MVPanelWidget2Private::update_panel_geometries()
{
    PanelWidget2Behavior B = m_behavior;

    if (B.adjust_layout_to_preferred_size) {
        if ((B.v_scrollable) && (!B.h_scrollable)) {
            double WW = q->width() - m_col_margin * 2;
            int num_cols = qMax(1.0, ceil(WW / (B.preferred_panel_width * m_zoom_factor + m_col_spacing)));
            for (int i = 0; i < m_panels.count(); i++) {
                m_panels[i].row = i / num_cols;
                m_panels[i].col = i % num_cols;
            }
        }
        else if ((!B.v_scrollable) && (B.h_scrollable)) {
            double HH = q->height() - m_row_margin * 2;
            int num_rows = qMax(1.0, ceil(HH / (B.preferred_panel_height * m_zoom_factor + m_row_spacing)));
            int num_cols = qMax(1, m_panels.count() / num_rows);
            for (int i = 0; i < m_panels.count(); i++) {
                m_panels[i].row = i / num_cols;
                m_panels[i].col = i % num_cols;
            }
        }
        else {
            qWarning() << "Unable to adjust layout to preferred size:" << B.h_scrollable << B.v_scrollable;
        }
    }

    int num_rows = qMax(1, q->rowCount());
    int num_cols = qMax(1, q->columnCount());

    double panel_width = 0;
    if (B.h_scrollable) {
        panel_width = B.preferred_panel_width * m_zoom_factor;
    }
    else {
        double W0 = q->width() - (num_cols - 1) * m_col_spacing - 2 * m_col_margin;
        panel_width = W0 / num_cols;
    }

    double panel_height = 0;
    if (B.v_scrollable) {
        panel_height = B.preferred_panel_height * m_zoom_factor;
    }
    else {
        double H0 = q->height() - (num_rows - 1) * m_row_spacing - 2 * m_row_margin;
        panel_height = H0 / num_rows;
    }

    for (int i = 0; i < m_panels.count(); i++) {
        double x0 = m_col_margin + (panel_width + m_col_spacing) * m_panels[i].col;
        double y0 = m_row_margin + (panel_height + m_row_spacing) * m_panels[i].row;
        m_panels[i].geometry = QRectF(x0, y0, panel_width, panel_height);
    }

    if (!B.h_scrollable) {
        m_scroll_offset.setX(0);
    }
    if (!B.v_scrollable) {
        m_scroll_offset.setY(0);
    }

    double W1 = 2 * m_col_margin + num_cols * (panel_width + m_col_spacing);
    if (m_scroll_offset.x() + q->width() > W1)
        m_scroll_offset.setX(W1 - q->width());
    if (m_scroll_offset.x() < 0)
        m_scroll_offset.setX(0);

    double H1 = 2 * m_row_margin + num_rows * (panel_height + m_row_spacing);
    if (m_scroll_offset.y() + q->height() > H1)
        m_scroll_offset.setY(H1 - q->height());
    if (m_scroll_offset.y() < 0)
        m_scroll_offset.setY(0);
}

int MVPanelWidget2Private::panel_index_at_pt(const QPointF& pt)
{
    for (int i = 0; i < m_panels.count(); i++) {
        if (m_panels[i].geometry.contains(pt + m_scroll_offset))
            return i;
    }
    return -1;
}
