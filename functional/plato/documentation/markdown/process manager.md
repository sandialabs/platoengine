A `process manager` component is generally responsible for assembling the defined `geometry`, `filter`, `objective` and `constraint` components and evaluating the objectives and constraints in some specified way.
For most problems, the main process manager is an optimization algorithm, however, others include gradient validation utilities, parameter sweeps, and mesh writing and processing utilities.
Multiple process managers may be defined and will run in serial in a given order not based on the input deck.
Typically, validation utilities are run first, then optimization algorithms, and finally any post-processing utilities.
