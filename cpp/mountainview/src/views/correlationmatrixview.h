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

#ifndef CORRELATIONMATRIXVIEW_H
#define CORRELATIONMATRIXVIEW_H

#include "mda.h"
#include <QWidget>

/**
 * \class CorrelationMatrixView
 * \brief Not used for now.
 *
 * It used to be a valuable view and we may use it again.
 *
 **/

class CorrelationMatrixViewPrivate;
class CorrelationMatrixView : public QWidget {
public:
    friend class CorrelationMatrixViewPrivate;
    CorrelationMatrixView(QWidget* parent = 0);
    virtual ~CorrelationMatrixView();
    void setMatrix(const Mda& CM);

protected:
    void paintEvent(QPaintEvent* evt);
    void mouseMoveEvent(QMouseEvent* evt);

signals:

public slots:

private:
    CorrelationMatrixViewPrivate* d;
};

#endif // CORRELATIONMATRIXVIEW_H
