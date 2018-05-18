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

#ifndef COMPUTATIONTHREAD
#define COMPUTATIONTHREAD

#include <QThread>
#include <QString>
#include <QMutex>

class ComputationThreadPrivate;
class ComputationThread : public QThread {
    Q_OBJECT
public:
    friend class ComputationThreadPrivate;
    ComputationThread();
    virtual ~ComputationThread();

    virtual void compute() = 0;

    void setDeleteOnComplete(bool val);
    void startComputation(); //will stop existing computation
    bool stopComputation(int timeout = 0); //will wait for stop before returning, returns true if successfully stopped
    bool isComputing();
    bool isFinished();
    bool hasError();
    QString errorMessage();

signals:
    void computationFinished();

protected:
    void setErrorMessage(const QString& error);

private:
    void run();
private slots:
    void slot_start();

private:
    ComputationThreadPrivate* d;
};

#endif // COMPUTATIONTHREAD
