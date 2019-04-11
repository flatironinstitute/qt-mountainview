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

#include "mountainprocessrunner.h"
#include <QCoreApplication>
#include <QMap>
#include <QProcess>
#include <QVariant>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "mlcommon.h"
#include "objectregistry.h"
#include "cachemanager.h"
#include "mlcommon.h"
#include "taskprogress.h"
#include <QThread>
//#include <objectregistry.h>
#include <icounter.h>
#include "qprocessmanager.h"

class MountainProcessRunnerPrivate {
public:
    MountainProcessRunner* q;
    QString m_processor_name;
    QMap<QString, QVariant> m_parameters;
    //QString m_mscmdserver_url;
    QString m_mlproxy_url;
    bool m_allow_gui_thread = false;
    bool m_detach = false;

    QString create_temporary_output_file_name(const QString& remote_url, const QString& processor_name, const QMap<QString, QVariant>& params, const QString& parameter_name);
};

MountainProcessRunner::MountainProcessRunner()
{
    d = new MountainProcessRunnerPrivate;
    d->q = this;
}

MountainProcessRunner::~MountainProcessRunner()
{
    delete d;
}

void MountainProcessRunner::setProcessorName(const QString& pname)
{
    d->m_processor_name = pname;
}

QString MountainProcessRunner::makeOutputFilePath(const QString& pname)
{
    QString ret = d->create_temporary_output_file_name(d->m_mlproxy_url, d->m_processor_name, d->m_parameters, pname);
    d->m_parameters[pname] = ret;
    return ret;
}

void MountainProcessRunner::setDetach(bool val)
{
    d->m_detach = val;
}

void MountainProcessRunner::setInputParameters(const QMap<QString, QVariant>& parameters)
{
    d->m_parameters = parameters;
}

void MountainProcessRunner::setMLProxyUrl(const QString& url)
{
    d->m_mlproxy_url = url;
}

void MountainProcessRunner::setAllowGuiThread(bool val)
{
    d->m_allow_gui_thread = val;
}

QJsonObject variantmap_to_json_obj(QVariantMap map)
{
    QJsonObject ret;
    QStringList keys = map.keys();
    // Use fromVariantMap
    foreach (QString key, keys) {
        ret[key] = QJsonValue::fromVariant(map[key]);
    }
    return ret;
}

QVariantMap json_obj_to_variantmap(QJsonObject obj)
{
    QVariantMap ret;
    QStringList keys = obj.keys();
    foreach (QString key, keys) {
        ret[key] = obj[key].toVariant();
    }
    return ret;
}

QJsonObject http_post(QString url, QJsonObject req)
{
    if (MLUtil::inGuiThread()) {
        qCritical() << "Cannot do an http_post within a gui thread: " + url;
        qCritical() << "Exiting.";
        exit(-1);
    }
    QTime timer;
    timer.start();
    QNetworkAccessManager manager;
    QNetworkRequest request;
    request.setUrl(url);
    QByteArray json_data = QJsonDocument(req).toJson();
    QNetworkReply* reply = manager.post(request, json_data);
    QEventLoop loop;
    QString ret;
    QObject::connect(reply, &QNetworkReply::readyRead, [&]() {
        ret+=reply->readAll();
    });
    /*
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    */
    while (!reply->isFinished()) {
        if (MLUtil::threadInterruptRequested()) {
            qWarning() << "Halting in http_post: " + url;
            reply->abort();
            loop.quit();
        }
        loop.processEvents();
    }

    if (MLUtil::threadInterruptRequested()) {
        QJsonObject obj;
        obj["success"] = false;
        obj["error"] = "Halting in http_post: " + url;
        return obj;
    }
    else {
        printf("RECEIVED TEXT (%d ms, %d bytes) from POST %s\n", timer.elapsed(), ret.count(), url.toLatin1().data());
        QString str = ret.mid(0, 5000) + "...";
        str.replace("\\n", "\n");
        printf("%s\n", (str.toLatin1().data()));
        QJsonObject obj = QJsonDocument::fromJson(ret.toLatin1()).object();
        ICounterManager* manager = ObjectRegistry::getObject<ICounterManager>();
        if (manager) {
            IIntCounter* bytesDownloadedCounter = static_cast<IIntCounter*>(manager->counter("bytes_downloaded"));
            bytesDownloadedCounter->add(ret.count());
        }
        //        TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_downloaded", ret.count());
        return obj;
    }
}

