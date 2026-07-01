The filter, `wedge_filter`, performs a distance weighted smoothing of the nodal design parameters while also enforcing a wedge symmetry about the z-axis.
The wedge-shaped design domain is reflected through planes formed by sweeping the XZ-plane about the Z-axis for a given `wedge_angle`.

This filter uses two meshes, a source mesh and a target mesh.
The source mesh is defined in the geometry block through the input `mesh_name` and the target mesh in the filter block as `target_mesh_name`.
The source mesh defines the wedge and must begin on the XZ-plane and sweep the XZ-plane about the Z-axis by the `wedge_angle`.
The wedge can be any angle that subdivides 360 degrees into an even number of sub-domains.
For example, 5, 12, and 22.5 are all valid choices for a `wedge_angle`, but 40, 72, and 120 are not.
The target mesh must contain the full symmetric design domain under the repeated reflections of the source mesh through the wedge planes.

```{include} include/centering-type-tip.md
```
```{include} include/filter-radius-warning.md
```
