The filter, `general_plane_filter`, in addition to performing a distance weighted smoothing of the nodal design parameters enforces a reflection symmetry through a user-defined plane. 

This filter uses two meshes, a source and a target mesh.
The source mesh is defined in the geometry input and the target mesh in the filter block.
The source mesh must be on the positive normal side of the plane.
The target mesh must contain the full symmetric design domain.
