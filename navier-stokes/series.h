#include "omp.h"
#include "mkl.h"
#include <vector>

// C++ includes
#include <iostream>
using namespace std;

// autodiff include
#include <autodiff/reverse.hpp>
using namespace autodiff;

#define calibrated_length 100

double units_per_cell(float lx, int length);

// The time series function which is t ^ 5
var time_series(var t);
var time_series_gradient(var t);

var estimated(var lx, var tsgrad, int length, const float p, 
const float dim_constant, double dt);
double estimated_gradient(var t, var lx, int length, const float prob, 
const float dim_constant, double dt, double &estimated);
