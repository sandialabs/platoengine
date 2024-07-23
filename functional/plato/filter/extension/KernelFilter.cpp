#include "plato/filter/extension/KernelFilter.hpp"

#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>

#include "plato/core/Function.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/extension/LinearMaskBuilder.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/filter/library/HashGeneration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshProxy.hpp"
#include "plato/mesh/MeshProxyViews.hpp"

namespace plato::filter::extension
{
namespace
{
[[maybe_unused]] static auto kKernelFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::kernel_filter>(), [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = core::validated_variant_raw_input<input_parser::kernel_filter>(aInput);
        auto tFilterCache = detail::create_filter_cache(tInput);

        return core::make_function([tFilterCache](const mesh::MeshProxy& aMeshProxy) mutable
                                   { return tFilterCache.compute(aMeshProxy)->filter(aMeshProxy); },
                                   [tFilterCache](const mesh::MeshProxy& aMeshProxy) mutable {
                                       return library::FilterJacobian{tFilterCache.compute(aMeshProxy), aMeshProxy};
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
      mFilterCentering{aFilterCentering},
      mCommunicator(aCommunicator)
{
}

mesh::MeshProxy KernelFilter::filter(const mesh::MeshProxy& aMeshProxy) const
{
    const auto [tDensityValues, tGlobalIDs] =
        mesh::split_densities(mesh::mesh_proxy_to_vector(mesh::MeshProxyDensitiesView{aMeshProxy}));
    auto tFilteredDensities = mLinearMask.matrixMultiply(tDensityValues);
    if (mFilterCentering == input_parser::KernelFilterCenteringTypes::kNodeCentered)
    {
        return mesh::nodal_densities_to_mesh_proxy(tFilteredDensities, mesh::Mesh{aMeshProxy.mFileName});
    }
    else
    {
        return mesh::element_densities_to_mesh_proxy(tFilteredDensities, mesh::Mesh{aMeshProxy.mFileName});
    }
}

linear_algebra::DynamicVector<double> KernelFilter::jacobianTimesVector(
    const mesh::MeshProxy& /*aMeshProxy*/, const linear_algebra::DynamicVector<double>& aV) const
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

LinearMask create_linear_mask(const std::filesystem::path& aMeshFileName,
                              const FilterRadius aFilterRadius,
                              const input_parser::KernelFilterCenteringTypes aFilterCentering,
                              const boost::mpi::communicator& aCommunicator)
{
    const auto tMesh = mesh::Mesh{aMeshFileName};
    return LinearMask{
        LinearMaskFactory{tMesh, aFilterCentering, SearchRadius{aFilterRadius.mValue}, aCommunicator}.mask(),
        aCommunicator};
}

FilterCache create_filter_cache(const input_parser::kernel_filter& aInput)
{
    return FilterCache{[aInput](const mesh::MeshProxy& aMeshProxy)
                       {
                           return std::make_shared<KernelFilter>(
                               aMeshProxy.mFileName, FilterRadius{aInput.filter_radius.value()},
                               aInput.centering_type.value(), boost::mpi::communicator{});
                       },
                       [](const mesh::MeshProxy& aMeshProxy) { return library::hash_mesh_coordinates(aMeshProxy); }};
}

}  // namespace detail

}  // namespace plato::filter::extension
