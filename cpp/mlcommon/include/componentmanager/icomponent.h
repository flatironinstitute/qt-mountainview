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
#ifndef ICOMPONENT_H
#define ICOMPONENT_H

#include <QObject>

class IComponent : public QObject {
    Q_OBJECT
public:
    IComponent(QObject* parent = 0);
    virtual QString name() const = 0;
    virtual bool initialize() = 0;
    virtual void extensionsReady();
    virtual void uninitialize() {}
    virtual QStringList dependencies() const;

protected:
    void addObject(QObject*);
    void addAutoReleasedObject(QObject*);
    void removeObject(QObject*);
};

#endif // ICOMPONENT_H
