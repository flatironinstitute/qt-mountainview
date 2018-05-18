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
#include "ftelectrodearrayview.h"
#include "math.h"

#include <QPainter>
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QMouseEvent>

class FTElectrodeArrayViewPrivate {
public:
    FTElectrodeArrayView* q;
    Mda m_waveform;
    Mda m_waveform_absmax_vals;
    Mda m_waveform_absmax_inds;
    bool m_normalize_intensity;
    float m_waveform_absmax;
    float m_global_absmax;
    Mda m_electrode_locations;
    float m_min_electrode_spacing;
    float m_electrode_minx, m_electrode_maxx;
    float m_electrode_miny, m_electrode_maxy;
    float m_pixel_spacing_x, m_pixel_spacing_y;
    int m_timepoint;
    int m_animate_timepoint;
    bool m_animation_paused;
    QList<QPointF> m_electrode_pixel_locations;
    int m_hovered_index;
    QSet<int> m_selected_indices;
    int m_colorbar_width;
    bool m_show_channel_numbers;
    bool m_auto_select_channels;
    float m_brightness;
    float m_animation_speed;
    QDateTime m_time_of_last_animation;
    bool m_loop_animation;

    QPointF ind2pix(int i);
    QColor fire_color_map(float pct);
    QColor color_map(float pct);
    int find_electrode_index_at(QPointF pt);
    float get_electrode_pixel_radius();
};

FTElectrodeArrayView::FTElectrodeArrayView(QWidget* parent)
    : QWidget(parent)
{
    d = new FTElectrodeArrayViewPrivate;
    d->q = this;

    d->m_waveform_absmax = 1;
    d->m_global_absmax = 0;
    d->m_normalize_intensity = true;

    d->m_min_electrode_spacing = 1;
    d->m_electrode_minx = 0;
    d->m_electrode_maxx = 1;
    d->m_electrode_miny = 0;
    d->m_electrode_maxy = 1;

    d->m_pixel_spacing_x = 10;
    d->m_pixel_spacing_y = 10;

    d->m_colorbar_width = 6;

    d->m_hovered_index = -1;

    d->m_show_channel_numbers = false;
    d->m_auto_select_channels = true;

    d->m_brightness = 0;

    d->m_animation_speed = 4;
    d->m_loop_animation = true;

    QPalette pal = this->palette();
    int tmp = (int)(255 * 0.8);
    pal.setColor(QPalette::Background, QColor(tmp, tmp, tmp));
    this->setPalette(pal);

    d->m_timepoint = -1;
    d->m_animate_timepoint = -1;
    d->m_animation_paused = true;

    this->setMouseTracking(true);

    QTimer::singleShot(100, this, SLOT(slot_timer()));
}

void FTElectrodeArrayView::setElectrodeLocations(const Mda& L)
{
    d->m_electrode_locations = L;
    d->m_min_electrode_spacing = 99999;
    d->m_electrode_minx = 99999;
    d->m_electrode_maxx = -99999;
    d->m_electrode_miny = 99999;
    d->m_electrode_maxy = -99999;
    int M = d->m_electrode_locations.N1();
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            float x1 = d->m_electrode_locations.value(i, 0);
            float y1 = d->m_electrode_locations.value(i, 1);
            float x2 = d->m_electrode_locations.value(j, 0);
            float y2 = d->m_electrode_locations.value(j, 1);
            float val = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
            if (val != 0) {
                if (val < d->m_min_electrode_spacing) {
                    d->m_min_electrode_spacing = val;
                }
            }
            if (x1 < d->m_electrode_minx)
                d->m_electrode_minx = x1;
            if (x1 > d->m_electrode_maxx)
                d->m_electrode_maxx = x1;
            if (y1 < d->m_electrode_miny)
                d->m_electrode_miny = y1;
            if (y1 > d->m_electrode_maxy)
                d->m_electrode_maxy = y1;
        }
    }
    this->update();
}

struct electrod_sort_struct {
    float x, y;
    int ind;
};

struct electrode_sort_comparer {
    bool operator()(const electrod_sort_struct& a, const electrod_sort_struct& b) const
    {
        if (a.y < b.y)
            return true;
        if (a.y > b.y)
            return false;
        if (a.x < b.x)
            return true;
        return false;
    }
};

