The process manager, `objective_gradient_check_without_geometry`, is a diagnostic tool to evaluate the consistency of the objective independently from the geometry and filtering defined in the Plato problem.
It facilitates bisection of any issue between the geometry and filtering from the objective itself.
It systematically checks smaller values of perturbations and computes finite differences which are compared against the computed gradient.
For most problems a log-log plot of the error between the finite difference approximation and the numerical gradient should be V-shaped. 
