The filter, `helmholtz_filter`, smooths a design field implicitly by solving a Helmholtz-type elliptic partial differential equation.
This filter only works with a node-centered design field

This approach naturally smooths design fields defined over highly unstructured meshes and can also be used to help deal with the artificial attraction of the design field toward the boundaries of the design domain.
Note that the ability to penalize "attraction" or "stickiness" of the boundary is an experimental capability and requires care to use properly.