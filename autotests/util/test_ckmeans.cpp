// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/util/ckmeans.h (class CKMeans).
#include <gtest/gtest.h>
#include <QObject>
#include <QString>
#include <QColor>
#include <QVector3D>
#include <QImage>
#include "util/ckmeans.h"

TEST(CKMeansTest, Construct_WithParent_NoThrow) {
    QObject parent;
    CKMeans km(&parent);
    SUCCEED();
}
TEST(CKMeansTest, SetGetPicPath_Roundtrip_Matches) {
    CKMeans km;
    km.setPicPath("/tmp/test-pic.png");
    EXPECT_EQ(km.PicPath(), QString("/tmp/test-pic.png"));
}
TEST(CKMeansTest, GetPicPath_AfterSet_MatchesPicPath) {
    CKMeans km;
    km.setPicPath("/tmp/another.png");
    EXPECT_EQ(km.getPicPath(), QString("/tmp/another.png"));
}
TEST(CKMeansTest, SetShowImage_GetShowImage_Roundtrip) {
    CKMeans km;
    QImage img(4, 4, QImage::Format_RGB32);
    img.fill(QColor(10, 20, 30));
    km.setShowImage(img);
    QImage out = km.getShowImage();
    EXPECT_FALSE(out.isNull());
}
TEST(CKMeansTest, GetColorMain_WithoutImage_NoThrow) {
    CKMeans km;
    EXPECT_NO_THROW((void)km.getColorMain());
}
TEST(CKMeansTest, GetColorSecond_WithoutImage_NoThrow) {
    CKMeans km;
    EXPECT_NO_THROW((void)km.getColorSecond());
}
TEST(CKMeansTest, GetCommColorMain_WithoutImage_NoThrow) {
    CKMeans km;
    EXPECT_NO_THROW((void)km.getCommColorMain());
}
TEST(CKMeansTest, GetCommColorSecond_WithoutImage_NoThrow) {
    CKMeans km;
    EXPECT_NO_THROW((void)km.getCommColorSecond());
}
TEST(CKMeansTest, GetColorMain_WithSmallImage_NoThrow) {
    CKMeans km;
    QImage img(8, 8, QImage::Format_RGB32);
    img.fill(QColor(100, 150, 200));
    km.setShowImage(img);
    EXPECT_NO_THROW((void)km.getColorMain());
}
