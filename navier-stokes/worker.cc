#include "worker.h"

using namespace std;
using namespace autodiff;

const float series_end = 2.0;
const float series_start = 0.0;
const float units_end = 1.0;
const float units_start = 0.0;
const float prob = 0.01f;
const float dim_constant = 0.01f;
const int calibrated_length = 100;

// the units per cell scale used for computing navier-stokes
const double units[] = {
       1.00000000e+00, 2.10928501e-05, 4.22668927e-05, 9.28754016e-05,
       3.50698741e-04, 5.02925657e-04, 6.84781233e-04, 8.93069664e-04,
       1.12852012e-03, 1.39805384e-03, 1.69291347e-03, 2.01788452e-03,
       2.36901664e-03, 2.75352690e-03, 3.15962429e-03, 3.60187003e-03,
       4.06487100e-03, 4.56326222e-03, 5.09004295e-03, 5.64330909e-03,
       5.49356686e-03, 6.83825137e-03, 7.47923879e-03, 7.34074181e-03,
       8.85663647e-03, 9.45384521e-03, 1.02779530e-02, 1.06612863e-02,
       1.10813342e-02, 1.20264553e-02, 1.31403124e-02, 1.37952426e-02,
       1.51834991e-02, 1.63221434e-02, 1.71942655e-02, 1.80842858e-02,
       1.89151857e-02, 2.02284139e-02, 2.11570039e-02, 2.25783903e-02,
       2.32277196e-02, 2.44590361e-02, 2.53428370e-02, 2.65777223e-02,
       2.84301341e-02, 2.94333939e-02, 3.12855244e-02, 3.25899869e-02,
       3.40904407e-02, 3.53039503e-02, 3.64986956e-02, 3.82397547e-02,
       3.93956378e-02, 4.07725722e-02, 4.28008474e-02, 4.44317088e-02,
       4.62290049e-02, 4.83420715e-02, 4.90488075e-02, 5.13426699e-02,
       5.26861213e-02, 5.51157556e-02, 5.65217063e-02, 5.89258447e-02,
       6.01932220e-02, 6.26796335e-02, 6.48390576e-02, 6.64413795e-02,
       6.88215643e-02, 7.02940226e-02, 7.21897259e-02, 7.44774640e-02,
       7.65357614e-02, 7.85461813e-02, 8.10530484e-02, 8.37445408e-02,
       8.53966698e-02, 8.82025287e-02, 9.00619477e-02, 9.27450135e-02,
       9.52559933e-02, 9.78993252e-02, 9.99141410e-02, 1.02832563e-01,
       1.05215050e-01, 1.07963011e-01, 1.10293441e-01, 1.12730660e-01,
       1.15587384e-01, 1.18331254e-01, 1.21248826e-01, 1.23875163e-01,
       1.28690630e-01, 1.33143842e-01, 1.36612982e-01, 1.39664695e-01,
       1.43197343e-01, 1.46461710e-01, 1.11266039e-01, 1.00000000e+00
};
var loop_index(var v, const double dx, int length) {
    return v * dx * calibrated_length/length * calibrated_length / (calibrated_length-2);
}
var velocity_computed(double u0, var x, double p, double alpha, int length) {
    return u0 - x * p / alpha * length;
}
double * navier_stokes_ref(double * u, double u0, 
const double dt, const double dx, 
const double p, const double alpha, int length, double * model) {
    double * grad = (double *) malloc(sizeof(double) * length);
    var diff = 0.0;
    var vi = 0.0;
    var qty = 0.0;
    var v0 = u0;
    var t;
    double estimate;
    for (int i = 0; i < length; i++) {
        var index = loop_index((var) i, dx, length);
        vi = velocity_computed(u0, index, p, alpha, length);
        t = (series_start + i * ( series_end - series_start ) / (length-1));
        grad[i] = estimated_gradient(t, index, length, prob, dim_constant, dt, dx, estimate);
        diff = (vi - v0) * units[i];
        qty = diff/dt - v0 * diff/dx; // force
        v0 = vi;
        u[i] = (double) qty;
        model[i] = estimate;
    }
    return &grad[1];
}
