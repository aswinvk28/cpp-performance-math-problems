# Navier Stokes Momentum Conservation Model

We take a segment of air of length `l`. Using the theory of Air Dynamics and Law of Diffusion, we find the discretised momentum conservation model evaluated from a velocity time series. 

The process of working with the example is to execute the model within specified time by not compromising the accuracy of the model. This model is developed using a benchmark model developed in Python.

The respository link for the benchmark model is provided below:

[https://github.com/aswinvk28/cpp-performance-math-problems-benchmark](https://github.com/aswinvk28/cpp-performance-math-problems-benchmark)

![eqn-navier](./navier-stokes/eqn-navier.png)

# Performance Optimization

As given in these examples, [https://github.com/nscalo/cpp-performance-samples](https://github.com/nscalo/cpp-performance-samples)

The parallisation techniques which can be used are:

    - Vectorization, or
    
    - Parallelization using Multithreading or multiprocessing, or
    
    - MPI

In order to minimise the time shown in the repository, without affecting the accuracy of the model, the reference function `navier_stokes_ref` compared with the implementation function `navier_stokes` must be optimized for parallel processing

# Measurement of Accuracy

```python

double * navier_stokes_ref(double * u1, double u0, 
const double dt, const double dx, 
const double p, const double alpha, int length, double * model)

```

The implementation:

```python

double * navier_stokes(double * u2, double u0, 
const double dt, const double dx, 
const double p, const double alpha, int length, double * model)

```

The MAE (Mean Absolute Error) = abs(u2 - u1) / length

# Precision Measurement

[Error to Norm Ratio Explained](http://www.math.pitt.edu/~sussmanm/2071Spring08/lab05/index.html#TypesOfErrors)

The Ratio of change of a value `Δx` to the value `x` is termed as: **relative solution error**

```markdown

( relative solution error )         = || (Δx) || / || x ||

```

```markdown

( condition number )

                        k₂(A)       = || A ||₂ || A ||⁻¹

```

# MOS QSR model

The Moving or Stationary model for Navier Stokes relation is determined by Force term.

```mathematica

IF   ρ * ( δu / δt ) > quantisation factor   --->       Moving, else Stationary

```

Original MOS model in QSR lib is based on distance vector, L2 norm, please checkout here [https://qsrlib.readthedocs.io/en/latest/](https://qsrlib.readthedocs.io/en/latest/). 

# TPCC QSR model

The Ternary Point Configuration Calculus (TPCC) model for Navier Stokes relation is determined by the potential term.

```mathematica

F  and  ( δF / δx ) projected to determine  --->        Partitions ( "lb", "bl", "fl", "lf", "rf", "fr", "br", "rb" )

```

# Reliability of the model

```mathematica

Reliability = ( 1 - probability_of_failure ) ^ Number of experiments

```

The reliability of the model refers to scaling up of the initial model from a length of:

`length = 100` to `length = N`

# Model Concordance with QSR

The chosen QSR model here is TPCC. TPCC allocates points in a polar space based on distance metric (L2 norm). The concordance of our model is performed with the original calibrated model and is irrespective of model scaling. 