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
#ifndef ClusterDetailViewPropertiesDialog_H
#define ClusterDetailViewPropertiesDialog_H

#include <QDialog>

struct ClusterDetailViewProperties {
    int cluster_number_font_size = 12;
    int export_image_width = 2000;
    int export_image_height = 800;
};

class ClusterDetailViewPropertiesDialogPrivate;
class ClusterDetailViewPropertiesDialog : public QDialog {
    Q_OBJECT
public:
    friend class ClusterDetailViewPropertiesDialogPrivate;
    ClusterDetailViewPropertiesDialog();
    virtual ~ClusterDetailViewPropertiesDialog();
    void setProperties(ClusterDetailViewProperties properties);
    ClusterDetailViewProperties properties() const;
private slots:
    void slot_update_enabled();

private:
    ClusterDetailViewPropertiesDialogPrivate* d;
};

#endif // ClusterDetailViewPropertiesDialog_H
