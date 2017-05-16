#pragma once

#include <QScopedPointer>
#include <QFrame>

class LoadWidgetPrivate;
class LoadWidget : public QFrame
{
    Q_OBJECT
public:
    explicit LoadWidget(QWidget *parent = 0);
    ~LoadWidget();

signals:

public slots:

private:
    QScopedPointer<LoadWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), LoadWidget)
};

