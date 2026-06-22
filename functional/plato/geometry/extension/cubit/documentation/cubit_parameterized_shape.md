The geometry component, `cubit_parameterized_shape`, uses a Cubit journal file to generate a mesh based on Aprepro design parameters.
These design parameters directly control the finite element mesh for the analysis.

The Cubit geometry component specifies that the geometry of the optimization problem will be defined by a parametrized CAD model and that the design variables will be the CAD parameters.
The optimizer will modify the CAD parameters to achieve the optimized geometry for the given objectives and constraints.
At each iteration a Cubit journal file is run to generate the new CAD model and corresponding finite element mesh to be used for calculating objectives and constraints.
Unlike the density and level set geometry representations that allow for free-form material distribution, the Cubit geometry representation only allows geometry modification/optimization by modifying the pre-defined CAD parameters.
The Cubit geometry component is very useful when a more constrained geometry is needed--a good example being if you need to be able to subtractively manufacture the final design.
