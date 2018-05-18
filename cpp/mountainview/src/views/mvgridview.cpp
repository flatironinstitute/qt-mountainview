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

#include "mvgridview.h"
#include "mvgridviewpropertiesdialog.h"
#include "mvutils.h"

#include <QHBoxLayout>
#include <QScrollArea>
#include "actionfactory.h"
#include <QScrollBar>

class ViewWrap : public QWidget {
public:
    ViewWrap(MVGridView* GV, QWidget* W, int index)
    {
        m_grid_view = GV;
        m_index = index;

        QHBoxLayout* layout = new QHBoxLayout;
        layout->setMargin(0);
        layout->setSpacing(0);
        this->setLayout(layout);
        layout->addWidget(W);
    }
    void mousePressEvent(QMouseEvent* evt) Q_DECL_OVERRIDE
    {
        emit m_grid_view->signalViewClicked(m_index, evt->modifiers());
    }

private:
    MVGridView* m_grid_view;
    int m_index;
};

class MVGridViewPrivate {
public:
    MVGridView* q;
    QList<RenderableWidget*> m_views;
    QList<ViewWrap*> m_view_wraps;
    QScrollArea* m_scroll_area;
    QWidget* m_grid_widget;
    QGridLayout* m_grid_layout;
    double m_preferred_width = 0; //zero means zoomed all the way out
    bool m_force_square_matrix = false;
    double m_preferred_aspect_ratio = 1.618; //golden mean
    QWidget* m_horizontal_scale_widget = 0;
    int m_current_view_index = 0;
    MVGridViewProperties m_properties;

    void on_resize();
    void setup_grid(int num_cols);
    void get_num_rows_cols_and_height_for_preferred_width(int& num_rows, int& num_cols, int& height, double preferred_width);
};

MVGridView::MVGridView(MVAbstractContext* context)
    : MVAbstractView(context)
{
    d = new MVGridViewPrivate;
    d->q = this;

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    QScrollArea* SA = new QScrollArea;
    SA->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //SA->setWidgetResizable(true);
    layout->addWidget(SA);
    this->setLayout(layout);
    d->m_scroll_area = SA;

    QWidget* GW = new QWidget;
    QGridLayout* GL = new QGridLayout;
    GW->setLayout(GL);
    SA->setWidget(GW);
    d->m_grid_layout = GL;
    d->m_grid_widget = GW;

    GL->setHorizontalSpacing(12);
    GL->setVerticalSpacing(0);
    GL->setMargin(0);

    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomIn, this, SLOT(slot_zoom_in()));
    ActionFactory::addToToolbar(ActionFactory::ActionType::ZoomOut, this, SLOT(slot_zoom_out()));

    {
        QAction* A = new QAction(QString("Grid properties"), this);
        A->setProperty("action_type", "toolbar");
        QObject::connect(A, SIGNAL(triggered(bool)), this, SLOT(slot_grid_properties()));
        this->addAction(A);
    }
    {
        QAction* a = new QAction("Export image", this);
        a->setProperty("action_type", "toolbar");
        this->addAction(a);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(slot_export_image()));
    }

    d->m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    d->m_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QObject::connect(this, SIGNAL(signalViewClicked(int, Qt::KeyboardModifiers)), this, SLOT(slot_signal_view_clicked(int, Qt::KeyboardModifiers)));
}

MVGridView::~MVGridView()
{
    clearViews();
    delete d;
}

void MVGridView::clearViews()
{
    qDeleteAll(d->m_view_wraps);
    d->m_views.clear();
    d->m_view_wraps.clear();
}

void MVGridView::addView(RenderableWidget* W)
{
    d->m_views << W;
}

int MVGridView::viewCount() const
{
    return d->m_views.count();
}

RenderableWidget* MVGridView::view(int j) const
{
    return d->m_views[j];
}

