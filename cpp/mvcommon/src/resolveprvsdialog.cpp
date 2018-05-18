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

#include "resolveprvsdialog.h"
#include "ui_resolveprvsdialog.h"

class ResolvePrvsDialogPrivate {
public:
    ResolvePrvsDialog* q;
    Ui_ResolvePrvsDialog ui;
};

ResolvePrvsDialog::ResolvePrvsDialog()
{
    d = new ResolvePrvsDialogPrivate;
    d->q = this;
    d->ui.setupUi(this);
}

ResolvePrvsDialog::~ResolvePrvsDialog()
{
    delete d;
}

ResolvePrvsDialog::Choice ResolvePrvsDialog::choice() const
{
    if (d->ui.open_prv_gui->isChecked())
        return ResolvePrvsDialog::OpenPrvGui;
    else if (d->ui.automatically_download->isChecked())
        return ResolvePrvsDialog::AutomaticallyDownloadAndRegenerate;
    else if (d->ui.proceed_anyway->isChecked())
        return ResolvePrvsDialog::ProceedAnyway;
    else
        return ResolvePrvsDialog::ProceedAnyway;
}
