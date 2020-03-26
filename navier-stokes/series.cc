#include "series.h"

double coef[] = {
    2.23021253e-03, -1.72195385e-04,  1.28483935e-05, -5.62817685e-07,
    1.44104377e-08, -2.12475897e-10,  1.66951848e-12, -5.40886780e-15
};
// The time series function which is t ^ 5
var time_series(var t) {
    return autodiff::reverse::pow(t, 5.0);
}
// gradient calculation which is at (t)
var power_series(var t) {
    int degree = sizeof(coef) / sizeof(coef[0]);
    auto u = time_series(t);  // the output variable u
    auto [ut] = derivatives(u, wrt(t)); // evaluate the derivative of u with respect to x
    // reduction sum
    double sum = 0.0;
    for(int i = 1; i <= degree; i++) {
        sum += std::pow(ut, i) * coef[i-1];
    }
    return (var) sum;
}
var estimated(var lx, var tsgrad, int length, const float p, 
const float dim_constant, double dt, double dx) {
    return (1e1 * (1 - lx * length)) * p * dim_constant * tsgrad;
}
double estimated_gradient(var t, var lx, int length, const float prob, 
const float dim_constant, double dt, double dx, double &estimate) {
    var tsgrad = power_series(t);
    var u = estimated(lx, tsgrad, length, prob, dim_constant, dt, dx);  // the output variable u
    estimate = (double) u;
    // std::tuple w = autodiff::forward::wrt(lx, tsgrad);
    // const autodiff::reverse::Wrt<var, var> args{w};
    auto [u1, u2] = derivatives(u, wrt(lx, tsgrad)); // evaluate the derivative of u with respect to x
    return (double) (u1 + u2);
}