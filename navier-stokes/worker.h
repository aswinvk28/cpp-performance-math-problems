#include "omp.h"
#include "mkl.h"
#include <vector>
#include "series.h"

// C++ includes
#include <iostream>
using namespace std;

// autodiff include
#include <autodiff/reverse.hpp>
using namespace autodiff;

double velocity_computed(double u0, double x, double p, double alpha, int length);
double * navier_stokes_ref(double * u, const double * x, double u0, 
const double dt, const double dx, 
const double p, const double alpha, int length);
