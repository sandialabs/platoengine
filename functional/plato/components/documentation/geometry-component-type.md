A `geometry` component is responsible for translating a set of design variables to a mesh that can be evaluated by a physics code defined in an objective or constraint.
Plato supports two main types of geometries: Topology representations and shape representations.
Topology representations are more general and include both level-set and density-based topology methods.
For shape optimization, Plato integrates with Cubit for parameterizing Cubit journal files, which is only available internally.
```{important}
Only a single `geometry` can be defined at a time in an input.
```
