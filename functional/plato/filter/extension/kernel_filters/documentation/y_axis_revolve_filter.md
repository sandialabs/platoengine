The filter, `y_axis_revolve _filter`, performs a distance weighted smoothing of the nodal design parameters while also enforcing rotational symmetry about the y-axis. 

This filter uses two meshes, a source mesh and a target mesh.
The source mesh is defined in the geometry block as `mesh_name` and the target in the filter block as `target_mesh_name`.
The source mesh must be in the XY-plane and exported as a 2D exodus mesh.
The target mesh must contain the full three dimensional design domain.

```{include} centering_type_tip.md
```
