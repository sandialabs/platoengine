The filter, `positive_octant_reflect_filter`, performs a distance weighted smoothing of the nodal design parameters while also enforcing reflection symmetry through all three coordinate planes.

This filter uses two meshes, a source mesh and a target mesh.
The source mesh is defined in the geometry block as `mesh_name` and the target in the filter block as `target_mesh_name`.
The source mesh must be entirely in the positive octant.
The target mesh must contain the full symmetric design domain.

```{include} include/centering-type-tip.md
```
```{include} include/filter-radius-warning.md
```
