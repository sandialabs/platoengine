The filter, `positive_octant_reflect_filter`, performs a distance weighted smoothing of the nodal design parameters while also enforcing reflection symmetry through all three coordinate planes.

This filter uses two distinct meshes, a source mesh and a target mesh.
The source mesh is defined in the geometry block through the input `mesh_name` and the target mesh in the filter block as `target_mesh_name`.
The source mesh must be defined entirely in the positive octant but does not have to touch the coordinate planes.
Any point in the target mesh must be mappable back to the source mesh under reflections through the coordinate planes.

```{include} include/centering-type-tip.md
```
```{include} include/filter-radius-warning.md
```
