// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
// Graph-driven unit tests for src/libdmusic/util/ykmeans.h (template KMEANS<T>).
#include <gtest/gtest.h>
#include <vector>
#include "util/ykmeans.h"

TEST(KmeansTest, Construct_Default_NoThrow) {
    KMEANS<float> k;
    EXPECT_NO_THROW((void)k.dataSet.size());
}
TEST(KmeansTest, LoadData_FourPoints_HoldsData) {
    KMEANS<float> k;
    std::vector<std::vector<float>> data = {{0,0},{1,1},{2,2},{3,3}};
    k.loadData(data);
    EXPECT_EQ(k.dataSet.size(), static_cast<size_t>(4));
}
TEST(KmeansTest, Kmeans_TwoClusters_NoThrow) {
    KMEANS<float> k;
    std::vector<std::vector<float>> data = {{0,0},{0.1f,0.1f},{5,5},{5.1f,5.1f}};
    k.loadData(data);
    EXPECT_NO_THROW(k.kmeans(2));
    EXPECT_EQ(k.clusterAssment.size(), static_cast<size_t>(4));
}
TEST(KmeansTest, Kmeans_CentroidsPopulated_NoThrow) {
    KMEANS<float> k;
    std::vector<std::vector<float>> data = {{0,0},{1,1},{10,10},{11,11}};
    k.loadData(data);
    k.kmeans(2);
    EXPECT_EQ(k.centroids.size(), static_cast<size_t>(2));
}
TEST(KmeansTest, DoubleKmeans_TwoClusters_NoThrow) {
    KMEANS<double> k;
    std::vector<std::vector<double>> data = {{0,0},{1,1},{9,9},{10,10}};
    k.loadData(data);
    EXPECT_NO_THROW(k.kmeans(2));
}
TEST(KmeansTest, NodeStruct_Construct_HoldsValues) {
    KMEANS<float>::Node n(2, 3.5);
    EXPECT_EQ(n.minIndex, 2);
    EXPECT_DOUBLE_EQ(n.minDist, 3.5);
}
