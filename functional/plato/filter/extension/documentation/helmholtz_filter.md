The filter, `helmholtz_filter`, smooths a design field implicitly by solving a Helmholtz-type elliptic partial differential equation.
This filter only works with a node-centered design field

This approach naturally smooths design fields defined over highly unstructured meshes and can also be used to help deal with the artificial attraction of the design field toward the boundaries of the design domain.
```{warning}
The ability to penalize attraction or "stickiness" of the boundary is experimental and requires care to use properly.
```
```{warning}
The `helmholtz_filter` currently only works with node-centered design fields.
```
