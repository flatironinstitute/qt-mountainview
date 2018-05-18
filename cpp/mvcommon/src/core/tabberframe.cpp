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

#include "tabberframe.h"

#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>
#include <QToolButton>
#include <QStackedLayout>
#include <QPainter>

class TabberFramePrivate {
public:
    TabberFrame* q;
    MVAbstractView* m_view;
    QString m_container_name;
    QToolBar* m_toolbar;
    QAction* m_recalc_action;
    QAction* m_never_recalc_action;
    QAction* m_move_up_action;
    QAction* m_move_down_action;
    QAction* m_pop_out_action;
    QAction* m_pop_in_action;
    QStackedLayout* m_stack;
    TabberFrameOverlay* m_overlay;

    void update_action_visibility();
    static QList<QAction*> find_actions_of_type(QList<QAction*> actions, QString str);
};

TabberFrame::TabberFrame(MVAbstractView* view)
{
    d = new TabberFramePrivate;
    d->q = this;
    d->m_view = view;
    d->m_toolbar = new QToolBar;
    {
        QAction* A = new QAction(QIcon(":/images/calculator.png"), "", this);
        A->setToolTip("Recalculate this view");
        QObject::connect(A, SIGNAL(triggered(bool)), view, SLOT(recalculate()));
        d->m_recalc_action = A;
    }
    {
        QAction* A = new QAction(QIcon(":/images/calculator-no.png"), "", this);
        A->setToolTip("Lock this view so that it will not auto-recalculate");
        QObject::connect(A, SIGNAL(triggered(bool)), view, SLOT(neverSuggestRecalculate()));
        d->m_never_recalc_action = A;
    }
    {
        QAction* A = new QAction(QIcon(":/images/move-up.png"), "", this);
        A->setToolTip("Move view to other container");
        QObject::connect(A, SIGNAL(triggered(bool)), this, SIGNAL(signalMoveToOtherContainer()));
        d->m_move_up_action = A;
    }
    {
        QAction* A = new QAction(QIcon(":/images/move-down.png"), "", this);
        A->setToolTip("Move view to other container");
        QObject::connect(A, SIGNAL(triggered(bool)), this, SIGNAL(signalMoveToOtherContainer()));
        d->m_move_down_action = A;
    }
    {
        QAction* A = new QAction(QIcon(":/images/pop-out.png"), "", this);
        A->setToolTip("Float this view");
        QObject::connect(A, SIGNAL(triggered(bool)), this, SIGNAL(signalPopOut()));
        d->m_pop_out_action = A;
    }
    {
        QAction* A = new QAction(QIcon(":/images/pop-in.png"), "", this);
        A->setToolTip("Dock this view");
        QObject::connect(A, SIGNAL(triggered(bool)), this, SIGNAL(signalPopIn()));
        d->m_pop_in_action = A;
    }

    QObject::connect(d->m_view, SIGNAL(calculationStarted()), this, SLOT(slot_update_calculating()));
    QObject::connect(d->m_view, SIGNAL(calculationFinished()), this, SLOT(slot_update_calculating()));

    QToolButton* tool_button = new QToolButton;
    QMenu* menu = new QMenu;
    menu->addActions(d->find_actions_of_type(view->actions(), ""));
    tool_button->setMenu(menu);
    tool_button->setIcon(QIcon(":/images/gear.png"));
    tool_button->setPopupMode(QToolButton::InstantPopup);
    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QList<QAction*> toolbar_actions = d->find_actions_of_type(view->actions(), "toolbar");
    foreach (QAction* a, toolbar_actions) {
        d->m_toolbar->addAction(a);
    }
    QList<QWidget*> toolbar_controls = view->toolbarControls();
    foreach (QWidget* W, toolbar_controls) {
        W->show();
        d->m_toolbar->addWidget(W);
    }

    d->m_toolbar->addWidget(spacer);

    d->m_toolbar->addAction(d->m_recalc_action);
    d->m_toolbar->addAction(d->m_never_recalc_action);
    d->m_toolbar->addAction(d->m_move_up_action);
    d->m_toolbar->addAction(d->m_move_down_action);
    d->m_toolbar->addAction(d->m_pop_out_action);
    d->m_toolbar->addAction(d->m_pop_in_action);

    d->m_toolbar->addWidget(tool_button);

    d->m_overlay = new TabberFrameOverlay;

    QStackedLayout* stack = new QStackedLayout;
    d->m_stack = stack;
    stack->addWidget(view);
    stack->addWidget(d->m_overlay);
    stack->setCurrentWidget(d->m_overlay);
    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->setSpacing(0);
    vlayout->setMargin(0);
    vlayout->addWidget(d->m_toolbar);
    vlayout->addLayout(stack);

    this->setLayout(vlayout);

    QObject::connect(view, SIGNAL(recalculateSuggestedChanged()), this, SLOT(slot_update_action_visibility()));
    d->update_action_visibility();

    QObject::connect(view, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
}

TabberFrame::~TabberFrame()
{
    delete d;
}

MVAbstractView* TabberFrame::view()
{
    return d->m_view;
}

void TabberFrame::setContainerName(QString name)
{
    if (d->m_container_name == name)
        return;
    d->m_container_name = name;
    d->update_action_visibility();
}

void TabberFrame::slot_update_action_visibility()
{
    d->update_action_visibility();
}

void TabberFrame::slot_update_calculating()
{
    if (d->m_view->isCalculating()) {
        if (d->m_view)
            d->m_overlay->calculating_message = d->m_view->calculatingMessage();
        d->m_stack->setCurrentWidget(d->m_overlay);
    }
    else {
        d->m_stack->setCurrentWidget(d->m_view);
    }
}

void TabberFramePrivate::update_action_visibility()
{
    if (m_view->recalculateSuggested()) {
        m_recalc_action->setVisible(true);
        m_never_recalc_action->setVisible(true);
    }
    else {
        m_recalc_action->setVisible(false);
        m_never_recalc_action->setVisible(false);
    }

    if (m_container_name == "north") {
        m_move_up_action->setVisible(false);
        m_move_down_action->setVisible(true);
        m_pop_out_action->setVisible(true);
        m_pop_in_action->setVisible(false);
    }
    else if (m_container_name == "south") {
        m_move_up_action->setVisible(true);
        m_move_down_action->setVisible(false);
        m_pop_out_action->setVisible(true);
        m_pop_in_action->setVisible(false);
    }
    else {
        m_move_up_action->setVisible(false);
        m_move_down_action->setVisible(false);
        m_pop_out_action->setVisible(false);
        m_pop_in_action->setVisible(true);
    }
}

QList<QAction*> TabberFramePrivate::find_actions_of_type(QList<QAction*> actions, QString str)
{
    QList<QAction*> ret;
    for (int i = 0; i < actions.count(); i++) {
        if (actions[i]->property("action_type").toString() == str) {
            ret << actions[i];
        }
    }
    return ret;
}

void TabberFrameOverlay::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)
    QPainter painter(this);
    QFont font = painter.font();
    font.setPointSize(font.pointSize() + 8);
    painter.setFont(font);
    QRectF rect(0, 0, this->width(), this->height());
    painter.drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, calculating_message);
}
