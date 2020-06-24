/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     yub.wang <yub.wang@deepin.io>
 *
 * Maintainer: yub.wang <yub.wang@deepin.io>
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
#ifndef MUSICBOXBUTTON_H
#define MUSICBOXBUTTON_H
#pragma once

#include <QScopedPointer>
#include <QMap>
#include <QVariant>

#include <DPushButton>
#include <DButtonBox>

DWIDGET_USE_NAMESPACE

class MusicBoxButton : public DButtonBoxButton
{
    struct MusicPicPathInfo {
        QString normalPicPath;
        QString hoverPicPath;
        QString pressPicPath;
        QString checkedPicPath;
    };

    Q_OBJECT
public:
    explicit MusicBoxButton(const QString &text, QWidget *parent = Q_NULLPTR);

    MusicBoxButton(const QString &text, const QString &normalPic, const QString &hoverPic,
                   const QString &pressPic, const QString &checkedPic = QString(), QWidget *parent = nullptr);

    void setPropertyPic(QString propertyName, const QVariant &value, const QString &normalPic, const QString &hoverPic,
                        const QString &pressPic, const QString &checkedPic = QString());
    void setPropertyPic(const QString &normalPic, const QString &hoverPic,
                        const QString &pressPic, const QString &checkedPic = QString());

    void setTransparent(bool flag);
    void setAutoChecked(bool flag);

protected:
    //void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    char                                               status                  = 0;
    bool                                               autoChecked             = false;
    MusicPicPathInfo                                   defaultPicPath;
    bool                                               transparent             = true;
    QPair<QString, QMap<QVariant, MusicPicPathInfo> >  propertyPicPaths;
};


#endif // MUSICBOXBUTTON_H








