/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
