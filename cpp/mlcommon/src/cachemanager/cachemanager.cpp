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

#include "cachemanager.h"

#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QThread>
#include <QJsonDocument>
#include "mlcommon.h"
#include <signal.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CM, "cache_manager");

#define DEFAULT_LOCAL_BASE_PATH MLUtil::tempPath()

class CacheManagerPrivate {
public:
    CacheManager* q;
    QString m_local_base_path;

    QString create_random_file_name();
};

CacheManager::CacheManager()
{
    d = new CacheManagerPrivate;
    d->q = this;
}

CacheManager::~CacheManager()
{
    delete d;
}

void CacheManager::setLocalBasePath(const QString& path)
{
    d->m_local_base_path = path;
    if (!QDir(path).exists()) {
        QString parent = QFileInfo(path).path();
        QString name = QFileInfo(path).fileName();
        if (!QDir(parent).mkdir(name)) {
            qCWarning(CM) << "Unable to create local base path" << path;
        }
    }
    QFile::Permissions perm = QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser | QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup | QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther;
    QFile::setPermissions(path, perm);
}


QString CacheManager::makeLocalFile(const QString& file_name_in)
{
    QString file_name = file_name_in;
    if (file_name.isEmpty())
        file_name = d->create_random_file_name();

    QString str;

    QString ret = QString("%1/%2").arg(localTempPath()).arg(file_name);

    return ret;
}

QString CacheManager::localTempPath()
{
    if (d->m_local_base_path.isEmpty()) {
        //qCWarning(CM) << "Local base path has not been set. Using default: " + QString(DEFAULT_LOCAL_BASE_PATH);
        this->setLocalBasePath(DEFAULT_LOCAL_BASE_PATH);
    }
    return d->m_local_base_path;
}

struct CMFileRec {
    QString path;
    int elapsed_sec;
    double size_gb;
};

QList<CMFileRec> get_file_records(const QString& path)
{
    QStringList fnames = QDir(path).entryList(QStringList("*"), QDir::Files, QDir::Name);
    QList<CMFileRec> records;
    foreach (QString fname, fnames) {
        CMFileRec rec;
        rec.path = path + "/" + fname;
        rec.elapsed_sec = QFileInfo(rec.path).lastModified().secsTo(QDateTime::currentDateTime());
        rec.size_gb = QFileInfo(rec.path).size() * 1.0 / 1e9;
        records << rec;
    }

    QStringList dirnames = QDir(path).entryList(QStringList("*"), QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name);
    foreach (QString dirname, dirnames) {
        QList<CMFileRec> RR = get_file_records(path + "/" + dirname);
        records.append(RR);
    }
    return records;
}

struct CMFileRec_comparer {
    bool operator()(const CMFileRec& a, const CMFileRec& b) const
    {
        if (a.elapsed_sec > b.elapsed_sec)
            return true;
        else
            return false;
    }
};

void sort_by_elapsed(QList<CMFileRec>& records)
{
    qSort(records.begin(), records.end(), CMFileRec_comparer());
}

bool pid_exists(int pid)
{
    return (kill(pid, 0) == 0);
}

void CacheManager::cleanUp()
{
    double max_gb = MLUtil::configValue("general", "max_cache_size_gb").toDouble();
    if (!max_gb) {
        qCWarning(CM) << "max_gb is zero. You probably need to adjust the mountainlab configuration files.";
        return;
    }
    if (!d->m_local_base_path.endsWith("/mountainlab")) {
        qWarning() << "For safety, the temporary path must end with /mountainlab";
        return;
    }
    QList<CMFileRec> records = get_file_records(d->m_local_base_path);
    double total_size_gb = 0;
    for (int i = 0; i < records.count(); i++) {
        total_size_gb += records[i].size_gb;
    }
    int num_files_removed = 0;
    double amount_removed = 0;
    if (total_size_gb > max_gb) {
        double amount_to_remove = total_size_gb - 0.75 * max_gb; //let's get it down to 75% of the max allowed
        sort_by_elapsed(records);
        for (int i = 0; i < records.count(); i++) {
            if (amount_removed >= amount_to_remove) {
                break;
            }
            if (!QFile::remove(records[i].path)) {
                qCWarning(CM) << "Unable to remove file while cleaning up cache: " + records[i].path;
                return;
            }
            amount_removed += records[i].size_gb;
            num_files_removed++;
        }
    }
    if (num_files_removed) {
        qCInfo(CM) << QString("CacheManager removed %1 GB and %2 files").arg(amount_removed).arg(num_files_removed);
    }
}

Q_GLOBAL_STATIC(CacheManager, theInstance)
CacheManager* CacheManager::globalInstance()
{
    return theInstance;
}

/*
void CacheManager::slot_remove_on_delete()
{
    QString fname=sender()->property("CacheManager_file_to_remove").toString();
    if (!fname.isEmpty()) {
        if (!QFile::remove(fname)) {
            qWarning() << "Unable to remove local cached file:" << fname;
        }
    }
    else {
        qWarning() << "Unexpected problem" << __FUNCTION__ << __FILE__ << __LINE__;
    }
}
*/

QString CacheManagerPrivate::create_random_file_name()
{

    int num1 = QDateTime::currentMSecsSinceEpoch();
    long num2 = (long)QThread::currentThreadId();
    int num3 = qrand();
    return QString("ms.%1.%2.%3.tmp").arg(num1).arg(num2).arg(num3);
}
