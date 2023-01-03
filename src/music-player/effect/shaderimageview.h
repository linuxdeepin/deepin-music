// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHADERIMAGEVIEW_H
#define SHADERIMAGEVIEW_H
#include <QQuickPaintedItem>
#include <QImage>
#include <QVariant>


class ShaderImageView: public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QVariant presenter READ presenter WRITE setPresenter NOTIFY presenterChanged)

public:
    ShaderImageView(QQuickItem *parent = 0);
    ~ShaderImageView();

    void paint(QPainter *painter);
        //the paint method is already implemented in QQuickPaintedItem
        //you just override it
    QString source() const;
    void setSource(const QString &source);
    QVariant presenter() const;
    void setPresenter(const QVariant &presenter);
private:
    QString m_source;
    QVariant m_pPresenter;
signals:
    void sourceChanged();
    void presenterChanged();

};

#endif // SHADERIMAGEVIEW_H
