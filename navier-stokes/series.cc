#include "series.h"

using namespace std;
using namespace autodiff;

double units_per_cell(float lx, int length) {
    return 1e1 * (1 - lx * length);
}

// The time series function which is t ^ 5
var time_series(var t) {
    return pow(t, 5.0);
}
auto time_series_gradient(var t) {
    var u = time_series(t);  // the output variable u
    auto [ux] = derivatives(u, wrt(t)); // evaluate the derivative of u with respect to x
    return ux;
}

double estimated(var t, float lx, int length, const float p, 
const float dim_constant, double dt, double dx) {
    return units_per_cell(lx, length) * p * dim_constant * time_series_gradient(t);
}
auto estimated_gradient(var t, float lx, int length, const float prob, 
const float dim_constant, double dt, double dx) {
    var u = estimated(t, lx, length, prob, dim_constant, dt, dx);  // the output variable u
    auto [ux] = derivatives(u, wrt(t)); // evaluate the derivative of u with respect to x
    return ux;
}