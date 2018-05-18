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
#include "mvabstractcontext.h"

#include <QAction>
#include <QDebug>

struct OptionChangedAction {
    QString option_name;
    QAction* action;
};

class MVAbstractContextPrivate {
public:
    MVAbstractContext* q;
    QString m_mv2_fname;
    QMap<QString, QVariant> m_options;
    QList<OptionChangedAction> m_option_changed_actions;
};

MVAbstractContext::MVAbstractContext()
{
    d = new MVAbstractContextPrivate;
    d->q = this;

    QObject::connect(this, SIGNAL(optionChanged(QString)), this, SLOT(slot_option_changed(QString)));
}

MVAbstractContext::~MVAbstractContext()
{
    delete d;
}

void MVAbstractContext::setMV2FileName(QString fname)
{
    d->m_mv2_fname = fname;
}

QString MVAbstractContext::mv2FileName()
{
    return d->m_mv2_fname;
}

void MVAbstractContext::slot_option_changed(QString name)
{
    for (int i = 0; i < d->m_option_changed_actions.count(); i++) {
        if (d->m_option_changed_actions[i].option_name == name) {
            d->m_option_changed_actions[i].action->trigger();
        }
    }
}

QVariant MVAbstractContext::option(QString name, QVariant default_val) const
{
    return d->m_options.value(name, default_val);
}

void MVAbstractContext::setOption(QString name, QVariant value)
{
    if (d->m_options[name] == value)
        return;
    d->m_options[name] = value;
    emit optionChanged(name);
}

void MVAbstractContext::onOptionChanged(QString name, const QObject* receiver, const char* member, Qt::ConnectionType type)
{
    QAction* action = new QAction(this);
    connect(action, SIGNAL(triggered(bool)), receiver, member, type);
    OptionChangedAction X;
    X.action = action;
    X.option_name = name;
    d->m_option_changed_actions << X;
}

QVariantMap MVAbstractContext::options() const
{
    return d->m_options;
}

void MVAbstractContext::setOptions(const QVariantMap& options)
{
    d->m_options = options;
}

void MVAbstractContext::clearOptions()
{
    d->m_options.clear();
}

bool MVRange::operator==(const MVRange& other) const
{
    return ((other.min == min) && (other.max == max));
}

MVRange MVRange::operator+(double offset)
{
    return MVRange(min + offset, max + offset);
}

MVRange MVRange::operator*(double scale)
{
    double center = (min + max) / 2;
    double span = (max - min);
    return MVRange(center - span / 2 * scale, center + span / 2 * scale);
}
