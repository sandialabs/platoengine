The filter, `positive_octant_reflect_filter`, in addition to performing a distance weighted smoothing of the nodal design parameters enforces a three-way reflection symmetry through the coordinate planes.

This filter uses two meshes, a source and a target mesh.
The source mesh is defined in the geometry input and the target mesh in the filter block.
The source `mesh_name` defined in the geometry block must be entirely in the positive octant.

```{important}
Match the `centering_type` to the objective being used.
```
