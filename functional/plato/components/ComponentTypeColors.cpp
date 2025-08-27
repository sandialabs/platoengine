#include "plato/components/ComponentTypeColors.hpp"

#include <unordered_map>

namespace plato::components
{
auto component_text_color(ComponentType aComponentType) -> utilities::TextColor
{
    static const auto kComponentTextColorMap = std::unordered_map<ComponentType, utilities::TextColor>{
        {ComponentType::kObjective, utilities::TextColor::kCyan},
        {ComponentType::kConstraint, utilities::TextColor::kLightGray},
        {ComponentType::kGeometry, utilities::TextColor::kMagenta},
        {ComponentType::kFilter, utilities::TextColor::kDarkGray},
        {ComponentType::kProcessManager, utilities::TextColor::kLightBlue}};

    return kComponentTextColorMap.at(aComponentType);
}
}  // namespace plato::components
