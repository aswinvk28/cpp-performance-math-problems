#include "series.h"

// power series coefficients theta1 * x + theta2 * x^2 + theta3 * x^3 + ...
double coef[] = {
  1.801559466450699e-07, -1.5454259212518823e-08,
  1.197345374981733e-09, -5.345555361073733e-11,
  1.3845833808638659e-12, -2.0574243813871034e-14,
  1.6256390453625797e-16, -5.28932370890908e-19
};
// The time series function which is t ^ 5
var time_series(var t) {
    return autodiff::reverse::pow(t, 5.0);
}
// gradient calculation which is at (t) usign autodiff reverse
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
// element-wise product terms formula for navier stokes result which represents force
var estimated(var lx, var tsgrad, int length, const float p, 
const float dim_constant, double dt) {
    return (1e11 * (1 - lx / length)) * p * dim_constant * tsgrad;
}
// gradient of force term
double estimated_gradient(var t, var lx, int length, const float prob, 
const float dim_constant, double dt, double &estimate) {
    var tsgrad = power_series(t) + 3.8328476732046615e-12;
    var u = estimated(lx, tsgrad, length, prob, dim_constant, dt);  // the output variable u
    estimate = (double) u;
    auto [u1, u2] = derivatives(u, wrt(lx, tsgrad)); // evaluate the derivative of u with respect to x
    return (double) (u1 + u2);
}