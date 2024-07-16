#include "plato/filter/extension/KernelFilter.hpp"

#include <boost/math/constants/constants.hpp>
#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
#include <memory>
#include <optional>

#include "plato/core/Function.hpp"
#include "plato/core/MeshProxy.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

namespace plato::filter::extension
{
namespace
{
[[maybe_unused]] static auto kKernelFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::kernel_filter>(), [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = core::validated_variant_raw_input<input_parser::kernel_filter>(aInput);
        return core::make_function(
            [&tInput](const core::MeshProxy& aMeshProxy)
            {
                const KernelFilter tFilter{aMeshProxy.mFileName, FilterRadius{tInput.filter_radius.value()},
                                           tInput.centering_type.value(), boost::mpi::communicator{}};
                return tFilter.filter(aMeshProxy);
            },
            [&tInput](const core::MeshProxy& aMeshProxy)
            {
                std::shared_ptr<library::FilterInterface> tSharedFilter =
                    std::make_shared<KernelFilter>(aMeshProxy.mFileName, FilterRadius{tInput.filter_radius.value()},
                                                   tInput.centering_type.value(), boost::mpi::communicator{});
                return library::FilterJacobian{tSharedFilter, aMeshProxy};
            });
    }};

[[maybe_unused]] static auto kKernelFilterValidationRegistration =
    core::ValidationRegistration<input_parser::kernel_filter>{
        [](const input_parser::kernel_filter& aInput) { return detail::validate_filter_radius(aInput); },
        [](const input_parser::kernel_filter& aInput)
        { return detail::validate_kernel_filter_centering_type(aInput); }};
}  // namespace

KernelFilter::KernelFilter(const std::filesystem::path& aMeshFileName,
                           const FilterRadius aFilterRadius,
                           const input_parser::KernelFilterCenteringTypes aFilterCentering,
                           const boost::mpi::communicator& aCommunicator)
    : mLinearMask(detail::create_linear_mask(aMeshFileName, aFilterRadius, aFilterCentering, aCommunicator)),
      mCommunicator(aCommunicator)

{
}

core::MeshProxy KernelFilter::filter(const core::MeshProxy& aMeshProxy) const
{
    core::MeshProxy tMeshProxy{aMeshProxy};
    tMeshProxy.mNodalDensities = mLinearMask.matrixMultiply(aMeshProxy.mNodalDensities);
    return tMeshProxy;
}

linear_algebra::DynamicVector<double> KernelFilter::jacobianTimesVector(
    const core::MeshProxy& /*aMeshProxy*/, const linear_algebra::DynamicVector<double>& aV) const
{
    return linear_algebra::DynamicVector<double>{mLinearMask.transposeMatrixMultiply(aV.stdVector())};
}

namespace detail
{
std::optional<std::string> validate_kernel_filter_centering_type(const input_parser::kernel_filter& aInput)
{
    if (!aInput.centering_type)
    {
        return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::kernel_filter>(),
                                                       aInput.centering_type, "centering_type");
    }
    else
    {
        return std::nullopt;
    }
}

double filter_volume(const FilterRadius aFilterRadius)
{
    return 4.0 / 3.0 * boost::math::constants::pi<double>() * aFilterRadius.mValue * aFilterRadius.mValue *
           aFilterRadius.mValue;
}

int determine_maximum_connectivity_estimate(const std::filesystem::path& aMeshFileName,
                                            const FilterRadius aFilterRadius)
{
    const auto tBulk = third_party_integration::stk_io::read_mesh_bulk_data(aMeshFileName);
    auto tNodalCoordinates = third_party_integration::stk_io::nodal_coordinates(*tBulk);
    const double tAverageNodalDensity = third_party_integration::stk_io::average_nodal_density(*tBulk);
    const double tSearchVolume = detail::filter_volume(aFilterRadius);
    return static_cast<int>(tSearchVolume * tAverageNodalDensity *
                            kMaxMultiplier);  // for Tpetra sparse matrix allocation
}

LinearMask create_linear_mask(const std::filesystem::path& aMeshFileName,
                              const FilterRadius aFilterRadius,
                              const input_parser::KernelFilterCenteringTypes aFilterCentering,
                              const boost::mpi::communicator& aCommunicator)
{
    const auto tBulk = third_party_integration::stk_io::read_mesh_bulk_data(aMeshFileName);
    auto tNodalCoordinates = third_party_integration::stk_io::nodal_coordinates(*tBulk);
    const int tMaximumConnectivityEstimate =
        detail::determine_maximum_connectivity_estimate(aMeshFileName, aFilterRadius);

    if (aFilterCentering == input_parser::KernelFilterCenteringTypes::kElementCentered)
    {
        auto tElementCentroids = third_party_integration::stk_io::element_centroids(*tBulk);
        return LinearMask(NodalVector{tNodalCoordinates}, CenterVector{tElementCentroids},
                          SearchRadius{aFilterRadius.mValue}, tMaximumConnectivityEstimate, aCommunicator);
    }
    else
    {
        return LinearMask(NodalVector{tNodalCoordinates}, SearchRadius{aFilterRadius.mValue},
                          tMaximumConnectivityEstimate, aCommunicator);
    }
}

}  // namespace detail

}  // namespace plato::filter::extension
