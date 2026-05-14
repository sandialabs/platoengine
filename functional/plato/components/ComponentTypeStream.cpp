#include "plato/components/ComponentTypeStream.hpp"

#include <cassert>
#include <cstddef>
#include <iterator>
#include <ostream>
#include <string>
#include <utility>

#include "plato/components/ComponentType.hpp"
#include "plato/utilities/EnumIndexing.hpp"
#include "plato/utilities/EnumTable.hpp"

namespace plato::components
{
namespace
{
const static auto kComponentNameTable =
    utilities::EnumTable<ComponentType>{{ComponentType::kConstraint, "constraint"},
                                        {ComponentType::kFilter, "filter"},
                                        {ComponentType::kGeometry, "geometry"},
                                        {ComponentType::kObjective, "objective"},
                                        {ComponentType::kProcessManager, "process manager"}};

}

auto operator<<(std::ostream& aStream, const ComponentType aComponentType) -> std::ostream&
{
    // Check that all enumerates are included in the table
    assert(utilities::number_of_enumerates<ComponentType>() ==
           static_cast<std::size_t>(std::distance(kComponentNameTable.begin(), kComponentNameTable.end())));

    const auto tComponentName = kComponentNameTable.toString(aComponentType);
    assert(tComponentName.has_value());

    aStream << tComponentName.value();
    return aStream;
}

auto to_string(const ComponentType aComponentType) -> std::string
{
    auto tComponentName = kComponentNameTable.toString(aComponentType);
    assert(tComponentName.has_value());
    return std::move(tComponentName).value();
}
}  // namespace plato::components
