#ifndef PLATO_PROCESS_MANAGER_EXTENSION_TEST_UTILITIES_NODALRESULTFILTERUTILITIES
#define PLATO_PROCESS_MANAGER_EXTENSION_TEST_UTILITIES_NODALRESULTFILTERUTILITIES

#include <filesystem>
#include <optional>

#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

namespace plato::test_utilities
{
struct TestContext;
}

namespace plato::process_manager::extension::test_utilities
{

/// @brief A test fixture which handles the lifetime of two meshes with nodal densities (one restart, one normal).
/// Provides a test to ensure the nodal_result_filter is working.
class NodalResultFilterRunFixture : public third_party_integration::stk_io::test_utilities::MeshWithNodalDensities
{
   public:
    NodalResultFilterRunFixture();
    ~NodalResultFilterRunFixture() override;

    /// @brief Creates an input and runs a nodal_result_filter. If @a aNodalResultOutMeshPath is specified, it will
    /// check that file for the nodal result. If @a aNumberOfProcessorsForFilter is specified, it will use that for the
    /// filter.
    void runAndCheckNodalFilterResult(const std::optional<std::filesystem::path>& aNodalResultOutMeshPath,
                                      const plato::test_utilities::TestContext& aTestContext,
                                      unsigned int aNumberOfProcessorsForFilter = 1U);

    NodalResultFilterRunFixture(const NodalResultFilterRunFixture&) = delete;
    NodalResultFilterRunFixture(NodalResultFilterRunFixture&&) = delete;
    auto operator=(const NodalResultFilterRunFixture&) -> NodalResultFilterRunFixture& = delete;
    auto operator=(NodalResultFilterRunFixture&&) -> NodalResultFilterRunFixture& = delete;
};

}  // namespace plato::process_manager::extension::test_utilities

#endif
