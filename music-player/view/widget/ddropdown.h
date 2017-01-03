/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QScopedPointer>
#include <QFrame>

class DDropdownPrivate;
class DDropdown : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
public:
    explicit DDropdown(QWidget *parent = 0);
    ~DDropdown();

    QString status() const;
    QList<QAction *> actions() const;

signals:
    void statusChanged(QString status);
    void requestContextMenu();
    void triggered(QAction *action);

public slots:
    void setCurrentAction(QAction *action);
    QAction *addAction(const QString &item, const QVariant &var);
    void setStatus(QString status);

protected:
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    QScopedPointer<DDropdownPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DDropdown)
};

