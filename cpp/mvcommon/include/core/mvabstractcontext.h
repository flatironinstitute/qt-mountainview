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
#ifndef MVABSTRACTCONTEXT_H
#define MVABSTRACTCONTEXT_H

#include <QObject>
#include <QVariant>
#include <QJsonObject>

struct MVRange {
    MVRange(double min0 = 0, double max0 = 1)
    {
        min = min0;
        max = max0;
    }
    bool operator==(const MVRange& other) const;
    MVRange operator+(double offset);
    MVRange operator*(double scale);
    double range() const { return max - min; }
    double min, max;
};

class MVAbstractContextPrivate;
class MVAbstractContext : public QObject {
    Q_OBJECT
public:
    friend class MVAbstractContextPrivate;
    MVAbstractContext();
    virtual ~MVAbstractContext();

    virtual void setFromMV2FileObject(QJsonObject obj) = 0;
    virtual QJsonObject toMV2FileObject() const = 0;

    void setMV2FileName(QString fname);
    QString mv2FileName();

    /////////////////////////////////////////////////
    QVariant option(QString name, QVariant default_val = QVariant()) const;
    void setOption(QString name, QVariant value);
    void onOptionChanged(QString name, const QObject* receiver, const char* member, Qt::ConnectionType type = Qt::DirectConnection);
    QVariantMap options() const;
    void setOptions(const QVariantMap& options);
    void clearOptions();

private slots:
    void slot_option_changed(QString name);

signals:
    void optionChanged(QString name);

private:
    MVAbstractContextPrivate* d;
};

#endif // MVABSTRACTCONTEXT_H
