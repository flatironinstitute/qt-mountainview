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

#include "mvabstractview.h"
#include <QAction>
#include <QJsonArray>
#include <QMenu>
#include <QThread>
#include <QTimer>
#include <QToolButton>
#include <QContextMenuEvent>
#include <QJsonObject>

class CalculationThread : public QThread {
public:
    void run();
    MVAbstractView* q;
};

class MVAbstractViewPrivate {
public:
    MVAbstractView* q;
    MVAbstractContext* m_context;
    QSet<QString> m_recalculate_on_option_names;
    QSet<QString> m_suggest_recalculate_on_option_names;
    bool m_calculation_scheduled;
    bool m_recalculate_suggested;
    bool m_never_suggest_recalculate = false;
    QString m_calculating_message = "Calculating...";
    QString m_title;
    QList<QWidget*> m_toolbar_controls;

    CalculationThread m_calculation_thread;

    void stop_calculation();
    void schedule_calculation();
    void set_recalculate_suggested(bool val);
};

MVAbstractView::MVAbstractView(MVAbstractContext* context)
{
    d = new MVAbstractViewPrivate;
    d->q = this;
    d->m_calculation_thread.q = this;
    d->m_recalculate_suggested = false;

    d->m_context = context;
    d->m_calculation_scheduled = false;

    // Very important to make this a queued connection because the subclass destructor
    // is called before MVAbstractView constructor, and therefore the calculation thread does not get stopped before the subclass gets destructed
    QObject::connect(&d->m_calculation_thread, SIGNAL(finished()), this, SLOT(slot_calculation_finished()), Qt::QueuedConnection);
    QObject::connect(context, SIGNAL(optionChanged(QString)), this, SLOT(slot_context_option_changed(QString)));

    {
        QAction* a = new QAction(QIcon(":/image/gear.png"), "Force recalculate", this);
        this->addAction(a);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(recalculate()));
    }

    this->setFocusPolicy(Qt::StrongFocus);
}

MVAbstractView::~MVAbstractView()
{
    d->stop_calculation();
    delete d;
}

bool MVAbstractView::isCalculating() const
{
    return d->m_calculation_thread.isRunning();
}

bool MVAbstractView::recalculateSuggested() const
{
    return ((!d->m_never_suggest_recalculate) && (d->m_recalculate_suggested));
}

void MVAbstractView::suggestRecalculate()
{
    d->set_recalculate_suggested(true);
}

void MVAbstractView::stopCalculation()
{
    d->stop_calculation();
}

QString MVAbstractView::title() const
{
    return d->m_title;
}

void MVAbstractView::setTitle(const QString& title)
{
    d->m_title = title;
}

QString MVAbstractView::calculatingMessage() const
{
    return d->m_calculating_message;
}

QJsonObject MVAbstractView::exportStaticView()
{
    QJsonObject ret;
    ret["title"] = this->title();
    return QJsonObject();
}

void MVAbstractView::loadStaticView(const QJsonObject& X)
{
    if (X.contains("title"))
        this->setTitle(X["title"].toString());
}

QList<QWidget*> MVAbstractView::toolbarControls()
{
    return d->m_toolbar_controls;
}

MVAbstractViewFactory* MVAbstractView::viewFactory() const
{
    return 0;
}

MVAbstractContext* MVAbstractView::mvContext()
{
    return d->m_context;
}

MVAbstractView::ViewFeatures MVAbstractView::viewFeatures() const
{
    return NoFeatures; // no features by default
}

void MVAbstractView::renderView(QPainter* painter)
{
    Q_UNUSED(painter)
    // do nothing in the base class
}

void MVAbstractView::recalculateOnOptionChanged(QString name, bool suggest_only)
{
    if (suggest_only)
        d->m_suggest_recalculate_on_option_names.insert(name);
    else
        d->m_recalculate_on_option_names.insert(name);
}

void MVAbstractView::recalculateOn(QObject* obj, const char* signal, bool suggest_only)
{
    if (suggest_only)
        QObject::connect(obj, signal, this, SLOT(slot_suggest_recalculate()));
    else
        QObject::connect(obj, signal, this, SLOT(recalculate()));
}

void MVAbstractView::onOptionChanged(QString name, QObject* receiver, const char* signal_or_slot)
{
    mvContext()->onOptionChanged(name, receiver, signal_or_slot);
}

void MVAbstractView::setCalculatingMessage(QString msg)
{
    d->m_calculating_message = msg;
}

void MVAbstractView::requestContextMenu(const QMimeData& md, const QPoint& pos)
{
    emit contextMenuRequested(md, mapToGlobal(pos));
}

void MVAbstractView::prepareMimeData(QMimeData& mimeData, const QPoint& pos)
{
    Q_UNUSED(pos)
    // add info about the view itself
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds << (quintptr) this;
    mimeData.setData("application/x-mv-view", ba); // this view
}

void MVAbstractView::contextMenuEvent(QContextMenuEvent* evt)
{
    QPoint pt = evt->pos();
    QMimeData mimeData;
    prepareMimeData(mimeData, pt);
    if (!mimeData.formats().isEmpty())
        requestContextMenu(mimeData, pt);
}

void MVAbstractView::addToolbarControl(QWidget* W)
{
    W->setParent(this);
    d->m_toolbar_controls << W;
    W->hide();
}

void MVAbstractView::recalculate()
{
    d->stop_calculation();
    d->schedule_calculation();
}

void MVAbstractView::neverSuggestRecalculate()
{
    d->m_never_suggest_recalculate = true;
    if (d->m_recalculate_suggested) {
        d->m_recalculate_suggested = false;
        emit recalculateSuggestedChanged();
    }
}

void MVAbstractView::slot_do_calculation()
{
    d->set_recalculate_suggested(false);
    this->update();
    prepareCalculation();
    d->m_calculation_thread.start();
    d->m_calculation_scheduled = false;
    this->update();
    emit this->calculationStarted();
}

void MVAbstractView::slot_calculation_finished()
{
    this->onCalculationFinished();
    this->update();
    d->set_recalculate_suggested(false);
    emit this->calculationFinished();
}

void MVAbstractView::slot_context_option_changed(QString name)
{
    if (d->m_recalculate_on_option_names.contains(name)) {
        recalculate();
    }
    if (d->m_suggest_recalculate_on_option_names.contains(name)) {
        slot_suggest_recalculate();
    }
}

void MVAbstractView::slot_suggest_recalculate()
{
    d->set_recalculate_suggested(true);
}

void CalculationThread::run()
{
    q->runCalculation();
}

void MVAbstractViewPrivate::stop_calculation()
{
    if (m_calculation_thread.isRunning()) {
        m_calculation_thread.requestInterruption();
        m_calculation_thread.wait();
    }
}

void MVAbstractViewPrivate::schedule_calculation()
{
    if (m_calculation_scheduled)
        return;
    m_calculation_scheduled = true;
    QTimer::singleShot(100, q, SLOT(slot_do_calculation()));
}

void MVAbstractViewPrivate::set_recalculate_suggested(bool val)
{
    if (m_never_suggest_recalculate) {
        val = false;
    }
    if (m_recalculate_suggested == val)
        return;
    m_recalculate_suggested = val;
    emit q->recalculateSuggestedChanged();
}
