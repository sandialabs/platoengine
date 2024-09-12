#ifndef PLATO_UTILITIES_ENUMERATE_HPP
#define PLATO_UTILITIES_ENUMERATE_HPP

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
Zip<std::vector<std::size_t>, Containers...> enumerate(Containers&&... containers)
{
    const auto min_size = std::min({containers.size()...});
    auto enumerates = std::vector<std::size_t>(min_size);
    std::iota(enumerates.begin(), enumerates.end(), 0U);
    return Zip{std::move(enumerates), std::forward<Containers>(containers)...};
}
}  // namespace plato::utilities

#endif
