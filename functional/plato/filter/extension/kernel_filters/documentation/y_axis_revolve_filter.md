The filter, `y_axis_revolve _filter`, performs a distance weighted smoothing of the nodal design parameters while also enforcing rotational symmetry about the y-axis. 

This filter uses two distinct meshes, a source mesh and a target mesh.
The source mesh is defined in the geometry block through the input `mesh_name` and the target mesh in the filter block as `target_mesh_name`.
The source mesh must be in the XY-plane and exported as a two-dimensional exodus mesh.
The target mesh must contain the source mesh rotated about the y-axis.
The rotation can be over any angle and need not be a full 360 degrees.

```{include} include/centering-type-tip.md
```
```{include} include/filter-radius-warning.md
```
