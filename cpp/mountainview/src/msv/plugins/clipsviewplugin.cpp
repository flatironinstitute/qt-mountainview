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

#include "clipsviewplugin.h"

#include <QAction>
#include <mvclipswidget.h>
#include <mvcontext.h>

class ClipsViewPluginPrivate {
public:
    ClipsViewPlugin* q;
};

ClipsViewPlugin::ClipsViewPlugin()
{
    d = new ClipsViewPluginPrivate;
    d->q = this;
}

ClipsViewPlugin::~ClipsViewPlugin()
{
    delete d;
}

QString ClipsViewPlugin::name()
{
    return "ClipsView";
}

QString ClipsViewPlugin::description()
{
    return "";
}

void ClipsViewPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new ClipsViewFactory(mw));
    MVAbstractPlugin::initialize(mw);
}

QList<QAction*> ClipsViewPlugin::actions(const QMimeData& md)
{
    (void)md;
    return QList<QAction*>();
    /*
    QList<QAction*> actions;
    if (!md.data("x-mv-main").isEmpty()) {
        QAction* action = new QAction("Clips", 0);
        MVMainWindow* mw = this->mainWindow();
        QObject::connect(action, &QAction::triggered, [mw]() {
            mw->openView("open-clips-view");
        });
        actions << action;
    }
    return actions;
    */
}

ClipsViewFactory::ClipsViewFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString ClipsViewFactory::id() const
{
    return QStringLiteral("open-clips-view");
}

QString ClipsViewFactory::name() const
{
    return tr("Clips");
}

QString ClipsViewFactory::title() const
{
    return tr("Clips");
}

MVAbstractView* ClipsViewFactory::createView(MVAbstractContext* context)
{
    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);
    MVClipsWidget* X = new MVClipsWidget(c);
    X->setLabelsToUse(c->selectedClusters());
    X->setTitle(title()+" ("+c->selectedClustersRangeText()+")");
    return X;
}
