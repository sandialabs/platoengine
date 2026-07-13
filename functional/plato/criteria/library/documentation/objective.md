An objective is the specific performance measure that the optimizer seeks to maximize or minimize.
Common objectives include minimizing structural compliance (maximizing stiffness), reducing weight, or minimizing stress concentrations.
The objective function drives the design process toward the most efficient or effective material distribution within the given constraints.

```{tip}
Non-dimensionalize the various physics objectives so that their values are near unity. For example, in a compliance problem scale all moduli and loads by the largest modulus in a problem. Then use `normalize_by_initial_value` to bring the objective to unity to set meaningful weights for aggregators.
```
```{attention}
If multiple objectives are specified, they are summed with weights given by `aggregation_weight`.
```