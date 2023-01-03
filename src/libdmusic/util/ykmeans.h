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
#ifndef _YKMEANS_H_
#define _YKMEANS_H_

#include <cstdlib>	//for rand()
#include <vector>	//for vector<>
#include <time.h>	//for srand
#include <limits.h>	//for INT_MIN INT_MAX
using namespace std;

template<typename T>
class KMEANS
{
protected:
    //colLen:the dimension of vector;rowLen:the number of vectors
    int colLen, rowLen;
    //count to be clustered
    int k;
    //mark the min and max value of a array
    typedef struct MinMax
    {
        T Min;
        T Max;
        MinMax(T min, T max) :Min(min), Max(max) {}
    }tMinMax;
    //distance function
    //reload this function if necessary
    double (*distEclud)(vector<T> &v1, vector<T> &v2);

    //get the min and max value in idx-dimension of dataSet
    tMinMax getMinMax(int idx)
    {
        T min, max;
        dataSet[0].at(idx) > dataSet[1].at(idx) ? (max = dataSet[0].at(idx), min = dataSet[1].at(idx)) : (max = dataSet[1].at(idx), min = dataSet[0].at(idx));

        for (int i = 2; i < rowLen; i++)
        {
            if (dataSet[i].at(idx) < min)	min = dataSet[i].at(idx);
            else if (dataSet[i].at(idx) > max) max = dataSet[i].at(idx);
            else continue;
        }

        tMinMax tminmax(min, max);
        return tminmax;
    }
    //generate clusterCount centers randomly
    void randCent(int clusterCount)
    {
        this->k = clusterCount;
        //init centroids
        centroids.clear();
        vector<T> vec(colLen, 0);
        for (int i = 0; i < k; i++)
            centroids.push_back(vec);

        //set values by column
        srand(time(NULL));
        for (int j = 0; j < colLen; j++)
        {
            tMinMax tminmax = getMinMax(j);
            T rangeIdx = tminmax.Max - tminmax.Min;
            for (int i = 0; i < k; i++)
            {
                /* generate float data between 0 and 1 */
                centroids[i].at(j) = tminmax.Min + rangeIdx * (rand() / (double)RAND_MAX);
            }
        }
    }
    //default distance function ,defined as dis = (x-y)'*(x-y)
    static double defaultDistEclud(vector<T> &v1, vector<T> &v2)
    {
        double sum = 0;
        int size = v1.size();
        for (int i = 0; i < size; i++)
        {
            sum += (v1[i] - v2[i])*(v1[i] - v2[i]);
        }
        return sum;
    }

public:
    typedef struct Node
    {
        int minIndex; //the index of each node
        double minDist;
        Node(int idx, double dist) :minIndex(idx), minDist(dist) {}
    }tNode;

    KMEANS(void)
    {
        k = 0;
        colLen = 0;
        rowLen = 0;
        distEclud = defaultDistEclud;
    }
    ~KMEANS(void){}
    //data to be clustered
    vector< vector<T> > dataSet;
    //cluster centers
    vector< vector<T> > centroids;
    //mark which cluster the data belong to
    vector<tNode>  clusterAssment;

    //load data into dataSet
    void loadData(vector< vector<T> > data)
    {
        this->dataSet = data; //kmeans do not change the original data;
        this->rowLen = data.capacity();
        this->colLen = data.at(0).capacity();
    }
    //running the kmeans algorithm
    void kmeans(int clusterCount)
    {
        this->k = clusterCount;

        //initial clusterAssment
        this->clusterAssment.clear();
        tNode node(-1, -1);
        for (int i = 0; i < rowLen; i++)
            clusterAssment.push_back(node);

        //initial cluster center
        this->randCent(clusterCount);

        bool clusterChanged = true;
        //the termination condition can also be the loops less than	some number such as 1000
        while (clusterChanged)
        {
            clusterChanged = false;
            for (int i = 0; i < rowLen; i++)
            {
                int minIndex = -1;
                double minDist = INT_MAX;
                for (int j = 0; j < k; j++)
                {
                    double distJI = distEclud(centroids[j], dataSet[i]);
                    if (distJI < minDist)
                    {
                        minDist = distJI;
                        minIndex = j;
                    }
                }
                if (clusterAssment[i].minIndex != minIndex)
                {
                    clusterChanged = true;
                    clusterAssment[i].minIndex = minIndex;
                    clusterAssment[i].minDist = minDist;
                }
            }

            //step two : update the centroids
            for (int cent = 0; cent < k; cent++)
            {
                vector<T> vec(colLen, 0);
                int cnt = 0;
                for (int i = 0; i < rowLen; i++)
                {
                    if (clusterAssment[i].minIndex == cent)
                    {
                        ++cnt;
                        //sum of two vectors
                        for (int j = 0; j < colLen; j++)
                        {
                            vec[j] += dataSet[i].at(j);
                        }
                    }
                }

                //mean of the vector and update the centroids[cent]
                for (int i = 0; i < colLen; i++)
                {
                    if (cnt != 0)	vec[i] /= cnt;
                    centroids[cent].at(i) = vec[i];
                }
            }
        }
    }
};

#endif // _YKMEANS_H_

