#include "omp.h"
#include "mkl.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <functional>
#include <valarray>
#include <algorithm>
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
const double dt = 1e-3;
const double dx = 1e-6;
double quantisation_factor = 3.487607467973763e-05*calibrated_length*dx; // est.max() * 2/3

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

// concordance of the estimated model, computational model using intervals to determine the tpcc
double ModelConcordance(double * u, double * grad,
const int start_index, const int end_index, const int norm, const double dx, 
std::vector<int>& vec, std::map<string, int>& tpcc_map, const float graddx = 1.0f) {
  double sum = 0.0f;
  double numerator = 0.0f;
  double denominator = 0.0f;
  var index1;
  var index2;
  for(int i = start_index; i < end_index-1; i++) {
    index1 = loop_index((var) i, dx, calibrated_length);
    index2 = loop_index((var) (i+1), dx, calibrated_length);
    numerator = std::pow(grad[i] * graddx, norm);
    denominator = std::pow(u[i], norm);
    sum += numerator / denominator;
    if ((denominator <= std::pow(quantisation_factor, norm)) && grad[i] > 0) {
      vec[0] += 1;
    } else {
      vec[1] += 1;
    }
    double angle1 = arg((double) index1, u[i], dx, grad[i] + (grad[i+1] - grad[i]) * dx);
    double angle2 = arg((double) index2, u[i+1], dx, grad[i+1]);
    std::string tpcc = tpcc_string(angle1, angle2);
    tpcc_map[tpcc] += 1;
  }
  return sum;
}

// error precision of the model in design
void ModelPrecision(double * u, double * model, std::vector<double>& precision,
const int start_index = 0, const int end_index = 0, const int multiplier_value = 1) {
  int start = start_index ? start_index : 1;
  int end = end_index ? end_index-1 : length-1;
  for(int i = start; i < end; i++) {
    precision[i-start] = abs(model[i] - u[i]) * multiplier_value;
  }
}

// reliability of the model in failure mode for Scaling 
std::vector<double> ModelReliability(double * model, std::vector<double> precision, int N, 
const int start_index = 0, const int end_index = 0, const int multiplier_value = 1) {
  int start = start_index ? start_index : 1;
  int end = end_index ? end_index-1 : length-1;
  int index = 0;
  std::for_each(precision.begin(), precision.end(), [N, multiplier_value, model, &index](double &x) {
    x = std::pow((1-x/multiplier_value),N);
    index++;
  });
  return precision;
}

// residual error of the model in design with norm-2
double ResidualError(double * u, int length, 
const int start_index, const int end_index, const int norm) {
  double sum = 0.0;
  for(int i = start_index; i < end_index-1; i++) {
    double error = std::pow(u[i+1] - u[i], norm);
    double value = (u[i+1] + u[i]) / 2;
    sum += error/value;
  }
  return sum;
}

