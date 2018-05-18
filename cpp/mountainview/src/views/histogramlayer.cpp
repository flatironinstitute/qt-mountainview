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
#include "histogramlayer.h"
#include <QPaintEvent>
#include <QPainter>
#include <QImage>
#include "mvutils.h"
#include "mlcommon.h"
#include <QMenu>

struct BinInfo {
    double bin_min = -1;
    double bin_max = 1;
    int num_bins = 200;
};

class HistogramLayerPrivate {
public:
    HistogramLayer* q;
    QVector<double> m_data;
    QVector<double> m_bin_lefts;
    QVector<double> m_bin_rights;
    QVector<int> m_bin_counts;
    QVector<double> m_bin_densities;
    QVector<double> m_second_data;
    QVector<int> m_second_bin_counts;
    QVector<double> m_second_bin_densities;

    BinInfo m_bin_info;
    double m_max_bin_density = 0;
    double m_time_constant = 30;
    HistogramLayer::TimeScaleMode m_time_scale_mode = HistogramLayer::Uniform;

    bool m_update_required = false;
    QColor m_fill_color = QColor(120, 120, 150);
    QColor m_line_color = QColor(100, 100, 130);
    int m_hovered_bin_index = -1;
    int m_margin_left = 5, m_margin_right = 5, m_margin_top = 5, m_margin_bottom = 14;
    QString m_title;
    QString m_caption;
    QMap<QString, QColor> m_colors;
    bool m_hovered = false;
    bool m_current = false;
    bool m_selected = false;
    bool m_draw_vertical_axis_at_zero = false;
    QList<double> m_vertical_lines;
    QList<double> m_tick_marks;
    MVRange m_xrange = MVRange(0, 0);

    void update_bin_counts();
    QPointF coord2pix(QPointF pt, int W = 0, int H = 0);
    QPointF pix2coord(QPointF pt, int W = 0, int H = 0);
    int get_bin_index_at(QPointF pt);
    void do_paint(QPainter& painter, int W, int H);

    double transform1(double t); //for example log, or identity
    double transform2(double x); //for example exp, or identity

    void set_bins();
};

HistogramLayer::HistogramLayer(QObject* parent)
    : PaintLayer(parent)
{
    d = new HistogramLayerPrivate;
    d->q = this;
}

HistogramLayer::~HistogramLayer()
{
    delete d;
}

void HistogramLayer::setData(const QVector<double>& values)
{
    d->m_data = values;
    d->m_update_required = true;
}

void HistogramLayer::setSecondData(const QVector<double>& values)
{
    d->m_second_data = values;
    d->m_update_required = true;
}

void HistogramLayer::setBinInfo(double bin_min, double bin_max, int num_bins)
{
    d->m_bin_info.bin_min = bin_min;
    d->m_bin_info.bin_max = bin_max;
    d->m_bin_info.num_bins = num_bins;
    d->set_bins();
}

void HistogramLayerPrivate::set_bins()
{
    double bin_min = m_bin_info.bin_min;
    double bin_max = m_bin_info.bin_max;
    double num_bins = m_bin_info.num_bins;

    int N = num_bins;

    m_bin_lefts = QVector<double>(N);
    m_bin_rights = QVector<double>(N);
    m_bin_counts = QVector<int>(N);
    m_bin_densities = QVector<double>(N);

    m_second_bin_counts = QVector<int>(N);
    m_second_bin_densities = QVector<double>(N);

    if (N <= 2)
        return;
    double spacing = (transform1(bin_max) - transform1(bin_min)) / N;
    for (int i = 0; i < N; i++) {
        m_bin_lefts[i] = transform2(transform1(bin_min) + i * spacing);
        m_bin_rights[i] = transform2(transform1(bin_min) + (i + 1) * spacing);
    }

    m_update_required = true;
    emit q->repaintNeeded();
}

void HistogramLayer::setFillColor(const QColor& col)
{
    d->m_fill_color = col;
    emit repaintNeeded();
}

void HistogramLayer::setLineColor(const QColor& col)
{
    d->m_line_color = col;
    emit repaintNeeded();
}

void HistogramLayer::setTitle(const QString& title)
{
    d->m_title = title;
    emit repaintNeeded();
}

void HistogramLayer::setCaption(const QString& caption)
{
    d->m_caption = caption;
    emit repaintNeeded();
}

void HistogramLayer::setColors(const QMap<QString, QColor>& colors)
{
    d->m_colors = colors;
    emit repaintNeeded();
}

void HistogramLayer::setTimeScaleMode(HistogramLayer::TimeScaleMode mode)
{
    if (d->m_time_scale_mode == mode)
        return;
    d->m_time_scale_mode = mode;
    d->m_update_required = true;
    d->set_bins();
}

