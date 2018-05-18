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

#include "mvclusterlegend.h"
#include <QDebug>

class MVClusterLegendPrivate {
public:
    MVClusterLegend* q;

    QList<QColor> m_cluster_colors;
    QList<int> m_cluster_numbers;
    QSize m_parent_window_size;
    QList<QRectF> m_cluster_number_rects;
    int m_hovered_cluster_number = -1;
    QSet<int> m_active_cluster_numbers;

    QColor cluster_color(int k) const;
    int cluster_number_at(QPointF pt);
};

MVClusterLegend::MVClusterLegend()
{
    d = new MVClusterLegendPrivate;
    d->q = this;
}

MVClusterLegend::~MVClusterLegend()
{
    delete d;
}

void MVClusterLegend::paint(QPainter* painter)
{
    double spacing = 6;
    double margin = 10;
    double W = windowSize().width();
    //double H = m_parent_window_size.height();
    double text_height = qBound(12.0, W * 1.0 / 10, 25.0);
    double y0 = margin;
    QFont font = painter->font();
    font.setPixelSize(text_height - 2);
    painter->setFont(font);
    d->m_cluster_number_rects.clear();
    for (int i = 0; i < d->m_cluster_numbers.count(); i++) {
        QString str = QString("%1").arg(d->m_cluster_numbers[i]);
        double text_width = QFontMetrics(painter->font()).width(str);
        QRectF rect(W - margin - text_width, y0, text_width, text_height);
        QPen pen = painter->pen();

        if (d->m_active_cluster_numbers.contains(d->m_cluster_numbers[i])) {
            pen.setColor(d->cluster_color(d->m_cluster_numbers[i]));
            if (d->m_cluster_numbers[i] == d->m_hovered_cluster_number) {
                pen.setColor(Qt::white);
            }
        }
        else {
            pen.setColor(Qt::gray);
            if (d->m_cluster_numbers[i] == d->m_hovered_cluster_number) {
                pen.setColor(QColor(180, 180, 200));
            }
        }
        painter->setPen(pen);
        painter->drawText(rect, Qt::AlignRight, str);

        d->m_cluster_number_rects << rect;
        y0 += text_height + spacing;
    }
}

void MVClusterLegend::mousePressEvent(QMouseEvent* evt)
{
    Q_UNUSED(evt)
}

void MVClusterLegend::mouseReleaseEvent(QMouseEvent* evt)
{
    int cluster_number_at_pos = d->cluster_number_at(evt->pos());
    if (cluster_number_at_pos >= 0) {
        QSet<int> tmp = d->m_active_cluster_numbers;
        if (tmp.contains(cluster_number_at_pos))
            tmp.remove(cluster_number_at_pos);
        else
            tmp.insert(cluster_number_at_pos);
        this->setActiveClusterNumbers(tmp);
        evt->accept();
    }
}

void MVClusterLegend::mouseMoveEvent(QMouseEvent* evt)
{
    QPoint pt = evt->pos();
    int hovered_cluster_number = d->cluster_number_at(pt);
    if ((hovered_cluster_number != d->m_hovered_cluster_number)) {
        d->m_hovered_cluster_number = hovered_cluster_number;
        emit hoveredClusterNumberChanged();
        emit repaintNeeded();
    }
}

void MVClusterLegend::setClusterColors(const QList<QColor>& colors)
{
    if (d->m_cluster_colors == colors)
        return;
    d->m_cluster_colors = colors;
    emit repaintNeeded();
}

void MVClusterLegend::setClusterNumbers(const QList<int>& numbers)
{
    if (d->m_cluster_numbers == numbers)
        return;
    d->m_cluster_numbers = numbers;
    this->setActiveClusterNumbers(numbers.toSet());
    emit repaintNeeded();
}

int MVClusterLegend::hoveredClusterNumber() const
{
    return d->m_hovered_cluster_number;
}

QSet<int> MVClusterLegend::activeClusterNumbers() const
{
    return d->m_active_cluster_numbers;
}

void MVClusterLegend::setActiveClusterNumbers(const QSet<int>& active_numbers)
{
    if (d->m_active_cluster_numbers == active_numbers)
        return;
    d->m_active_cluster_numbers = active_numbers;
    emit repaintNeeded();
    emit activeClusterNumbersChanged();
}

QColor MVClusterLegendPrivate::cluster_color(int k) const
{
    if (k < 0)
        return Qt::black;
    if (k == 0)
        return Qt::gray;
    if (m_cluster_colors.isEmpty())
        return Qt::black;
    return m_cluster_colors[(k - 1) % m_cluster_colors.count()];
}

int MVClusterLegendPrivate::cluster_number_at(QPointF pt)
{
    for (int i = 0; i < m_cluster_number_rects.count(); i++) {
        if (m_cluster_number_rects[i].contains(pt))
            return m_cluster_numbers.value(i, -1);
    }
    return -1;
}
