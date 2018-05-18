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
#ifndef PRVMANAGERDIALOG_H
#define PRVMANAGERDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include <QMap>
#include <QMutex>
#include <QThread>

namespace Ui {
class PrvManagerDialog;
}

class PrvManagerDialogPrivate;
class PrvManagerDialog : public QDialog {
    Q_OBJECT

    friend class PrvManagerDialogPrivate;

public:
    explicit PrvManagerDialog(QWidget* parent = 0);
    ~PrvManagerDialog();

    void setPrvObjects(const QMap<QString, QJsonObject>& prv_objects);
    void setServerNames(const QStringList& server_names);
    void refresh();

private slots:
    void slot_results_updated();
    void slot_upload_to_server();
    void slot_copy_to_local_disk();
    void slot_restart_thread();

private:
    Ui::PrvManagerDialog* ui;

    PrvManagerDialogPrivate* d;
};

enum fuzzybool {
    YES,
    NO,
    UNKNOWN
};

struct PrvManagerDialogResult {
    fuzzybool on_local_disk = UNKNOWN;
    QMap<QString, fuzzybool> on_server;
};

class PrvManagerDialogThread : public QThread {
    Q_OBJECT
public:
    //input
    QMap<QString, QJsonObject> prv_objects;
    QStringList server_names;

    //output
    QMutex results_mutex;
    QMap<QString, PrvManagerDialogResult> results;

    void run();

private:
    bool check_if_on_local_disk(QJsonObject prv_obj);
    bool check_if_on_server(QJsonObject prv_obj, QString server_name);

signals:
    void results_updated();
};

#endif // PRVMANAGERDIALOG_H
