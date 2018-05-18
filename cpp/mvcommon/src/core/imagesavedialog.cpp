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
#include "imagesavedialog.h"
#include <QImage>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QSettings>
#include <QFileDialog>
#include <QImageWriter>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QApplication>

class ImageSaveDialogPrivate {
public:
    ImageSaveDialog* q;
    QImage m_image;
    QLabel* m_label;
    QLineEdit* m_directory_edit;
    QLineEdit* m_file_edit;
};

ImageSaveDialog::ImageSaveDialog()
{
    d = new ImageSaveDialogPrivate;
    d->q = this;

    d->m_label = new QLabel;
    d->m_directory_edit = new QLineEdit;
    d->m_directory_edit->setReadOnly(true);
    d->m_file_edit = new QLineEdit;
    d->m_file_edit->setMaximumWidth(150);
    d->m_file_edit->setText("img.jpg");

    QSettings settings("SCDA", "mountainview");
    QString last_image_save_dir = settings.value("last_image_save_dir", QDir::homePath()).toString();
    d->m_directory_edit->setText(last_image_save_dir);

    QToolButton* save_button = new QToolButton;
    save_button->setText("Save");
    save_button->setToolTip("Save image");
    connect(save_button, SIGNAL(clicked(bool)), this, SLOT(slot_save()));

    QToolButton* save_as_button = new QToolButton;
    save_as_button->setText("Save As...");
    save_as_button->setToolTip("Save image as...");
    connect(save_as_button, SIGNAL(clicked(bool)), this, SLOT(slot_save_as()));

    QToolButton* cancel_button = new QToolButton;
    cancel_button->setText("Cancel");
    cancel_button->setToolTip("Cancel image export");
    connect(cancel_button, SIGNAL(clicked(bool)), this, SLOT(slot_cancel()));

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addWidget(d->m_label);
    QHBoxLayout* hlayout = new QHBoxLayout;
    vlayout->addLayout(hlayout);
    hlayout->addWidget(d->m_directory_edit);
    hlayout->addWidget(d->m_file_edit);
    hlayout->addWidget(save_button);
    hlayout->addWidget(save_as_button);
    hlayout->addWidget(cancel_button);
    this->setLayout(vlayout);
}

ImageSaveDialog::~ImageSaveDialog()
{
    delete d;
}

void ImageSaveDialog::presentImage(const QImage& img)
{
    ImageSaveDialog dlg;
    dlg.setImage(img);
    dlg.setModal(true);
    dlg.exec();
}

static QPixmap fit_display(const QImage& img)
{
    static QRect screenRect;
    //static int margin = 20;
    QPixmap result = QPixmap::fromImage(img);
    if (screenRect.isNull())
        screenRect = QApplication::desktop()->availableGeometry();
    int SW = screenRect.width() - 100;
    int SH = screenRect.height() - 100;
    if ((result.width() > SW) || (result.height() > SH)) {
        double scale_factor = qMin(SW * 1.0 / result.width(), SH * 1.0 / result.height());
        result = result.scaled(QSize(result.width() * scale_factor, result.height() * scale_factor), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    return result;
}

void ImageSaveDialog::setImage(const QImage& img)
{
    d->m_image = img;
    const QPixmap toDisplay = fit_display(img);
    d->m_label->setPixmap(toDisplay);
    d->m_label->resize(toDisplay.width(), toDisplay.height());
}

void ImageSaveDialog::slot_save()
{
    QString fname = d->m_file_edit->text();
    QString dirname = d->m_directory_edit->text();
    QString full_fname;
    if (fname.isEmpty()) {
        full_fname = QFileDialog::getSaveFileName(0, "Save image", dirname, "Image Files (*.png *.jpg *.bmp)");
        if (full_fname.isEmpty())
            return;
    }
    else {
        full_fname = dirname + "/" + fname;
    }
    QSettings settings("SCDA", "mountainview");
    settings.setValue("last_image_save_dir", QFileInfo(full_fname).path());
    QImageWriter writer(full_fname);
    if (!writer.write(d->m_image)) {
        QMessageBox::critical(0, "Error writing image", "Unable to write image. Please contact us for help with this option.");
    }
    this->accept();
}

void ImageSaveDialog::slot_save_as()
{
    d->m_file_edit->setText("");
    slot_save();
}

void ImageSaveDialog::slot_cancel()
{
    this->reject();
}
