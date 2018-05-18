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
#ifndef IsolationMatrixPLUGIN_H
#define IsolationMatrixPLUGIN_H

#include "mvmainwindow.h"

class IsolationMatrixPluginPrivate;
class IsolationMatrixPlugin : public MVAbstractPlugin {
public:
    friend class IsolationMatrixPluginPrivate;
    IsolationMatrixPlugin();
    virtual ~IsolationMatrixPlugin();

    QString name() Q_DECL_OVERRIDE;
    QString description() Q_DECL_OVERRIDE;
    void initialize(MVMainWindow* mw) Q_DECL_OVERRIDE;

private:
    IsolationMatrixPluginPrivate* d;
};

class IsolationMatrixFactory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    IsolationMatrixFactory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

#endif // IsolationMatrixPLUGIN_H