void MVGridView::slot_zoom_in(double factor)
{
    int hscroll_value = 0, vscroll_value = 0;
    if (d->m_scroll_area->horizontalScrollBar())
        hscroll_value = d->m_scroll_area->horizontalScrollBar()->value();
    if (d->m_scroll_area->verticalScrollBar())
        vscroll_value = d->m_scroll_area->verticalScrollBar()->value();
    bool done = false;
    double preferred_width = d->m_preferred_width;
    if ((factor > 1) && (preferred_width == 0)) {
        preferred_width = 1;
    }
    int iteration_count = 0;
    while (!done) {
        int num_rows1, num_cols1, height1, num_rows2, num_cols2, height2;
        d->get_num_rows_cols_and_height_for_preferred_width(num_rows1, num_cols1, height1, preferred_width);
        preferred_width = qMin(1.0e6, qMin(preferred_width + 10, preferred_width * factor));
        d->get_num_rows_cols_and_height_for_preferred_width(num_rows2, num_cols2, height2, preferred_width);
        bool something_changed = (height1 != height2);
        if (something_changed)
            done = true;
        iteration_count++;
        if (iteration_count > 1000) {
            preferred_width = d->m_preferred_width;
            if (factor < 1)
                preferred_width = 0;
            done = true;
        }
    }
    d->m_preferred_width = preferred_width;
    d->on_resize();
    if (d->m_scroll_area->horizontalScrollBar())
        d->m_scroll_area->horizontalScrollBar()->setValue(hscroll_value);
    if (d->m_scroll_area->verticalScrollBar())
        d->m_scroll_area->verticalScrollBar()->setValue(vscroll_value);
}

void MVGridView::slot_signal_view_clicked(int index, Qt::KeyboardModifiers)
{
    d->m_current_view_index = index;
}

void MVGridView::slot_grid_properties()
{
    MVGridViewPropertiesDialog dlg;
    dlg.setProperties(d->m_properties);
    if (dlg.exec() == QDialog::Accepted) {
        d->m_properties = dlg.properties();
        this->updateLayout();
    }
}

void MVGridView::slot_export_image()
{
    QImage img = this->renderImage();
    user_save_image(img);
}

void MVGridView::slot_zoom_out(double factor)
{
    slot_zoom_in(1 / factor);
}

void MVGridView::resizeEvent(QResizeEvent* evt)
{
    MVAbstractView::resizeEvent(evt);
    d->on_resize();
}

void MVGridViewPrivate::on_resize()
{
    int W = q->width();
    int scroll_bar_width = 30; //how to determine?
    m_grid_widget->setFixedWidth(W - scroll_bar_width);

    int preferred_width = m_preferred_width;

    int num_rows = 1;
    int num_cols = 1;
    int width = 0;
    int height = 0;
    if (!m_views.isEmpty()) {
        if (m_properties.use_fixed_number_of_columns) {
            num_rows = (m_views.count() + m_properties.fixed_number_of_columns - 1) / m_properties.fixed_number_of_columns;
            num_cols = m_properties.fixed_number_of_columns;
            width = m_grid_widget->width();
            height = width / m_preferred_aspect_ratio * num_rows;
        }
        else {
            get_num_rows_cols_and_height_for_preferred_width(num_rows, num_cols, height, preferred_width);
            width = m_grid_widget->width();
        }

        int height_per_row;

        if (m_properties.use_fixed_panel_size) {
            width = num_cols * m_properties.fixed_panel_width;
            height = num_rows * m_properties.fixed_panel_height;
            height_per_row = m_properties.fixed_panel_height;
            m_grid_widget->setFixedWidth(width);
        }
        else {
            height_per_row = height / num_rows;
            if (height_per_row > q->height())
                height = num_rows * q->height();
            if (preferred_width == 0) {
                height = q->height() - 5;
            }
        }

        m_grid_widget->setFixedHeight(height);
        if ((m_force_square_matrix) && (!m_properties.use_fixed_panel_size)) {
            double width = height * q->width() / q->height();
            if (preferred_width == 0)
                width = q->width() - 5;
            m_grid_widget->setFixedWidth(width);
        }
    }
    setup_grid(num_cols);
}

void MVGridViewPrivate::setup_grid(int num_cols)
{
    QGridLayout* GL = m_grid_layout;
    for (int i = GL->count() - 1; i >= 0; i--) {
        GL->takeAt(i);
    }
    int num_rows = 0;
    for (int jj = 0; jj < m_views.count(); jj++) {
        QWidget* HV = m_views[jj];
        int row0 = (jj) / num_cols;
        int col0 = (jj) % num_cols;
        ViewWrap* VW = new ViewWrap(q, HV, jj);
        m_view_wraps << VW;
        QFont fnt = HV->font();
        fnt.setPixelSize(m_properties.font_size);
        HV->setFont(fnt);
        GL->addWidget(VW, row0, col0);
        HV->setProperty("row", row0);
        HV->setProperty("col", col0);
        HV->setProperty("view_index", jj);
        num_rows = qMax(num_rows, row0 + 1);
    }

    if (m_horizontal_scale_widget)
        m_grid_layout->addWidget(m_horizontal_scale_widget, num_rows, 0, 1, 1);
}

