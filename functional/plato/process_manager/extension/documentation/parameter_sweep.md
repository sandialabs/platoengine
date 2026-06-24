The process manager, `parameter_sweep`, allows for the objective to be evaluated along a defined set of design variables on a Cartesian product grid.
Its primary use is with geometry inputs that have very few design parameters like `cubit_parameterized_shape`.

Output is to a specified text file, using indices into the aprepro variables in the order specified for `cubit_parameterized_shape` and Matlab-style range notation.
