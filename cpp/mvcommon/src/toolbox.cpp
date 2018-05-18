#include "toolbox.h"

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QStyleOption>
#include <QPainter>
#include <QPropertyAnimation>

class Header : public QFrame {
public:
    Header(QWidget *parent = Q_NULLPTR) : QFrame(parent) {
        setFrameShape(QFrame::StyledPanel);
        setFrameShadow(QFrame::Raised);
        setAttribute(Qt::WA_Hover);
    }
    void setText(const QString &l) {
        m_text = l;
        update();
    }
    const QString& text() const { return m_text; }
    void setIcon(const QIcon &icon) {
        m_icon = icon;
        update();
    }
    const QIcon& icon() const { return m_icon; }
    QSize sizeHint() const {
        QFontMetrics fm(font());
        return QSize(fm.width(m_text), fm.lineSpacing())+QSize(8, 8);

    }
    void setOpen(bool t) {
        m_open = t;
        update();
    }
    bool isOpen() const { return m_open; }
protected:
    void paintEvent(QPaintEvent *) {
        QPainter p(this);
        QStyleOptionFrame option;
        initStyleOption(&option);
        if (m_hovering) {
            p.fillRect(rect(), option.palette.light());
            option.palette.setColor(QPalette::Background, option.palette.light().color());
        }
        style()->drawControl(QStyle::CE_ShapedFrame, &option, &p, this);

        QIcon arrow = style()->standardIcon(isOpen() ? QStyle::SP_ArrowDown : QStyle::SP_ArrowRight, &option, this);
        int leftAdjustment = 4;
        QSize arrowSize = QSize(option.rect.height()-8, option.rect.height()-8).boundedTo(QSize(64, 64));
        style()->drawItemPixmap(&p, option.rect.adjusted(leftAdjustment, 4, -4, -4), Qt::AlignLeft | Qt::AlignVCenter, arrow.pixmap(arrowSize));
        leftAdjustment += arrowSize.width()+4;
        if (!icon().isNull()) {
            style()->drawItemPixmap(&p, option.rect.adjusted(leftAdjustment, 4, -4, -4), Qt::AlignLeft | Qt::AlignVCenter, icon().pixmap(arrowSize, QIcon::Normal, isOpen() ? QIcon::On : QIcon::Off));
            leftAdjustment += arrowSize.width()+4;
        }
        style()->drawItemText(&p, option.rect.adjusted(leftAdjustment, 4, -4, -4), Qt::AlignLeft|Qt::AlignVCenter, option.palette, isEnabled(), m_text);
    }
    void enterEvent(QEvent *event) {
        m_hovering = true;
        QFrame::enterEvent(event);
        update();
    }
    void leaveEvent(QEvent *event) {
        m_hovering = false;
        QFrame::enterEvent(event);
        update();
    }
private:
    bool m_hovering = false;
    bool m_open = false;
    QString m_text;
    QIcon m_icon;
};

class ContainerLayout : public QLayout {
    Q_OBJECT
    Q_PROPERTY(int maxHeight READ maxHeight WRITE setMaxHeight NOTIFY maxHeightChanged)
public:
    ContainerLayout(QWidget *parent) : QLayout(parent) {}
    ContainerLayout() : QLayout() {}
    QSize sizeHint() const
    {
        QSize sh = content ? content->sizeHint() : QSize();
        if (maxHeight() > 0) {
            sh.setHeight(qMin(sh.height(), maxHeight()));
        }
        return sh;
    }

    QSize contentSizeHint(int w) const {
        if (!content) {
            return QSize();
        }
        QSize sh;
        bool res;
        if(content->widget()) {
            content->widget()->ensurePolished();
            res = content->widget()->testAttribute(Qt::WA_Resized);
            content->widget()->setAttribute(Qt::WA_Resized);
        }
        if (content->hasHeightForWidth()) {
            int width = qBound(content->minimumSize().width(), w, content->maximumSize().width());
            sh = QSize(width, content->heightForWidth(width));
        } else {
         sh = content->widget() ? content->widget()->sizeHint() : content->sizeHint();
        }
        if (content->widget()) {
            content->widget()->setAttribute(Qt::WA_Resized, res);
        }
        return sh;
    }

