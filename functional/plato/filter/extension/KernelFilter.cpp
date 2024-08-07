#include "plato/filter/extension/KernelFilter.hpp"

#include <boost/mpi.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>
#include <optional>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/filter/extension/CommonInputValidation.hpp"
#include "plato/filter/extension/FilterMeshUtilities.hpp"
#include "plato/filter/extension/LinearMaskBuilder.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/filter/library/HashGeneration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/utilities/RankSplitVector.hpp"

namespace plato::filter::extension
{

namespace
{
boost::mpi::communicator subdivide_world_comm_into_groups(const unsigned int aGroupSize)
{
    auto tWorldComm = boost::mpi::communicator{};

    const auto tVectorSize = tWorldComm.size() / aGroupSize;
    const std::vector<unsigned int> tGroups(tVectorSize, aGroupSize);
    const auto tRank = tWorldComm.rank();
    const auto tColor = utilities::rank_group_color(tGroups, utilities::RankNamedType{tRank});
    return tWorldComm.split(tColor.mValue);
}

[[maybe_unused]] static auto kKernelFilterRegistration = library::FilterRegistration{
    input_parser::block_name<input_parser::kernel_filter>(), [](const library::ValidatedFilterInput& aInput)
    {
        const auto& tInput = core::validated_variant_raw_input<input_parser::kernel_filter>(aInput);
        return library::make_filter_function_from_cache([&tInput]() { return detail::create_filter_cache(tInput); });
    }};

[[maybe_unused]] static auto kKernelFilterValidationRegistration =
    core::ValidationRegistration<input_parser::kernel_filter>{
        [](const input_parser::kernel_filter& aInput) { return detail::validate_filter_radius_bounds(aInput); },
        [](const input_parser::kernel_filter& aInput) { return detail::validate_kernel_filter_centering_type(aInput); },
        [](const input_parser::kernel_filter& aInput) { return detail::validate_number_of_processors(aInput); },
        [](const input_parser::kernel_filter& aInput)
        { return detail::validate_number_of_processors_factor_of_comm_world(aInput); }};

[[maybe_unused]] static auto kKernelFilterMeshBasedValidationRegistration =
    core::ValidationRegistration<input_parser::kernel_filter, std::filesystem::path>{
        [](const input_parser::kernel_filter& aInput, const std::filesystem::path& aMeshPath)
        { return detail::validate_filter_radius_with_mesh(aInput, aMeshPath); }};

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

analysis::AnalysisDomainMesh KernelFilter::filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const auto tMesh = mesh::Mesh{aAnalysisDomainMesh};
    const auto tFieldValues =
        mesh::DesignVariablesConversion{tMesh}.meshDesignVariablesToNodalFieldVector(aAnalysisDomainMesh);

    const auto tFilteredField = mLinearMask.matrixMultiply(tFieldValues.mValue);
    if (mFilterCentering == input_parser::KernelFilterCenteringTypes::kNodeCentered)
    {
        return mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
            mesh::NodalFieldVectorReference{std::cref(tFilteredField)});
    }
    return mesh::DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{std::cref(tFilteredField)});
}

linear_algebra::DynamicVector<double> KernelFilter::jacobianTimesVector(
    const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/, const linear_algebra::DynamicVector<double>& aV) const
{
    std::cout << "Filter JV " << gFilterDFCount++ << std::endl;
    const auto tReturn = linear_algebra::DynamicVector<double>{mLinearMask.transposeMatrixMultiply(aV.stdVector())};

    return tReturn;
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

std::optional<std::string> validate_number_of_processors(const input_parser::kernel_filter& aInput)
{
    return core::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::kernel_filter>(), aInput.number_of_processors, "number_of_processors",
        utilities::lower_bounded(utilities::Inclusive{1u}));
}

std::optional<std::string> validate_number_of_processors_factor_of_comm_world(const input_parser::kernel_filter& aInput)
{
    const auto tRequestedRanks = aInput.number_of_processors.value_or(1u);
    const auto tTotalRanks = static_cast<std::size_t>(boost::mpi::communicator{}.size());
    if (tRequestedRanks > tTotalRanks)
    {
        return std::optional<std::string>{utilities::concatenate(
            "The number of MPI ranks requested for filter cannot exceed the available ranks for the entire run.\n",
            "The number of available ranks: ", tTotalRanks,
            ".\n The number of requested ranks for the filter: ", tRequestedRanks)};
    }
    if (tTotalRanks % tRequestedRanks != 0)
    {
        return std::optional<std::string>{
            utilities::concatenate("The number of MPI ranks requested for filter has to a factor of the "
                                   "available ranks for the entire run.\n",
                                   "The number of available ranks: ", tTotalRanks,
                                   ".\n The number of requested ranks for the filter: ", tRequestedRanks)};
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

library::FilterCache create_filter_cache(const input_parser::kernel_filter& aInput)
{
    const auto tRequestedRanks = aInput.number_of_processors.value_or(1u);
    const auto tSplitComm = subdivide_world_comm_into_groups(tRequestedRanks);
    return library::FilterCache{[aInput, tSplitComm](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
                                {
                                    return std::make_shared<KernelFilter>(
                                        mesh::Mesh{aAnalysisDomainMesh},
                                        FilterRadius{detail::get_filter_radius(aInput, aAnalysisDomainMesh.mFileName)},
                                        aInput.centering_type.value(), tSplitComm);
                                },
                                [](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
                                { return library::hash_mesh_coordinates(aAnalysisDomainMesh); }};
}

}  // namespace detail

}  // namespace plato::filter::extension
