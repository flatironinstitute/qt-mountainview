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

#ifndef GUIDEV2
#define GUIDEV2

#include <QWizard>
#include <mvcontext.h>
#include <mvmainwindow.h>

class GuideV2Private;
class GuideV2 : public QWizard {
    Q_OBJECT
public:
    friend class GuideV2Private;
    GuideV2(MVContext* mvcontext, MVMainWindow* mw);
    virtual ~GuideV2();

private slots:
    void slot_button_clicked();
    void slot_select_merge_candidates();
    void slot_merge_all_merge_candidates();

private:
    GuideV2Private* d;
};

#endif // GuideV2
