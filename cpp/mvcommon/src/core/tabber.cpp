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
#include "tabber.h"
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QMenu>
#include <QTabBar>
#include <QInputDialog>
#include "tabberframe.h"

struct TabberWidget {
    TabberFrame* frame;
    MVAbstractView* widget;
    QString label;
    QString current_container_name;
};

class TabberPrivate {
public:
    Tabber* q;
    QMap<QString, TabberTabWidget*> m_tab_widgets; //the tab widgets we are handling
    QList<TabberWidget> m_widgets; //the TabberWidget structs of widgets within the tab widgets
    QString m_current_container_name; //the name of the last selected container -- empty means floating

    ///Put the widget in the specified tab widget (removing from existing tab widget if necessary)
    void put_widget_in_container(QString container_name, MVAbstractView* W);
    ///Find the struct (TabberWidget) that corresponds to W
    TabberWidget* find_tabber_widget(MVAbstractView* W);
    TabberWidget* find_tabber_widget(TabberFrame* F);
    ///Find the index of a widget within a particular tab widget
    int find_widget_index_in_container(QString container_name, MVAbstractView* W);
    ///Remove widget (but don't delete) from its container (or tab widget)
    void remove_widget(MVAbstractView* W);
    ///Check if this widget is contained in the list of widgets
    bool contains_widget(MVAbstractView* W);
    ///Get the name of a different container, or "" if none
    QString find_other_container_name(QString name);

    void update_tab_labels();
};

Tabber::Tabber()
{
    d = new TabberPrivate;
    d->q = this;
}

Tabber::~Tabber()
{
    delete d;
}

TabberTabWidget* Tabber::createTabWidget(const QString& container_name)
{
    TabberTabWidget* TW = new TabberTabWidget;
    TW->setTabPosition(QTabWidget::South);
    d->m_tab_widgets[container_name] = TW;
    TW->setProperty("container_name", container_name);
    connect(TW, SIGNAL(tabCloseRequested(int)), this, SLOT(slot_tab_close_requested(int)));
    connect(TW, SIGNAL(signalRightClick(QPoint)), this, SLOT(slot_tab_bar_right_clicked(QPoint)));
    connect(TW, SIGNAL(tabBarDoubleClicked(int)), this, SLOT(slot_tab_bar_double_clicked(int)));
    if (!container_name.isEmpty()) {
        d->m_current_container_name = container_name;
    }
    return TW;
}

void Tabber::addWidget(const QString& container_name, const QString& label, MVAbstractView* W)
{
    TabberWidget X;
    X.frame = new TabberFrame(W);
    X.widget = W;
    X.label = label;
    d->m_widgets << X;
    d->put_widget_in_container(container_name, X.widget);

    connect(W, SIGNAL(recalculateSuggestedChanged()), this, SLOT(slot_recalculate_suggested_changed()));
    connect(W, SIGNAL(destroyed(QObject*)), this, SLOT(slot_widget_destroyed(QObject*)));
    connect(X.frame, SIGNAL(signalMoveToOtherContainer()), this, SLOT(slot_move_to_other_container()));
    connect(X.frame, SIGNAL(signalPopOut()), this, SLOT(slot_pop_out()));
    connect(X.frame, SIGNAL(signalPopIn()), this, SLOT(slot_pop_in()));

    emit widgetsChanged();
}

void Tabber::addWidget(TabberTabWidget* TW, const QString& label, MVAbstractView* W)
{
    QString container_name = TW->property("container_name").toString();
    if (container_name.isEmpty())
        return;
    addWidget(container_name, label, W);
}

QString Tabber::currentContainerName()
{
    return d->m_current_container_name;
}

void Tabber::setCurrentContainerName(const QString& container_name)
{
    d->m_current_container_name = container_name;
}

void Tabber::setCurrentContainer(TabberTabWidget* TW)
{
    if (!TW)
        return;
    d->m_current_container_name = TW->property("container_name").toString();
}

void Tabber::switchCurrentContainer()
{
    d->m_current_container_name = d->find_other_container_name(d->m_current_container_name);
}

