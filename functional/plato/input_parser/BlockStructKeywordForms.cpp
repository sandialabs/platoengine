#include "plato/input_parser/BlockStructKeywordForms.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/LevenshteinDistance.hpp"

namespace plato::input_parser::detail
{
namespace
{
constexpr auto kClosestKeywordFormSyntax = std::string_view{"Expected one of:"};
}  // namespace

auto format_keyword_forms(const std::vector<BlockStructKeywordForm>& aKeywordForms) -> std::vector<std::string>
{
    const auto tMaxKeywordNameLengthIterator =
        std::max_element(aKeywordForms.begin(), aKeywordForms.end(), [](const auto& aLeft, const auto& aRight)
                         { return aLeft.mName.size() < aRight.mName.size(); });

    const auto tMaxKeywordNameLength = tMaxKeywordNameLengthIterator == aKeywordForms.end()
                                           ? std::size_t{0}
                                           : tMaxKeywordNameLengthIterator->mName.size();

    auto tFormattedKeywordForms = utilities::reserved_container<std::vector<std::string>>(aKeywordForms.size());
    std::transform(
        aKeywordForms.begin(), aKeywordForms.end(), std::back_inserter(tFormattedKeywordForms),
        [tMaxKeywordNameLength](const auto& aKeywordForm)
        { return std::format("{:<{}} {}", aKeywordForm.mName, tMaxKeywordNameLength, aKeywordForm.mDataType); });

    return tFormattedKeywordForms;
}

auto sort_keyword_forms_by_distance(std::vector<BlockStructKeywordForm> aKeywordForms,
                                    const std::string_view aToken) -> std::vector<BlockStructKeywordForm>
{
    return utilities::levenshtein_distance_sorted_container(
        std::move(aKeywordForms), aToken,
        [](const auto& aBlockStructKeywordForm) { return std::string_view{aBlockStructKeywordForm.mName}; });
}

auto closest_keyword_form_syntax() -> std::string_view { return kClosestKeywordFormSyntax; }
}  // namespace plato::input_parser::detail
