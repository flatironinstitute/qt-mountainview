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
#ifndef MVGRIDVIEWPROPERTIESDIALOG_H
#define MVGRIDVIEWPROPERTIESDIALOG_H

#include <QDialog>

struct MVGridViewProperties {
    bool use_fixed_panel_size = false;
    bool use_fixed_number_of_columns = false;
    int fixed_panel_width = 600, fixed_panel_height = 350;
    int fixed_number_of_columns = 5;
    int font_size = 14;
    QColor font_color = Qt::blue; // not yet implemented
};

class MVGridViewPropertiesDialogPrivate;
class MVGridViewPropertiesDialog : public QDialog {
    Q_OBJECT
public:
    friend class MVGridViewPropertiesDialogPrivate;
    MVGridViewPropertiesDialog();
    virtual ~MVGridViewPropertiesDialog();
    void setProperties(MVGridViewProperties properties);
    MVGridViewProperties properties() const;
private slots:
    void slot_update_enabled();

private:
    MVGridViewPropertiesDialogPrivate* d;
};

#endif // MVGRIDVIEWPROPERTIESDIALOG_H
