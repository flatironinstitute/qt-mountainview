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

#ifndef TABBERFRAME_H
#define TABBERFRAME_H

#include "mvabstractview.h"

class TabberFramePrivate;
class TabberFrame : public QWidget {
    Q_OBJECT
public:
    friend class TabberFramePrivate;
    TabberFrame(MVAbstractView* view);
    virtual ~TabberFrame();
    MVAbstractView* view();
    void setContainerName(QString name);
signals:
    void signalMoveToOtherContainer();
    void signalPopOut();
    void signalPopIn();
private slots:
    void slot_update_action_visibility();
    void slot_update_calculating();

private:
    TabberFramePrivate* d;
};

class TabberFrameOverlay : public QWidget {
public:
    QString calculating_message;

protected:
    void paintEvent(QPaintEvent* evt);
};

#endif // TABBERFRAME_H
