The process manager, `gradient_check`, is a diagnostic tool to evaluate the consistency of the objective defined in the Plato problem.
It systematically checks smaller values of perturbations and computes finite differences which are compared against the computed gradient.
For most problems a log-log plot of the error between the finite difference approximation and the numerical gradient should be V-shaped.
At first smaller perturbations improve accuracy until precision/truncation/roundoff error dominate and then error grows with increasingly small step sizes. 
