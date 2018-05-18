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

#include "mvcontrolpanel2.h"
//#include "qAccordion/qaccordion.h"
#include "toolbox.h"
#include <QHBoxLayout>

#include <QPushButton>
//#include "mlcommon.h"
#include "mvmainwindow.h"

class MVControlPanel2Private {
public:
    MVControlPanel2* q;
    ToolBox* m_accordion;
    QList<MVAbstractControl*> m_controls;
    MVAbstractContext* m_context;
    MVMainWindow* m_main_window;
};

MVControlPanel2::MVControlPanel2(MVAbstractContext* context, MVMainWindow* mw)
{
    d = new MVControlPanel2Private;
    d->q = this;

    d->m_context = context;
    d->m_main_window = mw;

    QHBoxLayout* top_layout = new QHBoxLayout;
    top_layout->setSpacing(20);
    top_layout->setMargin(20);
    {
        QPushButton* B = new QPushButton("Recalc Visible");
        top_layout->addWidget(B);
        QObject::connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_recalc_visible()));
    }
    {
        QPushButton* B = new QPushButton("Recalc All");
        top_layout->addWidget(B);
        QObject::connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_recalc_all()));
    }

    d->m_accordion = new ToolBox;
//    d->m_accordion->setMultiActive(true);

//    QScrollArea* SA = new QScrollArea;
//    SA->setWidget(d->m_accordion);
//    SA->setWidgetResizable(true);

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->setSpacing(0);
    vlayout->setMargin(0);
    vlayout->addLayout(top_layout);
    vlayout->addWidget(d->m_accordion);
    this->setLayout(vlayout);
}

MVControlPanel2::~MVControlPanel2()
{
    delete d;
}

void MVControlPanel2::insertControl(int position, MVAbstractControl* mvcontrol, bool start_open)
{
    if (position < 0)
        d->m_controls << mvcontrol;
    else
        d->m_controls.insert(position, mvcontrol);

//    QFrame* frame = new QFrame;
//    QHBoxLayout* frame_layout = new QHBoxLayout;
//    frame_layout->addWidget(mvcontrol);
//    frame->setLayout(frame_layout);
//    ContentPane* CP = new ContentPane(mvcontrol->title(), frame);
//    CP->setMaximumHeight(1000);
    if (position < 0) {
        d->m_accordion->addWidget(mvcontrol, mvcontrol->title());
    } else
        d->m_accordion->insertWidget(position, mvcontrol, mvcontrol->title());
    if (start_open) {
        d->m_accordion->open(d->m_accordion->indexOf(mvcontrol));
        //CP->openContentPane();
    }
    mvcontrol->updateControls();
    mvcontrol->updateContext(); //do this so the default values get passed on to the context
}

void MVControlPanel2::addControl(MVAbstractControl* mvcontrol, bool start_open)
{
    insertControl(-1, mvcontrol, start_open);
}

void MVControlPanel2::slot_recalc_visible()
{
    d->m_main_window->recalculateViews(MVMainWindow::SuggestedVisible);
}

void MVControlPanel2::slot_recalc_all()
{
    d->m_main_window->recalculateViews(MVMainWindow::Suggested);
}
