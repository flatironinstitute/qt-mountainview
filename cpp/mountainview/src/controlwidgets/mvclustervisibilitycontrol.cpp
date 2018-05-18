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

#include "mvclustervisibilitycontrol.h"
#include "mvcontext.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QTimer>
#include <QToolButton>
#include "flowlayout.h"

class MVClusterVisibilityControlPrivate {
public:
    MVClusterVisibilityControl* q;
    FlowLayout* m_cluster_tag_flow_layout;
    FlowLayout* m_cluster_pair_tag_flow_layout;
    QWidgetList m_all_widgets;
    QCheckBox* m_subset_checkbox;
    QLineEdit* m_subset_line_edit;
};

MVClusterVisibilityControl::MVClusterVisibilityControl(MVAbstractContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVClusterVisibilityControlPrivate;
    d->q = this;

    d->m_cluster_tag_flow_layout = new FlowLayout;
    d->m_cluster_pair_tag_flow_layout = new FlowLayout;

    QHBoxLayout* subset_layout = new QHBoxLayout;
    {
        QCheckBox* CB = new QCheckBox("Use subset:");
        subset_layout->addWidget(CB);

        d->m_subset_line_edit = new QLineEdit;
        d->m_subset_checkbox = CB;
        subset_layout->addWidget(d->m_subset_line_edit);

        QToolButton* apply_button = new QToolButton;
        apply_button->setText("apply");
        subset_layout->addWidget(apply_button);
        QObject::connect(apply_button, SIGNAL(clicked(bool)), this, SLOT(updateContext()));
    }

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addLayout(d->m_cluster_tag_flow_layout);
    vlayout->addLayout(d->m_cluster_pair_tag_flow_layout);
    vlayout->addLayout(subset_layout);

    this->setLayout(vlayout);

    QObject::connect(context, SIGNAL(clusterVisibilityChanged()), this, SLOT(updateControls()));

    updateControls();
}

MVClusterVisibilityControl::~MVClusterVisibilityControl()
{
    delete d;
}

QString MVClusterVisibilityControl::title() const
{
    return "Cluster Visibility";
}

QSet<int> subset_str_to_set(const QString& txt)
{
    QStringList list = txt.split(",", QString::SkipEmptyParts);
    QSet<int> ret;
    foreach (QString str, list) {
        ret.insert(str.trimmed().toInt());
    }
    return ret;
}

void MVClusterVisibilityControl::updateContext()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    ClusterVisibilityRule rule = c->clusterVisibilityRule();

    rule.view_all_tagged = this->controlValue("all_tagged").toBool();
    rule.view_all_untagged = this->controlValue("all_untagged").toBool();
    rule.hide_rejected = this->controlValue("hide_rejected").toBool();

    rule.view_tags.clear();
    QStringList tags = c->allClusterTags().toList();
    foreach (QString tag, tags) {
        if (this->controlValue("tag-" + tag).toBool()) {
            rule.view_tags << tag;
        }
    }

    rule.use_subset = d->m_subset_checkbox->isChecked();
    {
        rule.subset = subset_str_to_set(d->m_subset_line_edit->text());
    }

    c->setClusterVisibilityRule(rule);
}

void MVClusterVisibilityControl::updateControls()
{
    MVContext* c = qobject_cast<MVContext*>(mvContext());
    Q_ASSERT(c);

    ClusterVisibilityRule rule = c->clusterVisibilityRule();

    QStringList tags = c->allClusterTags().toList();
    qSort(tags);

    qDeleteAll(d->m_all_widgets);
    d->m_all_widgets.clear();

    {
        QCheckBox* CB = this->createCheckBoxControl("all_tagged", "All tagged clusters");
        CB->setChecked(rule.view_all_tagged);
        d->m_cluster_tag_flow_layout->addWidget(CB);
        d->m_all_widgets << CB;
    }
    {
        QCheckBox* CB = this->createCheckBoxControl("all_untagged", "All untagged");
        CB->setChecked(rule.view_all_untagged);
        d->m_cluster_tag_flow_layout->addWidget(CB);
        d->m_all_widgets << CB;
    }
    {
        QCheckBox* CB = this->createCheckBoxControl("hide_rejected", "Hide rejected");
        CB->setChecked(rule.hide_rejected);
        d->m_cluster_tag_flow_layout->addWidget(CB);
        d->m_all_widgets << CB;
    }
    foreach (QString tag, tags) {
        if (tag != "rejected") {
            QCheckBox* CB = this->createCheckBoxControl("tag-" + tag, tag);
            CB->setChecked(rule.view_tags.contains(tag));
            d->m_cluster_tag_flow_layout->addWidget(CB);
            d->m_all_widgets << CB;
            if (rule.view_all_tagged)
                CB->setEnabled(false);
        }
    }

    {
        d->m_subset_line_edit->setEnabled(d->m_subset_checkbox->isChecked());
        if (rule.subset != subset_str_to_set(d->m_subset_line_edit->text())) {
            QList<int> list = rule.subset.toList();
            qSort(list);
            QStringList list2;
            foreach (int k, list) {
                list2 << QString("%1").arg(k);
            }
            d->m_subset_line_edit->setText(list2.join(","));
        }
    }
}
