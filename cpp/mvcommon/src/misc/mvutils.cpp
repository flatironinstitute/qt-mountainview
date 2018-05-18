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
#include "mvutils.h"

#include <QCoreApplication>
//#include "get_pca_features.h"
#include <math.h>

#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QString>
#include <QImageWriter>
#include <QMessageBox>
#include <QJsonArray>
#include "imagesavedialog.h"
#include "taskprogress.h"
#include "mlcommon.h"

QColor get_heat_map_color(double val)
{
    double r = 0, g = 0, b = 0;
    if (val < 0.2) {
        double tmp = (val - 0) / 0.2;
        r = 200 * (1 - tmp) + 150 * tmp;
        b = 200 * (1 - tmp) + 255 * tmp;
        g = 0 * (1 - tmp) + 0 * tmp;
    }
    else if (val < 0.4) {
        double tmp = (val - 0.2) / 0.2;
        r = 150 * (1 - tmp) + 0 * tmp;
        b = 255 * (1 - tmp) + 255 * tmp;
        g = 0 * (1 - tmp) + 100 * tmp;
    }
    else if (val < 0.6) {
        double tmp = (val - 0.4) / 0.2;
        r = 0 * (1 - tmp) + 255 * tmp;
        b = 255 * (1 - tmp) + 0 * tmp;
        g = 100 * (1 - tmp) + 20 * tmp;
    }
    else if (val < 0.8) {
        double tmp = (val - 0.6) / 0.2;
        r = 255 * (1 - tmp) + 255 * tmp;
        b = 0 * (1 - tmp) + 0 * tmp;
        g = 20 * (1 - tmp) + 255 * tmp;
    }
    else if (val <= 1.0) {
        double tmp = (val - 0.8) / 0.2;
        r = 255 * (1 - tmp) + 255 * tmp;
        b = 0 * (1 - tmp) + 255 * tmp;
        g = 255 * (1 - tmp) + 255 * tmp;
    }

    return QColor((int)r, (int)g, (int)b);
}

QList<Epoch> read_epochs(const QString& path)
{
    QList<Epoch> ret;
    QString txt = TextFile::read(path);
    QStringList lines = txt.split("\n");
    foreach (QString line, lines) {
        QList<QString> vals = line.split(QRegExp("\\s+"));
        if (vals.value(0) == "EPOCH") {
            if (vals.count() == 4) {
                Epoch E;
                E.name = vals.value(1);
                E.t_begin = vals.value(2).toDouble() - 1;
                E.t_end = vals.value(3).toDouble() - 1;
                ret << E;
            }
            else {
                qWarning() << "Problem parsing epochs file:" << path;
            }
        }
    }
    return ret;
}

void user_save_image(const QImage& img)
{
    ImageSaveDialog::presentImage(img);
}

double round_down_to_2_significant_figures(double val)
{
    if (!val)
        return 0;
    if (val < 0)
        return -round_down_to_2_significant_figures(-val);
    double factor = 1;
    while (val * factor >= 100) {
        factor /= 10;
    }
    while (val * factor < 10) {
        factor *= 10;
    }
    return ((int)(val * factor)) * 1.0 / factor;
}

