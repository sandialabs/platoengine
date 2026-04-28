#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_BACKGROUNDMESH
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_BACKGROUNDMESH

#include <Akri_MeshInterface.hpp>
#include <Akri_Phase_Support.hpp>
#include <memory>
#include <vector>

namespace plato::third_party_integration::krino
{
/// @brief a struct for holding both the krino representation of mesh data and the level set fields associated with that
/// mesh.
struct BackgroundMesh
{
    std::unique_ptr<::krino::MeshInterface> mKrinoMesh;
    std::vector<::krino::LS_Field> mLevelSetFields;
};

}  // namespace plato::third_party_integration::krino

#endif
