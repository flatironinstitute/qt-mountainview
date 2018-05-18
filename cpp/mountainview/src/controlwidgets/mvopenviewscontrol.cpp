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

#include "flowlayout.h"
#include "mvabstractview.h"
#include "mvabstractviewfactory.h"
#include "mvopenviewscontrol.h"
#include "mvmainwindow.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QTimer>
#include <QSignalMapper>
#include <QToolButton>
#include <QAction>

class MVOpenViewsControlPrivate {
public:
    MVOpenViewsControl* q;
    FlowLayout* m_flow_layout;
    QSignalMapper* m_viewMapper;
    QMap<QString, QAbstractButton*> m_buttons;

    QAbstractButton* m_close_all_button;
};

MVOpenViewsControl::MVOpenViewsControl(MVAbstractContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVOpenViewsControlPrivate;
    d->q = this;

    d->m_viewMapper = new QSignalMapper(this);
    connect(d->m_viewMapper, SIGNAL(mapped(QObject*)),
        this, SLOT(slot_open_view(QObject*)));

    d->m_flow_layout = new FlowLayout;

    QList<QAction*> actions;
    QMimeData md;
    /// Witold, is there a better way to do this than to set a "non-empty" dummy string?
    md.setData("x-mv-main", "non-empty");
    QStringList plugin_names = this->mainWindow()->loadedPluginNames();
    foreach (QString plugin_name, plugin_names) {
        actions.append(this->mainWindow()->loadedPlugin(plugin_name)->actions(md));
    }
    foreach (QAction* action, actions) {
        QToolButton* button = new QToolButton;
        QFont font = button->font();
        font.setPixelSize(14);
        button->setFont(font);
        button->setText(action->text());
        d->m_flow_layout->addWidget(button);
        QObject::connect(button, SIGNAL(clicked()), action, SLOT(trigger()));
    }

    QList<MVAbstractViewFactory*> factories = mw->viewFactories();
    foreach (MVAbstractViewFactory* f, factories) {
        if (!f->name().isEmpty()) {
            QToolButton* button = new QToolButton;
            QFont font = button->font();
            font.setPixelSize(14);
            button->setFont(font);
            button->setText(f->name());
            button->setProperty("action_name", f->id());
            d->m_flow_layout->addWidget(button);
            d->m_viewMapper->setMapping(button, f);
            QObject::connect(button, SIGNAL(clicked()), d->m_viewMapper, SLOT(map()));
            //QObject::connect(f, SIGNAL(enabledChanged(bool)), button, SLOT(setEnabled(bool)));
            d->m_buttons[f->name()] = button;
        }
    }

    {
        QToolButton* button = new QToolButton;
        QFont font = button->font();
        font.setPixelSize(14);
        button->setFont(font);
        button->setText("Close All");
        d->m_flow_layout->addWidget(button);
        QObject::connect(button, SIGNAL(clicked()), mw, SLOT(slotCloseAllViews()));
        d->m_close_all_button = button;
    }
    this->setLayout(d->m_flow_layout);

    QObject::connect(context, SIGNAL(selectedClustersChanged()), this, SLOT(slot_update_enabled()));
    QObject::connect(context, SIGNAL(currentClusterChanged()), this, SLOT(slot_update_enabled()));
    QObject::connect(context, SIGNAL(selectedClusterPairsChanged()), this, SLOT(slot_update_enabled()));

    QObject::connect(mw, SIGNAL(viewsChanged()), this, SLOT(updateControls()));
    QObject::connect(mw, SIGNAL(viewsChanged()), this, SLOT(slot_update_enabled()));

    updateControls();

    slot_update_enabled();
}

MVOpenViewsControl::~MVOpenViewsControl()
{
    delete d;
}

QString MVOpenViewsControl::title() const
{
    return "Open Views";
}

void MVOpenViewsControl::updateContext()
{
}

void MVOpenViewsControl::updateControls()
{
    d->m_close_all_button->setEnabled(!this->mainWindow()->allViews().isEmpty());
}

void MVOpenViewsControl::slot_open_view(QObject* obj)
{
    MVAbstractViewFactory* factory = qobject_cast<MVAbstractViewFactory*>(obj);
    if (!factory)
        return;
    this->mainWindow()->openView(factory->id());
}

void MVOpenViewsControl::slot_update_enabled()
{
    foreach (const MVAbstractViewFactory* factory, this->mainWindow()->viewFactories()) {
        if (d->m_buttons[factory->name()]) {
            d->m_buttons[factory->name()]->setEnabled(factory->isEnabled(mvContext()));
        }
    }
}

void MVOpenViewsControl::slot_close_all_views()
{
    this->mainWindow()->closeAllViews();
}
