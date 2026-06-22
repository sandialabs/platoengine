The filter, `z_swept_filter`, in addition to performing a distance weighted smoothing of the nodal design parameters enforces a solution that in uniform in the z-direction.

This filter uses two meshes, a source and a target mesh.
The source mesh is defined in the geometry input and the target mesh in the filter block.
The source mesh must be a 2D mesh defined entirely in the xy-plane. 