    void addItem(QLayoutItem *i)
    {
        if (content)
            return;
        content = i;
        invalidate();
    }
    QLayoutItem *itemAt(int index) const
    {
        if (index > 0) return Q_NULLPTR;
        return content;
    }

    QLayoutItem *takeAt(int index)
    {
        if (index > 0) return Q_NULLPTR;
        QLayoutItem *i = content;
        content = Q_NULLPTR;
        return i;
        invalidate();
    }

    int count() const
    {
        return content ? 1 : 0;
    }
    void setGeometry(const QRect &r)
    {
        QLayout::setGeometry(r);
        if (!content) return;
        QSize sh = contentSizeHint(r.width());
        QRect newRect = r;
        int diff = sh.height() - r.height();
        if (diff > 0) {
            newRect = QRect(r.left(), r.top()-diff, r.width(), r.height()+diff);
        }
        content->setGeometry(newRect);
    }

    bool hasHeightForWidth() const
    {
        return content ? content->hasHeightForWidth() : false;
    }
    int heightForWidth(int w) const
    {
        int h = content ? content->heightForWidth(w) : QLayout::heightForWidth(w);
        if (maxHeight() > 0)
            h = qMin(h, maxHeight());
        return h;
    }
    int maxHeight() const
    {
        return m_maxHeight;
    }
public slots:
    void setMaxHeight(int maxHeight)
    {
        if (m_maxHeight == maxHeight)
            return;

        m_maxHeight = maxHeight;
        emit maxHeightChanged(m_maxHeight);
        invalidate();
    }
private:
    QLayoutItem* content = Q_NULLPTR;
    int m_maxHeight = 0;

public:

signals:
    void maxHeightChanged(int maxHeight);
};

class Box : public QWidget {
    Q_OBJECT
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)
public:
    Box(QWidget* parent = Q_NULLPTR) : QWidget(parent) {
        m_layout = new QVBoxLayout(this);
        m_layout->setSpacing(1);
        m_layout->setMargin(0);
        m_header = new Header;
        m_header->setCursor(Qt::PointingHandCursor);
        m_layout->addWidget(m_header);
        m_header->installEventFilter(this);
        m_container = new QFrame;
        m_container->setFrameShape(QFrame::StyledPanel);
        m_container->setVisible(isOpen());
        m_containerLayout = new ContainerLayout(m_container);
        m_containerLayout->setMargin(0);
        m_layout->addWidget(m_container);
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    }
    void setLabel(const QString &l) {
        header()->setText(l);
    }
    QString label() const { return header()->text(); }
    void setIcon(const QIcon &icon) {
        header()->setIcon(icon);
    }
    const QIcon& icon() const { return header()->icon(); }
    void setContent(QWidget *c) {
        if (m_content)
            m_containerLayout->removeWidget(m_content);
        m_content = c;
        if (m_content) {
            m_containerLayout->addWidget(m_content);
        }
    }
    ~Box() {

    }
    Header* header() const { return m_header; }
    QFrame* container() const { return m_container; }
    QWidget* content() const { return m_content; }
    bool isOpen() const
    {
        return m_open;
    }
    void setOpen(bool open)
    {
        if (m_open == open)
            return;

        m_open = open;
        if (m_open) {
            QPropertyAnimation *openAnim = new QPropertyAnimation(m_containerLayout, "maxHeight");
            openAnim->setStartValue(1);
            openAnim->setEndValue(m_containerLayout->contentSizeHint(container()->width()).height());
            openAnim->setDuration(200);
            connect(openAnim, &QPropertyAnimation::finished, [this]() {
                m_containerLayout->setMaxHeight(0);
            });
            openAnim->start(QAbstractAnimation::DeleteWhenStopped);
            container()->setVisible(m_open);
        } else {
            QPropertyAnimation *closeAnim = new QPropertyAnimation(m_containerLayout, "maxHeight");
            closeAnim->setStartValue(m_containerLayout->contentSizeHint(container()->width()).height());
            closeAnim->setEndValue(1);\
            closeAnim->setDuration(200);
            connect(closeAnim, &QPropertyAnimation::finished, [this]() {
                m_containerLayout->setMaxHeight(0);
                container()->setVisible(m_open);
            });
            closeAnim->start(QAbstractAnimation::DeleteWhenStopped);
        }
        header()->setOpen(m_open);
        emit openChanged(m_open);
    }

