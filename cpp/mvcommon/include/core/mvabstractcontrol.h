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

#ifndef MVABSTRACTCONTROL_H
#define MVABSTRACTCONTROL_H

#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QToolButton>

#include "mvabstractcontext.h"

class MVMainWindow;
class MVAbstractControlPrivate;
class MVAbstractControl : public QWidget {
public:
    friend class MVAbstractControlPrivate;
    MVAbstractControl(MVAbstractContext* context, MVMainWindow* mw);
    virtual ~MVAbstractControl();

    virtual QString title() const = 0;
public slots:
    virtual void updateContext() = 0;
    virtual void updateControls() = 0;

protected:
    MVAbstractContext* mvContext();
    MVMainWindow* mainWindow();

    QVariant controlValue(QString name) const;
    void setControlValue(QString name, QVariant val);
    void setChoices(QString name, const QStringList& choices);
    void setControlEnabled(QString name, bool val);

    QWidget* createStringControl(QString name);
    QWidget* createIntControl(QString name);
    QWidget* createDoubleControl(QString name);
    QComboBox* createChoicesControl(QString name);
    QCheckBox* createCheckBoxControl(QString name, QString label);
    QRadioButton* createRadioButtonControl(QString name, QString label);
    QToolButton* createToolButtonControl(QString name, QString label, QObject* receiver = 0, const char* signal_or_slot = 0);

    void updateControlsOn(QObject* sender, const char* signal);

private slots:
    void slot_controls_changed();

private:
    MVAbstractControlPrivate* d;
};

#endif // MVABSTRACTCONTROL_H
