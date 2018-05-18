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

#include "clustermerge.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <QDebug>
#include "mlcommon.h"
#include "mlcommon.h"

struct merge_group {
    QSet<int> members;
    bool operator==(const merge_group& other) const
    {
        return (members == other.members);
    }
    QList<int> sortedList() const
    {
        QList<int> ret = members.toList();
        qSort(ret);
        return ret;
    }
};

uint qHash(const merge_group& mg)
{
    QString str;
    QList<int> list = mg.sortedList();
    QStringList strlist;
    foreach (int k, list) {
        strlist << QString("%1").arg(k);
    }
    return qHash(strlist.join("::"));
}

class ClusterMergePrivate {
public:
    ClusterMerge* q;
    QSet<merge_group> m_merge_groups;

    bool is_merged(int label1, int label2);
    void consolidate_merge_groups();
};

ClusterMerge::ClusterMerge()
{
    d = new ClusterMergePrivate;
    d->q = this;
}

ClusterMerge::ClusterMerge(const ClusterMerge& other)
{
    d = new ClusterMergePrivate;
    d->q = this;

    d->m_merge_groups = other.d->m_merge_groups;
}

ClusterMerge::~ClusterMerge()
{
    delete d;
}

void ClusterMerge::operator=(const ClusterMerge& other)
{
    d->m_merge_groups = other.d->m_merge_groups;
}

bool ClusterMerge::operator==(const ClusterMerge& other) const
{
    return (d->m_merge_groups == other.d->m_merge_groups);
}

void ClusterMerge::clear()
{
    d->m_merge_groups.clear();
}

void ClusterMerge::merge(const QSet<int>& labels)
{
    merge_group grp0;
    grp0.members = labels;
    d->m_merge_groups << grp0;
    d->consolidate_merge_groups();
}

void ClusterMerge::merge(const QList<int>& labels)
{
    merge(labels.toSet());
}

void ClusterMerge::unmerge(int label)
{
    QSet<int> tmp;
    tmp.insert(label);
    unmerge(tmp);
}

void ClusterMerge::unmerge(const QSet<int>& labels)
{
    QList<merge_group> list = d->m_merge_groups.toList();
    for (int i = 0; i < list.count(); i++) {
        foreach (int label, labels) {
            list[i].members.remove(label);
        }
    }
    d->m_merge_groups = list.toSet();
    d->consolidate_merge_groups();
}

void ClusterMerge::unmerge(const QList<int>& labels)
{
    unmerge(labels.toSet());
}

int ClusterMerge::representativeLabel(int label) const
{
    QList<int> tmp = this->getMergeGroup(label);
    if (tmp.isEmpty())
        return label;
    return tmp.value(0);
}

QList<int> ClusterMerge::representativeLabels() const
{
    QSet<int> ret;
    foreach (merge_group mg, d->m_merge_groups) {
        QList<int> tmp = mg.members.toList();
        qSort(tmp);
        ret.insert(tmp.value(0));
    }
    QList<int> ret2 = ret.toList();
    qSort(ret2);
    return ret2;
}

QList<int> ClusterMerge::getMergeGroup(int label) const
{
    foreach (merge_group mg, d->m_merge_groups) {
        if (mg.members.contains(label)) {
            QList<int> tmp = mg.members.toList();
            qSort(tmp);
            return tmp;
        }
    }

    QList<int> ret;
    ret << label;
    return ret;
}

void ClusterMerge::setFromJsonObject(QJsonObject obj)
{
    d->m_merge_groups.clear();

    QJsonArray merge_groups = obj["merge_groups"].toArray();

    for (int i = 0; i < merge_groups.count(); i++) {
        QJsonArray Y = merge_groups[i].toArray();
        QList<int> grp;
        for (int j = 0; j < Y.count(); j++) {
            grp << Y[j].toInt();
        }
        this->merge(grp);
    }
}

QJsonObject ClusterMerge::toJsonObject() const
{
    QJsonArray X;
    QList<int> rep_labels = this->representativeLabels();
    for (int i = 0; i < rep_labels.count(); i++) {
        QList<int> grp = this->getMergeGroup(rep_labels[i]);
        if (grp.count() > 1) {
            QJsonArray Y;
            for (int j = 0; j < grp.count(); j++)
                Y.append(grp[j]);
            X.append(Y);
        }
    }
    QJsonObject ret;
    ret["merge_groups"] = X;
    return ret;
}

QString ClusterMerge::toJson() const
{
    QJsonObject X = toJsonObject();
    return QJsonDocument(X).toJson();
}

QMap<int, int> ClusterMerge::labelMap(int K) const
{
    QMap<int, int> ret;
    for (int k = 1; k <= K; k++) {
        ret[k] = this->representativeLabel(k);
    }
    return ret;
}

QVector<int> ClusterMerge::mapLabels(const QVector<int>& labels) const
{
    QMap<int, int> map = this->labelMap(MLCompute::max<int>(labels));
    QVector<int> ret = labels;
    for (int i = 0; i < ret.count(); i++) {
        ret[i] = map[ret[i]];
    }
    return ret;
}

QString ClusterMerge::clusterLabelText(int label)
{
    QList<int> grp = this->getMergeGroup(label);
    if (grp.isEmpty())
        return QString("%1").arg(label);
    if (label != grp.value(0))
        return "";
    QString str;
    for (int j = 0; j < grp.count(); j++) {
        if (!str.isEmpty())
            str += ",";
        str += QString("%1").arg(grp[j]);
    }
    return str;
}

ClusterMerge ClusterMerge::fromJson(const QString& json)
{
    ClusterMerge ret;
    QJsonArray X = QJsonDocument::fromJson(json.toLatin1()).array();
    for (int i = 0; i < X.count(); i++) {
        QJsonArray Y = X[i].toArray();
        QList<int> grp;
        for (int j = 0; j < Y.count(); j++) {
            grp << Y[j].toInt();
        }
        ret.merge(grp);
    }
    return ret;
}

bool ClusterMergePrivate::is_merged(int label1, int label2)
{
    return q->getMergeGroup(label1).contains(label2);
}

void ClusterMergePrivate::consolidate_merge_groups()
{
    QList<merge_group> list = m_merge_groups.toList();
    QMap<int, int> assignments;
    for (int i = 0; i < list.count(); i++) {
        QList<int> tmp = list[i].members.toList();
        QSet<int> merge_to;
        foreach (int k, tmp) {
            if (assignments.contains(k)) {
                merge_to.insert(assignments[k]);
            }
        }
        int ind00;
        if (merge_to.isEmpty()) {
            ind00 = i;
        }
        else {
            QList<int> tmp0 = merge_to.toList();
            qSort(tmp0);
            ind00 = tmp0.value(0);
        }
        foreach (int k, tmp) {
            assignments[k] = ind00;
        }
    }
    QMap<int, QSet<int> > groups;
    QList<int> keys = assignments.keys();
    foreach (int key, keys) {
        groups[assignments[key]].insert(key);
    }
    m_merge_groups.clear();
    foreach (QSet<int> grp, groups) {
        if (grp.count() > 1) {
            merge_group tmp;
            tmp.members = grp;
            m_merge_groups.insert(tmp);
        }
    }
}
