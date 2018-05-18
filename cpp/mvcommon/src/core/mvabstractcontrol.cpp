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

#include "mvabstractcontrol.h"

#include <QComboBox>
#include <QLineEdit>
#include <QSet>
#include <QToolButton>

class MVAbstractControlPrivate {
public:
    MVAbstractControl* q;
    MVAbstractContext* m_context;
    MVMainWindow* m_main_window;

    QMap<QString, QLineEdit*> m_string_controls;
    QMap<QString, QLineEdit*> m_int_controls;
    QMap<QString, QLineEdit*> m_double_controls;
    QMap<QString, QComboBox*> m_choices_controls;
    QMap<QString, QCheckBox*> m_checkbox_controls;
    QMap<QString, QRadioButton*> m_radio_button_controls;
    QMap<QString, QToolButton*> m_toolbutton_controls;

    QMap<QString, QWidget*> all_control_widgets();
};

MVAbstractControl::MVAbstractControl(MVAbstractContext* context, MVMainWindow* mw)
{
    d = new MVAbstractControlPrivate;
    d->q = this;
    d->m_context = context;
    d->m_main_window = mw;
}

MVAbstractControl::~MVAbstractControl()
{
    delete d;
}

MVAbstractContext* MVAbstractControl::mvContext()
{
    return d->m_context;
}

MVMainWindow* MVAbstractControl::mainWindow()
{
    return d->m_main_window;
}

QVariant MVAbstractControl::controlValue(QString name) const
{
    if (d->m_string_controls.contains(name)) {
        return d->m_string_controls[name]->text();
    }
    if (d->m_int_controls.contains(name)) {
        return d->m_int_controls[name]->text().toInt();
    }
    if (d->m_double_controls.contains(name)) {
        return d->m_double_controls[name]->text().toDouble();
    }
    if (d->m_choices_controls.contains(name)) {
        return d->m_choices_controls[name]->currentText();
    }
    if (d->m_checkbox_controls.contains(name)) {
        return d->m_checkbox_controls[name]->isChecked();
    }
    if (d->m_radio_button_controls.contains(name)) {
        return d->m_radio_button_controls[name]->isChecked();
    }
    return QVariant();
}

void MVAbstractControl::setControlValue(QString name, QVariant val)
{
    if (controlValue(name) == val)
        return;
    if (d->m_string_controls.contains(name)) {
        QString txt = QString("%1").arg(val.toString());
        d->m_string_controls[name]->setText(txt);
    }
    if (d->m_int_controls.contains(name)) {
        QString txt = QString("%1").arg(val.toInt());
        d->m_int_controls[name]->setText(txt);
    }
    if (d->m_double_controls.contains(name)) {
        QString txt = QString("%1").arg(val.toDouble());
        d->m_double_controls[name]->setText(txt);
    }
    if (d->m_choices_controls.contains(name)) {
        QString txt = QString("%1").arg(val.toString());
        d->m_choices_controls[name]->setCurrentText(txt);
    }
    if (d->m_checkbox_controls.contains(name)) {
        d->m_checkbox_controls[name]->setChecked(val.toBool());
    }
    if (d->m_radio_button_controls.contains(name)) {
        d->m_radio_button_controls[name]->setChecked(val.toBool());
    }
}

QWidget* MVAbstractControl::createIntControl(QString name)
{
    QLineEdit* X = new QLineEdit;
    X->setText("0");
    d->m_int_controls[name] = X;
    QObject::connect(X, SIGNAL(textEdited(QString)), this, SLOT(updateContext()));
    return X;
}

QWidget* MVAbstractControl::createStringControl(QString name)
{
    QLineEdit* X = new QLineEdit;
    X->setText("");
    d->m_string_controls[name] = X;
    QObject::connect(X, SIGNAL(textEdited(QString)), this, SLOT(updateContext()));
    return X;
}

QWidget* MVAbstractControl::createDoubleControl(QString name)
{
    QLineEdit* X = new QLineEdit;
    X->setText("0");
    d->m_double_controls[name] = X;
    QObject::connect(X, SIGNAL(textEdited(QString)), this, SLOT(updateContext()));
    return X;
}