void FTElectrodeArrayView::setWaveform(const Mda& X)
{
    d->m_waveform = X;
    int M = d->m_waveform.N1();
    int T = d->m_waveform.N2();
    d->m_waveform_absmax_vals.allocate(M, 1);
    d->m_waveform_absmax_inds.allocate(M, 1);
    d->m_waveform_absmax = 0;
    for (int m = 0; m < M; m++) {
        float maxval = 0;
        int maxind = 0;
        for (int t = 0; t < T; t++) {
            float val = qAbs(d->m_waveform.value(m, t));
            if (val > maxval) {
                maxval = val;
                maxind = t;
            }
        }
        d->m_waveform_absmax_vals.setValue(d->m_waveform.value(m, maxind), m, 0);
        d->m_waveform_absmax_inds.setValue(maxind, m, 0);
        if (maxval > d->m_waveform_absmax)
            d->m_waveform_absmax = maxval;
    }

    if (d->m_auto_select_channels) {
        int ct = d->m_selected_indices.count();
        d->m_selected_indices.clear();
        M = d->m_waveform_absmax_vals.N1();
        QList<electrod_sort_struct> list;
        for (int i = 0; i < M; i++) {
            electrod_sort_struct tmp0;
            tmp0.ind = i;
            tmp0.x = 0;
            tmp0.y = qAbs(d->m_waveform_absmax_vals.value(i, 0));
            list << tmp0;
        }
        qSort(list.begin(), list.end(), electrode_sort_comparer());
        for (int i = 0; i < ct; i++) {
            if (i < list.count()) {
                d->m_selected_indices.insert(list[list.count() - 1 - i].ind);
            }
        }
    }

    this->update();
}

Mda* FTElectrodeArrayView::waveform()
{
    return &d->m_waveform;
}

int FTElectrodeArrayView::timepoint()
{
    return d->m_timepoint;
}

void FTElectrodeArrayView::setTimepoint(int val)
{
    d->m_animate_timepoint = val;
    if (d->m_timepoint != val) {
        d->m_timepoint = val;
        update();
        repaint();
        emit signalTimepointChanged();
    }
}

void FTElectrodeArrayView::animate()
{
    d->m_animation_paused = false;
    d->m_time_of_last_animation = QDateTime::currentDateTime();
    if (d->m_animate_timepoint < 0)
        d->m_animate_timepoint = 0;
}

void FTElectrodeArrayView::setLoopAnimation(bool val)
{
    d->m_loop_animation = val;
}

void FTElectrodeArrayView::pauseAnimation()
{
    d->m_animation_paused = true;
}

void FTElectrodeArrayView::stopAnimation()
{
    d->m_animate_timepoint = -1;
}

bool FTElectrodeArrayView::isAnimating()
{
    return ((d->m_animate_timepoint >= 0) && (!d->m_animation_paused));
}

void FTElectrodeArrayView::setAnimationSpeed(float hz)
{
    d->m_animation_speed = hz;
}

void FTElectrodeArrayView::setShowChannelNumbers(bool val)
{
    if (d->m_show_channel_numbers != val) {
        d->m_show_channel_numbers = val;
        this->update();
    }
}

void FTElectrodeArrayView::setAutoSelectChannels(bool val)
{
    d->m_auto_select_channels = val;
}

void FTElectrodeArrayView::setGlobalAbsMax(float val)
{
    d->m_global_absmax = val;
}

void FTElectrodeArrayView::setNormalizeIntensity(bool val)
{
    if (d->m_normalize_intensity != val) {
        d->m_normalize_intensity = val;
        this->update();
    }
}

void FTElectrodeArrayView::setBrightness(float val)
{
    if (d->m_brightness != val) {
        d->m_brightness = val;
        update();
    }
}

QList<int> FTElectrodeArrayView::selectedElectrodeIndices()
{
    QList<electrod_sort_struct> list;
    foreach (int ind, d->m_selected_indices) {
        electrod_sort_struct aa;
        aa.x = d->m_electrode_locations.value(ind, 0);
        aa.y = d->m_electrode_locations.value(ind, 1);
        aa.ind = ind;
        list.append(aa);
    }
    qSort(list.begin(), list.end(), electrode_sort_comparer());

    QList<int> ret;
    for (int i = 0; i < list.count(); i++)
        ret << list[i].ind;
    return ret;
}

void FTElectrodeArrayView::setSelectedElectrodeIndices(const QList<int>& X)
{
    d->m_selected_indices = QSet<int>::fromList(X);
    update();
}

