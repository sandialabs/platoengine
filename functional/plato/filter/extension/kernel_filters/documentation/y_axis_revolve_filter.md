The filter, `y_axis_revolve _filter`, in addition to performing a distance weighted smoothing of the nodal design parameters enforces rotational symmetry about the y-axis. 

This filter uses two meshes, a source and a target mesh.
The source mesh is defined in the geometry input and the target mesh in the filter block.
The source `mesh_name` defined in the geometry block must be in the XY-plane and exported as a 2D exodus mesh.

```{important}
Match the `centering_type` to the objective being used.
```
