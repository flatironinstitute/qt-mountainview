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

#ifndef MVTEMPLATESVIEW2PANEL_H
#define MVTEMPLATESVIEW2PANEL_H

#include "paintlayer.h"
#include <QWidget>
#include <mda.h>
#include <mvcontext.h>

class MVTemplatesView2PanelPrivate;
class MVTemplatesView2Panel : public PaintLayer {
public:
    friend class MVTemplatesView2PanelPrivate;
    MVTemplatesView2Panel();
    virtual ~MVTemplatesView2Panel();
    void setTemplate(const Mda& X);
    void setElectrodeGeometry(const ElectrodeGeometry& geom);
    void setVerticalScaleFactor(double factor);
    void setChannelColors(const QList<QColor>& colors);
    void setColors(const QMap<QString, QColor>& colors);
    void setCurrent(bool val);
    void setSelected(bool val);
    void setTitle(const QString& txt);
    void setFiringRateDiskDiameter(double val);

protected:
    void paint(QPainter* painter) Q_DECL_OVERRIDE;

private:
    MVTemplatesView2PanelPrivate* d;
};

#endif // MVTEMPLATESVIEW2PANEL_H
