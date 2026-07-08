The filter, `rz_axisymmetric_revolve_filter`, performs a distance weighted smoothing of the nodal design parameters while also enforcing cylindrical symmetry about the z-axis. 

This filter uses two distinct meshes, a source mesh and a target mesh.
The source mesh is defined in the geometry block through the input `mesh_name` and the target mesh in the filter block as `target_mesh_name`.
The source mesh is defined in the Cartesian XZ-plane, where X is interpreted as the radial coordinate R.
The source mesh may contain positive and negative Z values, but must have non-negative X values.
The source mesh is exported as a three-dimensional Exodus mesh.
The target mesh must contain the source mesh rotated about the Z-axis.
The rotation can be over any angle and need not be a full 360 degrees.

```{include} include/centering-type-tip.md
```
```{include} include/filter-radius-warning.md
```
