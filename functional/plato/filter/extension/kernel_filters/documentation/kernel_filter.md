The filter, `kernel_filter`, takes node-centered design variables and performs a distance-weighted sum to smooth design parameters.
These can be mapped to either node-centered or element-centered design fields depending on the element_centering parameter. 

Note that for highly unstructured meshes with a wide range of element sizes, this approach may result in unwanted artifacts and the filter radius must be chosen carefully.

```{important}
Match the `centering_type` to the design field. For density_topology the design field can be defined nodally or piecewise over elements, depending on the physics app used to compute objectives and constraints. For level_set_topology the design field is defined nodally. 
```
