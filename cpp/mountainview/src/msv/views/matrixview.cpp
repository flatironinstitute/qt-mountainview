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
#include "matrixview.h"

#include <QMouseEvent>
#include <QPainter>
#include <mvmisc.h>
#include "mlcommon.h"

class MatrixViewPrivate {
public:
    MatrixView* q;
    Mda m_matrix;
    MVRange m_value_range = MVRange(0, 1);
    MatrixView::Mode m_mode;
    QVector<int> m_perm_rows;
    QVector<int> m_perm_cols;
    QStringList m_row_labels;
    QStringList m_col_labels;
    bool m_draw_divider_for_final_row = true;
    bool m_draw_divider_for_final_column = true;
    QPoint m_hovered_element = QPoint(-1, -1);
    QPoint m_current_element = QPoint(-1, -1);
    QString m_title;
    QString m_row_axis_label;
    QString m_column_axis_label;
    int m_max_row_map = 0;
    int m_max_col_map = 0;

    //left,right,top,bottom
    double m_margins[4] = { 50, 0, 40, 50 };

    QRectF get_entry_rect(int m, int n);
    QPointF coord2pix(double m, double n);
    QPointF pix2coord(QPointF pix);
    QColor get_color(double val);
    void draw_string_in_rect(QPainter& painter, QRectF r, QString txt, QColor col, int fontsize = 0);
    QColor complementary_color(QColor col);
    int row_map(int m);
    int col_map(int n);
    int row_map_inv(int m);
    int col_map_inv(int n);
    void set_hovered_element(QPoint a);
    int max_row_map();
    int max_col_map();
};

MatrixView::MatrixView()
{
    d = new MatrixViewPrivate;
    d->q = this;

    this->setMouseTracking(true);
}

MatrixView::~MatrixView()
{
    delete d;
}

void MatrixView::setMode(MatrixView::Mode mode)
{
    d->m_mode = mode;
}

void MatrixView::setMatrix(const Mda& A)
{
    d->m_matrix = A;
    update();
}

void MatrixView::setValueRange(double minval, double maxval)
{
    d->m_value_range = MVRange(minval, maxval);
    update();
}

void MatrixView::setIndexPermutations(const QVector<int>& perm_rows, const QVector<int>& perm_cols)
{
    d->m_perm_rows = perm_rows;
    d->m_perm_cols = perm_cols;

    d->m_max_row_map = 0;
    for (int i = 0; i < perm_rows.count(); i++) {
        d->m_max_row_map = qMax(d->m_max_row_map, perm_rows[i]);
    }

    d->m_max_col_map = 0;
    for (int i = 0; i < perm_cols.count(); i++) {
        d->m_max_col_map = qMax(d->m_max_col_map, perm_cols[i]);
    }

    update();
}

void MatrixView::setLabels(const QStringList& row_labels, const QStringList& col_labels)
{
    d->m_row_labels = row_labels;
    d->m_col_labels = col_labels;
    update();
}

void MatrixView::setTitle(QString title)
{
    d->m_title = title;
    update();
}

void MatrixView::setRowAxisLabel(QString label)
{
    d->m_row_axis_label = label;
    update();
}

void MatrixView::setColumnAxisLabel(QString label)
{
    d->m_column_axis_label = label;
    update();
}

void MatrixView::setDrawDividerForFinalRow(bool val)
{
    d->m_draw_divider_for_final_row = val;
    update();
}

void MatrixView::setDrawDividerForFinalColumn(bool val)
{
    d->m_draw_divider_for_final_column = val;
    update();
}

void MatrixView::setCurrentElement(QPoint pt)
{
    if ((pt.x() < 0) || (pt.y() < 0)) {
        pt = QPoint(-1, -1);
    }
    if (pt == d->m_current_element)
        return;
    d->m_current_element = pt;
    emit currentElementChanged();
    update();
}

QPoint MatrixView::currentElement() const
{
    return d->m_current_element;
}

