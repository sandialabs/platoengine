The filter, `general_plane_filter`, performs a distance weighted smoothing of the nodal design parameters while also enforcing reflection symmetry through a user-defined plane. 

This filter uses two distinct meshes, a source mesh and a target mesh.
The source mesh is defined in the geometry input block as `mesh_name` and the target in the filter input block as `target_mesh_name`.
The source mesh must be on the positive normal side of the plane, verified during validation.
The target mesh must contain the full symmetric design domain.

```{include} include/centering-type-tip.md
```
```{include} include/filter-radius-warning.md
```
