// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ckmeans.h"
#include "util/log.h"
#include <vector>
#include <iostream>
#include <math.h>
#include <QImage>
using namespace std;



CKMeans::CKMeans(QObject *parent)
    : QObject(parent)
{
    qCDebug(dmMusic) << "Initializing CKMeans with default values";
    m_pointsCount = 1200;
    m_clusterCount = 1;
    qCDebug(dmMusic) << "Default points count:" << m_pointsCount << "Cluster count:" << m_clusterCount;
}

void CKMeans::kMeans()
{
    //构造待聚类数据集
    vector< vector<float> > data;
    QImage image = m_showImage;
    if(image.isNull()) {
        qCWarning(dmMusic) << "Cannot perform k-means: input image is null";
        return;
    }
    
    qCDebug(dmMusic) << "Processing image for k-means, original size:" << image.width() << "x" << image.height();
    image = image.scaled(420*1.8/3, 420*1.8/3, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_showImage = image;
    
    int width = image.width(), height = image.height();
    m_pointsCount = width * height;
    vector<float> points[m_pointsCount];
    int picCnt = 0;
    
    qCDebug(dmMusic) << "Extracting RGB data from image, scaled size:" << width << "x" << height;
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

    qCDebug(dmMusic) << "Starting k-means clustering with" << m_clusterCount << "clusters and" << picCnt << "points";
    m_kmeans.loadData(data);
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
    qCDebug(dmMusic) << "Getting picture path:" << m_sPicPath;
    return m_sPicPath;
}

QVector3D CKMeans::getColorMain()
{
    qCDebug(dmMusic) << "Getting main color from k-means centroids";
    if(m_kmeans.centroids.size() > 0) {
        qCDebug(dmMusic) << "ids size > 0";
        return QVector3D(m_kmeans.centroids[0][0]/255, m_kmeans.centroids[0][1]/255, m_kmeans.centroids[0][2]/255);
    } else {
        qCDebug(dmMusic) << "ids size <= 0";
        return QVector3D(0, 0, 0);
    }
}

QVector3D CKMeans::getColorSecond()
{
    qCDebug(dmMusic) << "Getting second color from k-means centroids";
    if(m_kmeans.centroids.size() > 1) {
        qCDebug(dmMusic) << "ids size > 1";
        return QVector3D(m_kmeans.centroids[1][0]/255, m_kmeans.centroids[1][1]/255, m_kmeans.centroids[1][2]/255);
    } else {
        qCDebug(dmMusic) << "ids size <= 1";
        return QVector3D(0, 0, 0);
    }
}

QColor CKMeans::getCommColorMain()
{
    qCDebug(dmMusic) << "Getting common main color from k-means centroids";
//    return QColor(Qt::black);
    if(m_kmeans.centroids.empty()) return QColor(Qt::black);
    return QColor(m_kmeans.centroids[0][0], m_kmeans.centroids[0][1], m_kmeans.centroids[0][2]);
}

QColor CKMeans::getCommColorSecond()
{
    qCDebug(dmMusic) << "Getting common second color from k-means centroids";
//    return QColor(Qt::blue);
    if(m_kmeans.centroids.empty()) return QColor(Qt::black);
    return QColor(m_kmeans.centroids[1][0], m_kmeans.centroids[1][1], m_kmeans.centroids[1][2]);
}

void CKMeans::setShowImage(const QImage &img)
{
    qCDebug(dmMusic) << "Setting show image, size:" << img.width() << "x" << img.height();
    m_showImage = img;
    kMeans();
}

void CKMeans::setPicPath(QString sPicPath)
{
    qCDebug(dmMusic) << "Setting picture path:" << sPicPath;
    m_sPicPath = sPicPath;
    if(m_sPicPath.toLower().startsWith("qrc")) {
        m_sPicPath = m_sPicPath.replace("qrc", "");
        qCDebug(dmMusic) << "Adjusted resource path to:" << m_sPicPath;
    }
    
    m_showImage = QImage(m_sPicPath);
    if (m_showImage.isNull()) {
        qCWarning(dmMusic) << "Failed to load image from path:" << m_sPicPath;
    } else {
        qCDebug(dmMusic) << "Successfully loaded image, size:" << m_showImage.width() << "x" << m_showImage.height();
    }
    
    kMeans();
    emit picPathChanged(sPicPath);
}

QString CKMeans::PicPath()
{
    qCDebug(dmMusic) << "Getting picture path:" << m_sPicPath;
    return m_sPicPath;
}

QImage CKMeans::getShowImage()
{
    qCDebug(dmMusic) << "Getting show image, size:" << m_showImage.width() << "x" << m_showImage.height();
    return m_showImage;
}
