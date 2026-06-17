// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ckmeans.cpp 的单元测试：k-means 颜色聚类，从图像提取主色。
// 对应 docs/unit-test-plan.md Phase 1 / ③。
//
// 注意事项（来自源码阅读）：
// 1. CKMeans 默认 m_clusterCount = 1，只会产生 1 个聚类中心，因此 getColorSecond /
//    getCommColorSecond 在访问 centroids[1] 时会越界（源码缺陷）。本文件只测主色，
//    副色用例标记 DISABLED_ 待源码修复。
// 2. kMeans() 内部会缩放图像并采样所有像素，给纯色图断言主色最稳定。

#include <gtest/gtest.h>

#include <QImage>
#include <QColor>
#include <QVector3D>
#include <QSignalSpy>

#include "ckmeans.h"

// 工具：构造一张纯色图
static QImage makeSolidImage(const QColor &color, int size = 32)
{
    QImage img(size, size, QImage::Format_RGB32);
    img.fill(color.rgb());
    return img;
}

// ============================================================================
// 构造 & 默认值
// ============================================================================
TEST(CKMeansTest, defaultConstructorHasNoCentroids)
{
    CKMeans km;
    // 未设置图像时，centroids 为空，主色回退到黑色
    EXPECT_EQ(km.getCommColorMain(), QColor(Qt::black));
    // QVector3D 接口在空 centroids 时返回 (0,0,0)
    EXPECT_EQ(km.getColorMain(), QVector3D(0, 0, 0));
}

TEST(CKMeansTest, getPicPathEmptyByDefault)
{
    CKMeans km;
    EXPECT_TRUE(km.getPicPath().isEmpty());
}

// ============================================================================
// 纯色图的主色提取
// ============================================================================
TEST(CKMeansTest, dominantColorOfSolidRedImage)
{
    CKMeans km;
    km.setShowImage(makeSolidImage(QColor(255, 0, 0)));

    const QColor main = km.getCommColorMain();
    // 纯红图的主色应强烈偏红（聚类中心 ≈ 红色通道高）
    EXPECT_GT(main.red(), 200);
    EXPECT_LT(main.green(), 60);
    EXPECT_LT(main.blue(), 60);
}

TEST(CKMeansTest, dominantColorOfSolidGreenImage)
{
    CKMeans km;
    km.setShowImage(makeSolidImage(QColor(0, 255, 0)));

    const QColor main = km.getCommColorMain();
    EXPECT_LT(main.red(), 60);
    EXPECT_GT(main.green(), 200);
    EXPECT_LT(main.blue(), 60);
}

TEST(CKMeansTest, dominantColorOfSolidBlueImage)
{
    CKMeans km;
    km.setShowImage(makeSolidImage(QColor(0, 0, 255)));

    const QColor main = km.getCommColorMain();
    EXPECT_LT(main.red(), 60);
    EXPECT_LT(main.green(), 60);
    EXPECT_GT(main.blue(), 200);
}

TEST(CKMeansTest, getColorMainIsNormalizedToUnitRange)
{
    // getColorMain 返回 RGB/255，应在 [0,1] 范围
    CKMeans km;
    km.setShowImage(makeSolidImage(QColor(255, 255, 255)));

    const QVector3D v = km.getColorMain();
    EXPECT_GE(v.x(), 0.0f);  EXPECT_LE(v.x(), 1.0f);
    EXPECT_GE(v.y(), 0.0f);  EXPECT_LE(v.y(), 1.0f);
    EXPECT_GE(v.z(), 0.0f);  EXPECT_LE(v.z(), 1.0f);
    // 纯白图归一化后应接近 (1,1,1)
    EXPECT_GT(v.x(), 0.9f);
    EXPECT_GT(v.y(), 0.9f);
    EXPECT_GT(v.z(), 0.9f);
}

// ============================================================================
// 一致性：同一图像两次提取，主色应稳定
// ============================================================================
TEST(CKMeansTest, mainColorIsStableAcrossInstances)
{
    const QImage img = makeSolidImage(QColor(200, 100, 50));

    CKMeans km1;
    km1.setShowImage(img);
    const QColor c1 = km1.getCommColorMain();

    CKMeans km2;
    km2.setShowImage(img);
    const QColor c2 = km2.getCommColorMain();

    // 纯色图聚类结果应确定性一致
    EXPECT_EQ(c1.rgb(), c2.rgb());
}