// ./app --intervals 2 --iterations 7 --quantisation_factor 0.6 --condition_factor 2
int main(int argc, const char** argv) {

    CmdParserWithHelp   cmd( argc, argv );
    CmdOption<int>   intervals(
        cmd,
        'i',
        "intervals",
        "",
        "Number of intervals as 2^{--intervals}",
        8
    );
    CmdOption<int>   num(
        cmd,
        't',
        "iterations",
        "",
        "Number of iterations as 2^{--iterations} which must be gtreater than calibrated_length = 100",
        -1
    );
    CmdOption<double>   qf(
        cmd,
        'qf',
        "quantisation_factor",
        "",
        "Quantisation Factor input",
        -1.0
    );
    CmdOption<int>   condition(
        cmd,
        'cf',
        "condition_factor",
        "",
        "Condition Number Factor as power",
        2
    );
    CmdOption<int>   multiplier(
        cmd,
        'm',
        "multiplier",
        "",
        "Multiplier for precision",
        1
    );
    cmd.parse();

    const int nIntervals = 1 << intervals.getValue();
    int n = 0;

    if (num.getValue() == -1) {
      n = length;
    } else {
      n = 1 << num.getValue();
    }

    if (qf.getValue() > 0) {
      quantisation_factor = qf.getValue();
    }

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
    printf("\033[1m%5s %15s %15s %20s %20s %15s \t %2s  %2s  %2s  %2s  %2s  %2s  %2s  %2s %15s %15s %15s\n", 
    "Step", "Time, ms", "GSteps/s", "Concordance", "Stationary mode(s)", "Moving mode(s)", 
    __partition_names[0].c_str(),__partition_names[1].c_str(),
    __partition_names[2].c_str(),__partition_names[3].c_str(),
    __partition_names[4].c_str(),__partition_names[5].c_str(),
    __partition_names[6].c_str(),__partition_names[7].c_str(), 
    "Precision", "Residual", "Reliability");
    fflush(stdout);

    double time, dtime, f, df, * grad, precision_sum = 0.0, reliability_sum = 0.0;
    
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

        const int start_index = int(a * length / nIntervals) + 1;
        const int end_index = int((b-1) * length / nIntervals) - 1;
        std::vector<int> vec (2,0);
        std::map<string, int> tpcc_map;

        for (int p = 0; p < __partition_names.size(); p++) {
          tpcc_map[__partition_names[p]] = 0;
        }

        const double concordance = ModelConcordance(u, grad, 
        start_index, end_index, 1, dx, vec, tpcc_map);

        int start = start_index ? start_index : 1;
        int end = end_index ? end_index-1 : length-1;

        std::vector<double> precision (end-start,0.0);
        std::vector<double> reliability (end-start,0.0);

        ModelPrecision(u, &model[0], precision, 
        start_index, end_index, multiplier.getValue());
        std::for_each(precision.begin(), precision.end(), [&precision_sum](double &p) {
          precision_sum += p;
        });
        // residual error calculated with norm-2 similar to condition number for 1 dimensional data
        const double residual = ResidualError(u, length, start_index, end_index, condition.getValue());

        reliability = ModelReliability(&model[0], precision, int(length / calibrated_length), 
        start_index, end_index, multiplier.getValue());
        std::for_each(reliability.begin(), reliability.end(), [&reliability_sum](double &r) {
          reliability_sum += r;
        });

        // Output performance
        printf("%5d %15.3f %15.8f%s %19.5e %15d %15d \t\t %d   %d   %d   %d   %d   %d   %d   %d %15.6e %15.6e %15.6e\n", 
        iInterval, tms, fpps, (iInterval<=skipIntervals?"*":"+"), 
        concordance, vec[0], vec[1], tpcc_map[__partition_names[0]], 
        tpcc_map[__partition_names[1]], tpcc_map[__partition_names[2]], 
        tpcc_map[__partition_names[3]], tpcc_map[__partition_names[4]], 
        tpcc_map[__partition_names[5]], tpcc_map[__partition_names[6]], 
        tpcc_map[__partition_names[7]], 
        precision_sum/precision.size(), residual, 
        reliability_sum/reliability.size());
        fflush(stdout);
    }

    std::vector<double> precision (length-2,0.0);
    std::vector<double> reliability (length-2,0.0);

    ModelPrecision(u, &model[0], precision, 0, 0, multiplier.getValue());
    precision_sum = 0.0;
    std::for_each(precision.begin(), precision.end(), [&precision_sum](double &p) {
      precision_sum += p;
    });
    reliability = ModelReliability(&model[0], precision, 
    int(length / calibrated_length), multiplier.getValue());
    reliability_sum = 0.0;
    std::for_each(reliability.begin(), reliability.end(), [&reliability_sum](double &r) {
      reliability_sum += r;
    });

    printf("Precision: %15.8e\n", precision_sum/precision.size());
    printf("Reliability: %15.8f\n", reliability_sum/reliability.size());

    Stats(time, dtime, nIntervals);
    Stats(f, df, nIntervals);
    printf("-----------------------------------------------------\n");
    printf("\033[1m%s\033[0m\n%8s   \033[42m%8.1f+-%.1f\033[0m   \033[42m%8.1f+-%.1f\033[0m\n",
        "Average performance:", "", time, dtime, f, df);
    printf("-----------------------------------------------------\n");

}