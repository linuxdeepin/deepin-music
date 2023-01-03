// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ckmeans.h"
#include <vector>
#include <iostream>
#include <math.h>
#include <QImage>
using namespace std;



CKMeans::CKMeans(QObject *parent)
    : QObject(parent)
{
    m_pointsCount = 1200;
    m_clusterCount = 1;
}

void CKMeans::kMeans()
{
    //构造待聚类数据集
    vector< vector<float> > data;
    QImage image = m_showImage;
    if(image.isNull()) return;
    image = image.scaled(420*1.8/3, 420*1.8/3, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_showImage = image;
    int width = image.width(), height = image.height();
    m_pointsCount = width * height;
    vector<float> points[m_pointsCount];
    int picCnt = 0;
    for (int i = 0; i < height; i++){
        for (int k = 0; k < width; k++){
            QRgb bits = image.pixel(i, k);
            points[picCnt].push_back(qRed(bits));
            points[picCnt].push_back(qGreen(bits));
            points[picCnt].push_back(qBlue(bits));

            data.push_back(points[picCnt]);
            picCnt++;
         }
    }

    //构建聚类算法
    //数据加载入算法
    m_kmeans.loadData(data);
    //运行k均值聚类算法
    m_kmeans.kmeans(m_clusterCount);
    //输出类中心
//    for (int i = 0; i < m_clusterCount; i++)
//    {
//        for (int j = 0; j < 3; j++)
//            cout << m_kmeans.centroids[i][j] << ',' << '\t' ;
//        cout << endl;
//    }

}

QString CKMeans::getPicPath()
{
    return m_sPicPath;
}

QVector3D CKMeans::getColorMain()
{
    if(m_kmeans.centroids.size() > 0) {
        return QVector3D(m_kmeans.centroids[0][0]/255, m_kmeans.centroids[0][1]/255, m_kmeans.centroids[0][2]/255);
    } else {
        return QVector3D(0, 0, 0);
    }
}

QVector3D CKMeans::getColorSecond()
{
    if(m_kmeans.centroids.size() > 1) {
        return QVector3D(m_kmeans.centroids[1][0]/255, m_kmeans.centroids[1][1]/255, m_kmeans.centroids[1][2]/255);
    } else {
        return QVector3D(0, 0, 0);
    }
}

QColor CKMeans::getCommColorMain()
{
//    return QColor(Qt::black);
    if(m_kmeans.centroids.empty()) return QColor(Qt::black);
    return QColor(m_kmeans.centroids[0][0], m_kmeans.centroids[0][1], m_kmeans.centroids[0][2]);
}

QColor CKMeans::getCommColorSecond()
{
//    return QColor(Qt::blue);
    if(m_kmeans.centroids.empty()) return QColor(Qt::black);
    return QColor(m_kmeans.centroids[1][0], m_kmeans.centroids[1][1], m_kmeans.centroids[1][2]);
}

void CKMeans::setShowImage(const QImage &img)
{
    m_showImage = img;
    kMeans();
}

void CKMeans::setPicPath(QString sPicPath)
{
    m_sPicPath = sPicPath;
    if(m_sPicPath.toLower().startsWith("qrc"))
        m_sPicPath = m_sPicPath.replace("qrc", "");
    m_showImage = QImage(m_sPicPath);
    kMeans();
    emit picPathChanged(sPicPath);
}

QString CKMeans::PicPath()
{
    return m_sPicPath;
}

QImage CKMeans::getShowImage()
{
    return m_showImage;
}
