The process manager, `sensitivity_check`, is a diagnostic tool to evaluate the consistency of the sensitivities of the design variables in the geometry and filtering defined in the Plato problem.
It facilitates bisection of any issue between the geometry and filtering from the objective itself.
It systematically checks smaller values of perturbations and computes finite differences which are compared against the computed gradient.
For most problems a log-log plot of the error between the finite difference approximation and the numerical gradient should be V-shaped.
