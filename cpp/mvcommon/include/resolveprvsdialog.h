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
#ifndef RESOLVEPRVSDIALOG_H
#define RESOLVEPRVSDIALOG_H

#include <QDialog>

class ResolvePrvsDialogPrivate;
class ResolvePrvsDialog : public QDialog {
    Q_OBJECT
public:
    enum Choice {
        OpenPrvGui,
        AutomaticallyDownloadAndRegenerate,
        ProceedAnyway
    };

    friend class ResolvePrvsDialogPrivate;
    ResolvePrvsDialog();
    virtual ~ResolvePrvsDialog();
    Choice choice() const;

private:
    ResolvePrvsDialogPrivate* d;
};

#endif // RESOLVEPRVSDIALOG_H
