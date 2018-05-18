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
#include "curationprogramcontroller.h"
#include "mlcommon.h"

#include <QJsonDocument>

class CurationProgramControllerPrivate {
public:
    CurationProgramController* q;

    MVContext* m_context;
    QString m_log;
};

CurationProgramController::CurationProgramController(MVContext* mvcontext)
{
    d = new CurationProgramControllerPrivate;
    d->q = this;
    d->m_context = mvcontext;
}

CurationProgramController::~CurationProgramController()
{
    delete d;
}

QString CurationProgramController::log() const
{
    return d->m_log;
}

QString CurationProgramController::clusterNumbers()
{
    QList<int> list = d->m_context->clusterAttributesKeys();
    QJsonArray list2;
    for (int i = 0; i < list.count(); i++) {
        list2.append(list[i]);
    }
    return QJsonDocument(list2).toJson();
}

double CurationProgramController::metric(int cluster_number, QString metric_name)
{
    QJsonObject attributes = d->m_context->clusterAttributes(cluster_number);
    return attributes["metrics"].toObject()[metric_name].toDouble();
}

void CurationProgramController::setMetric(int cluster_number, QString metric_name, double val)
{
    QJsonObject attributes = d->m_context->clusterAttributes(cluster_number);
    QJsonObject metrics = attributes["metrics"].toObject();
    metrics[metric_name] = val;
    attributes["metrics"] = metrics;
    d->m_context->setClusterAttributes(cluster_number, attributes);
}

QString CurationProgramController::clusterPairs()
{
    QList<ClusterPair> list = d->m_context->clusterPairAttributesKeys();
    QJsonArray list2;
    for (int i = 0; i < list.count(); i++) {
        QJsonObject obj;
        obj["k1"] = list[i].k1();
        obj["k2"] = list[i].k2();
        list2.append(obj);
    }
    return QJsonDocument(list2).toJson();
}

double CurationProgramController::pairMetric(int k1, int k2, QString metric_name)
{
    QJsonObject attributes = d->m_context->clusterPairAttributes(ClusterPair(k1, k2));
    return attributes["metrics"].toObject()[metric_name].toDouble();
}

void CurationProgramController::setPairMetric(int k1, int k2, QString metric_name, double val)
{
    QJsonObject attributes = d->m_context->clusterPairAttributes(ClusterPair(k1, k2));
    QJsonObject metrics = attributes["metrics"].toObject();
    metrics[metric_name] = val;
    attributes["metrics"] = metrics;
    d->m_context->setClusterPairAttributes(ClusterPair(k1, k2), attributes);
}

bool CurationProgramController::hasTag(int cluster_number, QString tag_name)
{
    QSet<QString> tags = d->m_context->clusterTags(cluster_number);
    return tags.contains(tag_name);
}

void CurationProgramController::addTag(int cluster_number, QString tag_name)
{
    QSet<QString> tags = d->m_context->clusterTags(cluster_number);
    tags.insert(tag_name);
    d->m_context->setClusterTags(cluster_number, tags);
}

void CurationProgramController::removeTag(int cluster_number, QString tag_name)
{
    QSet<QString> tags = d->m_context->clusterTags(cluster_number);
    tags.remove(tag_name);
    d->m_context->setClusterTags(cluster_number, tags);
}

void CurationProgramController::log(const QString& message)
{
    d->m_log += message + "\n";
}
