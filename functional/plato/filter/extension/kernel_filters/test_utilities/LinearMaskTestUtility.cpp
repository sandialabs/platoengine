#include "plato/filter/extension/kernel_filters/test_utilities/LinearMaskTestUtility.hpp"

#include <vector>

#include "plato/filter/extension/kernel_filters/CanonicalKernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/LinearMask.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::filter::extension::kernel_filters::test_utilities
{

auto create_simple_linear_mask_builder(const std::vector<third_party_integration::common::Coordinate>& aCentroids)
    -> LinearMaskBuilder
{
    const auto tCentering = aCentroids.empty() ? input_parser::KernelFilterCenteringTypes::kNodeCentered
                                               : input_parser::KernelFilterCenteringTypes::kElementCentered;

    const auto tFilterRadius = 2.0;
    const auto tKernelFilterType =
        detail::make_kernel_filter_type(tFilterRadius, tCentering, mesh::Mesh{"not-a-mesh.exo"});

    const auto tCoordinates =
        std::vector<third_party_integration::common::Coordinate>{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}};
    if (!aCentroids.empty())
    {
        return {SourceColumnVector{tCoordinates}, TargetRowVector{aCentroids}, tKernelFilterType.mKernelFilterFunctions,
                kMaximumConnectivity, boost::mpi::communicator{}};
    }

    return {SourceColumnVector{tCoordinates}, TargetRowVector{tCoordinates}, tKernelFilterType.mKernelFilterFunctions,
            kMaximumConnectivity, boost::mpi::communicator{}};
}

auto create_simple_linear_mask(const std::vector<third_party_integration::common::Coordinate>& aCentroids) -> LinearMask
{
    return LinearMask{create_simple_linear_mask_builder(aCentroids).mask(), boost::mpi::communicator{}};
}

}  // namespace plato::filter::extension::kernel_filters::test_utilities
