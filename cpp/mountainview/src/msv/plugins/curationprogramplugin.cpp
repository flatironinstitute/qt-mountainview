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

#include "curationprogramplugin.h"

#include "curationprogramview.h"

#include <QThread>
#include <mountainprocessrunner.h>

class CurationProgramPluginPrivate {
public:
    CurationProgramPlugin* q;
};

CurationProgramPlugin::CurationProgramPlugin()
{
    d = new CurationProgramPluginPrivate;
    d->q = this;
}

CurationProgramPlugin::~CurationProgramPlugin()
{
    delete d;
}

QString CurationProgramPlugin::name()
{
    return "CurationProgram";
}

QString CurationProgramPlugin::description()
{
    return "";
}

void compute_basic_metrics(MVAbstractContext* mv_context);

void CurationProgramPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new CurationProgramFactory(mw));
    MVContext* c = qobject_cast<MVContext*>(mw->mvContext());
    Q_ASSERT(c);
    //if (c->firings().N2() > 1)
    //    compute_basic_metrics(mw->mvContext());
}

CurationProgramFactory::CurationProgramFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString CurationProgramFactory::id() const
{
    return QStringLiteral("open-curation-program");
}

QString CurationProgramFactory::name() const
{
    return tr("Curation Program");
}

QString CurationProgramFactory::title() const
{
    return tr("Curation Program");
}

MVAbstractView* CurationProgramFactory::createView(MVAbstractContext* context)
{
    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);
    CurationProgramView* X = new CurationProgramView(c);
    return X;
}
