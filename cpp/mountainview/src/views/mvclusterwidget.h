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

#ifndef MVCLUSTERWIDGET_H
#define MVCLUSTERWIDGET_H

#include <QWidget>
#include "mvutils.h"
#include "affinetransformation.h"
#include "mvclusterview.h" //for FilterInfo
#include "mvabstractview.h"
#include "mvabstractviewfactory.h"
#include "mvmainwindow.h"

/** \class MVClusterWidget
 *  \brief Presents one or more cluster views and a synchronized clip view
 *
 */

class MVClusterWidgetPrivate;
class MVClusterWidget : public MVAbstractView {
    Q_OBJECT
public:
    friend class MVClusterWidgetPrivate;
    MVClusterWidget(MVAbstractContext* context);
    virtual ~MVClusterWidget();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void setLabelsToUse(const QList<int>& labels);
    void setFeatureMode(QString mode); //"pca", "channels"
    void setChannels(QVector<int> channels); //relevant for "channels" mode

    void setTransformation(const AffineTransformation& T);

private:
    void setData(const Mda& X);
    void setTimes(const QVector<double>& times);
    void setLabels(const QVector<int>& labels);
    void setAmplitudes(const QVector<double>& amps);

signals:
private slots:
    void slot_current_event_changed();
    void slot_view_current_event_changed();
    void slot_view_transformation_changed();
    void slot_view_active_cluster_numbers_changed();
    void slot_show_clip_view_clicked();
    void slot_show_view_clicked();
    void slot_clips_view_thread_finished();
    void slot_shift_colors_left(int step = 1);
    void slot_shift_colors_right();
    void slot_export_features();
    void slot_export_labels();

private:
    MVClusterWidgetPrivate* d;
};

class MVPCAFeaturesFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVPCAFeaturesFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    bool isEnabled(MVAbstractContext* context) const Q_DECL_OVERRIDE;
};

class MVChannelFeaturesFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVChannelFeaturesFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    bool isEnabled(MVAbstractContext* context) const Q_DECL_OVERRIDE;
};

#endif // MVCLUSTERWIDGET_H
