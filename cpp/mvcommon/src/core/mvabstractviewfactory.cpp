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
#include "mvabstractviewfactory.h"

MVAbstractViewFactory::MVAbstractViewFactory(MVMainWindow* mw, QObject* parent)
    : QObject(parent)
{
    m_main_window = mw;
}

QString MVAbstractViewFactory::group() const { return QString(); }

QString MVAbstractViewFactory::toolTip() const { return QString(); }

QString MVAbstractViewFactory::title() const { return name(); }

MVAbstractViewFactory::PreferredOpenLocation MVAbstractViewFactory::preferredOpenLocation() const
{
    return PreferredOpenLocation::NoPreference;
}

bool MVAbstractViewFactory::isEnabled(MVAbstractContext* context) const
{
    Q_UNUSED(context)
    return true;
}

MVMainWindow* MVAbstractViewFactory::mainWindow()
{
    return m_main_window;
}

QList<QAction*> MVAbstractViewFactory::actions(const QMimeData& md)
{
    Q_UNUSED(md)
    return QList<QAction*>();
}
