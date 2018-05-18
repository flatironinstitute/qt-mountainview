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
#ifndef MATRIXVIEW_H
#define MATRIXVIEW_H

#include <QWidget>
#include <mda.h>

class MatrixViewPrivate;
class MatrixView : public QWidget {
    Q_OBJECT
public:
    enum Mode {
        PercentMode,
        CountsMode
    };

    friend class MatrixViewPrivate;
    MatrixView();
    virtual ~MatrixView();
    void setMode(Mode mode);
    void setMatrix(const Mda& A);
    void setValueRange(double minval, double maxval);
    void setIndexPermutations(const QVector<int>& perm_rows, const QVector<int>& perm_cols);
    void setLabels(const QStringList& row_labels, const QStringList& col_labels);
    void setTitle(QString title);
    void setRowAxisLabel(QString label);
    void setColumnAxisLabel(QString label);
    void setDrawDividerForFinalRow(bool val);
    void setDrawDividerForFinalColumn(bool val);
    void setCurrentElement(QPoint pt);
    QPoint currentElement() const;
signals:
    void currentElementChanged();

protected:
    void paintEvent(QPaintEvent* evt);
    void mouseMoveEvent(QMouseEvent* evt);
    void leaveEvent(QEvent*);
    void mousePressEvent(QMouseEvent* evt);

private:
    MatrixViewPrivate* d;
};

#endif // MATRIXVIEW_H
