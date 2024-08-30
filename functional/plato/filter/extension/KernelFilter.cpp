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
#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/mesh/MeshDesignVariablesSequentialView.hpp"

namespace plato::filter::extension
{
namespace
{
[[maybe_unused]] static auto kKernelFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::kernel_filter>(), [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = core::validated_variant_raw_input<input_parser::kernel_filter>(aInput);
        auto tFilterCache = detail::create_filter_cache(tInput);

        return core::make_function(
            [tFilterCache](const mesh::MeshDesignVariables& aMeshDesignVariables) mutable
            { return tFilterCache.compute(aMeshDesignVariables)->filter(aMeshDesignVariables); },
            [tFilterCache](const mesh::MeshDesignVariables& aMeshDesignVariables) mutable {
                return library::FilterJacobian{tFilterCache.compute(aMeshDesignVariables), aMeshDesignVariables};
            });
    }};

[[maybe_unused]] static auto kKernelFilterValidationRegistration =
    core::ValidationRegistration<input_parser::kernel_filter>{
        [](const input_parser::kernel_filter& aInput) { return detail::validate_filter_radius(aInput); },
        [](const input_parser::kernel_filter& aInput)
        { return detail::validate_kernel_filter_centering_type(aInput); }};
}  // namespace

KernelFilter::KernelFilter(const mesh::Mesh& aMesh,
                           const FilterRadius aFilterRadius,
                           const input_parser::KernelFilterCenteringTypes aFilterCentering,
                           const boost::mpi::communicator& aCommunicator)
    : mLinearMask{detail::create_linear_mask(aMesh, aFilterRadius, aFilterCentering, aCommunicator)},
      mFilterCentering{aFilterCentering},
      mCommunicator{aCommunicator}
{
}

mesh::MeshDesignVariables KernelFilter::filter(const mesh::MeshDesignVariables& aMeshDesignVariables) const
{
    const auto tMesh = mesh::Mesh{aMeshDesignVariables};
    const auto tFieldValues =
        mesh::DesignVariablesConversion{tMesh}.meshDesignVariablesToNodalFieldVector(aMeshDesignVariables);

    const auto tFilteredField = mLinearMask.matrixMultiply(tFieldValues.mValue);
    if (mFilterCentering == input_parser::KernelFilterCenteringTypes::kNodeCentered)
    {
        return mesh::DesignVariablesConversion{tMesh}.nodalFieldToMeshDesignVariables(
            mesh::NodalFieldVectorReference{std::cref(tFilteredField)});
    }
    return mesh::DesignVariablesConversion{tMesh}.elementFieldToMeshDesignVariables(
        mesh::ElementFieldVectorReference{std::cref(tFilteredField)});
}

linear_algebra::DynamicVector<double> KernelFilter::jacobianTimesVector(
    const mesh::MeshDesignVariables& /*aMeshDesignVariables*/, const linear_algebra::DynamicVector<double>& aV) const
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
    return std::nullopt;
}

LinearMask create_linear_mask(const mesh::Mesh& aMesh,
                              const FilterRadius aFilterRadius,
                              const input_parser::KernelFilterCenteringTypes aFilterCentering,
                              const boost::mpi::communicator& aCommunicator)
{
    return LinearMask{
        LinearMaskBuilder{aMesh, aFilterCentering, SearchRadius{aFilterRadius.mValue}, aCommunicator}.mask(),
        aCommunicator};
}

FilterCache create_filter_cache(const input_parser::kernel_filter& aInput)
{
    return FilterCache{[aInput](const mesh::MeshDesignVariables& aMeshDesignVariables)
                       {
                           return std::make_shared<KernelFilter>(
                               mesh::Mesh{aMeshDesignVariables}, FilterRadius{aInput.filter_radius.value()},
                               aInput.centering_type.value(), boost::mpi::communicator{});
                       },
                       [](const mesh::MeshDesignVariables& aMeshDesignVariables)
                       { return library::hash_mesh_coordinates(aMeshDesignVariables); }};
}

}  // namespace detail

}  // namespace plato::filter::extension
