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

#include "closemehandler.h"
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QDir>
#include <QApplication>
#include <QFileInfo>
#include "mlcommon.h"

class CloseMeHandlerPrivate {
public:
    CloseMeHandler* q;
    QDateTime m_start_time;
    void do_start();
};

CloseMeHandler::CloseMeHandler()
{
    d = new CloseMeHandlerPrivate;
    d->q = this;
}

CloseMeHandler::~CloseMeHandler()
{
    delete d;
}

void CloseMeHandler::start()
{
    CloseMeHandler* X = new CloseMeHandler();
    X->d->do_start();
}

void CloseMeHandler::slot_timer()
{
    QString fname = MLUtil::tempPath() + "/closeme.tmp";
    if (QFile::exists(fname)) {
        QDateTime dt = QFileInfo(fname).created();
        if (dt > d->m_start_time) {
            exit(0);
        }
    }
    QTimer::singleShot(1000, this, SLOT(slot_timer()));
}

void CloseMeHandlerPrivate::do_start()
{
    m_start_time = QDateTime::currentDateTime();
    QTimer::singleShot(2000, q, SLOT(slot_timer()));
}