void draw_axis(QPainter* painter, draw_axis_opts opts)
{
    QPen pen = painter->pen();
    pen.setColor(opts.color);
    pen.setWidth(opts.line_width);
    painter->setPen(pen);
    QFont font = painter->font();
    font.setPixelSize(opts.font_size_pix);
    painter->setFont(font);
    if (opts.orientation == Qt::Vertical) {
        //ensure that pt2.y>=pt1.y
        if (opts.pt2.y() < opts.pt1.y()) {
            QPointF tmp = opts.pt2;
            opts.pt2 = opts.pt1;
            opts.pt1 = tmp;
        }
    }

    double range = opts.maxval - opts.minval;

    if (opts.draw_range) {
        //adjust so that range has 2 significant figures
        double range2 = round_down_to_2_significant_figures(range);
        double frac = range2 / range; //this is the fraction by which we are reducing the range
        QPointF pt_avg = (opts.pt1 + opts.pt2) / 2;
        opts.pt1 = pt_avg + frac * (opts.pt1 - pt_avg);
        opts.pt2 = pt_avg + frac * (opts.pt2 - pt_avg);
        double avg_val = (opts.minval + opts.maxval) / 2;
        opts.minval = avg_val + frac * (opts.minval - avg_val);
        opts.maxval = avg_val + frac * (opts.maxval - avg_val);
        range = opts.maxval - opts.minval; //recompute the range
    }

    painter->drawLine(opts.pt1, opts.pt2);

    if (opts.maxval <= opts.minval)
        return;
    QVector<double> possible_tick_intervals;
    for (double x = 0.00001; x <= 10000; x *= 10) {
        possible_tick_intervals << x << x * 2 << x * 5;
    }

    int best_count = 0;
    double best_interval = 0;
    for (int i = 0; i < possible_tick_intervals.count(); i++) {
        int count = (int)range / possible_tick_intervals[i];
        if (count >= 4) {
            if ((best_interval == 0) || (count < best_count)) {
                best_count = count;
                best_interval = possible_tick_intervals[i];
            }
        }
    }
    double tick_length = opts.tick_length;
    if (best_interval) {
        int ind1 = (int)(opts.minval / best_interval) - 1;
        int ind2 = (int)(opts.maxval / best_interval) + 1;
        for (int ind = ind1; ind <= ind2; ind++) {
            if ((opts.minval <= ind * best_interval) && (ind * best_interval <= opts.maxval)) {
                double pct = (ind * best_interval - opts.minval) / (opts.maxval - opts.minval);
                if (opts.orientation == Qt::Vertical)
                    pct = 1 - pct;
                QPointF ptA = opts.pt1 + pct * (opts.pt2 - opts.pt1);
                QPointF ptB;
                QRectF text_rect;
                int align;
                if (opts.orientation == Qt::Horizontal) {
                    ptB = ptA + QPointF(0, tick_length);
                    text_rect = QRectF(ptB + QPointF(20, 0), QSize(40, 50 - 3));
                    align = Qt::AlignTop | Qt::AlignCenter;
                }
                else { //vertical
                    ptB = ptA + QPointF(-tick_length, 0);
                    text_rect = QRectF(ptB + QPointF(-50, -20), QSize(50 - 3, 40));
                    align = Qt::AlignRight | Qt::AlignVCenter;
                }
                painter->drawLine(ptA, ptB);
                QString text = QString("%1").arg(ind * best_interval);
                if (opts.draw_tick_labels) {
                    painter->drawText(text_rect, align, text);
                }
            }
        }
    }
    if (opts.draw_range) {
        if (opts.orientation == Qt::Horizontal) {
            /// TODO (LOW) handle horizontal case of drawing axis range
            //draw little segments pointing up
            painter->drawLine(opts.pt1.x(), opts.pt1.y(), opts.pt1.x(), opts.pt1.y() - 4);
            painter->drawLine(opts.pt2.x(), opts.pt2.y(), opts.pt2.x(), opts.pt2.y() - 4);
        }
        else {
            painter->save();
            painter->rotate(-90);
            QRectF rect(opts.pt1.x() - 50, opts.pt1.y() - 50, 50 - 3, opts.pt2.y() - opts.pt1.y() + 100);
            QTransform transform;
            transform.rotate(90);
            rect = transform.mapRect(rect);
            QString text = QString("%1").arg(range);
            int align = Qt::AlignBottom | Qt::AlignCenter;
            painter->drawText(rect, align, text);
            painter->restore();
            //draw little segments pointing right
            painter->drawLine(opts.pt1.x(), opts.pt1.y(), opts.pt1.x() + 4, opts.pt1.y());
            painter->drawLine(opts.pt2.x(), opts.pt2.y(), opts.pt2.x() + 4, opts.pt2.y());
        }
    }
}

QJsonArray stringset2jsonarray(QSet<QString> set)
{
    QJsonArray ret;
    QStringList list = set.toList();
    qSort(list);
    foreach (QString str, list) {
        ret.append(str);
    }
    return ret;
}

QSet<QString> jsonarray2stringset(QJsonArray X)
{
    QSet<QString> ret;
    for (int i = 0; i < X.count(); i++) {
        ret.insert(X[i].toString());
    }
    return ret;
}

QStringList jsonarray2stringlist(QJsonArray X)
{
    QStringList list = jsonarray2stringset(X).toList();
    qSort(list);
    return list;
}
