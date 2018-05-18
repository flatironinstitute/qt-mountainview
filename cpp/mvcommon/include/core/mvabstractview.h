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

#ifndef MVABSTRACTVIEW_H
#define MVABSTRACTVIEW_H

#include "mvabstractcontext.h"
#include "mlcommon.h"
#include <QMimeData>
#include <QWidget>

class MVAbstractViewFactory;
class MVAbstractViewPrivate;
class CalculationThread;
class MVAbstractView : public QWidget {
    Q_OBJECT
public:
    enum ViewFeature {
        NoFeatures = 0x0,
        RenderView = 0x1 // can render itself on a given painter (e.g. for export)
    };
    Q_DECLARE_FLAGS(ViewFeatures, ViewFeature)

    friend class MVAbstractViewPrivate;
    friend class CalculationThread;
    MVAbstractView(MVAbstractContext* context);
    virtual ~MVAbstractView();

    bool isCalculating() const;
    bool recalculateSuggested() const;
    void suggestRecalculate();
    void stopCalculation();

    virtual QString title() const;
    virtual void setTitle(const QString& title);

    QString calculatingMessage() const;

    virtual QJsonObject exportStaticView();
    virtual void loadStaticView(const QJsonObject& X);

    QList<QWidget*> toolbarControls();

    virtual MVAbstractViewFactory* viewFactory() const;
    MVAbstractContext* mvContext();
    virtual ViewFeatures viewFeatures() const;

public slots:
    void recalculate();
    void neverSuggestRecalculate();
    virtual void renderView(QPainter* painter); // add render opts

signals:
    void calculationStarted();
    void calculationFinished();
    void recalculateSuggestedChanged();
    void signalClusterContextMenu();
    void contextMenuRequested(const QMimeData&, const QPoint& globalPos);

protected:
    /*
     * prepareCalculation() and onCalculationFinished() are called in the gui thread,
     * whereas runCalculation() is called in the worker thread.
     * Therefore, it is important that runCalculation() does not access any
     * variables that may be used in the gui thread. However, it is guaranteed
     * that prepareCalculation() and onCalculationFinished() will not be called
     * in while runCalculation() is running. The calculation should check periodically
     * whether MLUtil::threadInterruptRequested() is true. In that case, it is
     * important to return from runCalculation() prematurely.
     */
    virtual void prepareCalculation() = 0;
    virtual void runCalculation() = 0;
protected slots:
    virtual void onCalculationFinished() = 0;

protected:
    void recalculateOnOptionChanged(QString name, bool suggest_only = true);
    void recalculateOn(QObject*, const char* signal, bool suggest_only = true);
    void onOptionChanged(QString name, QObject* receiver, const char* signal_or_slot);
    void setCalculatingMessage(QString msg);

    void requestContextMenu(const QMimeData& md, const QPoint& pos);
    virtual void prepareMimeData(QMimeData& mimeData, const QPoint& pos);

    void contextMenuEvent(QContextMenuEvent* evt);

    void addToolbarControl(QWidget* W);
private slots:
    void slot_do_calculation();
    void slot_calculation_finished();
    void slot_context_option_changed(QString name);
    void slot_suggest_recalculate();

private:
    MVAbstractViewPrivate* d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MVAbstractView::ViewFeatures)

#endif // MVABSTRACTVIEW_H