public slots:

    void open() { setOpen(true); }
    void close() { setOpen(false); }
signals:
    void openChanged(bool open);

protected:
    bool eventFilter(QObject *watched, QEvent *event) {
        if (watched == header() && event->type() == QEvent::MouseButtonRelease) {
            setOpen(!isOpen());
            return true;
        }
        return false;
    }

private:
    Header* m_header;
    QFrame* m_container;
    QWidget* m_content;
    QVBoxLayout* m_layout;
    ContainerLayout* m_containerLayout;
    bool m_open = false;
};

class ToolBoxPrivate {
public:
    QVector<Box*> widgets;
    int currentIndex = -1;
    QScrollArea* scrollArea;
    QWidget* content;
    QVBoxLayout* layout;
    QSpacerItem* spacer;
};

ToolBox::ToolBox(QWidget *parent)
    : QWidget(parent), d(new ToolBoxPrivate)
{
    d->scrollArea = new QScrollArea(this);
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->addWidget(d->scrollArea);
    d->content = new QWidget;
    d->scrollArea->setWidget(d->content);
    d->scrollArea->setWidgetResizable(true);
    d->layout = new QVBoxLayout(d->content);
    d->layout->setMargin(1);
    d->spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
    d->layout->addItem(d->spacer);
}

ToolBox::~ToolBox()
{
    clear();
    delete d;
}

void ToolBox::addWidget(QWidget *page, const QString &label)
{
    insertWidget(count(), page, label);
}

void ToolBox::addWidget(QWidget *page, const QIcon &icon, const QString &label)
{
    insertWidget(count(), page, icon, label);
}

void ToolBox::clear()
{
//    m_widgets.clear();
}

int ToolBox::count() const
{
    return d->widgets.count();
}

int ToolBox::currentIndex() const
{
    return d->currentIndex;
}

QWidget *ToolBox::currentWidget() const
{
    if (currentIndex() == -1) return Q_NULLPTR;
    return widget(currentIndex());
}

int ToolBox::indexOf(QWidget *w) const
{
    for (int i = 0 ; i < d->widgets.count(); ++i) {
        if (d->widgets.at(i)->content() == w)
            return i;
    }
    return -1;
}

int ToolBox::insertWidget(int index, QWidget *page, const QString &label)
{
    Box* box = new Box;
    box->setLabel(label);
    box->setContent(page);
    d->widgets.insert(index, box);
    d->layout->insertWidget(index, box);
    return d->layout->indexOf(box);
}

int ToolBox::insertWidget(int index, QWidget *page, const QIcon &icon, const QString &label)
{
    Box* box = new Box;
    box->setLabel(label);
    box->setIcon(icon);
    box->setContent(page);
    d->widgets.insert(index, box);
    d->layout->insertWidget(index, box);
    return d->layout->indexOf(box);
}

QString ToolBox::widgetText(int index) const
{
    return d->widgets.at(index)->header()->text();
}

QWidget *ToolBox::widget(int index) const
{
    return d->widgets.at(index)->content();
}

void ToolBox::setCurrentIndex(int)
{
}

void ToolBox::open(int index)
{
    if (index >=0)
        d->widgets.at(index)->open();
}

#include "toolbox.moc"
