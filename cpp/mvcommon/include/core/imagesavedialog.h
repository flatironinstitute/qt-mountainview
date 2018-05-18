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

#ifndef IMAGESAVEDIALOG_H
#define IMAGESAVEDIALOG_H

#include <QDialog>

class ImageSaveDialogPrivate;
class ImageSaveDialog : public QDialog {
    Q_OBJECT
public:
    friend class ImageSaveDialogPrivate;
    ImageSaveDialog();
    virtual ~ImageSaveDialog();
    static void presentImage(const QImage& img);

    void setImage(const QImage& img);
private slots:
    void slot_save();
    void slot_save_as();
    void slot_cancel();

private:
    ImageSaveDialogPrivate* d;
};

#endif // IMAGESAVEDIALOG_H
