#include "omp.h"
#include "mkl.h"
#include <iostream>
#include <vector>
#include <array>
#include <map>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstdlib>
#include <string>
#include "math.h"
#include "worker.h"

#include "intel/vx_samples/cmdparser.hpp"

using namespace std;

int length = 100;
const int skipIntervals = 3; // Skip first iteration as warm-up
const double quantisation_factor = 3.487607467973763e-05; // est.max() * 2/3
const double dt = 1e-3;
const double dx = 1e-6;

const std::array<std::string,8> __partition_names = {
  string("lb"),string("bl"),string("fl"),string("lf"),
  string("rf"),string("fr"),string("br"),string("rb")
};
const double __partition_size = 2 * M_PI / __partition_names.size();

double Stats(double & x, double & dx, const int nIntervals) {
  x  /= (double)(nIntervals - skipIntervals);
  dx  = sqrt(dx/double(nIntervals - skipIntervals) - x*x);
}

// return the complex arg
double arg(double x, double y, double graddx, double graddy) {
  return atan2(-1 * (y * graddx - x * graddy), x * graddx + y * graddy);
}

// provides tpcc result performed on temporal differences
std::string tpcc_string(double angle1, double angle2) {
  double anglediff = angle2 - angle1;
  int partition = (int) floor( anglediff / __partition_size ) + 4;
  return __partition_names[partition];
}

// concordance of the estimated model, computational model 
double ModelConcordance(double * u, double * grad, int length, 
const int start_index, const int end_index, const int norm, 
std::vector<int>& vec, std::map<string, int>& tpcc_map, const float graddx = 1.0f) {
  double error = 0.0f;
  double value = 0.0f;
  double numerator = 0.0f;
  double denominator = 0.0f;
  var index1;
  var index2;
  for(int i = start_index; i < end_index; i++) {
    index1 = loop_index((var) i, dx, length);
    index2 = loop_index((var) (i+1), dx, length);
    numerator = std::pow(grad[i] * graddx, norm);
    denominator = std::pow(u[i], norm);
    error += numerator;
    value += denominator;
    if ((denominator <= std::pow(quantisation_factor, norm)) && grad > 0) {
      vec[0] += 1;
    } else {
      vec[1] += 1;
    }
    double angle1 = arg((double) index1, u[i], dx, grad[i] + (grad[i+1] - grad[i]) * dx * length);
    double angle2 = arg((double) index2, u[i+1], dx, grad[i+1]);
    std::string tpcc = tpcc_string(angle1, angle2);
    tpcc_map[tpcc] += 1;
  }
  return error/value;
}

// precision of the model in design
double ModelPrecision(double * u, double * model) {
  double precision = 0.0;
  for(int i = 1; i < length; i++) {
    precision += abs(model[i] - u[i]) / abs(model[i]);
  }
  return 1.0 - precision / length;
}

// resuidual error of the model in design with norm-2
double ResidualError(double * u, int length, 
const int start_index, const int end_index, const int norm) {
  double sum = 0.0;
  for(int i = start_index; i < end_index-1; i++) {
    double error = std::pow(u[i+1] - u[i], norm);
    double value = std::pow((u[i+1] + u[i]) / 2, norm);
    sum += error/value;
  }
  return sum;
}

