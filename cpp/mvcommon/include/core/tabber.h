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

#ifndef TABBER_H
#define TABBER_H

#include "mvabstractview.h"

#include <QTabWidget>
#include <QString>

/**
 *  \class Tabber
 *  \brief Use this class to control two or more tab widgets so that the user
 * may move widgets from one to the other. For now user double clicks on a tab
 * to move it to the other, but in the future, drag/drop should be enabled. Also
 * we'd like user to be able to drag it off the widget to become undocked (floating).
 */

class TabberTabWidget;
class TabberPrivate;
class Tabber : public QObject {
    Q_OBJECT
public:
    friend class TabberPrivate;
    Tabber();
    virtual ~Tabber();
    TabberTabWidget* createTabWidget(const QString& container_name);
    void addWidget(const QString& container_name, const QString& label, MVAbstractView* W);
    void addWidget(TabberTabWidget* TW, const QString& label, MVAbstractView* W);
    QString currentContainerName();
    void setCurrentContainerName(const QString& container_name);
    void setCurrentContainer(TabberTabWidget* TW);
    void switchCurrentContainer();
    QList<MVAbstractView*> allWidgets();

    void moveWidgetToOtherContainer(MVAbstractView* W);
    void popOutWidget(MVAbstractView* W);
    MVAbstractView* currentView() const;
    void closeView(MVAbstractView*);
signals:
    void widgetsChanged();
private slots:
    void slot_tab_close_requested(int index);
    void slot_tab_bar_double_clicked(int index);
    void slot_tab_bar_right_clicked(QPoint pt);
    void slot_widget_destroyed(QObject* obj);
    void slot_recalculate_suggested_changed();
    void slot_move_to_other_container();
    void slot_pop_out();
    void slot_pop_in();

private:
    TabberPrivate* d;
};

class TabberTabWidgetPrivate;
class TabberTabWidget : public QTabWidget {
    Q_OBJECT
public:
    friend class TabberTabWidgetPrivate;
    TabberTabWidget();
    virtual ~TabberTabWidget();
    MVAbstractView* view(int index);
signals:
    void signalRightClick(QPoint pt);

private:
    TabberTabWidgetPrivate* d;
};

#endif // TABBER_H