void FTElectrodeArrayView::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    d->ind2pix(0); //to set d->m_pixel_spacing_*
    float spacing = d->get_electrode_pixel_radius();
    float absmax = 0;
    if (d->m_normalize_intensity)
        absmax = d->m_waveform_absmax;
    else
        absmax = d->m_global_absmax;
    if (absmax == 0)
        absmax = 1;
    int M = d->m_electrode_locations.N1();
    d->m_electrode_pixel_locations.clear();
    double miny = 9999;
    double maxy = -9999;
    for (int i = 0; i < M; i++) {
        float val = 0;
        if (d->m_timepoint < 0)
            val = (d->m_waveform_absmax_vals.value(i, 0) / absmax);
        else
            val = (d->m_waveform.value(i, d->m_timepoint)) / absmax;

        if (val > 0)
            val = 1 - pow((1 - val), 2);
        else
            val = -(1 - pow(1 + val, 2));
        float brightness_factor = exp(d->m_brightness / 100 * 3);
        QColor col = d->color_map(val * brightness_factor);
        QPointF pt = d->ind2pix(i);
        d->m_electrode_pixel_locations << pt;
        painter.setBrush(QBrush(col));
        int tmp = (int)(255 * 0.7);
        QColor pen_color = QColor(tmp, tmp, tmp);
        int pen_width = 1;
        if (i == d->m_hovered_index) {
            pen_color = Qt::yellow;
            pen_width = 3;
        }
        if (d->m_selected_indices.contains(i)) {
            pen_color = Qt::darkGreen;
            pen_width = qMax(pen_width, 2);
        }
        painter.setPen(QPen(QBrush(pen_color), pen_width));
        painter.drawEllipse(pt, spacing, spacing);
        miny = qMin(miny, pt.y() - spacing);
        maxy = qMax(maxy, pt.y() + spacing);

        if (d->m_show_channel_numbers) {
            QString txt = QString("%1").arg(i + 1);
            QFont fnt = painter.font();
            int pixsize = qMin(spacing - 1, 16.0F);
            fnt.setPixelSize(pixsize);
            painter.setFont(fnt);
            painter.setPen(Qt::darkGreen);
            painter.drawText(QRectF(pt.x() - spacing - 1, pt.y() - spacing, spacing * 2, spacing * 2), Qt::AlignCenter | Qt::AlignVCenter | Qt::TextDontClip, txt);
        }
    }

    int y1 = (int)miny;
    int y2 = (int)maxy;
    for (int y = y1; y <= y2; y++) {
        float pct = -(y - (y1 + y2) / 2) * 2.0 / (y1 + y2);
        QColor col = d->color_map(pct);
        painter.setPen(col);
        painter.drawLine(width() - d->m_colorbar_width - d->m_pixel_spacing_x / 2, y, width() - d->m_pixel_spacing_x / 2, y);
    }
}

void FTElectrodeArrayView::mouseMoveEvent(QMouseEvent* evt)
{
    int ind = d->find_electrode_index_at(evt->pos());
    if (ind != d->m_hovered_index) {
        d->m_hovered_index = ind;
        update();
    }
}

void FTElectrodeArrayView::mousePressEvent(QMouseEvent* evt)
{
    int ind = d->find_electrode_index_at(evt->pos());
    if (ind >= 0) {
        if (evt->button() == Qt::RightButton) {
            if (d->m_selected_indices.contains(ind)) {
                d->m_selected_indices.remove(ind);
            }
            else {
                d->m_selected_indices.insert(ind);
            }
            emit signalSelectedElectrodesChanged();
            update();
        }
        else if (evt->button() == Qt::LeftButton) {
            emit signalElectrodeLeftClicked(ind);
        }
    }
}

