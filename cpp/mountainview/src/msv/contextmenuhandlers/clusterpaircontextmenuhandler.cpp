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
#include "clusterpaircontextmenuhandler.h"
#include "mvmainwindow.h"
#include <QAction>
#include <QMenu>
#include <QSet>
#include <QSignalMapper>

MVClusterPairContextMenuHandler::MVClusterPairContextMenuHandler(MVMainWindow* mw, QObject* parent)
    : QObject(parent)
    , MVAbstractContextMenuHandler(mw)
{
}

bool MVClusterPairContextMenuHandler::canHandle(const QMimeData& md) const
{
    return md.hasFormat("application/x-msv-cluster-pairs");
}

QSet<ClusterPair> string_list_to_cluster_pair_set(QStringList list)
{
    QSet<ClusterPair> ret;
    foreach (QString str, list) {
        ret.insert(ClusterPair::fromString(str));
    }
    return ret;
}

QList<QAction*> MVClusterPairContextMenuHandler::actions(const QMimeData& md)
{
    QStringList clusters_strlist;
    QDataStream ds(md.data("application/x-msv-cluster-pairs"));
    ds >> clusters_strlist;
    QSet<ClusterPair> cluster_pairs = string_list_to_cluster_pair_set(clusters_strlist);
    QList<QAction*> actions;

    MVMainWindow* mw = this->mainWindow();
    MVContext* c = qobject_cast<MVContext*>(mw->mvContext());
    Q_ASSERT(c);

    //TAGS
    {
        actions << addTagMenu(cluster_pairs);
        actions << removeTagMenu(cluster_pairs);
        QAction* action = new QAction("Clear tags", 0);
        connect(action, &QAction::triggered, [cluster_pairs, c]() {
            foreach (ClusterPair cluster_pair, cluster_pairs) {
                c->setClusterPairTags(cluster_pair, QSet<QString>());
            }
        });
        actions << action;
    }

    //Separator
    {
        QAction* action = new QAction(0);
        action->setSeparator(true);
        actions << action;
    }

    //MERGE
    {
        {
            QAction* action = new QAction("Merge", 0);
            action->setToolTip("Tag selected pairs as merged");
            action->setEnabled(cluster_pairs.count() >= 1);
            connect(action, &QAction::triggered, [cluster_pairs, c]() {
                foreach (ClusterPair cluster_pair,cluster_pairs) {
                    QSet<QString> tags = c->clusterPairTags(cluster_pair);
                    tags.insert("merged");
                    c->setClusterPairTags(cluster_pair,tags);
                }
            });
            actions << action;
        }
        {
            QAction* action = new QAction("Unmerge", 0);
            action->setToolTip("Remove merged tag from selected clusters");
            action->setEnabled(cluster_pairs.count() >= 1);
            connect(action, &QAction::triggered, [cluster_pairs, c]() {
                foreach (ClusterPair cluster_pair,cluster_pairs) {
                    QSet<QString> tags = c->clusterPairTags(cluster_pair);
                    tags.remove("merged");
                    c->setClusterPairTags(cluster_pair,tags);
                }
            });
            actions << action;
        }
    }

    //CROSS-CORRELOGRAMS
    {
        {
            QAction* action = new QAction("Cross-correlograms", 0);
            action->setToolTip("Open cross-correlograms for these pairs");
            action->setEnabled(cluster_pairs.count() >= 1);
            connect(action, &QAction::triggered, [mw]() {
                mw->openView("open-selected-cross-correlograms");
            });
            actions << action;
        }
    }

    //Separator
    {
        QAction* action = new QAction(0);
        action->setSeparator(true);
        actions << action;
    }

    //DISCRIMINATION HISTOGRAMS
    {
        /// TODO (HIGH) implement discrim hists for selected pairs
    }

    return actions;
}

QAction* MVClusterPairContextMenuHandler::addTagMenu(const QSet<ClusterPair>& cluster_pairs) const
{
    MVAbstractContext* context = this->mainWindow()->mvContext();

    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    QMenu* M = new QMenu;
    M->setTitle("Add tag");
    QSignalMapper* mapper = new QSignalMapper(M);
    foreach (const QString& tag, validTags()) {
        QAction* a = new QAction(tag, M);
        a->setData(tag);
        M->addAction(a);
        mapper->setMapping(a, tag);
        connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
    }
    /// Witold, I am a bit nervous about passing the context pointer into the lambda function below
    connect(mapper, static_cast<void (QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped),
        [cluster_pairs, c](const QString& tag) {
        foreach(ClusterPair cluster_pair, cluster_pairs) {
            QSet<QString> clTags = c->clusterPairTags(cluster_pair);
            clTags.insert(tag);
            c->setClusterPairTags(cluster_pair, clTags);
        }
        });

    return M->menuAction();
}

QAction* MVClusterPairContextMenuHandler::removeTagMenu(const QSet<ClusterPair>& cluster_pairs) const
{
    MVAbstractContext* context = this->mainWindow()->mvContext();

    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    QSet<QString> tags_set;
    foreach (ClusterPair cluster_pair, cluster_pairs) {
        QJsonObject attributes = c->clusterPairAttributes(cluster_pair);
        QJsonArray tags = attributes["tags"].toArray();
        for (int i = 0; i < tags.count(); i++) {
            tags_set.insert(tags[i].toString());
        }
    }
    QStringList tags_list = tags_set.toList();
    qSort(tags_list);
    QMenu* M = new QMenu;
    M->setTitle("Remove tag");
    QSignalMapper* mapper = new QSignalMapper(M);
    foreach (const QString& tag, tags_list) {
        QAction* a = new QAction(tag, M);
        a->setData(tag);
        M->addAction(a);
        mapper->setMapping(a, tag);
        connect(a, SIGNAL(triggered()), mapper, SLOT(map()));
    }
    connect(mapper, static_cast<void (QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped),
        [cluster_pairs, c](const QString& tag) {
        foreach(ClusterPair cluster_pair, cluster_pairs) {
            QSet<QString> clTags = c->clusterPairTags(cluster_pair);
            clTags.remove(tag);
            c->setClusterPairTags(cluster_pair, clTags);
        }
        });
    M->setEnabled(!tags_list.isEmpty());
    return M->menuAction();
}

QStringList MVClusterPairContextMenuHandler::validTags() const
{
    MVContext* c = qobject_cast<MVContext*>(mainWindow()->mvContext());
    Q_ASSERT(c);

    QSet<QString> set = c->allClusterPairTags();
    /// TODO (LOW) these go in a configuration file
    set << "merge_candidate"
        << "merged";
    QStringList ret = set.toList();
    qSort(ret);
    return ret;
}
