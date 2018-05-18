#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QWidget>

class ToolBoxPrivate;
class ToolBox : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged)

public:
    ToolBox(QWidget *parent = 0);
    ~ToolBox();
    void addWidget(QWidget* page, const QString& label = QString());
    void addWidget(QWidget* page, const QIcon &icon, const QString &label = QString());
    void clear();
    int count() const;
    int currentIndex() const;
    QWidget *currentWidget() const;
    int indexOf(QWidget *w) const;
    int insertWidget(int index, QWidget *page, const QString &label = QString());
    int insertWidget(int index, QWidget *page, const QIcon &icon, const QString &label = QString());
    void removeWidget(int index);
//    QIcon widgetIcon(int index) const;
    QString widgetText(int index) const;
    QWidget* widget(int index) const;
public slots:
    void setCurrentIndex(int);
    void open(int);

signals:
    void countChanged(int count);
    void currentChanged(int);
protected:

private:
    ToolBoxPrivate *d;
};

#endif // TOOLBOX_H
