#ifndef PLATO_DOCUMENTATION_PLATODOCUMENTATION
#define PLATO_DOCUMENTATION_PLATODOCUMENTATION

#include <string_view>

namespace plato::documentation
{
struct PlatoDocumentation
{
    constexpr static auto mDocumentation = std::string_view{"Plato is..."};
};
}  // namespace plato::documentation

#endif