// ./app 20000 100000
int main(int argc, const char** argv) {

    CmdParserWithHelp   cmd( argc, argv);
    CmdOption<int>   intervals(
        cmd,
        'intervals',
        "intervals",
        "",
        "Number of intervals as 2^{--intervals}",
        8
    );
    CmdOption<int>   num(
        cmd,
        'iterations',
        "iterations",
        "",
        "Number of iterations as 2^{--iterations} which must be gtreater than calibrated_length = 100",
        10
    );
    cmd.parse();

    const int nIntervals = 1 << intervals;
    const int n = 1 << num;

    // rescale length parameter from the input number of iterations
    length = n;

    if(length < 100) {
      printf("The calibrated_length is defined to be 100 and thus cannot be lesser than 100...\n");
      exit(0);
    }

    if ( (int) (length / nIntervals) == 0 ) {
      printf("The calibrated_length is defined to be 100 and thus cannot exceed 50...\n");
      exit(0);
    }

    const double p = 0.02614316; // probability of movement
    const double alpha = 1.5878459; // normalization parameter for probability of movement

    double u0 = 0.26612195; // initial velocity during the analysis of the unit length segment

    double * u = (double *) malloc(sizeof(double)*length);
    double * model = (double *) malloc(sizeof(double)*length);

    printf("\n\033[1mInterval parameters with nIntervals=%d\033[0m\n", nIntervals);
    printf("\033[1m%5s %15s %15s %15s %15s %15s \t %2s  %2s  %2s  %2s  %2s  %2s  %2s  %2s %15s %15s\n", 
    "Step", "Time, ms", "GSteps/s", "Concordance", "Stationary mode(s)", "Moving mode(s)", 
    __partition_names[0].c_str(),__partition_names[1].c_str(),
    __partition_names[2].c_str(),__partition_names[3].c_str(),
    __partition_names[4].c_str(),__partition_names[5].c_str(),
    __partition_names[6].c_str(),__partition_names[7].c_str(), 
    "Precision", "Residual");
    fflush(stdout);

    double time, dtime, f, df, * grad;

    for (int iInterval = 1; iInterval <= nIntervals; iInterval++) {

        const double a = double(iInterval - 1);
        const double b = double(iInterval + 1);

        const double t0 = omp_get_wtime();
        grad = navier_stokes_ref(&u[0], u0, dt, dx, p, alpha, length, &model[0]);
        const double t1 = omp_get_wtime();

        const double ts   = t1-t0; // time in seconds
        const double tms  = ts*1.0e3; // time in milliseconds
        const double fpps = double(n)*1e-9/ts; // performance in steps/s

        if (iInterval > skipIntervals) { // Collect statistics
            time  += tms; 
            dtime += tms*tms;
            f  += fpps;
            df += fpps*fpps;
        }

        const int start_index = (int) a * length / nIntervals;
        const int end_index = (int) (b-1) * length / nIntervals - 1;
        std::vector<int> vec (2,0);
        std::map<string, int> tpcc_map;

        for (int p = 0; p < __partition_names.size(); p++) {
          tpcc_map[__partition_names[p]] = 0;
        }

        const double concordance = ModelConcordance(u, grad, length, 
        start_index, end_index, 1, vec, tpcc_map);

        const double precision = ModelPrecision(u, &model[0]);
        // residual error calculated with norm-2 similar to condition number for 1 dimensional data
        const double residual = ResidualError(u, length, start_index, end_index, 2);

        // Output performance
        printf("%5d %15.3f %15.8f %15.8e%s %15d %15d \t\t %d   %d   %d   %d   %d   %d   %d   %d %15.8f %15.8f\n", 
        iInterval, tms, fpps, (iInterval<=skipIntervals?"*":"+"), 
        concordance, vec[0], vec[1], tpcc_map[__partition_names[0]], 
        tpcc_map[__partition_names[1]], tpcc_map[__partition_names[2]], 
        tpcc_map[__partition_names[3]], tpcc_map[__partition_names[4]], 
        tpcc_map[__partition_names[5]], tpcc_map[__partition_names[6]], 
        tpcc_map[__partition_names[7]], precision, residual);
        fflush(stdout);
    }

    Stats(time, dtime, nIntervals);
    Stats(f, df, nIntervals);
    printf("-----------------------------------------------------\n");
    printf("\033[1m%s\033[0m\n%8s   \033[42m%8.1f+-%.1f\033[0m   \033[42m%8.1f+-%.1f\033[0m\n",
        "Average performance:", "", time, dtime, f, df);
    printf("-----------------------------------------------------\n");
    printf("* - warm-up, not included in average\n\n");

}