void MountainProcessRunner::runProcess()
{

    if (!d->m_allow_gui_thread) {
        if (MLUtil::inGuiThread()) {
            qCritical() << "Cannot run mountain process in gui thread: " + d->m_processor_name;
            exit(-1);
        }
    }

    TaskProgress task(TaskProgress::Calculate, "MS: " + d->m_processor_name);

    //if (d->m_mscmdserver_url.isEmpty()) {
    if (d->m_mlproxy_url.isEmpty()) {
        //QString mountainsort_exe = mountainlabBasePath() + "/mountainsort/bin/mountainsort";
        //QString mountainprocess_exe = MLUtil::mountainlabBasePath() + "/cpp/mountainprocess/bin/mountainprocess";
        //QString mountainprocess_exe = "mountainprocess"; // jfm changed on 9/7/17
        QStringList args;
        //args << "run-process";
        args << d->m_processor_name;
        args << "--iops";
        QStringList keys = d->m_parameters.keys();
        foreach (QString key, keys) {
            args << QString("%1:%2").arg(key).arg(d->m_parameters.value(key).toString());
        }
        //right now we can't detach while running locally
        //if (d->m_detach) {
        //    args << QString("--_detach=1");
        //}
        //task.log(QString("Executing locally: %1").arg(mountainprocess_exe));
        foreach (QString key, keys) {
            QString val = d->m_parameters[key].toString();
            task.log(QString("%1 = %2").arg(key).arg(val));
            if (val.startsWith("http")) {
                task.error("Executing locally, but parameter starts with http. Probably mlproxy url has not been set.");
                return;
            }
        }

        QString exe = "ml-run-process";
        task.log(exe + " " + args.join(" "));

        QProcess* process0 = new QProcess;
        process0->setProcessChannelMode(QProcess::MergedChannels);

        //process0.start(mountainprocess_exe, args);

        process0->setProgram(exe);
        process0->setArguments(args);

        QProcessManager* manager = ObjectRegistry::getObject<QProcessManager>();
        if (!manager) {
            qCritical() << "No process manager object found in registry. Aborting.";
            abort();
        }
        QSharedPointer<QProcess> store_it = manager->start(process0);

        if (!process0->waitForStarted()) {
            task.error("Error starting process.");
            return;
        }

        QString stdout;
        QEventLoop loop;
        while (process0->state() == QProcess::Running) {
            loop.processEvents();
            QString out = process0->readAll();
            if (!out.isEmpty()) {
                printf("%s", out.toLatin1().data());
                task.log("STDOUT: " + out);
                stdout += out;
            }
            if (MLUtil::threadInterruptRequested()) {
                task.error("Terminating due to interrupt request");
                process0->terminate();
                return;
            }
        }

        /*
        if (QProcess::execute(mountainprocess_exe, args) != 0) {
            qWarning() << "Problem running mountainprocess" << mountainprocess_exe << args;
            task.error("Problem running mountainprocess");
        }
        */
    }
    else {
        /*
        QString url = d->m_mscmdserver_url + "/?";
        url += "processor=" + d->m_processor_name + "&";
        QStringList keys = d->m_parameters.keys();
        foreach(QString key, keys)
        {
            url += QString("%1=%2&").arg(key).arg(d->m_parameters.value(key).toString());
        }
        this->setStatus("Remote " + d->m_processor_name, "MLNetwork::httpGetText: " + url, 0.5);
        MLNetwork::httpGetText(url);
        this->setStatus("", "", 1);
        */

        task.log("Setting up pp_process");
        QJsonObject pp_process;
        pp_process["processor_name"] = d->m_processor_name;
        pp_process["parameters"] = variantmap_to_json_obj(d->m_parameters);
        QJsonArray pp_processes;
        pp_processes.append(pp_process);
        QJsonObject pp;
        pp["processes"] = pp_processes;
        QString pipeline_json = QJsonDocument(pp).toJson(QJsonDocument::Compact);

        task.log(pipeline_json);

        QString script;
        script += QString("function main(params) {\n");
        script += QString("  MP.runPipeline('%1');\n").arg(pipeline_json);
        script += QString("}\n");

        QJsonObject req;
        req["action"] = "queueScript";
        req["script"] = script;
        if (d->m_detach) {
            req["detach"] = 1;
        }
        QString url = d->m_mlproxy_url + "/mpserver";
        task.log("POSTING: " + url);
        task.log(QJsonDocument(req).toJson());
        if (MLUtil::threadInterruptRequested()) {
            task.error("Halted before post.");
            return;
        }
        QTime post_timer;
        post_timer.start();
        QJsonObject resp = http_post(url, req);
        ICounterManager* manager = ObjectRegistry::getObject<ICounterManager>();
        if (manager) {
            IIntCounter* remote_processing_time_counter = static_cast<IIntCounter*>(manager->counter("remote_processing_time"));
            remote_processing_time_counter->add(post_timer.elapsed());
        }
        if (MLUtil::threadInterruptRequested()) {
            task.error("Halted during post: " + url);
            return;
        }
        task.log("GOT RESPONSE: ");
        task.log(QJsonDocument(resp).toJson());
        if (!resp["error"].toString().isEmpty()) {
            task.error(resp["error"].toString());
        }
    }
}

QString MountainProcessRunnerPrivate::create_temporary_output_file_name(const QString& mlproxy_url, const QString& processor_name, const QMap<QString, QVariant>& params, const QString& parameter_name)
{
    (void)mlproxy_url;
    QString str = processor_name + ":";
    QStringList keys = params.keys();
    qSort(keys);
    foreach (QString key, keys) {
        str += key + "=" + params.value(key).toString() + "&";
    }

    QString file_name = QString("%1_%2.tmp").arg(MLUtil::computeSha1SumOfString(str)).arg(parameter_name);
    //QString ret = CacheManager::globalInstance()->makeRemoteFile(mlproxy_url, file_name);
    QString ret = CacheManager::globalInstance()->makeLocalFile(file_name);
    return ret;
}
