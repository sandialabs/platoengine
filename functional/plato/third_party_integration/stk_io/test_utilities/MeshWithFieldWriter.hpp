#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_MESHWITHFIELDWRITER
#define PLATO_THIRDPARTYINTEGRATION_STKIO_MESHWITHFIELDWRITER

#include <gtest/gtest.h>

#include <filesystem>
#include <map>

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"

namespace plato::third_party_integration::stk_io::test_utilities
{

/// @brief A parent class mesh test fixture for meshes with a predefined field for reading.
class MeshWithDensities : public ::testing::Test
{
   public:
    MeshWithDensities(const std::map<std::size_t, double>& aGoldNumbering);
    ~MeshWithDensities();

   protected:
    std::string mFieldName;
    std::map<std::size_t, double> mGoldNumbering;
};

/// @brief A test fixture for meshes with a predefined Element field for reading.
/// Elements are assigned a field value based on their ID
class MeshWithElementDensities : public MeshWithDensities
{
   public:
    MeshWithElementDensities();
    ~MeshWithElementDensities();

   protected:
    std::filesystem::path mMeshName;
};

/// @brief A test fixture for meshes with a predefined Nodal field for reading.
/// Nodes are assigned a field value based on their ID
class MeshWithNodalDensities : public MeshWithDensities
{
   public:
    MeshWithNodalDensities();
    ~MeshWithNodalDensities();

   protected:
    std::filesystem::path mMeshName;
};

namespace detail
{

void create_element_density_field_mesh_for_reading(const CommandGenerator& aCommandGenerator,
                                                   const std::filesystem::path& aFileName);
void create_node_density_field_mesh_for_reading(const CommandGenerator& aCommandGenerator,
                                                const std::filesystem::path& aFileName);

}  // namespace detail
}  // namespace plato::third_party_integration::stk_io::test_utilities

#endif
