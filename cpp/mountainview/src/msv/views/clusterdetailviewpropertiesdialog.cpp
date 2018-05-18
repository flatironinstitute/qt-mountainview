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

#include "clusterdetailviewpropertiesdialog.h"
#include "ui_clusterdetailviewpropertiesdialog.h"

class ClusterDetailViewPropertiesDialogPrivate {
public:
    ClusterDetailViewPropertiesDialog* q;
    Ui_ClusterDetailViewPropertiesDialog ui;
    ClusterDetailViewProperties m_properties;
};

ClusterDetailViewPropertiesDialog::ClusterDetailViewPropertiesDialog()
{
    d = new ClusterDetailViewPropertiesDialogPrivate;
    d->q = this;

    d->ui.setupUi(this);
    //QObject::connect(d->ui.use_fixed_number_of_columns, SIGNAL(clicked(bool)), this, SLOT(slot_update_enabled()));
    //QObject::connect(d->ui.use_fixed_panel_sizes, SIGNAL(clicked(bool)), this, SLOT(slot_update_enabled()));
}

ClusterDetailViewPropertiesDialog::~ClusterDetailViewPropertiesDialog()
{
    delete d;
}

ClusterDetailViewProperties ClusterDetailViewPropertiesDialog::properties() const
{
    d->m_properties.cluster_number_font_size = d->ui.cluster_number_font_size->text().toInt();
    d->m_properties.export_image_width = d->ui.export_image_width->text().toInt();
    d->m_properties.export_image_height = d->ui.export_image_height->text().toInt();
    /*
    d->m_properties.use_fixed_panel_size = d->ui.use_fixed_panel_sizes->isChecked();
    d->m_properties.use_fixed_number_of_columns = d->ui.use_fixed_number_of_columns->isChecked();
    d->m_properties.fixed_panel_width = d->ui.fixed_panel_width->text().toInt();
    d->m_properties.fixed_panel_height = d->ui.fixed_panel_height->text().toInt();
    d->m_properties.fixed_number_of_columns = d->ui.fixed_number_of_columns->text().toInt();
    d->m_properties.font_size = d->ui.font_size->text().toInt();
    */
    return d->m_properties;
}

void ClusterDetailViewPropertiesDialog::slot_update_enabled()
{
    //d->ui.fixed_panel_width->setEnabled(d->ui.use_fixed_panel_sizes->isChecked());
    //d->ui.fixed_panel_height->setEnabled(d->ui.use_fixed_panel_sizes->isChecked());
    //d->ui.fixed_number_of_columns->setEnabled(d->ui.use_fixed_number_of_columns->isChecked());
}

void ClusterDetailViewPropertiesDialog::setProperties(ClusterDetailViewProperties properties)
{
    d->m_properties = properties;
    d->ui.cluster_number_font_size->setText(QString("%1").arg(properties.cluster_number_font_size));
    d->ui.export_image_width->setText(QString("%1").arg(properties.export_image_width));
    d->ui.export_image_height->setText(QString("%1").arg(properties.export_image_height));
    /*
    d->ui.use_fixed_panel_sizes->setChecked(properties.use_fixed_panel_size);
    d->ui.use_fixed_number_of_columns->setChecked(properties.use_fixed_number_of_columns);
    d->ui.fixed_panel_width->setText(QString("%1").arg(properties.fixed_panel_width));
    d->ui.fixed_panel_height->setText(QString("%1").arg(properties.fixed_panel_height));
    d->ui.fixed_number_of_columns->setText(QString("%1").arg(properties.fixed_number_of_columns));
    d->ui.font_size->setText(QString("%1").arg(properties.font_size));
    */
    slot_update_enabled();
}
