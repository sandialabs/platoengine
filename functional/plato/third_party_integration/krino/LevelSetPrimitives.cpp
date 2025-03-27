#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"

#include <Akri_AnalyticSurf.hpp>
#include <Akri_Composite_Surface.hpp>

namespace plato::third_party_integration::krino
{

namespace
{
constexpr auto kInitializationSurfaces = std::string_view{"initialization surfaces"};
}

auto make_krino_composite_surface() -> ::krino::Composite_Surface
{
    return ::krino::Composite_Surface{std::string{kInitializationSurfaces}};
}

void append_spheres(::krino::Composite_Surface& aSurfaces, const std::vector<Sphere>& aSpheres)
{
    for (const auto& tSphere : aSpheres)
    {
        aSurfaces.add(new ::krino::Sphere({tSphere.mCenter.x, tSphere.mCenter.y, tSphere.mCenter.z}, tSphere.mRadius));
    }
}

void append_planes(::krino::Composite_Surface& aSurfaces, const std::vector<Plane>& aPlanes)
{
    for (const auto& tPlane : aPlanes)
    {
        const auto tNormal = std::array{tPlane.mNormal.x, tPlane.mNormal.y, tPlane.mNormal.z};
        aSurfaces.add(new ::krino::Plane(tNormal.data(), tPlane.mOffset, 1.0));
    }
}

}  // namespace plato::third_party_integration::krino
