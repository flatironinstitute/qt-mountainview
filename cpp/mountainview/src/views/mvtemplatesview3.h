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

#ifndef MVTEMPLATESVIEW3_H
#define MVTEMPLATESVIEW3_H

#include "mvabstractview.h"
#include "mvgridview.h"

#include <mvabstractviewfactory.h>
#include <paintlayer.h>

class MVTemplatesView3Private;
class MVTemplatesView3 : public MVGridView {
    Q_OBJECT
public:
    friend class MVTemplatesView3Private;
    MVTemplatesView3(MVAbstractContext* mvcontext);
    virtual ~MVTemplatesView3();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

    void zoomAllTheWayOut();

protected:
    void keyPressEvent(QKeyEvent* evt) Q_DECL_OVERRIDE;
    void prepareMimeData(QMimeData& mimeData, const QPoint& pos) Q_DECL_OVERRIDE;

private slots:
    void slot_update_panels();
    void slot_update_highlighting();
    void slot_panel_clicked(int index, Qt::KeyboardModifiers modifiers);
    void slot_vertical_zoom_in();
    void slot_vertical_zoom_out();

private:
    MVTemplatesView3Private* d;
};

class MVTemplatesView3Factory : public MVAbstractViewFactory {
    Q_OBJECT
public:
    MVTemplatesView3Factory(MVMainWindow* mw, QObject* parent = 0);
    QString id() const Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QString title() const Q_DECL_OVERRIDE;
    MVAbstractView* createView(MVAbstractContext* context) Q_DECL_OVERRIDE;
private slots:
    //void openClipsForTemplate();
};

#endif // MVTEMPLATESVIEW3_H
