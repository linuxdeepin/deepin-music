/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZouYa <zouya@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QScopedPointer>
#include <QString>
#include <DWidget>
#include <DLabel>
#include <DIconButton>
#include <DToolButton>

DWIDGET_USE_NAMESPACE


class DDropdown : public DWidget
{
    Q_OBJECT
    //Q_PROPERTY(QString status READ getStatus WRITE setStatus NOTIFY statusChanged)
public:
    explicit DDropdown(QWidget *parent = Q_NULLPTR);
    ~DDropdown();

    //QString getStatus() const;
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

protected:
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    DMenu            *menu       = nullptr;
    DToolButton      *dropdown   = nullptr;
    QString           status;
};

