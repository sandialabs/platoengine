## Criteria

Plato uses optimization algorithms to minimize or maximize an objective function subject to a set of constraints.
Both the objective function and constraints are essentially mathematical functions of a mesh and its material properties.
Because of this similarity, objectives and constraints are both referred to as criteria, and in some cases may be used interchangeably.
Currently, the main restriction is that constraints may be vector or scalar functions, while objectives must be scalar.

The following table lists the currently implemented criteria.
A criterion's app refers to the code in which it is implemented, and thus its input file format.
Also listed are the criteria names (which is used as the `criterion` input in Plato's input file), whether they can be executed in parallel or only in serial, and whether they are scalar or vector functions.
