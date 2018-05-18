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

#include "mvtemplatesview2panel.h"
#include "mlcommon.h"

class MVTemplatesView2PanelPrivate {
public:
    MVTemplatesView2Panel* q;
    Mda m_template;
    ElectrodeGeometry m_electrode_geometry;
    QList<QColor> m_channel_colors;
    QList<QRectF> m_electrode_boxes;
    int m_clip_size = 0;
    double m_vertical_scale_factor = 1;
    bool m_current = false;
    bool m_selected = false;
    QMap<QString, QColor> m_colors;
    QString m_title;
    double m_bottom_section_height = 0;
    double m_firing_rate_disk_diameter = 0;
    bool m_draw_ellipses = false;
    bool m_draw_disks = false;

    void setup_electrode_boxes(double W, double H);
    QPointF coord2pix(int m, int t, double val);
};

MVTemplatesView2Panel::MVTemplatesView2Panel()
{
    d = new MVTemplatesView2PanelPrivate;
    d->q = this;
}

MVTemplatesView2Panel::~MVTemplatesView2Panel()
{
    delete d;
}

void MVTemplatesView2Panel::setTemplate(const Mda& X)
{
    d->m_template = X;
}

void MVTemplatesView2Panel::setElectrodeGeometry(const ElectrodeGeometry& geom)
{
    d->m_electrode_geometry = geom;
}

void MVTemplatesView2Panel::setVerticalScaleFactor(double factor)
{
    d->m_vertical_scale_factor = factor;
}

void MVTemplatesView2Panel::setChannelColors(const QList<QColor>& colors)
{
    d->m_channel_colors = colors;
}

void MVTemplatesView2Panel::setColors(const QMap<QString, QColor>& colors)
{
    d->m_colors = colors;
}

void MVTemplatesView2Panel::setCurrent(bool val)
{
    d->m_current = val;
}

void MVTemplatesView2Panel::setSelected(bool val)
{
    d->m_selected = val;
}

void MVTemplatesView2Panel::setTitle(const QString& txt)
{
    d->m_title = txt;
}

void MVTemplatesView2Panel::setFiringRateDiskDiameter(double val)
{
    d->m_firing_rate_disk_diameter = val;
}

void MVTemplatesView2Panel::paint(QPainter* painter)
{
    painter->setRenderHint(QPainter::Antialiasing);

    QSize ss = this->windowSize();
    QPen pen = painter->pen();

    d->m_bottom_section_height = qMax(20.0, ss.height() * 0.1);

    if (!d->m_draw_disks)
        d->m_bottom_section_height = 0;

    QRect R(0, 0, ss.width(), ss.height());

    //BACKGROUND
    if (!this->exportMode()) {
        if (d->m_current) {
            painter->fillRect(R, d->m_colors["view_background_highlighted"]);
        }
        else if (d->m_selected) {
            painter->fillRect(R, d->m_colors["view_background_selected"]);
        }
        //else if (d->m_hovered) {
        //    painter->fillRect(R, d->m_colors["view_background_hovered"]);
        //}
        else {
            painter->fillRect(R, d->m_colors["view_background"]);
        }
    }

    if (!this->exportMode()) {
        //FRAME
        if (d->m_selected) {
            painter->setPen(QPen(d->m_colors["view_frame_selected"], 1));
        }
        else {
            painter->setPen(QPen(d->m_colors["view_frame"], 1));
        }
        painter->drawRect(R);
    }

    //TOP SECTION
    {
        QString txt = d->m_title;
        QFont fnt = this->font();
        QRectF R(5, 3, ss.width() - 10, fnt.pixelSize());
        //fnt.setPixelSize(qMin(16.0, qMax(10.0, qMin(d->m_top_section_height, ss.width() * 1.0) - 4)));
        //fnt.setPixelSize(14);
        painter->setFont(fnt);

        QPen pen = painter->pen();
        pen.setColor(d->m_colors["cluster_label"]);
        painter->setPen(pen);
        painter->drawText(R, Qt::AlignLeft | Qt::AlignVCenter, txt);
    }

    //BOTTOM SECTION
    if (d->m_bottom_section_height) {
        if (d->m_firing_rate_disk_diameter) {
            QPen pen_hold = painter->pen();
            QBrush brush_hold = painter->brush();
            painter->setPen(Qt::NoPen);
            painter->setBrush(QBrush(d->m_colors["firing_rate_disk"]));
            QRectF R(0, ss.height() - d->m_bottom_section_height, ss.width(), d->m_bottom_section_height);
            double tmp = qMin(R.width(), R.height());
            double rad = tmp * d->m_firing_rate_disk_diameter / 2 * 0.9;
            painter->drawEllipse(R.center(), rad, rad);
            painter->setPen(pen_hold);
            painter->setBrush(brush_hold);
        }
    }

    //SETUP ELECTRODE LOCATIONS
    d->setup_electrode_boxes(ss.width(), ss.height());

    //ELECTRODES AND WAVEFORMS
    int M = d->m_template.N1();
    int T = d->m_template.N2();
    d->m_clip_size = T;
    for (int m = 0; m < M; m++) {
        QPainterPath path;
        for (int t = 0; t < T; t++) {
            double val = d->m_template.value(m, t);
            QPointF pt = d->coord2pix(m, t, val);
            if (t == 0)
                path.moveTo(pt);
            else
                path.lineTo(pt);
        }
        pen.setColor(d->m_channel_colors.value(m, Qt::black));
        pen.setWidth(2);
        if (this->exportMode())
            pen.setWidth(6);
        painter->strokePath(path, pen);
        QRectF box = d->m_electrode_boxes.value(m);
        pen.setColor(QColor(180, 200, 200));
        pen.setWidth(1);
        painter->setPen(pen);
        //if (d->m_draw_ellipses)
        if (true)
            painter->drawEllipse(box);
    }
}

