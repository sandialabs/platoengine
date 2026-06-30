An `objective` component is the specific performance measure that the optimizer seeks to maximize or minimize.
Common objectives include minimizing structural compliance (maximizing stiffness), reducing weight, or minimizing stress concentrations.
The objective function drives the design process toward the most efficient or effective material distribution within the given constraints.
An `objective` should be given a name in its definition, this name will be used in logging messages and other output.
```plato
begin objective compliance
    ...
end
```