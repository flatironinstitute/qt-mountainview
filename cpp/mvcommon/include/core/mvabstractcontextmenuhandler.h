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
#ifndef MVABSTRACTCONTEXTMENUHANDLER_H
#define MVABSTRACTCONTEXTMENUHANDLER_H

#include <QMimeData>

/*!
 * Context menu handler is an object that can populate context menu with
 * context sensitive actions. QMimeData is used as a carrier for determining
 * the context. A view can request a menu for a given context by creating
 * a QMimeData object and adding custom MIME types as well as associating
 * extra data with a given MIME type. Registered context menu handlers are
 * queried with the MIME type to return a list of supported actions for that
 * type and its data. Actions from all handlers are then assembled and
 * shown in form of a context menu at a given position on the screen.
 *
 * Actions should make use of the associated data to trigger appropriate
 * functionality.
 */

#include "mvabstractcontext.h"
#include "mvmainwindow.h"

class QAction;

class MVAbstractContextMenuHandler {
public:
    MVAbstractContextMenuHandler(MVMainWindow* mw);
    virtual ~MVAbstractContextMenuHandler() {}
    virtual bool canHandle(const QMimeData& md) const { Q_UNUSED(md) return false; }
    virtual QList<QAction*> actions(const QMimeData& md) = 0;

protected:
    MVMainWindow* mainWindow() const;

private:
    MVMainWindow* m_main_window;
};
#endif // MVABSTRACTCONTEXTMENUHANDLER_H