void FTElectrodeArrayView::slot_timer()
{
    int T = d->m_waveform.N2();
    int msec = 1000;
    if ((d->m_animate_timepoint >= 0) && (!d->m_animation_paused)) {
        int msec_incr = (int)(1 / d->m_animation_speed * 1000);
        int elapsed = d->m_time_of_last_animation.msecsTo(QDateTime::currentDateTime());
        while ((elapsed >= msec_incr) && (msec_incr > 0)) {
            elapsed -= msec_incr;
            d->m_animate_timepoint++;
            d->m_time_of_last_animation = QDateTime::currentDateTime();
        }
        if (d->m_timepoint != d->m_animate_timepoint) {
            d->m_timepoint = d->m_animate_timepoint;
            emit signalTimepointChanged();
        }
        msec = msec_incr - elapsed + 1;
        if (msec <= 0)
            msec = 1;
        if (d->m_loop_animation) {
            if (d->m_animate_timepoint >= T) {
                d->m_animate_timepoint = 0;
                //emit signalLoop(); don't do this for now
            }
        }
        if ((d->m_animate_timepoint >= T) || (msec_incr == 0)) {
            d->m_animate_timepoint = -1;
            d->m_timepoint = -1;
            emit signalTimepointChanged();
        }
        this->update();
        this->repaint();
    }
    else
        msec = 400;
    QTimer::singleShot(msec, this, SLOT(slot_timer()));
}

FTElectrodeArrayView::~FTElectrodeArrayView()
{
    delete d;
}

QPointF FTElectrodeArrayViewPrivate::ind2pix(int i)
{
    float x0 = m_electrode_locations.value(i, 0);
    float y0 = m_electrode_locations.value(i, 1);

    float minx = m_electrode_minx - m_min_electrode_spacing;
    float maxx = m_electrode_maxx + m_min_electrode_spacing;
    float miny = m_electrode_miny - m_min_electrode_spacing;
    float maxy = m_electrode_maxy + m_min_electrode_spacing;
    float spacing = m_min_electrode_spacing;

    float width0 = maxx - minx + 2 * spacing;
    float height0 = maxy - miny + 2 * spacing;

    float factorx = (q->width() - m_colorbar_width) / width0;
    float factory = q->height() / height0;
    float factor0 = qMin(factorx, factory);

    float offsetx = (q->width() - width0 * factor0) / 2;
    float offsety = (q->height() - height0 * factor0) / 2;

    float x1 = offsetx + (spacing + x0 - minx) * factor0;
    float y1 = offsety + (spacing + y0 - miny) * factor0;

    m_pixel_spacing_x = m_min_electrode_spacing * factor0;
    m_pixel_spacing_y = m_min_electrode_spacing * factor0;

    return QPointF(x1, y1);
}

QColor FTElectrodeArrayViewPrivate::color_map(float pct)
{
    float r, g, b;
    if (pct > 0) {
        pct = qMin(1.0F, pct);
        b = 1;
        r = 1 - pct;
        g = 1 - pct * pct;
    }
    else {
        pct = -pct;
        pct = qMin(1.0F, pct);
        r = 1;
        g = 1 - pct;
        b = 1 - pct * pct;
    }
    return QColor((int)qMin(255.0F, 255 * r), (int)qMin(255.0F, 255 * g), (int)qMin(255.0F, 255 * b));
}

QColor FTElectrodeArrayViewPrivate::fire_color_map(float pct)
{
    bool neg = (pct < 0);
    pct = qAbs(pct);
    if (pct > 1)
        pct = 1;
    float r = 0, g = 0, b = 0;

    if (pct < 0.5)
        r = pct / 0.5 * 255;
    else
        r = 255;

    if (pct < 0.4)
        g = 0;
    else if (pct < 0.9)
        g = (pct - 0.4) / 0.5 * 255;
    else
        g = 255;

    if (pct < 0.5)
        b = 220 - qAbs(pct - 0.25) / 0.25 * 220;
    else if (pct < 0.8)
        b = 0;
    else
        b = 255 - (1 - pct) / 0.2 * 255;

    if (neg) {
        return QColor((int)g, (int)r, (int)b);
    }
    else {
        return QColor((int)r, (int)g, (int)b);
    }
}

int FTElectrodeArrayViewPrivate::find_electrode_index_at(QPointF pt)
{
    int ret = -1;
    float spacing = get_electrode_pixel_radius();
    for (int i = 0; i < m_electrode_pixel_locations.count(); i++) {
        QPointF diff = pt - m_electrode_pixel_locations[i];
        float dist = sqrt(diff.x() * diff.x() + diff.y() * diff.y());
        if (dist <= spacing * 1.5)
            ret = i;
    }
    return ret;
}

float FTElectrodeArrayViewPrivate::get_electrode_pixel_radius()
{
    return qMax(3.0, (qMin(m_pixel_spacing_x, m_pixel_spacing_y) - 3.0) / 2);
}
