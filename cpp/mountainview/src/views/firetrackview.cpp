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

#include "firetrackview.h"
#include "ftelectrodearrayview.h"

#include <QHBoxLayout>
#include <mvcontext.h>

/// TODO show current cluster waveform in firetrackview

class FireTrackViewPrivate {
public:
    FireTrackView* q;

    FTElectrodeArrayView* m_electrode_array_view;
};

FireTrackView::FireTrackView(MVAbstractContext* context)
    : MVAbstractView(context)
{
    d = new FireTrackViewPrivate;
    d->q = this;

    d->m_electrode_array_view = new FTElectrodeArrayView;
    d->m_electrode_array_view->setShowChannelNumbers(true);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(d->m_electrode_array_view);

    this->setLayout(layout);

    MVContext* c = qobject_cast<MVContext*>(context);
    Q_ASSERT(c);

    this->recalculateOn(c, SIGNAL(electrodeGeometryChanged()));

    recalculate();
}

FireTrackView::~FireTrackView()
{
    delete d;
}

void FireTrackView::prepareCalculation()
{
}

void FireTrackView::runCalculation()
{
}

void FireTrackView::onCalculationFinished()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    ElectrodeGeometry geom = c->electrodeGeometry();
    Mda locations;
    for (int i = 0; i < geom.coordinates.count(); i++) {
        if (i == 0) {
            locations.allocate(geom.coordinates.count(), geom.coordinates[i].count());
        }
        for (int j = 0; j < locations.N1(); j++) {
            locations.setValue(geom.coordinates[i][j], i, j);
        }
    }
    d->m_electrode_array_view->setElectrodeLocations(locations);
}

MVFireTrackFactory::MVFireTrackFactory(MVMainWindow* mw, QObject* parent)
    : MVAbstractViewFactory(mw, parent)
{
}

QString MVFireTrackFactory::id() const
{
    return QStringLiteral("open-firetrack");
}

QString MVFireTrackFactory::name() const
{
    return tr("FireTrack");
}

QString MVFireTrackFactory::title() const
{
    return tr("FireTrack");
}

MVAbstractView* MVFireTrackFactory::createView(MVAbstractContext* context)
{
    FireTrackView* X = new FireTrackView(context);
    return X;
}
