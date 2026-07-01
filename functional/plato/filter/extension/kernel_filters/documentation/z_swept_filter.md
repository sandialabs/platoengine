The filter, `z_swept_filter`, performs a distance weighted smoothing of the nodal design parameters while also enforcing a solution that in uniformly extruded in the z-direction.

This filter uses two distinct meshes, a source mesh and a target mesh.
The source mesh is defined in the geometry block through the input `mesh_name` and the target mesh in the filter block as `target_mesh_name`.
The source mesh must be in the XY-plane and exported as a two-dimensional exodus mesh.
The target mesh's extent in the XY-plane cannot exceed that of the source mesh.

```{include} include/centering-type-tip.md
```
```{include} include/filter-radius-warning.md
```
