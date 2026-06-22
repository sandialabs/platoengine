The filter, `wedge_filter`, in addition to performing a distance weighted smoothing of the nodal design parameters enforces a wedge symmetry about the z-axis.
The symmetry is through mirrored planes for a given wedge_angle.

This filter uses two meshes, a source and a target mesh.
The source mesh is defined in the geometry input and the target mesh in the filter block.
The source mesh must be a 3D mesh contained within the wedge defined by the wedge_angle. 
