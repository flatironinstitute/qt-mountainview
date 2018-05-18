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

#ifndef MVCROSSCORRELOGRAMSWIDGET3_H
#define MVCROSSCORRELOGRAMSWIDGET3_H

#include "mvabstractviewfactory.h"
#include "mvhistogramgrid.h"

#include <QWidget>

enum CrossCorrelogramMode3 {
    Undefined3,
    All_Auto_Correlograms3,
    Selected_Auto_Correlograms3,
    Cross_Correlograms3,
    Matrix_Of_Cross_Correlograms3,
    Selected_Cross_Correlograms3
};

struct CrossCorrelogramOptions3 {
    CrossCorrelogramMode3 mode = Undefined3;
    QList<int> ks;
    QList<ClusterPair> pairs;

    QJsonObject toJsonObject();
    void fromJsonObject(const QJsonObject& X);
};

class MVCrossCorrelogramsWidget3Private;
class MVCrossCorrelogramsWidget3 : public MVHistogramGrid {
    Q_OBJECT
public:
    friend class MVCrossCorrelogramsWidget3Private;
    MVCrossCorrelogramsWidget3(MVAbstractContext* context);
    virtual ~MVCrossCorrelogramsWidget3();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void setOptions(CrossCorrelogramOptions3 opts);
    void setTimeScaleMode(HistogramView::TimeScaleMode mode);
    HistogramView::TimeScaleMode timeScaleMode() const;

    QJsonObject exportStaticView() Q_DECL_OVERRIDE;
    void loadStaticView(const QJsonObject& X) Q_DECL_OVERRIDE;

signals:
private slots:
    void slot_log_time_scale();
    void slot_warning();
    void slot_export_static_view();

private:
    MVCrossCorrelogramsWidget3Private* d;
};

class MVAutoCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVAutoCorrelogramsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    bool isEnabled(MVAbstractContext* context) const Q_DECL_OVERRIDE;
private slots:
};

class MVSelectedAutoCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVSelectedAutoCorrelogramsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    bool isEnabled(MVAbstractContext* context) const Q_DECL_OVERRIDE;
};

class MVCrossCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVCrossCorrelogramsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    bool isEnabled(MVAbstractContext* context) const Q_DECL_OVERRIDE;
};

class MVMatrixOfCrossCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVMatrixOfCrossCorrelogramsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    bool isEnabled(MVAbstractContext* context) const Q_DECL_OVERRIDE;
};

class MVSelectedCrossCorrelogramsFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVSelectedCrossCorrelogramsFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
    bool isEnabled(MVAbstractContext* context) const Q_DECL_OVERRIDE;
};

#endif // MVCROSSCORRELOGRAMSWIDGET3_H
