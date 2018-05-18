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

#include "mlnetwork.h"

#include <QFile>
#include <QTextStream>
#include <QTime>
#include <QThread>
#include <QCoreApplication>
#include <QUrl>
#include <QDir>
#include "cachemanager.h"
#include "taskprogress.h"

#include "mlcommon.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

bool curl_is_installed()
{
    int exit_code = system("curl --version");
    return (exit_code == 0);
}

/// TODO: (MEDIUM) handle memory cache for MLNetwork::httpGetText in a better way
static QMap<QString, QString> s_get_http_text_curl_cache;

QString get_http_text_curl(const QString& url)
{
    if (MLUtil::inGuiThread()) {
        if (s_get_http_text_curl_cache.contains(url)) {
            return s_get_http_text_curl_cache[url];
        }
    }
    if (!curl_is_installed()) {
#ifdef QT_GUI_LIB
        qWarning() << "Problem in http request. It appears that curl is not installed.";
#else
        qWarning() << "There is no reason we should be calling MLNetwork::httpGetText_curl in a non-gui application!!!!!!!!!!!!!!!!!!!!!!!";
#endif
        return "";
    }
    QString tmp_fname = CacheManager::globalInstance()->makeLocalFile("", CacheManager::ShortTerm);
    QString cmd = QString("curl \"%1\" > %2").arg(url).arg(tmp_fname);
    int exit_code = system(cmd.toLatin1().data());
    if (exit_code != 0) {
        qWarning() << "Problem with system call: " + cmd;
        QFile::remove(tmp_fname);
#ifdef QT_GUI_LIB
        QMessageBox::critical(0, "Problem downloading text file", "Problem in http request. Are you connected to the internet?");
#else
        qWarning() << "There is no reason we should be calling MLNetwork::httpGetText_curl * in a non-gui application!!!!!!!!!!!!!!!!!!!!!!!";
#endif
        return "";
    }
    QString ret = TextFile::read(tmp_fname);
    QFile::remove(tmp_fname);
    if (MLUtil::inGuiThread()) {
        s_get_http_text_curl_cache[url] = ret;
    }
    return ret;
}

QString get_temp_fname()
{
    return CacheManager::globalInstance()->makeLocalFile();
    //int rand_num = qrand() + QDateTime::currentDateTime().toMSecsSinceEpoch();
    //return QString("%1/MdaClient_%2.tmp").arg(QDir::tempPath()).arg(rand_num);
}

QString get_http_binary_file_curl(const QString& url)
{
    QString tmp_fname = get_temp_fname();
    QString cmd = QString("curl \"%1\" > %2").arg(url).arg(tmp_fname);
    int exit_code = system(cmd.toLatin1().data());
    if (exit_code != 0) {
        qWarning() << "Problem with system call: " + cmd;
        QFile::remove(tmp_fname);
        return "";
    }
    return tmp_fname;
}

QString MLNetwork::httpGetBinaryFile(const QString& url)
{
    if (MLUtil::inGuiThread()) {
        qCritical() << "Cannot call MLNetwork::httpGetBinaryFile from within the GUI thread: " + url;
        exit(-1);
    }

    //TaskProgress task("Downloading binary file");
    //task.log(url);
    QTime timer;
    timer.start();
    QString fname = get_temp_fname();
    QNetworkAccessManager manager; // better make it a singleton
    QNetworkReply* reply = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;
    QFile temp(fname);
    int num_bytes = 0;
    temp.open(QIODevice::WriteOnly);
    QObject::connect(reply, &QNetworkReply::readyRead, [&]() {
        if (MLUtil::threadInterruptRequested()) {
            TaskProgress errtask("Download halted");
            errtask.error("Thread interrupt requested");
            errtask.log(url);
            reply->abort();
        }
        QByteArray X=reply->readAll();
        temp.write(X);
        num_bytes+=X.count();
    });
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    //task.setLabel(QString("Downloaded %1 MB in %2 sec").arg(num_bytes * 1.0 / 1e6).arg(timer.elapsed() * 1.0 / 1000));
    printf("RECEIVED BINARY (%d ms, %d bytes) from %s\n", timer.elapsed(), num_bytes, url.toLatin1().data());
    TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_downloaded", num_bytes);
    if (MLUtil::threadInterruptRequested()) {
        return "";
    }
    return fname;
}

QString abbreviate(const QString& str, int len1, int len2)
{
    if (str.count() <= len1 + len2 + 20)
        return str;
    return str.mid(0, len1) + "...\n...\n..." + str.mid(str.count() - len2);
}

QString MLNetwork::httpGetText(const QString& url)
{
    if (MLUtil::inGuiThread()) {
        return get_http_text_curl(url);
    }
    QTime timer;
    timer.start();
    QString fname = get_temp_fname();
    QNetworkAccessManager manager; // better make it a singleton
    QNetworkReply* reply = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;
    QString ret;
    QObject::connect(reply, &QNetworkReply::readyRead, [&]() {
        ret+=reply->readAll();
    });
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    printf("RECEIVED TEXT (%d ms, %d bytes) from GET %s\n", timer.elapsed(), ret.count(), url.toLatin1().data());
    QString str = abbreviate(ret, 200, 200);
    printf("%s\n", (str.toLatin1().data()));

    TaskManager::TaskProgressMonitor::globalInstance()->incrementQuantity("bytes_downloaded", ret.count());
    return ret;
}
