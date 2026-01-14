#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_TESTUTILITIES_SYMMETRYFILTERINFOTESTS
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_TESTUTILITIES_SYMMETRYFILTERINFOTESTS

#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::filter::extension::kernel_filters::test_utilities
{

template <typename T>
concept HasBoostOptionalMembers = requires(T a) {
    { a.filter_radius } -> std::same_as<boost::optional<double>&>;
    { a.use_relative_radius } -> std::same_as<boost::optional<bool>&>;
    { a.number_of_processors } -> std::same_as<boost::optional<unsigned int>&>;
    { a.centering_type } -> std::same_as<decltype(a.centering_type)&>;
};

template <typename T>
concept HasBoostOptionalTargetMeshNameMember = requires(T a) {
    { a.target_mesh_name } -> std::same_as<decltype(a.target_mesh_name)&>;
};

template <typename InputParserType>
concept HasAllOptionalMembers =
    HasBoostOptionalMembers<InputParserType> && HasBoostOptionalTargetMeshNameMember<InputParserType>;

/// @brief Use a radius @a aRadius and a centering type @a aCenteringType to create a generic symmetry filter input.
template <typename InputFilterType>
    requires HasAllOptionalMembers<InputFilterType>
[[nodiscard]] auto make_filter_input(const double aRadius, const input_parser::KernelFilterCenteringTypes& aCentering)
    -> InputFilterType;

/// @brief A test funtion that uses the input @a aInput to call a @a aModifiedMakeFunctionLabmda and verify radius and
/// centering are correctly assigned.
template <typename InputParserType>
    requires HasBoostOptionalMembers<InputParserType>
void check_make_filter_type_radius_and_centering(const InputParserType& aInput,
                                                 const auto& aModifiedMakeFunctionLambda,
                                                 const plato::test_utilities::TestContext& aTestContext);

/// @brief A test funtion that uses the input @a aInput to call a @a aModifiedMakeFunctionLabmda and verify the target
/// mesh gets set up correctly including its fixed blocks.
template <typename InputParserType>
    requires HasBoostOptionalTargetMeshNameMember<InputParserType>
void check_target_mesh_function(const InputParserType& aInput,
                                const auto& aMakeFunctionLambda,
                                const plato::test_utilities::TestContext& aTestContext);

/// @brief A test funtion that compares the construction of the symmetry filter from using the detail function and the
/// input function.
template <typename InputParserType>
    requires HasAllOptionalMembers<InputParserType>
void check_make_function_from_input_against_parameter_function(const InputParserType& aInput,
                                                               const auto& aDetailMakeFunctionLambda,
                                                               const auto& aInputMakeFunctionLambda,
                                                               const plato::test_utilities::TestContext& aTestContext);

template <typename InputFilterType>
    requires HasAllOptionalMembers<InputFilterType>
auto make_filter_input(const double aRadius, const input_parser::KernelFilterCenteringTypes& aCentering)
    -> InputFilterType
{
    return InputFilterType{/*.filter_radius=*/aRadius,
                           /*.centering_type=*/aCentering,
                           /*.use_relative_radius=*/boost::none,
                           /*.target_mesh_name=*/input_parser::FileName{"target.exo"},
                           /*.number_of_processors=*/boost::none,
                           /*.fixed_blocks=*/boost::none};
}

template <typename InputParserType>
    requires HasBoostOptionalMembers<InputParserType>
void check_make_filter_type_radius_and_centering(const InputParserType& aInput,
                                                 const auto& aModifiedMakeFunctionLambda,
                                                 const plato::test_utilities::TestContext& aTestContext)
{
    const auto tFilterType = aModifiedMakeFunctionLambda(aInput.filter_radius.value(), aInput.centering_type.value());
    EXPECT_EQ(tFilterType.mRadius, aInput.filter_radius.value()) << aTestContext;
    EXPECT_EQ(tFilterType.mFilterCentering, aInput.centering_type.value()) << aTestContext;
}

template <typename InputParserType>
    requires HasBoostOptionalTargetMeshNameMember<InputParserType>
void check_target_mesh_function(const InputParserType& aInput,
                                const auto& aMakeFunctionLambda,
                                const plato::test_utilities::TestContext& aTestContext)
{
    const auto tAnalysisDomainMesh = analysis::AnalysisDomainMesh{"dummySourceMesh", {}};
    const auto tFilterType = aMakeFunctionLambda(aInput, tAnalysisDomainMesh);
    const auto tResultTargetMesh = tFilterType.mTargetMeshFunction(aInput);
    EXPECT_EQ(tResultTargetMesh.filePath(), aInput.target_mesh_name.value().mToken) << aTestContext;
    const auto tGoldMeshFixedOrdinals =
        mesh::Mesh{aInput.target_mesh_name.value().mToken, mesh::fixed_blocks(aInput)}.fixedBlockOrdinals();
    EXPECT_EQ(tResultTargetMesh.fixedBlockOrdinals(), tGoldMeshFixedOrdinals) << aTestContext;
}

template <typename InputParserType>
    requires HasAllOptionalMembers<InputParserType>
void check_make_function_from_input_against_parameter_function(const InputParserType& aInput,
                                                               const auto& aDetailMakeFunctionLambda,
                                                               const auto& aInputMakeFunctionLambda,
                                                               const plato::test_utilities::TestContext& aTestContext)
{
    const auto tFilterTypeFromDetail =
        aDetailMakeFunctionLambda(aInput.filter_radius.value(), aInput.centering_type.value());
    const auto tAnalysisDomainMesh = analysis::AnalysisDomainMesh{"dummySourceMesh", {}};
    const auto tFilterTypeFromInputAndAnalysisDomainMesh = aInputMakeFunctionLambda(aInput, tAnalysisDomainMesh);
    EXPECT_EQ(tFilterTypeFromDetail.mRadius, tFilterTypeFromInputAndAnalysisDomainMesh.mRadius) << aTestContext;
    EXPECT_EQ(tFilterTypeFromDetail.mFilterCentering, tFilterTypeFromInputAndAnalysisDomainMesh.mFilterCentering)
        << aTestContext;
    EXPECT_EQ(tFilterTypeFromDetail.mTargetMeshFunction(aInput).filePath(),
              tFilterTypeFromInputAndAnalysisDomainMesh.mTargetMeshFunction(aInput).filePath())
        << aTestContext;
}

}  // namespace plato::filter::extension::kernel_filters::test_utilities

#endif
