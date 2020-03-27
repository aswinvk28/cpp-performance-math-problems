# Navier Stokes Momentum Conservation Model

We take a segment of air of length `l`. Using the theory of Air Dynamics and Law of Diffusion, we find the discretised momentum conservation model evaluated from a velocity time series. 

The process of working with the example is to execute the model within specified time by not compromising the accuracy of the model. This mdoel is developed using a benchmark model developed in Python.

The respository link for the benchmark model:

[https://github.com/aswinvk28/cpp-performance-math-problems-benchmark](https://github.com/aswinvk28/cpp-performance-math-problems-benchmark)

![eqn-navier](./navier-stokes/eqn-navier.png)

# Performance Optimization Explanation

[https://github.com/nscalo/cpp-performance-samples](https://github.com/nscalo/cpp-performance-samples)

You can use:

    - Vectorization, or
    
    - Parallelization using Multithreading or multiprocessing, or
    
    - MPI

to minimise the time shown in the repository, without affecting the accuracy of the model

# Accuracy Measurement

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

##  IF   ρ * ( δu / δt ) > quantisation factor   --->       Moving, else Stationary

Original MOS model in QSR lib is based on distance vector, L2 norm, please checkout here

[https://qsrlib.readthedocs.io/en/latest/](https://qsrlib.readthedocs.io/en/latest/)

# TPCC QSR model

The Ternary Point Configuration Calculus (TPCC) model for Navier Stokes relation is determined by the potential term.

##  F  and  ( δF / δx ) projected to determine  --->        Partitions ( "lb", "bl", "fl", "lf", "rf", "fr", "br", "rb" )
