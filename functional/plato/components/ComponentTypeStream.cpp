#include "plato/components/ComponentTypeStream.hpp"

#include "plato/utilities/EnumTable.hpp"

namespace plato::components
{
auto operator<<(std::ostream& aStream, const ComponentType aComponentType) -> std::ostream&
{
    const static auto tComponentNameTable =
        utilities::EnumTable<ComponentType>{{ComponentType::kConstraint, "constraint"},
                                            {ComponentType::kFilter, "filter"},
                                            {ComponentType::kGeometry, "geometry"},
                                            {ComponentType::kObjective, "objective"},
                                            {ComponentType::kProcessManager, "process manager"}};
    // Check that all enumerates are included in the table
    assert(utilities::number_of_enumerates<ComponentType>() ==
           std::distance(tComponentNameTable.begin(), tComponentNameTable.end()));

    const auto tComponentName = tComponentNameTable.toString(aComponentType);
    assert(tComponentName.has_value());

    aStream << tComponentName.value();
    return aStream;
}
}  // namespace plato::components
