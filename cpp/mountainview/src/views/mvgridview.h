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
#ifndef MVGRIDVIEW_H
#define MVGRIDVIEW_H

#include "renderablewidget.h"

#include <mvabstractview.h>
#include <QResizeEvent>

class MVGridViewPrivate;
class MVGridView : public MVAbstractView {
    Q_OBJECT
public:
    friend class MVGridViewPrivate;
    MVGridView(MVAbstractContext* context);
    virtual ~MVGridView();

    void setPreferredViewWidth(int width);
    void updateLayout();
    void setHorizontalScaleWidget(QWidget* W);
    void setForceSquareMatrix(bool val);
    void setPreferredHistogramWidth(int width); //use 0 for zoomed all the way out
    void updateViews();
    int currentViewIndex() const;

    QImage renderImage(int W = 0, int H = 0);

signals:
    void signalViewClicked(int index, Qt::KeyboardModifiers modifiers);

protected:
    void resizeEvent(QResizeEvent* evt);

    void clearViews();
    void addView(RenderableWidget* W);
    int viewCount() const;
    RenderableWidget* view(int j) const;

private slots:
    void slot_zoom_out(double factor = 1.2);
    void slot_zoom_in(double factor = 1.2);
    void slot_signal_view_clicked(int, Qt::KeyboardModifiers);
    void slot_grid_properties();
    void slot_export_image();

private:
    MVGridViewPrivate* d;
};

#endif // MVGRIDVIEW_H