QComboBox* MVAbstractControl::createChoicesControl(QString name)
{
    QComboBox* X = new QComboBox;
    d->m_choices_controls[name] = X;
    QObject::connect(X, SIGNAL(currentTextChanged(QString)), this, SLOT(updateContext()), Qt::QueuedConnection);
    return X;
}

QCheckBox* MVAbstractControl::createCheckBoxControl(QString name, QString label)
{
    QCheckBox* X = new QCheckBox;
    d->m_checkbox_controls[name] = X;
    X->setText(label);
    QObject::connect(X, SIGNAL(clicked(bool)), this, SLOT(updateContext()));
    return X;
}

QRadioButton* MVAbstractControl::createRadioButtonControl(QString name, QString label)
{
    QRadioButton* X = new QRadioButton;
    d->m_radio_button_controls[name] = X;
    X->setText(label);
    QObject::connect(X, SIGNAL(clicked(bool)), this, SLOT(updateContext()));
    return X;
}

QToolButton* MVAbstractControl::createToolButtonControl(QString name, QString label, QObject* receiver, const char* signal_or_slot)
{
    QToolButton* X = new QToolButton;
    X->setText(label);
    d->m_toolbutton_controls[name] = X;
    if ((receiver) && (signal_or_slot)) {
        QObject::connect(X, SIGNAL(clicked(bool)), receiver, signal_or_slot);
    }
    return X;
}

void MVAbstractControl::updateControlsOn(QObject* sender, const char* signal)
{
    QObject::connect(sender, signal, this, SLOT(updateControls()));
}

void MVAbstractControl::setChoices(QString name, const QStringList& choices)
{
    if (!d->m_choices_controls.contains(name))
        return;
    QComboBox* X = d->m_choices_controls[name];
    QStringList strings;
    for (int i = 0; i < X->count(); i++) {
        strings << X->itemText(i);
    }
    if (strings.toSet() == choices.toSet())
        return;
    QString val = X->currentText();
    X->clear();
    for (int i = 0; i < choices.count(); i++) {
        X->addItem(choices[i]);
    }
    if ((!val.isEmpty()) && (choices.contains(val))) {
        X->setCurrentText(val);
    }
}

void MVAbstractControl::setControlEnabled(QString name, bool val)
{
    QMap<QString, QWidget*> all = d->all_control_widgets();
    if (all.contains(name)) {
        all[name]->setEnabled(val);
    }
}

void add_to(QMap<QString, QWidget*>& A, QMap<QString, QLineEdit*>& B)
{
    QStringList keys = B.keys();
    foreach (QString key, keys) {
        A[key] = B[key];
    }
}

void add_to(QMap<QString, QWidget*>& A, QMap<QString, QComboBox*>& B)
{
    QStringList keys = B.keys();
    foreach (QString key, keys) {
        A[key] = B[key];
    }
}

void add_to(QMap<QString, QWidget*>& A, QMap<QString, QCheckBox*>& B)
{
    QStringList keys = B.keys();
    foreach (QString key, keys) {
        A[key] = B[key];
    }
}

void add_to(QMap<QString, QWidget*>& A, QMap<QString, QToolButton*>& B)
{
    QStringList keys = B.keys();
    foreach (QString key, keys) {
        A[key] = B[key];
    }
}

void add_to(QMap<QString, QWidget*>& A, QMap<QString, QRadioButton*>& B)
{
    QStringList keys = B.keys();
    foreach (QString key, keys) {
        A[key] = B[key];
    }
}

QMap<QString, QWidget*> MVAbstractControlPrivate::all_control_widgets()
{
    QMap<QString, QWidget*> ret;
    add_to(ret, m_string_controls);
    add_to(ret, m_int_controls);
    add_to(ret, m_double_controls);
    add_to(ret, m_choices_controls);
    add_to(ret, m_checkbox_controls);
    add_to(ret, m_toolbutton_controls);
    add_to(ret, m_radio_button_controls);
    return ret;
}
