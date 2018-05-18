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
#include "correlationmatrixview.h"

#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

class CorrelationMatrixViewPrivate {
public:
    CorrelationMatrixView* q;
    Mda m_matrix;
    QColor get_color_at(QPointF pt);
    QPoint pixel2index(QPointF pt);
    int m_margin_x, m_margin_y;
    QPoint m_hover_index;
    QString m_status_text;
    void schedule_update();
    void update_status();
    void set_status(const QString& txt);
};

CorrelationMatrixView::CorrelationMatrixView(QWidget* parent)
    : QWidget(parent)
{
    d = new CorrelationMatrixViewPrivate;
    d->q = this;
    d->m_margin_x = 50;
    d->m_margin_y = 50;
    d->m_hover_index = QPoint(-1, -1);

    this->setMouseTracking(true);
}

CorrelationMatrixView::~CorrelationMatrixView()
{
    delete d;
}

void CorrelationMatrixView::setMatrix(const Mda& CM)
{
    d->m_matrix = CM;
    update();
}

void CorrelationMatrixView::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)
    QPainter painter(this);

    QImage X = QImage(width(), height(), QImage::Format_RGB32);
    X.fill(0);
    for (int y = 0; y < height(); y++) {
        for (int x = 0; x < width(); x++) {
            QColor col = d->get_color_at(QPointF(x, y));
            X.setPixel(QPoint(x, y), col.rgb());
        }
    }

    painter.drawImage(0, 0, X);

    QRect text_rect(d->m_margin_x, height() - d->m_margin_y, width(), d->m_margin_y);
    painter.drawText(text_rect, d->m_status_text, Qt::AlignTop | Qt::AlignLeft);
}

void CorrelationMatrixView::mouseMoveEvent(QMouseEvent* evt)
{
    QPoint pos = evt->pos();
    QPoint idx = d->pixel2index(pos);
    if (idx != d->m_hover_index) {
        d->m_hover_index = idx;
        d->update_status();
        update();
    }
}

QColor CorrelationMatrixViewPrivate::get_color_at(QPointF pt)
{
    QPoint index = pixel2index(pt);
    if (index.x() < 0) {
        return QColor(150, 150, 200);
    }
    float val = m_matrix.value(index.x(), index.y());
    int val0 = val * 256;
    if (val0 < 0)
        val0 = 0;
    if (val0 > 255)
        val0 = 255;
    int rr = val0, gg = val0, bb = val0;
    if (index == m_hover_index) {
        bb = 255;
        rr = rr * 3 / 4;
        gg = gg * 3 / 4;
    }
    return QColor(rr, gg, bb);
}

QPoint CorrelationMatrixViewPrivate::pixel2index(QPointF pt)
{
    int W0 = q->width() - m_margin_x * 2;
    int H0 = q->height() - m_margin_y * 2;
    int N1 = m_matrix.N1();
    int N2 = m_matrix.N2();
    float square_size = 1;
    if (W0 * N2 < H0 * N1) {
        //limited by width
        square_size = W0 * 1.0 / N1;
    }
    else {
        //limited by height
        square_size = H0 * 1.0 / N2;
    }
    float offset_x = (q->width() - N1 * square_size) / 2;
    float offset_y = (q->height() - N2 * square_size) / 2;
    if (square_size <= 0)
        square_size = 1;
    float xx = (pt.x() - offset_x) / square_size;
    float yy = (pt.y() - offset_y) / square_size;

    if ((xx < 0) || (yy < 0))
        return QPoint(-1, -1);
    if ((xx >= N1) || (yy >= N2))
        return QPoint(-1, -1);

    return QPoint((int)xx, (int)yy);
}

void CorrelationMatrixViewPrivate::update_status()
{
    QString txt = "";
    if (m_hover_index.x() >= 0) {
        float val = m_matrix.value(m_hover_index.x(), m_hover_index.y());
        txt = QString("[%1,%2] : %3").arg(m_hover_index.x() + 1).arg(m_hover_index.y() + 1).arg(QString::number(val, 'f', 2));
    }
    set_status(txt);
}

void CorrelationMatrixViewPrivate::set_status(const QString& txt)
{
    if (txt != m_status_text) {
        m_status_text = txt;
        q->update();
    }
}
