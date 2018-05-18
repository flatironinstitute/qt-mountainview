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

#ifndef GUIDEV1
#define GUIDEV1

#include <QWizard>
#include <mvcontext.h>
#include <mvmainwindow.h>

class GuideV1Private;
class GuideV1 : public QWizard {
    Q_OBJECT
public:
    friend class GuideV1Private;
    GuideV1(MVContext* mvcontext, MVMainWindow* mw);
    virtual ~GuideV1();

private slots:
    void slot_button_clicked();
    void slot_next_channel(int offset = 1);
    void slot_previous_channel();
    void slot_cluster_channel_matrix_computed();

private:
    GuideV1Private* d;
};

#endif // GUIDEV1
