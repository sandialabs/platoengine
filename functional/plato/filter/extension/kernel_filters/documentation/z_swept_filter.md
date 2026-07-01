The filter, `z_swept_filter`, performs a distance weighted smoothing of the nodal design parameters while also enforcing a solution that in uniformly extruded in the z-direction.

This filter uses two meshes, a source mesh and a target mesh.
The source mesh is defined in the geometry block as `mesh_name` and the target in the filter block as `target_mesh_name`.
The source mesh must be in the XY-plane and exported as a 2D exodus mesh.
The target mesh must contain the full three dimensional design domain.

```{include} include/centering-type-tip.md
```
```{include} include/filter-radius-warning.md
```
