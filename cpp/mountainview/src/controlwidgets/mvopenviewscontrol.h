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

#ifndef MVOPENVIEWSCONTROL_H
#define MVOPENVIEWSCONTROL_H

#include "mvabstractcontrol.h"

class MVOpenViewsControlPrivate;
class MVOpenViewsControl : public MVAbstractControl {
    Q_OBJECT
public:
    friend class MVOpenViewsControlPrivate;
    MVOpenViewsControl(MVAbstractContext* context, MVMainWindow* mw);
    virtual ~MVOpenViewsControl();

    QString title() const Q_DECL_OVERRIDE;
public slots:
    virtual void updateContext() Q_DECL_OVERRIDE;
    virtual void updateControls() Q_DECL_OVERRIDE;

private slots:
    void slot_open_view(QObject* obj);
    void slot_update_enabled();
    void slot_close_all_views();

private:
    MVOpenViewsControlPrivate* d;
};

#endif // MVOPENVIEWSCONTROL_H