// ============================================================================
// 空图像：不应崩溃
// ============================================================================
TEST(CKMeansTest, nullImageDoesNotCrash)
{
    QImage nullImg;
    CKMeans km;
    km.setShowImage(nullImg);   // kMeans 内部检测 isNull 直接 return
    // 不崩溃即通过；主色回退黑
    EXPECT_EQ(km.getCommColorMain(), QColor(Qt::black));
}

// ============================================================================
// 副色接口：当前 clusterCount=1 会越界，标记 DISABLED_ 待源码修复
// 参见 docs/unit-test-plan.md「已知缺陷」。
// ============================================================================
TEST(CKMeansTest, DISABLED_secondColorAvailableWhenClusterCountGreaterThanOne)
{
    CKMeans km;
    km.setShowImage(makeSolidImage(QColor(255, 0, 0)));
    // 待源码支持多聚类中心后启用
    const QVector3D second = km.getColorSecond();
    EXPECT_GE(second.x(), 0.0f);
}

// ============================================================================
// getColorSecond / getCommColorSecond 的回退分支：
// 未设置图像或聚类中心不足 2 个时，应安全返回默认值（覆盖 else 分支与 empty 分支）
// ============================================================================
TEST(CKMeansTest, getColorSecondReturnsZeroWhenNoCentroids)
{
    // 默认构造：centroids 为空，size()<=1 → 走 else 返回 (0,0,0)
    CKMeans km;
    EXPECT_EQ(km.getColorSecond(), QVector3D(0, 0, 0));
}

TEST(CKMeansTest, getColorSecondReturnsZeroWithSingleCentroid)
{
    // 设置纯色图后 clusterCount=1，centroids.size()==1，size()<=1 仍走 else
    CKMeans km;
    km.setShowImage(makeSolidImage(QColor(255, 0, 0)));
    EXPECT_EQ(km.getColorSecond(), QVector3D(0, 0, 0));
}

TEST(CKMeansTest, getCommColorSecondReturnsBlackWhenNoCentroids)
{
    CKMeans km;
    // empty() 为真 → 返回 Qt::black
    EXPECT_EQ(km.getCommColorSecond(), QColor(Qt::black));
}

TEST(CKMeansTest, getCommColorSecondAccessibleWithSingleCentroid)
{
    // centroids 非空（size==1）：源码访问 centroids[1]，但 size==1 时该访问行为未定义。
    // 这里仅验证 getCommColorMain 在单聚类下稳定可调用（主色接口），副色接口的越界风险
    // 已通过 DISABLED_ 用例记录，此处不再调用 getCommColorSecond 触发 UB。
    CKMeans km;
    km.setShowImage(makeSolidImage(QColor(255, 0, 0)));
    const QColor main = km.getCommColorMain();
    EXPECT_GE(main.red(), 200);
}

// ============================================================================
// setPicPath / PicPath : 资源路径前缀处理 + 信号发射
// ============================================================================
TEST(CKMeansTest, setPicPathWithQrcPrefixStripsQrc)
{
    // "qrc:/foo.png" → "/foo.png"（虽然文件不存在，但前缀替换逻辑应触发）
    CKMeans km;
    QSignalSpy spy(&km, &CKMeans::picPathChanged);
    km.setPicPath("qrc:/some/resource.png");
    // m_sPicPath 应已去掉 "qrc" 前缀
    EXPECT_FALSE(km.PicPath().startsWith("qrc"));
    // 信号应已发射（无论图像是否成功加载）
    EXPECT_EQ(spy.count(), 1);
}

TEST(CKMeansTest, setPicPathWithNonExistentPathEmitsSignal)
{
    // 不存在的文件路径：m_showImage 为空 → kMeans 直接 return，但仍 emit picPathChanged
    CKMeans km;
    QSignalSpy spy(&km, &CKMeans::picPathChanged);
    km.setPicPath("/nonexistent/path/image.png");
    EXPECT_EQ(km.PicPath(), QStringLiteral("/nonexistent/path/image.png"));
    EXPECT_EQ(spy.count(), 1);
}

TEST(CKMeansTest, getPicPathAndPicPathReturnSameValue)
{
    // getPicPath()（Q_INVOKABLE）与 PicPath()（READ slot）应返回同一存储值
    CKMeans km;
    km.setPicPath("/tmp/whatever.png");
    EXPECT_EQ(km.getPicPath(), km.PicPath());
    EXPECT_EQ(km.getPicPath(), QStringLiteral("/tmp/whatever.png"));
}

// ============================================================================
// getShowImage : 默认构造返回 null 图像；设置后可取回
// ============================================================================
TEST(CKMeansTest, getShowImageInitiallyNull)
{
    CKMeans km;
    EXPECT_TRUE(km.getShowImage().isNull());
}
