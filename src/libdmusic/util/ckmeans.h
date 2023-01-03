// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CK_MEANS_H
#define CK_MEANS_H

#include <QObject>
#include <QColor>
#include "ykmeans.h"
#include <QVector3D>
#include <QImage>

class CKMeans: public QObject
{
    Q_OBJECT
    //属性声明
    Q_PROPERTY(QString getPicPath READ PicPath WRITE setPicPath NOTIFY picPathChanged)
public:
    explicit CKMeans(QObject *parent = NULL);

    Q_INVOKABLE QString getPicPath();
    Q_INVOKABLE QVector3D getColorMain();
    Q_INVOKABLE QVector3D getColorSecond();
    Q_INVOKABLE QColor getCommColorMain();
    Q_INVOKABLE QColor getCommColorSecond();
    Q_INVOKABLE void setShowImage(const QImage &img);

private:
    void kMeans();

signals:
    void picPathChanged(const QString sPicPath);
public slots:
    void setPicPath(QString sPicPath);
    QString PicPath();
    QImage getShowImage();
private:
    int m_pointsCount;
    int m_clusterCount;
    QColor m_colorMain;
    QColor m_colorSecond;
    QString m_sPicPath;
    KMEANS<float> m_kmeans;
    QImage m_showImage;
};

#endif // CK_MEANS_H
