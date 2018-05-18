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

#include "curationprogramcontroller.h"
#include "curationprogramview.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QTextEdit>
#include <mountainprocessrunner.h>
#include <taskprogress.h>
#include "actionfactory.h"
#include <QJSEngine>

class CurationProgramViewPrivate {
public:
    CurationProgramView* q;
    QTextEdit* m_input_editor;
    QTextEdit* m_output_editor;

    void refresh_editor();
};

CurationProgramView::CurationProgramView(MVAbstractContext* mvcontext)
    : MVAbstractView(mvcontext)
{
    d = new CurationProgramViewPrivate;
    d->q = this;

    QHBoxLayout* editor_layout = new QHBoxLayout;

    d->m_input_editor = new QTextEdit;
    d->m_input_editor->setLineWrapMode(QTextEdit::NoWrap);
    d->m_input_editor->setTabStopWidth(30);
    editor_layout->addWidget(d->m_input_editor);

    d->m_output_editor = new QTextEdit;
    d->m_output_editor->setLineWrapMode(QTextEdit::NoWrap);
    d->m_output_editor->setTabStopWidth(30);
    d->m_output_editor->setReadOnly(true);
    editor_layout->addWidget(d->m_output_editor);

    QHBoxLayout* button_layout = new QHBoxLayout;
    {
        QPushButton* B = new QPushButton("Apply");
        button_layout->addWidget(B);
        QObject::connect(B, SIGNAL(clicked(bool)), this, SLOT(slot_apply()));
    }
    button_layout->addStretch();

    QVBoxLayout* vlayout = new QVBoxLayout;
    this->setLayout(vlayout);
    vlayout->addLayout(editor_layout);
    vlayout->addLayout(button_layout);

    this->recalculateOnOptionChanged("curation_program", false);

    QObject::connect(d->m_input_editor, SIGNAL(textChanged()), this, SLOT(slot_text_changed()), Qt::QueuedConnection);

    this->recalculate();
}

CurationProgramView::~CurationProgramView()
{
    this->stopCalculation();
    delete d;
}

void CurationProgramView::prepareCalculation()
{
}

void CurationProgramView::runCalculation()
{
}

void CurationProgramView::onCalculationFinished()
{
    d->refresh_editor();
}

QString display_error(QJSValue result)
{
    QString ret;
    ret += result.property("name").toString() + "\n";
    ret += result.property("message").toString() + "\n";
    ret += QString("%1 line %2\n").arg(result.property("fileName").toString()).arg(result.property("lineNumber").toInt()); //okay
    return ret;
}

QString CurationProgramView::applyCurationProgram(MVAbstractContext* mv_context)
{
    MVContext* c = qobject_cast<MVContext*>(mv_context);
    Q_ASSERT(c);

    QJSEngine engine;
    CurationProgramController controller(c);
    QJSValue CP = engine.newQObject(&controller);
    engine.globalObject().setProperty("_CP", CP);

    QString js = TextFile::read(":msv/views/curationprogram.js");
    engine.evaluate(js);

    //QString program = d->m_input_editor->toPlainText();
    QString program = mv_context->option("curation_program").toString();

    QJSValue ret = engine.evaluate(program);
    QString output;
    output += controller.log();
    output += "\n";
    if (ret.isError()) {
        output += display_error(ret);
        output += "ERROR: " + ret.toString() + "\n";
    }
    return output;
}

void CurationProgramView::slot_text_changed()
{
    QString txt = d->m_input_editor->toPlainText();
    mvContext()->setOption("curation_program", txt);
}

void CurationProgramView::slot_apply()
{
    QString output = CurationProgramView::applyCurationProgram(this->mvContext());
    d->m_output_editor->setText(output);
}

void CurationProgramViewPrivate::refresh_editor()
{
    QString txt = q->mvContext()->option("curation_program").toString();
    if (txt == m_input_editor->toPlainText())
        return;
    m_input_editor->setText(txt);
}
