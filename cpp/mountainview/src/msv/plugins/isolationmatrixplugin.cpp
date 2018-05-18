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

#include "isolationmatrixplugin.h"

#include "isolationmatrixview.h"

class IsolationMatrixPluginPrivate {
public:
    IsolationMatrixPlugin* q;
};

IsolationMatrixPlugin::IsolationMatrixPlugin()
{
    d = new IsolationMatrixPluginPrivate;
    d->q = this;
}

IsolationMatrixPlugin::~IsolationMatrixPlugin()
{
    delete d;
}

QString IsolationMatrixPlugin::name()
{
    return "IsolationMatrix";
}

QString IsolationMatrixPlugin::description()
{
    return "";
}

void IsolationMatrixPlugin::initialize(MVMainWindow* mw)
{
    mw->registerViewFactory(new IsolationMatrixFactory(mw));
}

IsolationMatrixFactory::IsolationMatrixFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString IsolationMatrixFactory::id() const
{
    return QStringLiteral("open-isolation-matrix");
}

QString IsolationMatrixFactory::name() const
{
    return tr("Isolation Matrix");
}

QString IsolationMatrixFactory::title() const
{
    return tr("Isolation Matrix");
}

MVAbstractView* IsolationMatrixFactory::createView(MVAbstractContext* context)
{
    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);
    IsolationMatrixView* X = new IsolationMatrixView(c);
    return X;
}
