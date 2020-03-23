#include "omp.h"
#include "mkl.h"
#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstdlib>
#include "worker.h"

const int nTrials = 10;
const int skipTrials = 3; // Skip first iteration as warm-up

double Stats(double & x, double & dx) {
  x  /= (double)(nTrials - skipTrials);
  dx  = sqrt(dx/double(nTrials - skipTrials) - x*x);
}

double Accuracy(double * u, const double a, const double b, int length) {
  const double I0 = InverseDerivative(b) - InverseDerivative(a);
  double error = 0.0;
  double norm = 0.0;
  for(int i = 0; i < length; i++) {
    error += (u[i] - I0)*(u[i] - I0);
    norm += (u[i] + I0)*(u[i] + I0);
  }
  return error/norm;
}

int main(int argc, char** argv) {

    int length = 100;

    double dt = 1e-6;
    double dx = 1e-3;

    double p = 0.02614316; // probability of movement
    double alpha = 1.5878459; // normalization parameter for probability of movement

    double u0 = 0.26612195; // initial velocity during the analysis of the unit length segment

    // the velocity scale used for computing the flow model
    const double x[] = {
        0.0, 0.01, 0.02, 0.03, 0.04,
        0.05, 0.05999999, 0.06999999, 0.07999999, 0.08999999,
        0.09999999, 0.10999998, 0.11999998, 0.12999998, 0.13999999,
        0.14999999, 0.16, 0.17, 0.18, 0.19000001,
        0.20000002, 0.21000002, 0.22000003, 0.23000003, 0.24000004,
        0.25000003, 0.26000002, 0.27, 0.28, 0.29,
        0.29999998, 0.30999997, 0.31999996, 0.32999995, 0.33999994,
        0.34999993, 0.35999992, 0.36999992, 0.3799999, 0.3899999,
        0.3999999, 0.40999988, 0.41999987, 0.42999986, 0.43999985,
        0.44999984, 0.45999983, 0.46999982, 0.4799998, 0.4899998,
        0.4999998, 0.5099998, 0.5199998, 0.5299998, 0.5399998,
        0.5499998, 0.55999976, 0.56999975, 0.57999974, 0.58999974,
        0.5999997, 0.6099997, 0.6199997, 0.6299997, 0.6399997,
        0.6499997, 0.65999967, 0.66999966, 0.67999965, 0.68999964,
        0.69999963, 0.7099996, 0.7199996, 0.7299996, 0.7399996,
        0.7499996, 0.7599996, 0.76999956, 0.77999955, 0.78999954,
        0.79999954, 0.8099995,0.8199995, 0.8299995, 0.8399995,
        0.8499995, 0.8599995, 0.86999947, 0.87999946, 0.88999945,
        0.89999944, 0.90999943, 0.9199994, 0.9299994, 0.9399994,
        0.9499994, 0.9599994, 0.9699994, 0.97999936, 0.98999935
    };

    double * u = (double *) malloc(sizeof(double)*(length-1));

    const int n = 1000*100;

    printf("\n\033[1mNumerical integration with n=%d\033[0m\n", n);
    printf("\033[1m%5s %15s %15s %15s\033[0m\n", "Step", "Time, ms", "GSteps/s", "Accuracy"); fflush(stdout);

    double time, dtime, f, df, * grad;

    for (int iTrial = 1; iTrial <= nTrials; iTrial++) {

        const double a = double(iTrial - 1);
        const double b = double(iTrial + 1);
    
        const double t0 = omp_get_wtime();
        grad = navier_stokes_ref(u, x, u0, dt, dx, p, alpha, length);
        const double t1 = omp_get_wtime();

        const double ts   = t1-t0; // time in seconds
        const double tms  = ts*1.0e3; // time in milliseconds
        const double fpps = double(n)*1e-9/ts; // performance in steps/s

        if (iTrial > skipTrials) { // Collect statistics
            time  += tms; 
            dtime += tms*tms;
            f  += fpps;
            df += fpps*fpps;
        }

        const double acc = Accuracy(u, a, b, length);

        // Output performance
        printf("%5d %15.3f %15.3f %15.3e%s\n", 
        iTrial, tms, fpps, acc, (iTrial<=skipTrials?"*":""));
        fflush(stdout);

    }

    Stats(time, dtime);
    Stats(f, df);
    printf("-----------------------------------------------------\n");
    printf("\033[1m%s\033[0m\n%8s   \033[42m%8.1f+-%.1f\033[0m   \033[42m%8.1f+-%.1f\033[0m\n",
        "Average performance:", "", time, dtime, f, df);
    printf("-----------------------------------------------------\n");
    printf("* - warm-up, not included in average\n\n");

}