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
#ifndef JSCOUNTER_H
#define JSCOUNTER_H

#include "icounter.h"
#include <QJSEngine>
#include <QJSValue>

class JSCounter : public IAggregateCounter {
public:
    JSCounter(const QString& name);
    void setEvaluationFunction(const QString& text)
    {
        m_evalFunction = m_engine.evaluate(text);
        m_evalExp.clear();
        updateValue();
    }
    void setEvaluationExpression(const QString& text)
    {
        m_evalExp = text;
        m_evalFunction = QJSValue();
        updateValue();
    }

    void setLabelFunction(const QString& text)
    {
        m_labelFunction = m_engine.evaluate(text);
        m_labelExp.clear();
        // updateValue();
    }
    void setLabelExpression(const QString& text)
    {
        m_labelExp = text;
        m_labelFunction = QJSValue();
        // updateValue();
    }

    Type type() const
    {
        return Variant;
    }
    QString label() const
    {
        if (m_labelExp.isEmpty() && m_labelFunction.isCallable()) {
            QJSValueList args;
            args << m_engine.toScriptValue(genericValue());
            QJSValue result = m_labelFunction.call(args);
            return result.toString();
        }
        else {
            QJSValue value = m_engine.toScriptValue(genericValue());
            m_engine.globalObject().setProperty("value", value);
            QJSValue result = m_engine.evaluate(m_labelExp);
            m_engine.globalObject().deleteProperty("value");
            return result.toString();
        }
    }
    QVariant genericValue() const
    {
        return m_value;
    }

    // IAggregateCounter interface
protected slots:
    void updateValue()
    {
        QJSValue countersObject = m_engine.newObject();
        QJSValue countersArray = m_engine.newArray(counters().size());
        int idx = 0;
        foreach (ICounterBase* ctr, counters()) {
            QString counterName = ctr->name();
            QVariant counterValue = ctr->genericValue();
            QJSValue scriptValue = m_engine.toScriptValue(counterValue);
            QJSValue counterObject = m_engine.newObject();
            counterObject.setProperty("name", counterName);
            counterObject.setProperty("value", scriptValue);
            countersArray.setProperty(idx++, counterObject);
            m_engine.globalObject().setProperty(counterName, scriptValue);
        }
        countersObject.setProperty("data", countersArray);
        m_engine.globalObject().setProperty("Counters", countersObject);
        if (m_evalExp.isEmpty() && m_evalFunction.isCallable()) {
            QJSValue result = m_evalFunction.call();
            m_value = result.toVariant();
        }
        else {
            QJSValue result = m_engine.evaluate(m_evalExp);
            m_value = result.toVariant();
        }
        // clear context
        foreach (ICounterBase* ctr, counters()) {
            QString counterName = ctr->name();
            m_engine.globalObject().deleteProperty(counterName);
        }
        m_engine.globalObject().deleteProperty("Counters");
    }

private:
    mutable QJSEngine m_engine;
    QJSValue m_evalFunction;
    mutable QJSValue m_labelFunction;
    QString m_evalExp;
    QString m_labelExp;
    QVariant m_value;
};

#endif // JSCOUNTER_H
