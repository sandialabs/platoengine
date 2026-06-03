#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <boost/none.hpp>

#include "plato/components/ComponentType.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputDefinitions.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/DesignVariableTypes.hpp"
#include "plato/process_manager/extension/DirectionVectorTypes.hpp"
#include "plato/process_manager/extension/ObjectiveGradientCheck.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/MPIUtilities.hpp"

namespace plato::process_manager::extension::parallel_unittest
{
namespace
{
using ObjectiveGradientCheckParallelFixture = third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;

constexpr auto kOutputFile = std::string_view{"output.txt"};

const auto kPlatoEngineQuadraticObjectiveInput =
    input_parser::objective{/*.name=*/std::string{"quadratic"},
                            /*.active=*/true,
                            /*.app=*/input_parser::AppName{std::string{input_parser::kBuiltinAppName}},
                            /*.criterion=*/input_parser::CriterionName{"quadratic"},
                            /*.number_of_processors=*/1U,
                            /*.input_files=*/boost::none,
                            /*.aggregation_weight=*/1.0,
                            /*.normalize_by_initial_value=*/false,
                            /*.objective_goal=*/boost::none};

[[nodiscard]] auto make_parallel_input(const input_parser::objective_gradient_check& aObjectiveGradientCheckInput)
{
    auto tInput =
        geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() | aObjectiveGradientCheckInput;
    for ([[maybe_unused]] const auto tIndex : utilities::IndexRange{boost::mpi::communicator{}.size()})
    {
        tInput = tInput | kPlatoEngineQuadraticObjectiveInput;
    }
    return tInput;
}

}  // namespace
TEST_F(ObjectiveGradientCheckParallelFixture, ParallelRun)
{
    const auto tObjectiveGradientCheckInput =
        input_parser::objective_gradient_check{input_parser::FileName{mMeshFilePath},
                                               input_parser::FileName{std::string{kOutputFile}},
                                               input_parser::DesignVariableTypes::kCoordinate,
                                               /*number_of_steps*/ 10U,
                                               /*initial_direction_magnitude*/ 0.1,
                                               /*step_size_reduction_factor*/ 0.1,
                                               /*random_direction_seed*/ 42U,
                                               input_parser::DirectionVectorTypes::kRandom};
    const auto tInput = make_parallel_input(tObjectiveGradientCheckInput);
    const auto tValidatedInput = input_validation::make_validated_input(tInput);
    ASSERT_FALSE(tValidatedInput.hasError()) << tValidatedInput.error();
    const auto tProblem = library::make_process_manager_data(tValidatedInput.value());
    const auto tValidatedCriteriaGradientInput =
        tValidatedInput.value().get<components::ComponentType::kProcessManager>().rawInput().back();
    const auto tObjectiveGradientCheck = ObjectiveGradientCheck{tValidatedCriteriaGradientInput};
    tObjectiveGradientCheck.run(tProblem);

    utilities::execute_on_root(boost::mpi::communicator{},
                               []()
                               {
                                   plato::test_utilities::test_for_existence_and_remove(
                                       {kOutputFile}, TEST_CONTEXT("Parallel run of objective gradient check."));
                               });
}

}  // namespace plato::process_manager::extension::parallel_unittest