QList<MVAbstractView*> Tabber::allWidgets()
{
    QList<MVAbstractView*> ret;
    for (int i = 0; i < d->m_widgets.count(); i++) {
        d->m_widgets[i].widget->setProperty("container", d->m_widgets[i].current_container_name);
        ret << d->m_widgets[i].widget;
    }
    return ret;
}

void Tabber::moveWidgetToOtherContainer(MVAbstractView* W)
{
    TabberWidget* X = d->find_tabber_widget(W);
    if (!X)
        return;
    QString cname = X->current_container_name;
    QString cname2 = d->find_other_container_name(cname);
    d->put_widget_in_container(cname2, W);
}

void Tabber::popOutWidget(MVAbstractView* W)
{
    TabberWidget* X = d->find_tabber_widget(W);
    if (!X)
        return;
    d->put_widget_in_container("", W);
}

MVAbstractView *Tabber::currentView() const
{
    if (!d->m_current_container_name.isEmpty()) {
        TabberTabWidget *tabWidget = d->m_tab_widgets.value(d->m_current_container_name, nullptr);
        return tabWidget->view(tabWidget->currentIndex());
    }
    qWarning() << "Current view not available";
    return nullptr;
}

void Tabber::closeView(MVAbstractView *view)
{
    if (d->contains_widget(view)) { // I think this condition is important so we don't delete the same widget twice if the user requests close twice
        TabberWidget* X = d->find_tabber_widget(view);
        if (X) {
            delete X->frame;
            d->remove_widget(view);
        }
    }
}

void Tabber::slot_tab_close_requested(int index)
{
    Q_UNUSED(index)
    TabberTabWidget* TW = qobject_cast<TabberTabWidget*>(sender());
    if (!TW)
        return;
    MVAbstractView* W = TW->view(index);
    if (!W)
        return;
    if (d->contains_widget(W)) { // I think this condition is important so we don't delete the same widget twice if the user requests close twice
        TabberWidget* X = d->find_tabber_widget(W);
        if (X) {
            delete X->frame;
            d->remove_widget(W);
        }
    }
}

void Tabber::slot_tab_bar_double_clicked(int index)
{
    TabberTabWidget* TW = qobject_cast<TabberTabWidget*>(sender());
    if (!TW)
        return;
    MVAbstractView* W = TW->view(index);
    if (!W)
        return;
    moveWidgetToOtherContainer(W);
}

void Tabber::slot_tab_bar_right_clicked(QPoint pt)
{
    TabberTabWidget* TW = qobject_cast<TabberTabWidget*>(sender());
    if (!TW)
        return;
    int index = TW->tabBar()->tabAt(pt);
    if (index < 0)
        return;

    QString new_txt = QInputDialog::getText(0, "Rename tab", TW->tabBar()->tabText(index));
    if (!new_txt.isEmpty()) {
        TW->tabBar()->setTabText(index, new_txt);
        MVAbstractView* W = TW->view(index);
        if (!W)
            return;
        W->setTitle(new_txt);
    }
}

void Tabber::slot_widget_destroyed(QObject* obj)
{
    d->remove_widget((MVAbstractView*)obj);
}

void Tabber::slot_recalculate_suggested_changed()
{
    d->update_tab_labels();
}

void Tabber::slot_move_to_other_container()
{
    TabberFrame* FF = qobject_cast<TabberFrame*>(sender());
    if (!FF)
        return;
    TabberWidget* WW = d->find_tabber_widget(FF);
    if (!WW)
        return;
    this->moveWidgetToOtherContainer(WW->widget);
}

void Tabber::slot_pop_out()
{
    TabberFrame* FF = qobject_cast<TabberFrame*>(sender());
    if (!FF)
        return;
    TabberWidget* WW = d->find_tabber_widget(FF);
    if (!WW)
        return;
    this->popOutWidget(WW->widget);
}

void Tabber::slot_pop_in()
{
    TabberFrame* FF = qobject_cast<TabberFrame*>(sender());
    if (!FF)
        return;
    TabberWidget* WW = d->find_tabber_widget(FF);
    if (!WW)
        return;
    this->moveWidgetToOtherContainer(WW->widget);
}

