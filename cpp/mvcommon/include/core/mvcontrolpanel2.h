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

#ifndef MVCONTROLPANEL2_H
#define MVCONTROLPANEL2_H

#include "mvabstractcontrol.h"
#include <QWidget>

class MVControlPanel2Private;
class MVControlPanel2 : public QWidget {
    Q_OBJECT
public:
    friend class MVControlPanel2Private;
    MVControlPanel2(MVAbstractContext* context, MVMainWindow* mw);
    virtual ~MVControlPanel2();
    void insertControl(int position, MVAbstractControl* mvcontrol, bool start_open);
    void addControl(MVAbstractControl* mvcontrol, bool start_open);

private slots:
    void slot_recalc_visible();
    void slot_recalc_all();

private:
    MVControlPanel2Private* d;
};

#endif // MVCONTROLPANEL2_H
