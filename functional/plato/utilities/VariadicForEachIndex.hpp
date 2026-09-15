#ifndef PLATO_UTILITIES_VARIADICFOREACHINDEX
#define PLATO_UTILITIES_VARIADICFOREACHINDEX

#include <concepts>
#include <cstddef>
#include <utility>

namespace plato::utilities
{

/// @brief Utility to facilitate applying a function to each index at compile time
///
/// @code{.cpp}
/// auto tCounter = 0U;
/// for_each_index<3>([&tCounter]<std::size_t kIndex>(){tCounter++;});
/// @endcode
///
template <std::size_t kNumberOfIndices, typename Function>
    requires requires(const Function& aFunction) { aFunction.template operator()<std::size_t{}>(); }
void for_each_index(const Function& aFunction)
{
    [&]<std::size_t... kIndices>(std::index_sequence<kIndices...>)
    { (aFunction.template operator()<kIndices>(), ...); }(std::make_index_sequence<kNumberOfIndices>{});
}

}  // namespace plato::utilities

#endif
