#include "plato/third_party_integration/stk_io/SidesetUtilities.hpp"

#include <stk_mesh/base/Bucket.hpp>
#include <stk_mesh/base/BulkData.hpp>
// #include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/base/ExodusTranslator.hpp>
// #include <stk_mesh/base/GetEntities.hpp>
// #include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Part.hpp>
// #include <stk_mesh/base/Selector.hpp>

namespace plato::third_party_integration::stk_io
{
namespace
{

}  // namespace

auto get_sideset_triangles(const stk::mesh::BulkData& aBulkData,
                           const std::string& aSidesetName) -> std::vector<Triangle>
{
    std::vector<Triangle> tTriList;
    stk::mesh::Part& tSensitivitySurfaceSideset = *aBulkData.mesh_meta_data().get_part(aSidesetName);
    stk::mesh::Selector tTriangleSelector(tSensitivitySurfaceSideset);
    const stk::mesh::BucketVector& tTriangleBuckets =
        aBulkData.get_buckets(stk::topology::FACE_RANK, tTriangleSelector);
    CoordinatesField_t const& tCoordField =
        *dynamic_cast<CoordinatesField_t const*>(aBulkData.mesh_meta_data().coordinate_field());
    for (const auto& tCurBucket : tTriangleBuckets)
    {
        for (const auto& tFaceEntity : *tCurBucket)
        {
            const stk::mesh::ConnectedEntities tNodes =
                aBulkData.get_connected_entities(tFaceEntity, stk::topology::NODE_RANK);
            if (tNodes.size() == 3)
            {
                std::vector<Coordinate> tTriCoords(3);
                for (size_t i = 0; i < 3; ++i)
                {
                    const double* tCoords = stk::mesh::field_data(tCoordField, tNodes[i]);
                    tTriCoords[i].x = tCoords[0];
                    tTriCoords[i].y = tCoords[1];
                    tTriCoords[i].z = tCoords[2];
                }
                Triangle tNewTri{tTriCoords[0], tTriCoords[1], tTriCoords[2]};
                tTriList.push_back(Triangle{tTriCoords[0], tTriCoords[1], tTriCoords[2]});
            }
        }
    }
    return tTriList;
}

}  // namespace plato::third_party_integration::stk_io
