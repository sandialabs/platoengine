The filter, `y_axis_revolve _filter`, in addition to performing a distance weighted smoothing of the nodal design parameters enforces rotational symmetry about the y-axis. 

This filter uses two meshes, a source and a target mesh.
The source mesh is defined in the geometry input and the target mesh in the filter block.
The source mesh must be a 2D mesh defined entirely in the xy-plane with positive x-values.
