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
#ifndef MVCLUSTERPAIRCONTEXTMENUHANDLER_H
#define MVCLUSTERPAIRCONTEXTMENUHANDLER_H

#include <QObject>
#include <mvcontext.h>
#include "mvabstractcontextmenuhandler.h"
#include "mvabstractcontext.h"

class MVClusterPairContextMenuHandler : public QObject, public MVAbstractContextMenuHandler {
public:
    MVClusterPairContextMenuHandler(MVMainWindow* mw, QObject* parent = 0);

    bool canHandle(const QMimeData& md) const Q_DECL_OVERRIDE;
    QList<QAction*> actions(const QMimeData& md) Q_DECL_OVERRIDE;

private:
    QAction* addTagMenu(const QSet<ClusterPair>& cluster_pairs) const;
    QAction* removeTagMenu(const QSet<ClusterPair>& cluster_pairs) const;
    QStringList validTags() const;
};

#endif // MVCLUSTERPAIRCONTEXTMENUHANDLER_H
