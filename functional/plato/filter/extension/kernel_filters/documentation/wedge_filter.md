The filter, `wedge_filter`, in addition to performing a distance weighted smoothing of the nodal design parameters enforces a wedge symmetry about the z-axis.
The symmetry is through mirrored planes for a given wedge_angle.

This filter uses two meshes, a source and a target mesh.
The source mesh is defined in the geometry input and the target mesh in the filter block.
The wedge must be defined in the domain starting on the XZ-plane and that formed by sweeping the XZ-plane about the Z-axis by the `wedge_angle`.

```{important}
Match the `centering_type` to the objective being used.
```
