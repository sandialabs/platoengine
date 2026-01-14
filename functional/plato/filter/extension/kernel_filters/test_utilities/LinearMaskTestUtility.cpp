#include "plato/filter/extension/kernel_filters/test_utilities/LinearMaskTestUtility.hpp"

#include "plato/filter/extension/kernel_filters/CanonicalKernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"

namespace plato::filter::extension::kernel_filters::test_utilities
{

auto create_simple_linear_mask_builder(const std::vector<third_party_integration::common::Coordinate>& aCentroids)
    -> LinearMaskBuilder<input_parser::kernel_filter>
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
        return LinearMaskBuilder<input_parser::kernel_filter>(SourceColumnVector{tCoordinates},
                                                              TargetRowVector{aCentroids}, tKernelFilterType,
                                                              kMaximumConnectivity, boost::mpi::communicator{});
    }

    return LinearMaskBuilder<input_parser::kernel_filter>(SourceColumnVector{tCoordinates},
                                                          TargetRowVector{tCoordinates}, tKernelFilterType,
                                                          kMaximumConnectivity, boost::mpi::communicator{});
}

LinearMask create_simple_linear_mask(const std::vector<third_party_integration::common::Coordinate>& aCentroids)
{
    return LinearMask{create_simple_linear_mask_builder(aCentroids).mask(), boost::mpi::communicator{}};
}

}  // namespace plato::filter::extension::kernel_filters::test_utilities
