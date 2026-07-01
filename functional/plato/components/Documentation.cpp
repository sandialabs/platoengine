#include "plato/components/Documentation.hpp"

#include <string>
#include <unordered_map>

#include "plato/components/ComponentType.hpp"

namespace plato::components
{
auto component_documentation_brief(ComponentType aComponentType) -> std::string
{
    static const auto tComponentDocumentationMap = std::unordered_map<ComponentType, std::string>{
        {ComponentType::kObjective,
         "Defines the performance measure that the optimization algorithm seeks to maximize or minimize."},
        {ComponentType::kConstraint,
         "Limits the design space or the solution to ensure practical, physical, or performance requirements are met."},
        {ComponentType::kGeometry,
         "Translates a set of design variables to a mesh that can be evaluated by a physics code."},
        {ComponentType::kFilter, "Provides regularization for topology-based `geometry` components."},
        {ComponentType::kProcessManager,
         "Assembles all components and evaluates `objective` and `constraint` components according to an algorithm."}};

    return tComponentDocumentationMap.at(aComponentType);
}
}  // namespace plato::components
