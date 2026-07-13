A constraint is a condition that limits the design space or the solution to ensure practical, physical, or performance requirements are met.
Constraints can include limits on material usage, stress, or other structural properties.
They guide the optimization process by restricting feasible designs, helping to achieve an optimal structure that satisfies both performance objectives and real-world limitations.

```{note}
Keep track of whether a constraint is linear or nonlinear when composed with a geometry type to alert the optimizer of this change.
For instance, a volume or volume fraction constraint is linear for `density_topology` but nonlinear for `level_set_topology`.
```