double estimate_spacing(const QList<QVector<double> >& coords)
{
    QVector<double> vals;
    for (int m = 0; m < coords.length(); m++) {
        QVector<double> dists;
        for (int i = 0; i < coords.length(); i++) {
            if (i != m) {
                double dx = coords[i].value(0) - coords[m].value(0);
                double dy = coords[i].value(1) - coords[m].value(1);
                double dist = sqrt(dx * dx + dy * dy);
                dists << dist;
            }
        }
        vals << MLCompute::min(dists);
    }
    return MLCompute::mean(vals);
}

void MVTemplatesView2PanelPrivate::setup_electrode_boxes(double W, double H)
{
    m_electrode_boxes.clear();

    int top_section_height = q->font().pixelSize();

    double W1 = W;
    double H1 = H - top_section_height - m_bottom_section_height;

    QList<QVector<double> > coords = m_electrode_geometry.coordinates;
    if (coords.isEmpty()) {
        int M = m_template.N1();
        int num_rows = qMax(1, (int)sqrt(M));
        int num_cols = ceil(M * 1.0 / num_rows);
        for (int m = 0; m < M; m++) {
            QVector<double> tmp;
            tmp << (m % num_cols) + 0.5 * ((m / num_cols) % 2) << m / num_cols;
            coords << tmp;
        }
    }
    if (coords.isEmpty())
        return;

    int D = coords[0].count();
    QVector<double> mins(D), maxs(D);
    for (int d = 0; d < D; d++) {
        mins[d] = maxs[d] = coords.value(0).value(d);
    }
    for (int m = 0; m < coords.count(); m++) {
        for (int d = 0; d < D; d++) {
            mins[d] = qMin(mins[d], coords[m][d]);
            maxs[d] = qMax(maxs[d], coords[m][d]);
        }
    }

    double spacing = estimate_spacing(coords);
    spacing = spacing * 0.75;

    //double W0 = maxs.value(0) - mins.value(0);
    //double H0 = maxs.value(1) - mins.value(1);
    double W0 = maxs.value(1) - mins.value(1);
    double H0 = maxs.value(0) - mins.value(0);

    double W0_padded = W0 + spacing * 4 / 3;
    double H0_padded = H0 + spacing * 4 / 3;

    double hscale_factor = 1;
    double vscale_factor = 1;

    if (W0_padded * H1 > W1 * H0_padded) {
        //limited by width
        if (W0_padded) {
            hscale_factor = W1 / W0_padded;
            vscale_factor = W1 / W0_padded;
        }
    }
    else {
        if (H0_padded)
            vscale_factor = H1 / H0_padded;
        if (W0_padded)
            hscale_factor = W1 / W0_padded;
    }

    double offset_x = (W1 - W0 * hscale_factor) / 2;
    double offset_y = top_section_height + (H1 - H0 * vscale_factor) / 2;
    for (int m = 0; m < coords.count(); m++) {
        QVector<double> c = coords[m];
        //double x0 = offset_x + (c.value(0) - mins.value(0)) * hscale_factor;
        //double y0 = offset_y + (c.value(1) - mins.value(1)) * vscale_factor;
        double x0 = offset_x + (c.value(1) - mins.value(1)) * hscale_factor;
        double y0 = offset_y + (c.value(0) - mins.value(0)) * vscale_factor;
        double radx = spacing * hscale_factor / 2;
        double rady = spacing * vscale_factor / 3.5;
        m_electrode_boxes << QRectF(x0 - radx, y0 - rady, radx * 2, rady * 2);
    }
}

QPointF MVTemplatesView2PanelPrivate::coord2pix(int m, int t, double val)
{
    QRectF R = m_electrode_boxes.value(m);
    QPointF Rcenter = R.center();
    double pctx = t * 1.0 / m_clip_size;
    double x0 = R.left() + (R.width()) * pctx;
    double y0 = Rcenter.y() - val * R.height() / 2 * m_vertical_scale_factor;
    return QPointF(x0, y0);
}
