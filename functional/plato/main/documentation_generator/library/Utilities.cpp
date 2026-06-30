#include "plato/main/documentation_generator/library/Utilities.hpp"

#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "plato/criteria/library/CriterionDocumentation.hpp"

namespace plato::main::documentation_generator::library
{
namespace
{
[[nodiscard]] auto consolidate_traits(const criteria::library::CriterionDocumentation& aNewDocumentation,
                                      std::map<std::string, std::string> aCurrentTraitsFromMap)
{
    for (auto& tTraitInCriterionMap : aCurrentTraitsFromMap)
    {
        if (const auto tTraitInDocumentation = aNewDocumentation.mTraits.find(tTraitInCriterionMap.first);
            tTraitInDocumentation != aNewDocumentation.mTraits.end() &&
            tTraitInCriterionMap.second != tTraitInDocumentation->second)
        {
            tTraitInCriterionMap.second += std::string{", "} + tTraitInDocumentation->second;
        }
    }
    return aCurrentTraitsFromMap;
}
}  // namespace

auto consolidated_criteria_documentation(
    const std::vector<criteria::library::CriterionDocumentation>& aCriteriaDocumentation)
    -> std::vector<criteria::library::CriterionDocumentation>
{
    using AppCriterionPair = std::pair<std::string_view, std::string_view>;

    auto tCriteriaMap = std::map<AppCriterionPair, criteria::library::CriterionDocumentation>{};

    for (const auto& tDocumentation : aCriteriaDocumentation)
    {
        if (const auto tCriterionInMap =
                tCriteriaMap.find(AppCriterionPair{tDocumentation.mAppName, tDocumentation.mCriterionName});
            tCriterionInMap != tCriteriaMap.end())
        {
            tCriterionInMap->second.mTraits =
                consolidate_traits(tDocumentation, std::move(tCriterionInMap->second.mTraits));
        }
        else
        {
            tCriteriaMap.emplace(AppCriterionPair{tDocumentation.mAppName, tDocumentation.mCriterionName},
                                 tDocumentation);
        }
    }
    auto tToConsolidatedDocumentation = tCriteriaMap | std::views::values;
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return std::vector<criteria::library::CriterionDocumentation>(tToConsolidatedDocumentation.begin(),
                                                                  tToConsolidatedDocumentation.end());
}

auto myst_table_footer() -> std::string { return {"```"}; }
}  // namespace plato::main::documentation_generator::library