void MatrixView::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)

    double left = d->m_margins[0];
    double right = d->m_margins[1];
    double top = d->m_margins[2];
    double bottom = d->m_margins[3];

    //The matrix
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int M = d->m_matrix.N1();
    int N = d->m_matrix.N2();
    for (int n = 0; n < N; n++) {
        for (int m = 0; m < M; m++) {
            QRectF r = d->get_entry_rect(m, n);
            double val = d->m_matrix.value(m, n);
            QColor col = d->get_color(val);
            painter.fillRect(r, col);
            QString str;
            if (d->m_mode == PercentMode) {
                str = QString("%1%").arg((int)(val * 100));
                if (val == 0)
                    str = "";
            }
            else if (d->m_mode == CountsMode) {
                str = QString("%1").arg(val);
                if (val == 0)
                    str = "";
            }
            d->draw_string_in_rect(painter, r, str, d->complementary_color(col));
        }
    }

    //Dividers for final row/column
    if (d->m_draw_divider_for_final_row) {
        QPointF pt1 = d->coord2pix(d->max_row_map(), 0);
        QPointF pt2 = d->coord2pix(d->max_row_map(), d->max_col_map());
        QPen pen(QBrush(Qt::darkGray), 1);
        painter.setPen(pen);
        painter.drawLine(pt1, pt2);
    }
    if (d->m_draw_divider_for_final_column) {
        QPointF pt1 = d->coord2pix(0, d->max_col_map());
        QPointF pt2 = d->coord2pix(d->max_row_map(), d->max_col_map());
        QPen pen(QBrush(Qt::darkGray), 1);
        painter.setPen(pen);
        painter.drawLine(pt1, pt2);
    }

    //The row labels
    for (int m = 0; m < M; m++) {
        int label_wid = 20;
        int m2 = d->row_map(m);
        if (m2 >= 0) {
            QPointF pt1 = d->coord2pix(m2, 0);
            QPointF pt2 = d->coord2pix(m2 + 1, 0);
            QRectF r = QRectF(pt1.x() - label_wid, pt1.y(), label_wid, pt2.y() - pt1.y());
            int fontsize = qMin(16.0, pt2.y() - pt1.y());
            d->draw_string_in_rect(painter, r, d->m_row_labels.value(m), Qt::black, fontsize);
        }
    }
    //The column labels
    for (int n = 0; n < N; n++) {
        int label_height = 20;
        int n2 = d->col_map(n);
        if (n2 >= 0) {
            QPointF pt1 = d->coord2pix(0, n2);
            QPointF pt2 = d->coord2pix(0, n2 + 1);
            QRectF r = QRectF(pt1.x(), height() - d->m_margins[3], pt2.x() - pt1.x(), label_height);
            int fontsize = qMin(16.0, (pt2.x() - pt1.x()) / 2);
            d->draw_string_in_rect(painter, r, d->m_col_labels.value(n), Qt::black, fontsize);
        }
    }

    //hovered row/column
    QColor col(255, 255, 220, 60);
    if (d->m_hovered_element.x() >= 0) {
        int m2 = d->row_map(d->m_hovered_element.x());
        if (m2 >= 0) {
            QPointF pt1 = d->coord2pix(m2, 0);
            QPointF pt2 = d->coord2pix(m2 + 1, N);
            QRectF r = QRectF(pt1 - QPointF(left, 0), pt2);
            painter.fillRect(r, col);
        }
    }
    if (d->m_hovered_element.y() >= 0) {
        int n2 = d->col_map(d->m_hovered_element.y());
        int m2 = d->row_map(d->m_hovered_element.x());
        if ((m2 >= 0) && (n2 >= 0)) {
            QPointF pt1 = d->coord2pix(0, n2);
            QPointF pt2 = d->coord2pix(m2, n2 + 1);
            QPointF pt3 = d->coord2pix(m2 + 1, n2);
            QPointF pt4 = d->coord2pix(M, n2 + 1);
            QRectF r1 = QRectF(pt1, pt2);
            QRectF r2 = QRectF(pt3, pt4 + QPointF(0, bottom));

            painter.fillRect(r1, col);
            painter.fillRect(r2, col);
        }
    }

    //current element
    if ((d->m_current_element.x() >= 0) && (d->m_current_element.y() >= 0)) {
        int m = d->m_current_element.x();
        int n = d->m_current_element.y();
        QRectF r = d->get_entry_rect(m, n);
        QPen pen(QBrush(QColor(255, 200, 180)), 3);
        painter.setPen(pen);
        painter.drawRect(r);
    }

    //title
    {
        QRectF r(left, 0, this->width() - left - right, top);
        d->draw_string_in_rect(painter, r, d->m_title, Qt::black, 16);
    }
}

void MatrixView::mouseMoveEvent(QMouseEvent* evt)
{
    int M = d->m_matrix.N1();
    int N = d->m_matrix.N2();

    QPointF pos = evt->pos();
    QPointF coord = d->pix2coord(pos);
    int m = (int)coord.x();
    int n = (int)coord.y();
    if ((0 <= m) && (m < M) && (0 <= n) && (n < N)) {
        int m2 = d->row_map_inv(m);
        int n2 = d->col_map_inv(n);
        if ((m2 >= 0) && (n2 >= 0)) {
            d->set_hovered_element(QPoint(m2, n2));
        }
    }
    else {
        d->set_hovered_element(QPoint(-1, -1));
    }
}

void MatrixView::leaveEvent(QEvent*)
{
    d->set_hovered_element(QPoint(-1, -1));
    update();
}

void MatrixView::mousePressEvent(QMouseEvent* evt)
{
    QPointF pos = evt->pos();
    QPointF coord = d->pix2coord(pos);
    int m2 = (int)coord.x();
    int n2 = (int)coord.y();
    if ((0 <= m2) && (m2 <= d->max_row_map()) && (0 <= n2) && (n2 <= d->max_col_map())) {
        int m = d->row_map_inv(m2);
        int n = d->col_map_inv(n2);
        if ((m >= 0) && (n >= 0)) {
            this->setCurrentElement(QPoint(m, n));
        }
    }
    else {
        this->setCurrentElement(QPoint(-1, -1));
    }
}

