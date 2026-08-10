// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "ykmeans.h"

#include <vector>
#include <algorithm>
#include <limits>

namespace {
// 两个充分分离的簇：A 位于原点附近，B 位于 (10,10) 附近。
std::vector<std::vector<float> > makeTwoClusters()
{
    return std::vector<std::vector<float> >{
        {0.0f, 0.0f}, {0.1f, 0.1f}, {0.2f, 0.0f},
        {10.0f, 10.0f}, {9.9f, 10.0f}, {10.0f, 9.8f}
    };
}
}  // namespace

// 构造函数：新实例公共成员应为空。
TEST(KMEANSTest, Constructor_NewInstance_HasEmptyPublicState)
{
    KMEANS<float> km;
    EXPECT_TRUE(km.dataSet.empty());
    EXPECT_TRUE(km.centroids.empty());
    EXPECT_TRUE(km.clusterAssment.empty());
}

// Node 内嵌结构构造函数：保存索引与距离。
TEST(KMEANSTest, Node_Constructor_StoresIndexAndDistance)
{
    KMEANS<float>::tNode node(2, 3.5);
    EXPECT_EQ(node.minIndex, 2);
    EXPECT_DOUBLE_EQ(node.minDist, 3.5);
}

// loadData：载入数据后 dataSet 按行/列保存。
TEST(KMEANSTest, LoadData_ValidDataset_StoresDataAndDimensions)
{
    KMEANS<float> km;
    km.loadData(makeTwoClusters());
    EXPECT_EQ(km.dataSet.size(), 6u);
    EXPECT_EQ(km.dataSet.front().size(), 2u);
    EXPECT_EQ(km.dataSet.back().size(), 2u);
}

// kmeans（双簇）：产生合法的 2 簇分配结果，覆盖 getMinMax / randCent /
// defaultDistEclud 等受保护/静态方法。
TEST(KMEANSTest, Kmeans_TwoClustersSeparated_ProducesValidAssignment)
{
    KMEANS<float> km;
    km.loadData(makeTwoClusters());
    km.kmeans(2);

    ASSERT_EQ(km.clusterAssment.size(), 6u);
    EXPECT_EQ(km.centroids.size(), 2u);

    for (const auto &node : km.clusterAssment) {
        EXPECT_GE(node.minIndex, 0);
        EXPECT_LT(node.minIndex, 2);
        EXPECT_GE(node.minDist, 0.0);
    }

    // 质心坐标应落在数据范围 [0,10] 内。
    for (const auto &center : km.centroids) {
        for (float v : center) {
            EXPECT_GE(v, -0.5f);
            EXPECT_LE(v, 10.5f);
        }
    }
}

// kmeans（单簇）：所有点归入唯一的簇 0。
TEST(KMEANSTest, Kmeans_SingleCluster_AssignsAllPointsToGroupZero)
{
    KMEANS<float> km;
    km.loadData(makeTwoClusters());
    km.kmeans(1);

    ASSERT_EQ(km.clusterAssment.size(), 6u);
    EXPECT_EQ(km.centroids.size(), 1u);
    for (const auto &node : km.clusterAssment) {
        EXPECT_EQ(node.minIndex, 0);
        EXPECT_GE(node.minDist, 0.0);
    }
}

// kmeans 收敛后再次调用仍稳定（覆盖重复执行路径）。
TEST(KMEANSTest, Kmeans_RepeatedRun_StaysConsistentInSize)
{
    KMEANS<float> km;
    km.loadData(makeTwoClusters());
    km.kmeans(2);
    auto firstSize = km.clusterAssment.size();
    km.kmeans(2);
    EXPECT_EQ(km.clusterAssment.size(), firstSize);
    EXPECT_EQ(km.centroids.size(), 2u);
}