void HistogramLayer::setTimeConstant(double timepoints)
{
    if (d->m_time_constant == timepoints)
        return;
    d->m_time_constant = timepoints;
    d->m_update_required = true;
    d->set_bins();
}

MVRange HistogramLayer::xRange() const
{
    return d->m_xrange;
}

void HistogramLayer::setXRange(MVRange range)
{
    if (range == d->m_xrange)
        return;
    d->m_xrange = range;
    emit repaintNeeded();
}

#include "mlcommon.h"
void HistogramLayer::autoCenterXRange()
{
    double mean_value = MLCompute::mean(d->m_data);
    MVRange xrange = this->xRange();
    double center1 = (xrange.min + xrange.max) / 2;
    xrange = xrange + (mean_value - center1);
    this->setXRange(xrange);
}

void HistogramLayer::setDrawVerticalAxisAtZero(bool val)
{
    if (d->m_draw_vertical_axis_at_zero == val)
        return;
    d->m_draw_vertical_axis_at_zero = val;
    emit repaintNeeded();
}

void HistogramLayer::setVerticalLines(const QList<double>& vals)
{
    d->m_vertical_lines = vals;
    emit repaintNeeded();
}

void HistogramLayer::setTickMarks(const QList<double>& vals)
{
    d->m_tick_marks = vals;
    emit repaintNeeded();
}

void HistogramLayer::setCurrent(bool val)
{
    if (d->m_current != val) {
        d->m_current = val;
        emit repaintNeeded();
    }
}
void HistogramLayer::setSelected(bool val)
{
    if (d->m_selected != val) {
        d->m_selected = val;
        emit repaintNeeded();
    }
}

void HistogramLayer::paint(QPainter* painter)
{
    d->do_paint(*painter, this->windowSize().width(), this->windowSize().height());
}

QRectF make_rect(QPointF p1, QPointF p2)
{
    double x = qMin(p1.x(), p2.x());
    double y = qMin(p1.y(), p2.y());
    double w = qAbs(p2.x() - p1.x());
    double h = qAbs(p2.y() - p1.y());
    return QRectF(x, y, w, h);
}

QColor lighten(QColor col, int dr, int dg, int db)
{
    int r = col.red() + dr;
    if (r > 255)
        r = 255;
    if (r < 0)
        r = 0;
    int g = col.green() + dg;
    if (g > 255)
        g = 255;
    if (g < 0)
        g = 0;
    int b = col.blue() + db;
    if (b > 255)
        b = 255;
    if (b < 0)
        b = 0;
    return QColor(r, g, b);
}

void HistogramLayer::mousePressEvent(QMouseEvent* evt)
{
    Q_UNUSED(evt);
    //if (evt->button() == Qt::LeftButton)
    //    emit clicked(evt->modifiers());
    //else if (evt->button() == Qt::RightButton)
    //    emit rightClicked(evt->modifiers());
}

void HistogramLayer::mouseReleaseEvent(QMouseEvent* evt)
{
    Q_UNUSED(evt);
}

void HistogramLayer::mouseMoveEvent(QMouseEvent* evt)
{
    QPointF pt1 = evt->pos();
    int bin_index = d->get_bin_index_at(pt1);
    if (d->m_hovered_bin_index != bin_index) {
        d->m_hovered_bin_index = bin_index;
        emit repaintNeeded();
    }
}

void HistogramLayerPrivate::update_bin_counts()
{
    int num_bins = m_bin_lefts.count();
    for (int i = 0; i < num_bins; i++) {
        m_bin_counts[i] = 0;
        m_second_bin_counts[i] = 0;
        m_bin_densities[i] = 0;
        m_second_bin_densities[i] = 0;
    }
    for (int pass = 1; pass <= 2; pass++) {
        QVector<double> list;
        if (pass == 1) {
            list = m_data;
        }
        else {
            list = m_second_data;
        }
        qSort(list);
        if (num_bins < 2)
            return;
        int jj = 0;
        for (int i = 0; i < list.count(); i++) {
            double val = list[i];
            while ((jj + 1 < num_bins) && (m_bin_rights[jj] < val)) {
                jj++;
            }
            if ((val >= m_bin_lefts[jj]) && (val <= m_bin_rights[jj])) {
                if (pass == 1) {
                    m_bin_counts[jj]++;
                }
                else {
                    m_second_bin_counts[jj]++;
                }
            }
        }
    }
    for (int i = 0; i < num_bins; i++) {
        double len = m_bin_rights[i] - m_bin_lefts[i];
        if (!len)
            len = 1;
        m_bin_densities[i] = m_bin_counts[i] / len;
        m_second_bin_densities[i] = m_second_bin_counts[i] / len;
    }
    m_max_bin_density = qMax(MLCompute::max(m_bin_densities), MLCompute::max(m_second_bin_densities));
}