QRectF MatrixViewPrivate::get_entry_rect(int m, int n)
{
    int m2 = row_map(m);
    int n2 = col_map(n);
    if ((m2 >= 0) && (n2 >= 0)) {
        QPointF pt1 = coord2pix(m2, n2);
        QPointF pt2 = coord2pix(m2 + 1, n2 + 1) + QPointF(1, 1);
        return QRectF(pt1, pt2);
    }
    else
        return QRectF();
}

QPointF MatrixViewPrivate::coord2pix(double m, double n)
{
    double left = m_margins[0];
    double right = m_margins[1];
    double top = m_margins[2];
    double bottom = m_margins[3];

    int W0 = q->width() - left - right;
    int H0 = q->height() - top - bottom;
    if (!(W0 * H0))
        return QPointF(-1, -1);
    //int M = m_matrix.N1();
    //int N = m_matrix.N2();
    int M = max_row_map() + 1;
    int N = max_col_map() + 1;
    if (!(M * N))
        return QPointF(0, 0);

    return QPointF(left + n / N * W0, top + m / M * H0);
}

QPointF MatrixViewPrivate::pix2coord(QPointF pix)
{
    double left = m_margins[0];
    double right = m_margins[1];
    double top = m_margins[2];
    double bottom = m_margins[3];

    int W0 = q->width() - left - right;
    int H0 = q->height() - top - bottom;
    if (!(W0 * H0))
        return QPointF(-1, -1);
    //int M = m_matrix.N1();
    //int N = m_matrix.N2();
    int M = max_row_map() + 1;
    int N = max_col_map() + 1;
    if (!(M * N))
        return QPointF(0, 0);

    return QPointF((pix.y() - top) * M / H0, (pix.x() - left) * N / W0);
}

QColor MatrixViewPrivate::get_color(double val)
{
    if (m_value_range.max <= m_value_range.min)
        return QColor(0, 0, 0);
    double tmp = (val - m_value_range.min) / m_value_range.range();
    tmp = qMax(0.0, qMin(1.0, tmp));
    //tmp=1-tmp;
    int tmp2 = (int)(tmp * 255);
    return QColor(tmp2 / 2, tmp2, tmp2);
}

void MatrixViewPrivate::draw_string_in_rect(QPainter& painter, QRectF r, QString txt, QColor col, int fontsize)
{
    painter.save();

    QPen pen = painter.pen();
    pen.setColor(col);
    painter.setPen(pen);

    QFont font = painter.font();
    if (fontsize) {
        font.setPixelSize(fontsize);
    }
    else {
        //determine automatically
        int pix = 16;
        font.setPixelSize(pix);
        while ((pix > 6) && (QFontMetrics(font).width(txt) >= r.width())) {
            pix--;
            font.setPixelSize(pix);
        }
    }
    painter.setFont(font);

    painter.setClipRect(r);
    painter.drawText(r, Qt::AlignCenter | Qt::AlignVCenter, txt);

    painter.restore();
}

QColor MatrixViewPrivate::complementary_color(QColor col)
{

    double r = col.red() * 1.0 / 255;
    double g = col.green() * 1.0 / 255;
    double b = col.blue() * 1.0 / 255;
    double r2, g2, b2;

    //(0,0,0) -> (0.5,0.5,0.5)
    //(0.5,0.5,0.5) -> (0.7,0.7,1)
    //(1,1,1) -> (0,0,0.4)
    if (r < 0.5) {
        r2 = 0.5 + r / 0.5 * 0.2;
    }
    else {
        r2 = 0.7 - (r - 0.5) / 0.5 * 0.7;
    }
    if (g < 0.5) {
        g2 = 0.5 + g / 0.5 * 0.2;
    }
    else {
        g2 = 0.7 - (g - 0.5) / 0.5 * 0.7;
    }
    if (b < 0.5) {
        b2 = 0.5 + b / 0.5 * 0.5;
    }
    else {
        b2 = 1 - (b - 0.5) / 0.5 * 0.6;
    }

    return QColor((int)(r2 * 255), (int)(g2 * 255), (int)(b2 * 255));
}

int MatrixViewPrivate::row_map(int m)
{
    if (m < m_perm_rows.count())
        return m_perm_rows[m];
    else
        return m;
}

int MatrixViewPrivate::col_map(int n)
{
    if (n < m_perm_cols.count())
        return m_perm_cols[n];
    else
        return n;
}

int MatrixViewPrivate::row_map_inv(int m)
{
    if (m_perm_rows.isEmpty())
        return m;
    int m2 = m_perm_rows.indexOf(m);
    return m2;
}

int MatrixViewPrivate::col_map_inv(int n)
{
    if (m_perm_cols.isEmpty())
        return n;
    int n2 = m_perm_cols.indexOf(n);
    return n2;
}

void MatrixViewPrivate::set_hovered_element(QPoint a)
{
    if (m_hovered_element == a)
        return;
    m_hovered_element = a;
    q->update();
}

int MatrixViewPrivate::max_row_map()
{
    if (m_max_row_map == 0)
        return m_matrix.N1() - 1;
    return m_max_row_map;
}

int MatrixViewPrivate::max_col_map()
{
    if (m_max_col_map == 0)
        return m_matrix.N2() - 1;
    return m_max_col_map;
}
