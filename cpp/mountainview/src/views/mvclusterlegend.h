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

#ifndef MVCLUSTERLEGEND_H
#define MVCLUSTERLEGEND_H

#include "paintlayer.h"

class MVClusterLegendPrivate;
class MVClusterLegend : public PaintLayer {
    Q_OBJECT
public:
    friend class MVClusterLegend2Private;
    MVClusterLegend();
    virtual ~MVClusterLegend();

    virtual void paint(QPainter* painter) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent* evt) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent* evt) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent* evt) Q_DECL_OVERRIDE;

    void setClusterColors(const QList<QColor>& colors);
    void setClusterNumbers(const QList<int>& numbers);

    int hoveredClusterNumber() const;
    QSet<int> activeClusterNumbers() const;
    void setActiveClusterNumbers(const QSet<int>& active_numbers);
signals:
    void hoveredClusterNumberChanged();
    void activeClusterNumbersChanged();

private:
    MVClusterLegendPrivate* d;
};

#endif // MVCLUSTERLEGEND_H
