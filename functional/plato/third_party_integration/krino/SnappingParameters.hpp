#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_SNAPPINGPARAMETERS
#define PLATO_THIRDPARTYINTEGRATION_KRINO_SNAPPINGPARAMETERS

namespace plato::third_party_integration::krino
{
/// @brief Struct to hold parameters controlling how krino does snapping
struct SnappingParameters
{
    constexpr static auto mDefaultMaxSnappingEdgeLength = 0.15;
    constexpr static auto mDefaultSharpFeatureAngle = 135.0;

    double mMaxSnappingEdgeLength{mDefaultMaxSnappingEdgeLength};
    double mSharpFeatureAngle{mDefaultSharpFeatureAngle};
};

[[nodiscard]] constexpr auto snapping_off_parameters() -> SnappingParameters
{
    return SnappingParameters{.mMaxSnappingEdgeLength = 0.0};
}

}  // namespace plato::third_party_integration::krino

#endif
