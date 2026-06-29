The filter, `helmholtz_filter`, smooths a design field implicitly by solving a Helmholtz-type elliptic partial differential equation.

This approach smooths design fields defined over unstructured meshes and can also be used to help deal with the artificial attraction of the design field toward the boundaries of the design domain.
```{warning}
The ability to penalize attraction or "stickiness" of the boundary is experimental and requires care to use properly.
```
```{warning}
Insufficiently refined meshes can cause the Helmholtz filter to produce negative densities.
```
```{warning}
The `helmholtz_filter` currently only works with node-centered design fields and is not MPI parallel.
```
