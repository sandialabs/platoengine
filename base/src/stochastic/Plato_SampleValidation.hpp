#pragma once

#include <optional>
#include <string>
#include <vector>

namespace Teuchos
{
class ParameterList;
}

namespace Plato
{

class Interface;
struct StochasticSampleSharedDataNames;

/// Checks that each sample has the same distributions defined and all are unique. 
/// Distributions are used to identify parameters and map between parameter indices.
/// @return An error message if an error was found, `std::nullopt` otherwise.
[[nodiscard]] std::optional<std::string> are_distributions_consistent_for_all_samples(
    const std::vector<StochasticSampleSharedDataNames>& aSampleData);

/// @return The first distribution in @a aSampleData that cannot be found in @a aDistributionsParameterList.
[[nodiscard]] std::optional<std::string> undefined_distributions(
    const std::vector<StochasticSampleSharedDataNames>& aSampleData,
    const Teuchos::ParameterList& aDistributionParameterList);

/// Checks that all shared data parameters named in @a aSampleData are defined in the DataLayer
/// object held by @a aInterface.
/// @return The first parameter name in @a aSampleData not found in @a aInterface.
[[nodiscard]] std::optional<std::string> undefined_parameters(
    const std::vector<StochasticSampleSharedDataNames>& aSampleData,
    const Plato::Interface& aInterface);
}
