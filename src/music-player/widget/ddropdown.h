// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QScopedPointer>
#include <QString>
#include <DWidget>
#include <DLabel>
#include <DIconButton>
#include <DToolButton>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE


class DDropdown : public DWidget
{
    Q_OBJECT
public:
    explicit DDropdown(QWidget *parent = Q_NULLPTR);
    ~DDropdown();

    QList<QAction *> actions() const;

signals:
    void statusChanged(QString status);
    void requestContextMenu();
    void triggered(QAction *action);

public slots:
    void setCurrentAction(QAction *action);
    void setCurrentAction(int index = 0);
    QAction *addAction(const QString &item, const QVariant &var);
    void setStatus(const QString &status);
#ifdef DTKWIDGET_CLASS_DSizeMode
    void slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode);
#endif

protected:
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    DMenu            *menu       = nullptr;
    DToolButton      *dropdown   = nullptr;
    QString           status;
};

