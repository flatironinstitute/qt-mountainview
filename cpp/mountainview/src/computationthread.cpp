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

#include "computationthread.h"

#include <QMutex>
#include <QTimer>
#include <QDebug>
#include <QTime>
#include <QCoreApplication>
#include "mlcommon.h"

class ComputationThreadPrivate {
public:
    ComputationThread* q;
    bool m_delete_on_complete;
    bool m_is_computing;
    bool m_is_finished;
    QString m_error_message;
    QMutex m_mutex;
    QMutex m_status_mutex;
    bool m_start_scheduled;
    int m_randomization_seed;

    void schedule_start();
};

ComputationThread::ComputationThread()
{
    d = new ComputationThreadPrivate;
    d->q = this;
    d->m_is_computing = false;
    d->m_is_finished = false;
    d->m_start_scheduled = false;
    d->m_delete_on_complete = false;
}

ComputationThread::~ComputationThread()
{
    this->stopComputation();
    delete d;
}

void ComputationThread::setDeleteOnComplete(bool val)
{
    d->m_delete_on_complete = val;
}

void ComputationThread::startComputation()
{
    stopComputation();
    {
        QMutexLocker locker(&d->m_mutex);
        d->m_is_computing = true;
        d->m_is_finished = false;
        d->m_error_message = "";
    }
    d->schedule_start();
}

bool ComputationThread::stopComputation(int timeout)
{
    {
        QMutexLocker locker(&d->m_mutex);
        if (!d->m_is_computing)
            return true;
        this->requestInterruption();
    }
    QTime timer;
    timer.start();
    while (((timer.elapsed() < timeout) || (timeout == 0)) && (this->isRunning())) {
        qApp->processEvents();
    }
    return (!this->isRunning());
}

bool ComputationThread::isComputing()
{
    QMutexLocker locker(&d->m_mutex);
    return d->m_is_computing;
}

bool ComputationThread::isFinished()
{
    QMutexLocker locker(&d->m_mutex);
    return d->m_is_finished;
}

bool ComputationThread::hasError()
{
    QMutexLocker locker(&d->m_mutex);
    return !d->m_error_message.isEmpty();
}

QString ComputationThread::errorMessage()
{
    QMutexLocker locker(&d->m_mutex);
    return d->m_error_message;
}

void ComputationThread::setErrorMessage(const QString& error)
{
    QMutexLocker locker(&d->m_mutex);
    d->m_error_message = error;
}

void ComputationThread::run()
{
    {
        qsrand(d->m_randomization_seed);
        compute();
    }
    if (!MLUtil::threadInterruptRequested()) {
        QMutexLocker locker(&d->m_mutex);
        d->m_is_finished = true;
        d->m_is_computing = false;
        emit computationFinished();
    }
    else {
    }
    if (d->m_delete_on_complete)
        this->deleteLater();
}

void ComputationThread::slot_start()
{
    d->m_randomization_seed = qrand();
    {
        QMutexLocker locker(&d->m_mutex);
        if (MLUtil::threadInterruptRequested()) {
            return;
        }
        d->m_start_scheduled = false;
    }
    this->start();
}

void ComputationThreadPrivate::schedule_start()
{
    QMutexLocker locker(&m_mutex);
    if (m_start_scheduled)
        return;
    QTimer::singleShot(100, q, SLOT(slot_start()));
    m_start_scheduled = true;
}
