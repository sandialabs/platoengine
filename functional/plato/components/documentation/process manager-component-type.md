A `process manager` component is generally responsible for assembling the defined `geometry`, `filter`, `objective` and `constraint` components and evaluating the objectives and constraints in some specified way.
For most problems, the main process manager is an optimization algorithm, however, others include gradient validation utilities, parameter sweeps, and mesh writing and processing utilities.
Multiple process managers may be defined and will run in serial in a given order not based on the input deck.
Typically, validation utilities are run first, then optimization algorithms, and finally any post-processing utilities.

```{tip}
Run at least `objective_gradient_check` and `constraint_check` for new problem formulations. It takes less time than being baffled wondering why the optimizer doesn't converge. 
```

The complexity associated with correct formulation of an optimization problem involving multiple software modules
describing geometry, meshing, Finite Element Method discretization, and optimization software has necessitated the
use of several derivative checking modules `*_check`. Ideally, the system would automatically formulate everything
correctly, but there are practical limitations. For example, the software does not automatically know whether the
constraint definition in a separate user input deck is a linear function of the design variables, or whether the
solver tolerances in the physics code are tight enough to compute the criterion with sufficient accuracy on a given mesh.
While standard problem setups generally work, it is recommended to run all of the checks for new setups.
