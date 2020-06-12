#pragma once
#include <complex>
using namespace std;

class CFFT
{
public:
    static void process(complex<float> *Data, int Log2N, int sign);
private:
    CFFT() {}
};


