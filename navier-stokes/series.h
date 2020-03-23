#include "omp.h"
#include "mkl.h"
#include <vector>

// C++ includes
#include <iostream>
using namespace std;

// autodiff include
#include <autodiff/reverse.hpp>
using namespace autodiff;

double units_per_cell(float lx, int length);

// The time series function which is t ^ 5
var time_series(var t);
auto time_series_gradient(var t);

double estimated(var t, float lx, int length, const float p, 
const float dim_constant, double dt, double dx);
auto estimated_gradient(var t, float lx, int length, const float prob, 
const float dim_constant, double dt, double dx);
