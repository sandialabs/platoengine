The filter, `wedge_filter`, performs a distance weighted smoothing of the nodal design parameters while also enforcing a wedge symmetry about the z-axis.
The wedge-shaped design domain is reflected through planes formed by sweeping the XZ-plane about the Z-axis for a given `wedge_angle`.

This filter uses two meshes, a source mesh and a target mesh.
The source mesh is defined in the geometry block as `mesh_name` and the target in the filter block as `target_mesh_name`.
The source mesh contains the wedge and must be defined in the domain starting on the XZ-plane and formed by sweeping the XZ-plane about the Z-axis by the `wedge_angle`.
The wedge can be any angle that is commensurate with 360 degrees.
The target mesh must contain the full symmetric design domain.

```{important}
Match the `centering_type` to the design field. For density_topology the design field can be defined nodally or piecewise over elements, depending on the physics app used to compute objectives and constraints. For level_set_topology the design field is defined nodally. 
```
