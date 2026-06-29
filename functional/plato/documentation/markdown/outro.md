
```{image} _static/Fin.png
:alt: Compliance
:width: 600px
:align: right
```

## Input Blocks

A Plato input deck is organized into blocks using `begin` and `end`:

```plato
begin <component>
    <keyword> <value>
end
```

The `<component>` names the type of block being defined, such as a geometry, filter, objective, constraint, or process manager.
The components documented in this user manual correspond to the valid block names.
Within a block, each option is written as a `<keyword>` followed by a `<value>`.
Some more complicated components may use a list of values or values with additional keywords. Plato input decks do **not** use `=` signs or assignment syntax.

Examples:

```plato
begin kernel_filter
    radius 1.0
    centering_type node
end
```

```plato
begin rol_optimization
    max_iterations 1000
end
```

Comments can be used in an input deck with the `#` or `/* ... */`:
```plato
begin density_topology # comment for geometry input
    mesh_name input_mesh.exo
    initial_density_value /* the starting value for the field */ 0.5
end
```

## Running Plato

Run Plato in serial by passing an input file to the `plato` executable:

```bash
plato input.i
```

For additional command-line options, run:

```bash
plato --help
```

For mixed MPI/OpenMP runs, use the `plato-parallel` launch script:

```bash
plato-parallel [-n <number-of-ranks>] [-t <number-of-threads>] [-s <shared,dedicated>] <input-file>
```

where:

- `-n <number-of-ranks>` specifies the number of MPI ranks.
- `-t <number-of-threads>` specifies the number of OpenMP threads per rank.
- `-s <shared,dedicated>` specifies whether hardware resources are shared or dedicated.
- `<input-file>` is the Plato input file to run.
