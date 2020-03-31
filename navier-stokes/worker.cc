#include "worker.h"

using namespace std;
using namespace autodiff;

const float series_end = 0.3f;
const float series_start = 0.0f;
const float units_end = 1.0f;
const float units_start = 0.0f;
const float prob = 0.01f;
const float dim_constant = 0.1f;

// the units per cell scale used for computing navier-stokes
const double units[] = {
    1.0,2.1092850147397257e-05,4.226689270581119e-05,
    9.287540160585195e-05,0.0003506987413857132,0.0005029256572015584,
    0.0006847812328487635,0.000893069664016366,0.001128520118072629,
    0.0013980538351461291,0.0016929134726524353,0.0020178845152258873,
    0.0023690166417509317,0.0027535269036889076,0.0031596242915838957,
    0.003601870033890009,0.004064870998263359,0.004563262220472097,
    0.005090042948722839,0.005643309094011784,0.005493566859513521,
    0.006838251370936632,0.007479238789528608,0.0073407418094575405,
    0.008856636472046375,0.009453845210373402,0.010277952998876572,
    0.010661286301910877,0.011081334203481674,0.012026455253362656,
    0.013140312395989895,0.01379524264484644,0.015183499082922935,
    0.016322143375873566,0.017194265499711037,0.018084285780787468,
    0.01891518570482731,0.020228413864970207,0.021157003939151764,
    0.022578390315175056,0.023227719590067863,0.02445903606712818,
    0.02534283697605133,0.026577722281217575,0.028430134057998657,
    0.029433393850922585,0.03128552436828613,0.03258998692035675,
    0.03409044072031975,0.03530395030975342,0.036498695611953735,
    0.03823975473642349,0.039395637810230255,0.04077257215976715,
    0.04280084744095802,0.04443170875310898,0.046229004859924316,
    0.04834207147359848,0.049048807471990585,0.0513426698744297,
    0.05268612131476402,0.05511575564742088,0.0565217062830925,
    0.058925844728946686,0.060193222016096115,0.06267963349819183,
    0.06483905762434006,0.0664413794875145,0.06882156431674957,
    0.07029402256011963,0.07218972593545914,0.07447746396064758,
    0.07653576135635376,0.0785461813211441,0.08105304837226868,
    0.08374454081058502,0.0853966698050499,0.08820252865552902,
    0.09006194770336151,0.09274501353502274,0.09525599330663681,
    0.09789932519197464,0.09991414099931717,0.10283256322145462,
    0.10521505028009415,0.10796301066875458,0.1102934405207634,
    0.11273065954446793,0.11558738350868225,0.11833125352859497,
    0.12124882638454437,0.12387516349554062,0.12869063019752502,
    0.1331438422203064,0.13661298155784607,0.1396646946668625,
    0.14319734275341034,0.14646171033382416,0.11126603931188583,
    1.0
};
// interpolating the units gradient using spline interpolation
double units_computed(const double * units, int length, int i, const double dx) {
    double fragment_length = double(length) / calibrated_length;
    double spline_l = double(i - int(i/fragment_length)*fragment_length) / fragment_length;
    double spline_r = 1 - spline_l;
    return (spline_l * units[1] + spline_r * units[0]);
}
// interpolating using np.linspace
var loop_index(var v, const double dx, int length) {
    return v * dx * length / (length-2);
}
// interpolating using linear formula: u0 - xcoord * prob / alpha * length
var velocity_computed(double u0, var x, double p, double alpha, int length) {
    return u0 - x * p / alpha * calibrated_length;
}
// problem of navier stokes computational model
double * navier_stokes_ref(double * u, double u0, 
const double dt, const double dx, 
const double p, const double alpha, int length, double * model) {
    double * grad = (double *) malloc(sizeof(double) * length);
    var diff = 0.0;
    var vi = 0.0;
    var qty = 0.0;
    var v0 = u0;
    var t, lx;
    double estimate, units_interpolated;
    for (int i = 0; i < length; i++) {
        var index = loop_index((var) i, dx, length);
        lx = i;
        vi = velocity_computed(u0, index, p, alpha, length);
        t = i * (series_end - series_start) / (length-1);
        grad[i] = estimated_gradient(t, lx, length, prob, dim_constant, dt, estimate);
        model[i] = estimate;
        units_interpolated = units_computed(&units[int(i*calibrated_length/length)], length, i, dx);
        diff = (vi - v0) * units_interpolated;
        qty = diff/dt - vi * diff/dx; // force
        v0 = vi;
        u[i] = (double) qty;
    }
    return &grad[1];
}
