#ifndef PLATO_CRITERIA_LIBRARY_CRITERIATRAITS
#define PLATO_CRITERIA_LIBRARY_CRITERIATRAITS

#include <cstdint>

#include "plato/utilities/EnumIndexing.hpp"

namespace plato::criteria::library
{
/// @brief Indicates whether or not a criterion function is a serial or parallel implementation.
///
/// The downstream use of this is whether or not to pass an MPI communicator to the function.
/// @note When adding a new enumerate, ensure that `kNumberOfEnumerates` is last.
enum struct Parallelization : std::uint8_t
{
    kParallel,
    kSerial,
    kNumberOfEnumerates
};

/// @brief Indicates if a function's return value is a scalar or vector.
/// @note When adding a new enumerate, ensure that `kNumberOfEnumerates` is last.
enum struct FunctionDimension : std::uint8_t
{
    kScalar,
    kVector,
    kNumberOfEnumerates
};

/// @brief Concept that requires a type to be one of the criterion traits
template <typename Trait>
concept CriterionTrait = std::same_as<Trait, Parallelization> || std::same_as<Trait, FunctionDimension>;

/// @brief Properties a criterion may have.
struct CriterionTraits
{
    Parallelization mParallelization;
    FunctionDimension mDimension;
};

/// @brief Converts a bool to a Parallelization enum, with `true` corresponding to `kParallel`.
[[nodiscard]] constexpr auto to_parallelization(const bool aIsParallel) -> Parallelization;

/// @brief Converts a bool to a FunctionDimension enum, with `true` corresponding to `kScalar`.
[[nodiscard]] constexpr auto to_function_dimension(const bool aIsScalar) -> FunctionDimension;

/// @brief Returns an index into FactoryRegistrars corresponding to the traits in @a aCriterionTraits.
[[nodiscard]] constexpr auto trait_index(const CriterionTraits aCriterionTraits) -> std::size_t;

/// @brief Returns an index into FactoryRegistrars corresponding to the traits @a aParallelization and @a aDimension.
[[nodiscard]] constexpr auto trait_index(Parallelization aParallelization, FunctionDimension aDimension) -> std::size_t;

/// @brief Returns the criterion traits associated with index @a aFactoryIndex
[[nodiscard]] constexpr auto traits_from_index(std::size_t aFactoryIndex) -> CriterionTraits;

/// @brief Returns the number of combinations of criterion factories (all combinations of criterion traits)
[[nodiscard]] constexpr auto number_of_traits() -> std::size_t;

/// @brief Returns true if @a aTraits has a trait that matches @a aTrait.
template <CriterionTrait Trait>
[[nodiscard]] constexpr auto has_trait(const CriterionTraits aTraits, const Trait aTrait) -> bool;

constexpr auto to_parallelization(const bool aIsParallel) -> Parallelization
{
    return aIsParallel ? Parallelization::kParallel : Parallelization::kSerial;
}

constexpr auto to_function_dimension(const bool aIsScalar) -> FunctionDimension
{
    return aIsScalar ? FunctionDimension::kScalar : FunctionDimension::kVector;
}

constexpr auto trait_index(const CriterionTraits aCriterionTraits) -> std::size_t
{
    return utilities::enum_index(aCriterionTraits.mParallelization, aCriterionTraits.mDimension);
}

constexpr auto trait_index(const Parallelization aParallelization, const FunctionDimension aDimension) -> std::size_t
{
    return trait_index(CriterionTraits{aParallelization, aDimension});
}

constexpr auto traits_from_index(const std::size_t aFactoryIndex) -> CriterionTraits
{
    const auto tEnumTuple = utilities::enums_from_index<Parallelization, FunctionDimension>(aFactoryIndex);
    return CriterionTraits{/*.mParallelization=*/std::get<0>(tEnumTuple), /*.mDimension=*/std::get<1>(tEnumTuple)};
}

constexpr auto number_of_traits() -> std::size_t
{
    return utilities::number_of_enumerates<Parallelization, FunctionDimension>();
}

template <CriterionTrait Trait>
constexpr auto has_trait(const CriterionTraits aTraits, const Trait aTrait) -> bool
{
    if constexpr (std::is_same_v<Trait, Parallelization>)
    {
        return aTraits.mParallelization == aTrait;
    }
    else if constexpr (std::is_same_v<Trait, FunctionDimension>)
    {
        return aTraits.mDimension == aTrait;
    }
    // return false;
}

}  // namespace plato::criteria::library

#endif