void MVGridViewPrivate::get_num_rows_cols_and_height_for_preferred_width(int& num_rows, int& num_cols, int& height, double preferred_width)
{
    int W = q->width();
    int H = q->height();
    if (!(W * H)) {
        num_rows = num_cols = 1;
        height = 0;
        return;
    }
    double preferred_aspect_ratio = m_preferred_aspect_ratio;
    if (m_force_square_matrix) {
        preferred_aspect_ratio = W * 1.0 / H;
    }

    if (m_force_square_matrix) {
        int NUM = m_views.count();
        num_rows = qMax(1, (int)sqrt(NUM));
        num_cols = qMax(1, (NUM + num_rows - 1) / num_rows);
        double hist_width = preferred_width;
        if (hist_width * num_cols < W) {
            hist_width = W / (num_cols);
        }
        double hist_height = hist_width / preferred_aspect_ratio;
        height = hist_height * num_rows;
    }
    else {
        bool done = false;
        while (!done) {
            if (preferred_width) {
                num_cols = qMax(1.0, W / preferred_width);
            }
            else {
                num_cols = qMax(1, (int)(m_views.count() / preferred_aspect_ratio + 0.5));
            }
            int hist_width = W / num_cols;
            int hist_height = hist_width / preferred_aspect_ratio;
            num_rows = qMax(1, (m_views.count() + num_cols - 1) / num_cols);
            height = num_rows * hist_height;
            if (height < H - hist_height * 0.5) {
                preferred_width += 1;
            }
            else
                done = true;
        }
    }
}

void MVGridView::setPreferredViewWidth(int width)
{
    d->m_preferred_width = width;
    d->on_resize();
}

void MVGridView::updateLayout()
{
    d->on_resize();
}

void MVGridView::setHorizontalScaleWidget(QWidget* W)
{
    d->m_horizontal_scale_widget = W;
}

void MVGridView::setForceSquareMatrix(bool val)
{
    d->m_force_square_matrix = val;
    if (val) {
        d->m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    else {
        d->m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    d->on_resize();
}

void MVGridView::setPreferredHistogramWidth(int width)
{
    d->m_preferred_width = width;
}

void MVGridView::updateViews()
{
    foreach (QWidget* W, d->m_views) {
        W->update();
    }
}

int MVGridView::currentViewIndex() const
{
    if (d->m_current_view_index >= d->m_views.count())
        return 0;
    return d->m_current_view_index;
}

QImage MVGridView::renderImage(int W, int H)
{
    if (!W) {
        W = 1800;
    }
    if (!H) {
        H = 900;
    }
    int max_row = 0, max_col = 0;
    for (int i = 0; i < d->m_views.count(); i++) {
        QWidget* W = d->m_views[i];
        int row = W->property("row").toInt();
        int col = W->property("col").toInt();
        if (row > max_row)
            max_row = row;
        if (col > max_col)
            max_col = col;
    }
    int NR = max_row + 1, NC = max_col + 1;
    if (d->m_properties.use_fixed_panel_size) {
        W = d->m_properties.fixed_panel_width * NC;
        H = d->m_properties.fixed_panel_height * NR;
    }
    int spacingx = (W / NC) * 0.1;
    int spacingy = (H / NR) * 0.1;
    int W0 = (W - spacingx * (NC + 1)) / NC;
    int H0 = (H - spacingy * (NR + 1)) / NR;

    QImage ret = QImage(W, H, QImage::Format_RGB32);
    QPainter painter(&ret);
    painter.fillRect(0, 0, ret.width(), ret.height(), Qt::white);

    for (int i = 0; i < d->m_views.count(); i++) {
        RenderableWidget* W = d->m_views[i];
        int row = W->property("row").toInt();
        int col = W->property("col").toInt();
        W->setExportMode(true);
        QImage img = W->renderImage(W0, H0);
        W->setExportMode(false);
        int x0 = spacingx + (W0 + spacingx) * col;
        int y0 = spacingy + (H0 + spacingy) * row;
        painter.drawImage(x0, y0, img);
        QPen pen = painter.pen();
        pen.setColor(Qt::lightGray);
        painter.setPen(pen);
        painter.drawRect(x0, y0, img.width(), img.height());
    }

    return ret;
}