void TabberPrivate::put_widget_in_container(QString container_name, MVAbstractView* W)
{
    TabberWidget* X = find_tabber_widget(W);
    if (!X)
        return;
    if (!container_name.isEmpty()) {
        if (!m_tab_widgets.contains(container_name))
            return;
        int index = m_tab_widgets[container_name]->addTab(X->frame, X->label);
        if (index >= 0) {
            m_tab_widgets[container_name]->setCurrentIndex(index);
        }
    }
    else {
        if (!X->current_container_name.isEmpty()) {
            QPoint position = X->frame->mapToGlobal(X->frame->pos());
            int index = find_widget_index_in_container(X->current_container_name, X->widget);
            if (index < 0)
                return;
            m_tab_widgets[X->current_container_name]->removeTab(index);

            X->frame->setParent(0);
            X->frame->show();
            X->frame->move(position);
        }
        else {
            QSize size(500, 300);
            if (m_tab_widgets.first()) {
                size = m_tab_widgets.first()->size();
            }
            X->frame->resize(size);
            X->frame->setParent(0);
            X->frame->show();
        }
    }
    X->current_container_name = container_name;
    if (!container_name.isEmpty()) {
        m_current_container_name = container_name;
    }
    X->frame->setContainerName(container_name);
}

TabberWidget* TabberPrivate::find_tabber_widget(MVAbstractView* W)
{
    for (int i = 0; i < m_widgets.count(); i++) {
        if (m_widgets[i].widget == W) {
            return &m_widgets[i];
        }
    }
    return 0;
}

TabberWidget* TabberPrivate::find_tabber_widget(TabberFrame* F)
{
    for (int i = 0; i < m_widgets.count(); i++) {
        if (m_widgets[i].frame == F) {
            return &m_widgets[i];
        }
    }
    return 0;
}

int TabberPrivate::find_widget_index_in_container(QString container_name, MVAbstractView* W)
{
    if (!m_tab_widgets.contains(container_name))
        return -1;
    TabberTabWidget* TW = m_tab_widgets[container_name];
    for (int i = 0; i < TW->count(); i++) {
        if (TW->view(i) == W)
            return i;
    }
    return -1;
}

void TabberPrivate::remove_widget(MVAbstractView* W)
{
    for (int i = 0; i < m_widgets.count(); i++) {
        if (m_widgets[i].widget == W) {
            m_widgets.removeAt(i);
            emit q->widgetsChanged();
            return;
        }
    }
}

bool TabberPrivate::contains_widget(MVAbstractView* W)
{
    for (int i = 0; i < m_widgets.count(); i++) {
        if (m_widgets[i].widget == W) {
            return true;
        }
    }
    return false;
}

QString TabberPrivate::find_other_container_name(QString name)
{
    QStringList keys = m_tab_widgets.keys();
    if (name.isEmpty())
        return keys.value(0);
    foreach (QString str, keys) {
        if (str != name)
            return str;
    }
    return "";
}

void TabberPrivate::update_tab_labels()
{
    foreach (TabberTabWidget* W, m_tab_widgets) {
        for (int i = 0; i < W->count(); i++) {
            MVAbstractView* w = W->view(i);
            TabberWidget* v = find_tabber_widget(w);
            if (v) {
                QString label = v->label;
                if (w->recalculateSuggested())
                    label += " (*)";
                W->setTabText(i, label);
            }
        }
    }
}

class TabberTabWidgetPrivate {
public:
    TabberTabWidget* q;
};

TabberTabWidget::TabberTabWidget()
    : QTabWidget()
{
    d = new TabberTabWidgetPrivate;
    d->q = this;
    this->setTabsClosable(true);
    this->setMovable(true);

    this->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this->tabBar(), SIGNAL(customContextMenuRequested(QPoint)), this, SIGNAL(signalRightClick(QPoint)));
}

TabberTabWidget::~TabberTabWidget()
{
    delete d;
}

MVAbstractView* TabberTabWidget::view(int index)
{
    TabberFrame* TF = qobject_cast<TabberFrame*>(this->widget(index));
    if (!TF)
        return 0;
    return TF->view();
}
