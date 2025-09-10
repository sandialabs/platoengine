#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_SNAPPINGPARAMETERS
#define PLATO_THIRDPARTYINTEGRATION_KRINO_SNAPPINGPARAMETERS

namespace plato::third_party_integration::krino
{
/// @brief Struct to hold parameters controlling how krino does snapping
struct SnappingParameters
{
    double mMaxSnappingEdgeLength{0.15};
    double mSharpFeatureAngle{135.0};
};

[[nodiscard]] constexpr auto snapping_off_parameters() -> SnappingParameters
{
    return SnappingParameters{.mMaxSnappingEdgeLength = 0.0, .mSharpFeatureAngle = 135.0};
}

}  // namespace plato::third_party_integration::krino

#endif
