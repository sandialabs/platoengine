The process manager, `rol_optimization`, uses the rapid optimization library (ROL) in Trilinos to perform an optimization of the defined Plato problem.
A few of the more important optimization parameters can be specified within this input block.
A more complete specification of parameters can be achieved through an auxiliary XML file.

ROL has many options, e.g., algorithm selection, iteration limits, tolerances, etc.
It is essential to select appropriate algorithms and settings based on the problem setup in question, particularly in the case of non-linear constraints.
ROL is based on matrix-free algorithms and so it can handle large-scale problems with dense Jacobian matrices via the application of Jacobian-vector products.
