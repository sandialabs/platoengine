The geometry component, `level_set_topology`, specifies a level-set field defined at the nodes of the provided input `mesh_name`.
The level-set values at each node range between a user-defined min and max and the zero-valued iso-surface of the level set field represents the material boundary, separating solid material (greater than zero) from void (less than zero).
Plato supports an implementation of level-set topology optimization that generates a conforming finite element mesh from this implicit geometry representation for use in evaluating objectives/constraints.
The `level_set_topology` parameterization works well for problems where `density_topology` may struggle due to poorly-resolved interfaces.
