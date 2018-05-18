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

#ifndef CLUSTERMERGE
#define CLUSTERMERGE

#include <QJsonObject>
#include <QMap>
#include <QSet>

class ClusterMergePrivate;
class ClusterMerge {
public:
    friend class ClusterMergePrivate;
    ClusterMerge();
    ClusterMerge(const ClusterMerge& other);
    virtual ~ClusterMerge();
    void operator=(const ClusterMerge& other);
    bool operator==(const ClusterMerge& other) const;
    void clear();
    void merge(const QSet<int>& labels);
    void merge(const QList<int>& labels);
    void unmerge(int label);
    void unmerge(const QSet<int>& labels);
    void unmerge(const QList<int>& labels);

    int representativeLabel(int label) const;
    QList<int> representativeLabels() const;
    QList<int> getMergeGroup(int label) const;
    void setFromJsonObject(QJsonObject obj);
    QJsonObject toJsonObject() const;
    QString toJson() const;
    QMap<int, int> labelMap(int K) const;
    QVector<int> mapLabels(const QVector<int>& labels) const;

    QString clusterLabelText(int label);

    static ClusterMerge fromJson(const QString& json);

private:
    ClusterMergePrivate* d;
};

#endif // CLUSTERMERGE