QPointF HistogramLayerPrivate::coord2pix(QPointF pt, int W, int H)
{
    if (!W)
        W = q->windowSize().width();
    if (!H)
        H = q->windowSize().height();

    int num_bins = m_bin_lefts.count();

    if (num_bins < 2) {
        return QPointF(0, 0);
    }
    if ((!W) || (!H))
        return QPointF(0, 0);
    if (W <= m_margin_left + m_margin_right + 5)
        return QPointF(0, 0);
    if (H <= m_margin_top + m_margin_bottom + 5)
        return QPointF(0, 0);

    double xmin = m_bin_lefts[0];
    double xmax = m_bin_rights[num_bins - 1];
    double ymin = 0;
    double ymax = m_max_bin_density;

    if (m_xrange.min != m_xrange.max) {
        xmin = m_xrange.min;
        xmax = m_xrange.max;
    }

    double xfrac = 0.5;
    if (xmax > xmin)
        xfrac = (transform1(pt.x()) - transform1(xmin)) / (transform1(xmax) - transform1(xmin));
    double yfrac = 0.5;
    if (ymax > ymin)
        yfrac = (pt.y() - ymin) / (ymax - ymin);

    double x0 = m_margin_left + xfrac * (W - m_margin_left - m_margin_right);
    double y0 = H - (m_margin_bottom + yfrac * (H - m_margin_top - m_margin_bottom));

    return QPointF(x0, y0);
}

QPointF HistogramLayerPrivate::pix2coord(QPointF pt, int W, int H)
{

    if (!W)
        W = q->windowSize().width();
    if (!H)
        H = q->windowSize().height();

    int num_bins = m_bin_lefts.count();

    if (num_bins < 2) {
        return QPointF(0, 0);
    }
    if ((!W) || (!H))
        return QPointF(0, 0);
    if (W <= m_margin_left + m_margin_right + 5)
        return QPointF(0, 0);
    if (H <= m_margin_top + m_margin_bottom + 5)
        return QPointF(0, 0);

    double xmin = m_bin_lefts[0];
    double xmax = m_bin_rights[num_bins - 1];
    double ymin = 0;
    double ymax = m_max_bin_density;

    if (m_xrange.min != m_xrange.max) {
        xmin = m_xrange.min;
        xmax = m_xrange.max;
    }

    double xfrac = (pt.x() - m_margin_left) / (W - m_margin_left - m_margin_right);
    double yfrac = 1 - (pt.y() - m_margin_top) / (H - m_margin_top - m_margin_bottom);

    double x0 = transform2(transform1(xmin) + xfrac * (transform1(xmax) - transform1(xmin)));
    double y0 = ymin + yfrac * (ymax - ymin);

    return QPointF(x0, y0);
}

int HistogramLayerPrivate::get_bin_index_at(QPointF pt_pix)
{
    int num_bins = m_bin_lefts.count();
    if (num_bins < 2) {
        return -1;
    }
    QPointF pt = pix2coord(pt_pix);
    for (int i = 0; i < num_bins; i++) {
        if ((pt.x() >= m_bin_lefts[i]) && (pt.x() <= m_bin_rights[i])) {
            //if ((0<=pt.y())&&(pt.y()<=m_bin_counts[i])) {
            return i;
            //}
        }
    }
    return -1;
}

QColor modify_color_for_second_histogram(QColor col)
{
    QColor ret = col;
    ret.setGreen(qMin(255, ret.green() + 30)); //more green
    ret = lighten(ret, -20, -20, -20); //darker
    return ret;
}

