#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONDOCUMENTATION
#define PLATO_CRITERIA_LIBRARY_CRITERIONDOCUMENTATION

#include <map>
#include <string>
#include <vector>

namespace plato::criteria::library
{
/// @brief Documentation for a criterion including the app name, criterion name, a brief description, and its registered
/// traits (such as parallelization and dimensionality).
struct CriterionDocumentation
{
    std::string mAppName;
    std::string mCriterionName;
    std::string mDescription;
    std::vector<std::string> mVectorComponents;
    std::map<std::string, std::string> mTraits;

    [[nodiscard]] auto operator==(const CriterionDocumentation& /*unused*/) const noexcept -> bool = default;
};

/// @brief Translates the app configurations in @a aAppConfigurations to a form suitable for documentation.
[[nodiscard]] auto criterion_documentation() -> std::vector<CriterionDocumentation>;
}  // namespace plato::criteria::library
#endif
