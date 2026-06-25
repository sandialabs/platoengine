The filter, `general_plane_filter`, performs a distance weighted smoothing of the nodal design parameters while also enforcing reflection symmetry through a user-defined plane. 

This filter uses two meshes, a source mesh and a target mesh.
The source mesh is defined in the geometry block as `mesh_name` and the target in the filter block as `target_mesh_name`.
The source mesh must be on the positive normal side of the plane.
The target mesh must contain the full symmetric design domain.

```{important}
Match the `centering_type` to the design field. For density_topology the design field can be defined nodally or piecewise over elements, depending on the physics app used to compute objectives and constraints. For level_set_topology the design field is defined nodally. 
```
