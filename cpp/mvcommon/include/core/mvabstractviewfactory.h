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
#ifndef MVABSTRACTVIEWFACTORY_H
#define MVABSTRACTVIEWFACTORY_H

#include <QWidget>
#include "mvabstractview.h"

class MVMainWindow;
class MVContext;
class MVAbstractViewFactory : public QObject {
    Q_OBJECT
public:
    explicit MVAbstractViewFactory(MVMainWindow* mw, QObject* parent = 0);

    enum PreferredOpenLocation {
        North,
        South,
        Floating,
        NoPreference
    };

    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString group() const;
    virtual QString toolTip() const;
    virtual QString title() const; /// TODO: move title to the view itself
    virtual PreferredOpenLocation preferredOpenLocation() const;
    virtual int order() const { return 0; }
    virtual bool isEnabled(MVAbstractContext* context) const;

    MVMainWindow* mainWindow();

    virtual MVAbstractView* createView(MVAbstractContext* context) {(void)context; return 0;}
    virtual MVAbstractView* createView(MVAbstractContext* context, const QJsonObject &data) {(void)context; (void)data; return 0;}
    virtual QList<QAction*> actions(const QMimeData& md);
signals:

private:
    MVMainWindow* m_main_window;
    bool m_enabled;
};

#endif // MVABSTRACTVIEWFACTORY_H
