The filter, `z_swept_filter`, in addition to performing a distance weighted smoothing of the nodal design parameters enforces a solution that in uniform in the z-direction.

This filter uses two meshes, a source and a target mesh.
The source mesh is defined in the geometry input as `mesh_name` and the target mesh as `target_mesh_name` in the filter block.
The source `mesh_name` defined in the geometry block must be in the XY-plane and exported as a 2D exodus mesh.

```{important}
Match the `centering_type` to the objective being used.
```
