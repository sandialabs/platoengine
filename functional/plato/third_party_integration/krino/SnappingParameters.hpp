#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_SNAPPINGPARAMETERS
#define PLATO_THIRDPARTYINTEGRATION_KRINO_SNAPPINGPARAMETERS

#include <optional>

namespace plato::third_party_integration::krino
{
/// @brief Struct to hold parameters controlling how krino does snapping
struct SnappingParameters
{
    constexpr static auto mDefaultMaxSnappingEdgeLength = 0.15;
    constexpr static auto mDefaultSharpFeatureAngle = 135.0;

    std::optional<double> mMaxSnappingEdgeLength{mDefaultMaxSnappingEdgeLength};
    double mSharpFeatureAngle{mDefaultSharpFeatureAngle};
};

[[nodiscard]] constexpr auto snapping_off_parameters() -> SnappingParameters
{
    return SnappingParameters{.mMaxSnappingEdgeLength = std::nullopt};
}

}  // namespace plato::third_party_integration::krino

#endif
