A `constraint` component is a condition that limits the design space or the solution to ensure practical, physical, or performance requirements are met.
Constraints can include limits on material usage, stress, or other structural properties.
They guide the optimization process by restricting feasible designs, helping to achieve an optimal structure that satisfies both performance objectives and real-world limitations.
A `constraint` should be given a name in its definition, this name will be used in logging messages and other output.
```plato
begin constraint volume_fraction
    ...
end
```
