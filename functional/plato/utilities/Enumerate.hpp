#ifndef PLATO_UTILITIES_ENUMERATE_HPP
#define PLATO_UTILITIES_ENUMERATE_HPP

#include <algorithm>
#include <numeric>

#include "plato/utilities/MultidimensionalRange.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::utilities
{
/// @brief Adds a 0-based index counter variable to a range-based for over containers.
///
/// Similar to Zip, but adds an index counter. An example usage is:
/// @code{.cpp}
/// for(const auto& [tIndex, tValue] : enumerate(tContainer))
/// {
///   tOtherContainer[tIndex] = tValue;
/// }
/// @endcode
/// The index starts at 0.
template <typename... Containers>
auto enumerate(Containers&&... aContainers) -> Zip<std::vector<std::size_t>, Containers...>
{
    const auto tMinSize = std::min({aContainers.size()...});
    auto tEnumerates = std::vector<std::size_t>(tMinSize);
    std::iota(tEnumerates.begin(), tEnumerates.end(), 0U);
    return Zip{std::move(tEnumerates), std::forward<Containers>(aContainers)...};
}
}  // namespace plato::utilities

#endif
