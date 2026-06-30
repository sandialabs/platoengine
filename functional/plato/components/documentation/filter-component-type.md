A `filter` component is responsible for regularizing topology optimization design problems.
It is recommended to use a filter for both `density_topology` and `level_set_topology` problems but never for `cubit_parameterized_shape`.
In general, filters smooth the design field and reduce high-frequency oscillations or small localized features that can cause numerical issues.

Plato supports kernel filters and Helmholtz filters.
Kernel filters smooth the design field using local weighted averages within a specified radius.
Helmholtz filters smooth the design field by solving a Helmholtz-type PDE.
Both use a filter radius to define the smoothing length scale and generally give similar results.
A kernel filter is recommended as the default unless the design mesh is highly unstructured or has a wide range of element sizes

For `density_topology`, filters are used to reduce mesh dependence, prevent checker-boarding, and provide feature size control.
Without filtering, mesh refinement can produce increasingly fine features, and low-order finite element meshes can produce nonphysical solid/void patterns.

For `level_set_topology`, filters are mainly used to improve optimization stability.
Since sensitivities are defined at the material/void interface, filtering spreads this information over a larger region and can help prevent stalled or oscillatory convergence.

```{tip}
Generally, the filter radius should be between two and three times the average edge length of the mesh.
To facilitate this, all filters have an option to set the filter radius relative to the average mesh edge length; however, for highly non-uniform meshes, this may not be the best option.
For non-uniform meshes, it is recommended to use an absolute radius based on the coarser mesh edge lengths.
```
