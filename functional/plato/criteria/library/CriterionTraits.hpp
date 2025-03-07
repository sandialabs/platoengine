#ifndef PLATO_CRITERIA_LIBRARY_CRITERIATRAITS
#define PLATO_CRITERIA_LIBRARY_CRITERIATRAITS

namespace plato::criteria::library
{
/// @brief Indicates whether or not a criterion function is a serial or parallel implementation.
///
/// The downstream use of this is whether or not to pass an MPI communicator to the function.
/// @note When adding a new enumerate, ensure that `kNumberOfEnumerates` is last.
enum struct Parallelization
{
    kParallel,
    kSerial,
    kNumberOfEnumerates
};

/// @brief Indicates if a function's return value is a scalar or vector.
/// @note When adding a new enumerate, ensure that `kNumberOfEnumerates` is last.
enum struct FunctionDimension
{
    kScalar,
    kVector,
    kNumberOfEnumerates
};

/// @brief Properties a criterion may have.
struct CriterionTraits
{
    Parallelization mParallelization;
    FunctionDimension mDimension;
};

/// @brief Converts a bool to a Parallelization enum, with `true` corresponding to `kParallel`.
[[nodiscard]] constexpr auto to_parallelization(const bool aIsParallel) -> Parallelization;

/// @brief Converts a bool to a Parallelization enum, with `true` corresponding to `kParallel`.
[[nodiscard]] constexpr auto to_function_dimension(const bool aIsSerial) -> FunctionDimension;

constexpr auto to_parallelization(const bool aIsParallel) -> Parallelization
{
    return aIsParallel ? Parallelization::kParallel : Parallelization::kSerial;
}

constexpr auto to_function_dimension(const bool aIsScalar) -> FunctionDimension
{
    return aIsScalar ? FunctionDimension::kScalar : FunctionDimension::kVector;
}

}  // namespace plato::criteria::library

#endif
