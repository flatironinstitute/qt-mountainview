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

#ifndef IsolationMatrixView_H
#define IsolationMatrixView_H

#include "mvabstractview.h"

#include <mvcontext.h>
#include <mvabstractviewfactory.h>

class IsolationMatrixViewPrivate;
class IsolationMatrixView : public MVAbstractView {
    Q_OBJECT
public:
    enum PermutationMode {
        NoPermutation,
        RowPermutation,
        ColumnPermutation,
        BothRowBasedPermutation,
        BothColumnBasedPermutation
    };

    friend class IsolationMatrixViewPrivate;
    IsolationMatrixView(MVContext* mvcontext);
    virtual ~IsolationMatrixView();

    void prepareCalculation() Q_DECL_OVERRIDE;
    void runCalculation() Q_DECL_OVERRIDE;
    void onCalculationFinished() Q_DECL_OVERRIDE;

protected:
    void keyPressEvent(QKeyEvent* evt) Q_DECL_OVERRIDE;
    void prepareMimeData(QMimeData& mimeData, const QPoint& pos) Q_DECL_OVERRIDE;

private slots:
    void slot_permutation_mode_button_clicked();
    void slot_matrix_view_current_element_changed();
    void slot_update_current_elements_based_on_context();

private:
    IsolationMatrixViewPrivate* d;
};

#endif // IsolationMatrixView_H