void HistogramLayerPrivate::do_paint(QPainter& painter, int W, int H)
{
    //d->m_colors["view_background"]=QColor(245,245,245);
    //d->m_colors["view_background_highlighted"]=QColor(250,220,200);
    //d->m_colors["view_background_hovered"]=QColor(240,245,240);

    //	QColor hover_color=QColor(150,150,150,80);
    //	QColor current_color=QColor(150,200,200,80);
    //	QColor hover_current_color=QColor(170,200,200,80);

    QRect R(0, 0, W, H);

    if (!q->exportMode()) {
        if (m_current) {
            painter.fillRect(R, m_colors["view_background_highlighted"]);
        }
        else if (m_selected) {
            painter.fillRect(R, m_colors["view_background_selected"]);
        }
        else if (m_hovered) {
            painter.fillRect(R, m_colors["view_background_hovered"]);
        }
        else {
            painter.fillRect(R, m_colors["view_background"]);
        }

        if (m_selected) {
            painter.setPen(QPen(m_colors["view_frame_selected"], 2));
        }
        else {
            painter.setPen(QPen(m_colors["view_frame"], 1));
        }
        painter.drawRect(R);
    }

    if (m_update_required) {
        update_bin_counts();
        m_update_required = false;
    }

    int num_bins = m_bin_lefts.count();
    if (num_bins < 2)
        return;

    //the order of drawing is important here, since things are set above

    if (m_draw_vertical_axis_at_zero) {
        QPointF pt0 = coord2pix(QPointF(0, 0));
        QPointF pt1 = coord2pix(QPointF(0, m_max_bin_density));
        QPen pen = painter.pen();
        pen.setColor(Qt::black);
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(3);
        painter.setPen(pen);
        painter.drawLine(pt0, pt1);
    }

    foreach (double val, m_tick_marks) {
        QPointF pt0 = coord2pix(QPointF(val, 0));
        QPointF pt1 = pt0;
        pt1.setY(pt1.y() + 5);
        QPen pen = painter.pen();
        pen.setColor(Qt::black);
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawLine(pt0, pt1);
    }

    foreach (double val, m_vertical_lines) {
        QPointF pt0 = coord2pix(QPointF(val, 0));
        QPointF pt1 = coord2pix(QPointF(val, m_max_bin_density));
        QPen pen = painter.pen();
        pen.setColor(Qt::darkGray);
        pen.setStyle(Qt::DashLine);
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawLine(pt0, pt1);
    }

    {
        //horizontal axis
        QPointF pt0 = coord2pix(QPointF(0, 0));
        QPointF pt1 = QPointF(0, pt0.y());
        QPointF pt2 = QPointF(q->windowSize().width(), pt0.y());
        QPen pen = painter.pen();
        pen.setColor(Qt::black);
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawLine(pt1, pt2);
    }

    for (int pass = 1; pass <= 2; pass++) {
        QVector<double> bin_densities;
        if (pass == 1)
            bin_densities = m_bin_densities;
        else
            bin_densities = m_second_bin_densities;
        QColor col = m_fill_color;
        QColor line_color = m_line_color;
        if (pass == 2) {
            col = modify_color_for_second_histogram(col);
            line_color = modify_color_for_second_histogram(line_color);
        }
        for (int i = 0; i < num_bins; i++) {
            QPointF pt1 = coord2pix(QPointF(m_bin_lefts[i], 0), W, H);
            QPointF pt2 = coord2pix(QPointF(m_bin_rights[i], bin_densities[i]), W, H);
            QRectF R = make_rect(pt1, pt2);
            if (i == m_hovered_bin_index)
                painter.fillRect(R, lighten(col, 25, 25, 25));
            else
                painter.fillRect(R, col);
            painter.setPen(line_color);
            painter.drawRect(R);
        }
    }

    if (!m_title.isEmpty()) {
        //int text_height = 14;
        QRect R(m_margin_left, 5, W - m_margin_left - m_margin_right, q->font().pixelSize());

        //QFont font = painter.font();
        //font.setFamily("Arial");
        //font.setPixelSize(text_height);
        //painter.setFont(font);
        QFont fnt = q->font();
        //fnt.setPixelSize(text_height);
        painter.setFont(fnt);

        painter.setPen(Qt::darkGray);
        painter.drawText(R, m_title, Qt::AlignLeft | Qt::AlignTop);
    }
    if (!m_caption.isEmpty()) {
        //int text_height = 14;
        QRect R(0, H - m_margin_bottom, W, m_margin_bottom);
        QFont font = q->font();
        font.setFamily("Arial");
        //font.setPixelSize(text_height);
        painter.setFont(font);
        painter.setPen(Qt::darkGray);
        painter.drawText(R, m_caption, Qt::AlignCenter | Qt::AlignVCenter);
    }
}

double HistogramLayerPrivate::transform1(double t)
{
    if (m_time_scale_mode == HistogramLayer::Uniform)
        return t;
    if (t < 0)
        return -transform1(-t); //now we can assume non-negative
    if (t == 0)
        return 0; //now we can assume positive

    switch (m_time_scale_mode) {
    case HistogramLayer::Uniform:
        return t;
        break;
    case HistogramLayer::Log:
        if (t < m_time_constant)
            return t;
        else
            return (1 + log(t / m_time_constant)) * m_time_constant;
        break;
    case HistogramLayer::Cubic:
        double val = t / m_time_constant;
        return exp(log(val) / 3);
    }
    return t;
}

double HistogramLayerPrivate::transform2(double x)
{
    if (m_time_scale_mode == HistogramLayer::Uniform)
        return x;
    if (x < 0)
        return -transform2(-x); //now we can assume non-negative
    if (x == 0)
        return 0; //now we can assume positive

    switch (m_time_scale_mode) {
    case HistogramLayer::Uniform:
        return x;
        break;
    case HistogramLayer::Log:
        if (x < m_time_constant)
            return x;
        else
            return m_time_constant * (exp(x / m_time_constant - 1));
        break;
    case HistogramLayer::Cubic:
        return x * x * x * m_time_constant;
    }
    return x;
}
