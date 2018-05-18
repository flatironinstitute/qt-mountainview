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
#include "mvstatusbar.h"
#include "taskprogress.h"

#include <QHBoxLayout>
#include <QLabel>
#include <objectregistry.h>
#include <icounter.h>

class MVStatusBarPrivate {
public:
    MVStatusBar* q;
    TaskManager::TaskProgressMonitor* m_tp_agent;
    QLabel m_bytes_downloaded_label;
    QLabel m_tasks_running_label;
    QLabel m_remote_processing_time_label;
    QLabel m_bytes_allocated_label;

    void update_font();
};

MVStatusBar::MVStatusBar()
{
    d = new MVStatusBarPrivate;
    d->q = this;

    QHBoxLayout* hlayout1 = new QHBoxLayout;
    hlayout1->setSpacing(10);
    hlayout1->setMargin(0);
    hlayout1->addWidget(&d->m_bytes_downloaded_label);
    hlayout1->addWidget(&d->m_tasks_running_label);
    hlayout1->addWidget(&d->m_remote_processing_time_label);
    hlayout1->addStretch();

    QHBoxLayout* hlayout2 = new QHBoxLayout;
    hlayout2->setSpacing(10);
    hlayout2->setMargin(0);
    hlayout2->addWidget(&d->m_bytes_allocated_label);
    hlayout2->addStretch();

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addLayout(hlayout1, 1);
    layout->addLayout(hlayout2, 1);
    layout->setAlignment(Qt::AlignLeft);
    setLayout(layout);

    d->m_tp_agent = TaskManager::TaskProgressMonitor::globalInstance();

    connect(d->m_tp_agent->model(), SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(slot_update_tasks()));
    connect(d->m_tp_agent->model(), SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(slot_update_tasks()));
    ICounterManager* manager = ObjectRegistry::getObject<ICounterManager>();
    if (manager) {
        SimpleAggregateCounter* meminuse = new SimpleAggregateCounter("bytes_in_use", SimpleAggregateCounter::Subtract, manager->counter("allocated_bytes"), manager->counter("freed_bytes"));
        meminuse->setParent(this);
        ObjectRegistry::addObject(meminuse);
    }

    if (manager) {
        QStringList counters = { "bytes_downloaded", "bytes_read", "bytes_in_use" };
        foreach (const QString& cntr, counters) {
            ICounterBase* counter = manager->counter(cntr);
            if (counter)
                connect(counter, SIGNAL(valueChanged()), this, SLOT(slot_update_quantities()));
            else
                qWarning() << cntr << "counter not present";
        }
    }
}

MVStatusBar::~MVStatusBar()
{
    delete d;
}

QString format_num_bytes(double num_bytes)
{
    if (num_bytes < 100)
        return QString("%1 bytes").arg(num_bytes);
    if (num_bytes < 1000 * 1e3)
        return QString("%1 KB").arg(num_bytes / 1e3, 0, 'f', 2);
    if (num_bytes < 1000 * 1e6)
        return QString("%1 MB").arg(num_bytes / 1e6, 0, 'f', 2);
    return QString("%1 GB").arg(num_bytes / 1e9, 0, 'f', 2);
}

QString format_duration(double msec)
{
    if (msec < 1e3)
        return QString("%1 msec").arg(msec);
    return QString("%1 sec").arg(msec / 1000, 0, 'f', 1);
}

void MVStatusBar::slot_update_quantities()
{
    ICounterManager* manager = ObjectRegistry::getObject<ICounterManager>();
    if (manager) {
        if (ICounterBase* counter = manager ? manager->counter("bytes_downloaded") : nullptr) {
            QString txt = QString("%1 downloaded |").arg(counter->label());
            d->m_bytes_downloaded_label.setText(txt);
        }
        if (ICounterBase* counter = manager ? manager->counter("remote_processing_time") : nullptr) {
            QString txt = QString("%1 remote processing").arg(format_duration(counter->value<int>()));
            d->m_remote_processing_time_label.setText(txt);
        }
    }
    {
        if (manager) {
            // TODO: Make the counters intelligent by using aggregate counters and labels for them.
            IIntCounter* bytesReadCounter = static_cast<IIntCounter*>(manager->counter("bytes_read"));
            ICounterBase* bytesInUseCounter = manager->counter("bytes_in_use");

            double using_bytes = bytesInUseCounter ? bytesInUseCounter->value<int64_t>() : 0;
            double bytes_read = bytesReadCounter ? bytesReadCounter->value() : 0;
            QString txt = QString("%1 RAM | %2 Read").arg(format_num_bytes(using_bytes)).arg(format_num_bytes(bytes_read));
            d->m_bytes_allocated_label.setText(txt);
            IIntCounter* allocatedCounter = static_cast<IIntCounter*>(manager->counter("allocated_bytes"));
            IIntCounter* freedCounter = static_cast<IIntCounter*>(manager->counter("freed_bytes"));
            if (allocatedCounter && freedCounter)
                d->m_bytes_allocated_label.setToolTip(QString("Allocated: <b>%1</b><br>Freed: <b>%2</b>").arg(format_num_bytes(allocatedCounter->value())).arg(format_num_bytes(freedCounter->value())));
        }
    }
}

void MVStatusBar::slot_update_tasks()
{
    d->update_font();
    {
        //        QString txt = QString("%1 tasks running |").arg(d->m_tp_agent->activeTasks().count());
        //        d->m_tasks_running_label.setText(txt);
    }
}

void MVStatusBarPrivate::update_font()
{
    QFont fnt = q->font();
    fnt.setPixelSize(qMax(9, q->height() / 2 - 4));
    q->setFont(fnt);
}
