#include "plato/criteria/library/CriterionDocumentation.hpp"

#include <cassert>
#include <iterator>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/CriterionTraits.hpp"
#include "plato/utilities/EnumTable.hpp"

namespace plato::criteria::library
{
namespace
{
constexpr auto kParallelizationLabel = std::string_view{"Parallelization"};
constexpr auto kDimensionalityLabel = std::string_view{"Dimensionality"};
const auto kParallelizationTable = utilities::EnumTable<Parallelization>{{Parallelization::kSerial, "serial"},
                                                                         {Parallelization::kParallel, "parallel"}};
const auto kDimensionalityTable = utilities::EnumTable<FunctionDimension>{{FunctionDimension::kScalar, "scalar"},
                                                                          {FunctionDimension::kVector, "vector"}};

[[nodiscard]] auto app_and_criterion_name(const std::string_view aRegistrationName)
    -> std::pair<std::string_view, std::string_view>
{
    assert(!aRegistrationName.empty());

    auto tSplitName = aRegistrationName | std::views::split(':') |
                      std::views::transform([](const auto& aSubRange)
                                            { return std::string_view{aSubRange.begin(), aSubRange.end()}; });

    const auto tAppName = tSplitName.front();
    const auto tCriterionName =
        std::next(tSplitName.begin()) == tSplitName.end() ? tAppName : *std::next(tSplitName.begin());

    return {tAppName, tCriterionName};
}

[[nodiscard]] auto traits_description(const CriterionTraits& aTraits)
{
    return std::map<std::string, std::string>{
        {std::string{kParallelizationLabel},
         kParallelizationTable.toString(aTraits.mParallelization).value_or("unknown")},
        {std::string{kDimensionalityLabel}, kDimensionalityTable.toString(aTraits.mDimension).value_or("unknown")}};
}

[[nodiscard]] auto documentation(const std::string_view aRegistrationName)
{
    const auto [tAppName, tCriterionName] = app_and_criterion_name(aRegistrationName);
    const auto tAllTraits = criterion_traits(aRegistrationName);
    const auto tToDocumentation =
        tAllTraits | std::views::transform(
                         [=](const auto& aTraits)
                         {
                             return CriterionDocumentation{.mAppName = std::string{tAppName},
                                                           .mCriterionName = std::string{tCriterionName},
                                                           .mDescription = {},
                                                           .mVectorComponents = {},
                                                           .mTraits = traits_description(aTraits)};
                         });
    return std::vector<CriterionDocumentation>{tToDocumentation.begin(), tToDocumentation.end()};
}
}  // namespace

auto criterion_documentation() -> std::vector<CriterionDocumentation>
{
    const auto tRegisteredCriteriaNames = registered_criteria_names();
    auto tToDocumentation =
        tRegisteredCriteriaNames |
        std::views::transform([](const auto& aRegistrationName) { return documentation(aRegistrationName); }) |
        std::views::join | std::views::common;
    return {tToDocumentation.begin(), tToDocumentation.end()};
}
}  // namespace plato::criteria::library
