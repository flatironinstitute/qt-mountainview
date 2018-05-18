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

#ifndef MVCLUSTERORDERCONTROL_H
#define MVCLUSTERORDERCONTROL_H

#include "mvabstractcontrol.h"

class MVClusterOrderControlPrivate;
class MVClusterOrderControl : public MVAbstractControl {
    Q_OBJECT
public:
    friend class MVClusterOrderControlPrivate;
    MVClusterOrderControl(MVAbstractContext* context, MVMainWindow* mw);
    virtual ~MVClusterOrderControl();

    QString title() const Q_DECL_OVERRIDE;
public slots:
    virtual void updateContext() Q_DECL_OVERRIDE;
    virtual void updateControls() Q_DECL_OVERRIDE;

private slots:
    void slot_order_by_detectability();
    void slot_computation_finished();

private:
    MVClusterOrderControlPrivate* d;
};

#endif // MVCLUSTERORDERCONTROL_H
