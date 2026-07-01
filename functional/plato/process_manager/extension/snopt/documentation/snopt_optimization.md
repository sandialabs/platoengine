The process manager, `snopt_optimization`, uses the sparse nonlinear optimizer library to perform an optimization of the defined Plato problem.
A few of the more important optimization parameters can be specified within this input block.
A more complete specification of parameters can be achieved through an auxiliary ".spc" file.

SNOPT is not open source and is not distributed with Plato.
Users must acquire a SNOPT license and build it via Spack in order to use it with Plato.
Detailed optimization parameters may be specified in a ".spc" file, which is a text file with a format described in the SNOPT user manual.
For problems with non-linear constraints, it is helpful to start at a feasible point.
SNOPT can't be used for problems with large non-sparse constraint Jacobians because it requires the Jacobian explicitly as a sparse matrix.
