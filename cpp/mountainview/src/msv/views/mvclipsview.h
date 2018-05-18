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

#ifndef MVCLIPSVIEW_H
#define MVCLIPSVIEW_H

#include "mvutils.h"

#include <QWidget>
#include <diskreadmda32.h>
#include <mvabstractcontext.h>

/** \class MVClipsView
 *  \brief View a set of clips. Usually each clip contains a single spike.
 */

class MVClipsViewPrivate;
class MVClipsView : public QWidget {
    Q_OBJECT
public:
    friend class MVClipsViewPrivate;
    MVClipsView(MVAbstractContext* context);
    virtual ~MVClipsView();

    void setClips(const DiskReadMda32& clips);
    /// TODO: (MEDIUM) in mvclipsview implement times/labels for purpose of current event and labeling
    //void setTimes(const QVector<double>& times);
    //void setLabels(const QVector<int>& labels);
protected:
    void paintEvent(QPaintEvent* evt);
signals:
private slots:

private:
    MVClipsViewPrivate* d;
};

#endif // MVCLIPSVIEW_H
