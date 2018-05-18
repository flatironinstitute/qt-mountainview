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

#ifndef MVUTILS_H
#define MVUTILS_H

#include <QList>
#include <QColor>
#include <math.h>
#include <QPainter>
#include <QJsonArray>

///A firing event defined by a time and a label
struct MVEvent {
    MVEvent(double time0 = -1, int label0 = -1)
    {
        time = time0;
        label = label0;
    }
    bool operator==(const MVEvent& other) const
    {
        return ((time == other.time) && (label == other.label));
    }

    double time;
    int label;
};

///An epoch or time interval within a timeseries. t_begin and t_end are in timepoint units
struct Epoch {
    QString name;
    double t_begin;
    double t_end;
};
///Read a set of epochs from a text file (special format)
QList<Epoch> read_epochs(const QString& path);

QColor get_heat_map_color(double val);

void user_save_image(const QImage& img);

struct draw_axis_opts {
    draw_axis_opts()
    {
        orientation = Qt::Vertical;
        minval = 0;
        maxval = 1;
        tick_length = 5;
        draw_tick_labels = true;
        draw_range = false;
        color = Qt::black;
        line_width = 1;
        font_size_pix = 11;
    }

    QPointF pt1, pt2;
    Qt::Orientation orientation;
    double minval, maxval;
    double tick_length;
    bool draw_tick_labels;
    bool draw_range;
    QColor color;
    int line_width;
    int font_size_pix;
};

void draw_axis(QPainter* painter, draw_axis_opts opts);

QJsonArray stringset2jsonarray(QSet<QString> set);
QSet<QString> jsonarray2stringset(QJsonArray X);
QStringList jsonarray2stringlist(QJsonArray X);

#endif // MVUTILS_H
