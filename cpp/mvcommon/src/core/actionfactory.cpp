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
#include "actionfactory.h"

void ActionFactory::addToToolbar(ActionType action, QWidget* container, QObject* receiver, const char* signalOrSlot)
{
    Q_ASSERT(container);
    Q_ASSERT(receiver);
    Q_ASSERT(signalOrSlot);

    QString name;
    QString tooltip;
    QIcon icon;

    switch (action) {
    case ActionType::ZoomIn:
        name = "Zoom In";
        tooltip = "Zoom in.";
        icon = QIcon(":/images/zoom-in.png");
        break;
    case ActionType::ZoomOut:
        name = "Zoom Out";
        tooltip = "Zoom out.";
        icon = QIcon(":/images/zoom-out.png");
        break;
    case ActionType::ZoomInVertical:
        name = "Vertical Zoom In";
        tooltip = "Vertical zoom in.";
        icon = QIcon(":/images/vertical-zoom-in.png");
        break;
    case ActionType::ZoomOutVertical:
        name = "Vertical Zoom Out";
        tooltip = "Vertical zoom out.";
        icon = QIcon(":/images/vertical-zoom-out.png");
        break;
    case ActionType::ZoomInHorizontal:
        name = "Horizontal Zoom In";
        tooltip = "";
        icon = QIcon(":/images/horizontal-zoom-in.png");
        break;
    case ActionType::ZoomOutHorizontal:
        name = "Horizontal Zoom Out";
        tooltip = "";
        icon = QIcon(":/images/horizontal-zoom-out.png");
        break;
    case ActionType::PanLeft:
        name = "Pan left";
        tooltip = "Pan left";
        icon = QIcon(":/images/left.png");
        break;
    case ActionType::PanRight:
        name = "Pan right";
        tooltip = "Pan right";
        icon = QIcon(":/images/right.png");
        break;
    default:
        break;
    }

    QAction* a = new QAction(icon, name, container);
    a->setProperty("action_type", "toolbar");
    a->setToolTip(tooltip);
    container->addAction(a);
    QObject::connect(a, SIGNAL(triggered(bool)), receiver, signalOrSlot);
}
