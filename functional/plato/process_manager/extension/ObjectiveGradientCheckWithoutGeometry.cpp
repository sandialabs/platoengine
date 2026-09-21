#include "plato/process_manager/extension/ObjectiveGradientCheckWithoutGeometry.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "plato/core/Function.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/DynamicVectorSerialization.hpp"
#include "plato/linear_algebra/RandomPerturbationVector.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/NodalCoordinateGradientChecker.hpp"
#include "plato/process_manager/extension/CommonInputValidation.hpp"
#include "plato/process_manager/extension/DesignVariableTypes.hpp"
#include "plato/process_manager/extension/DirectionVectorTypes.hpp"
#include "plato/process_manager/extension/ROLUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerLogger.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/utilities/GradientChecker.hpp"
#include "plato/utilities/MPIUtilities.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::process_manager::extension
{
namespace
{

constexpr auto kDefaultDensityValue = 0.5;

[[nodiscard]] auto make_objective_gradient_check_process_manager(
    const library::ValidatedProcessManagerInput& aValidInput) -> library::StageAndProcessManager
{
    return {library::RunStage::kValidate, [aValidInput](const library::ProcessManagerData& aProcessManangerData)
            { ObjectiveGradientCheckWithoutGeometry{aValidInput}.run(aProcessManangerData); }};
}

[[maybe_unused]] const auto kObjectiveGradientCheckParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::objective_gradient_check_without_geometry>{};

[[maybe_unused]] const auto kObjectiveGradientCheckProcessManagerRegistration = library::ProcessManagerRegistration{
    input_parser::block_name<input_parser::objective_gradient_check_without_geometry>(),
    [](const library::ValidatedProcessManagerInput& aValidInput)
    { return make_objective_gradient_check_process_manager(aValidInput); }};

[[maybe_unused]] const auto kObjectiveGradientCheckValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{
        [](const input_parser::objective_gradient_check_without_geometry& aInput)
        { return detail::validate_output_file_name(aInput); },
        [](const input_parser::objective_gradient_check_without_geometry& aInput)
        { return detail::validate_input_mesh_name(aInput); },
        [](const input_parser::objective_gradient_check_without_geometry& aInput)
        { return detail::validate_number_of_steps(aInput); },
        [](const input_parser::objective_gradient_check_without_geometry& aInput)
        { return detail::validate_initial_direction_magnitude(aInput); },
        [](const input_parser::objective_gradient_check_without_geometry& aInput)
        { return detail::validate_step_size_reduction_factor(aInput); },
        [](const input_parser::objective_gradient_check_without_geometry& aInput)
        { return detail::validate_random_direction_seed(aInput); },
        [](const input_parser::objective_gradient_check_without_geometry& aInput)
        {
            return input_validation::error_message_for_empty_parameter(
                input_parser::block_name<input_parser::objective_gradient_check_without_geometry>(),
                aInput.direction_vector_type, "direction_vector_type");
        },
        [](const input_parser::objective_gradient_check_without_geometry& aInput)
        {
            return input_validation::error_message_for_empty_parameter(
                input_parser::block_name<input_parser::objective_gradient_check_without_geometry>(),
                aInput.design_variable_type, "design_variable_type");
        }};

[[nodiscard]] auto objective_gradient_check_input(const library::ValidatedProcessManagerInput& aValidInput)
    -> const input_parser::objective_gradient_check_without_geometry&
{
    return input_validation::get_input_block<input_parser::objective_gradient_check_without_geometry>(aValidInput);
}

[[nodiscard]] auto make_density_analysis_domain_mesh(const std::vector<double>& aDesignVariables,
                                                     const input_parser::DesignVariableTypes& aDesignVariableType,
                                                     const std::filesystem::path& aMeshFile)
    -> analysis::AnalysisDomainMesh
{
    return aDesignVariableType == input_parser::DesignVariableTypes::kNodalDensity
               ? mesh::DesignVariablesConversion{mesh::Mesh{aMeshFile}}.nodalFieldToAnalysisDomainMesh(
                     mesh::NodalFieldVectorReference{aDesignVariables})
               : mesh::DesignVariablesConversion{mesh::Mesh{aMeshFile}}.elementFieldToAnalysisDomainMesh(
                     mesh::ElementFieldVectorReference{aDesignVariables});
}
}  // namespace

ObjectiveGradientCheckWithoutGeometry::ObjectiveGradientCheckWithoutGeometry(
    const library::ValidatedProcessManagerInput& aInput)
    : mInputMeshName(objective_gradient_check_input(aInput).input_mesh_name.value().mToken),
      mOutputFileName(objective_gradient_check_input(aInput).output_file_name.value().mToken),
      mDesignVariableType(objective_gradient_check_input(aInput).design_variable_type.value()),
      mGradientCheckParameters(objective_gradient_check_input(aInput).step_size_reduction_factor.value(),
                               objective_gradient_check_input(aInput).number_of_steps.value(),
                               objective_gradient_check_input(aInput).initial_direction_magnitude.value()),
      mDirectionVectorType(objective_gradient_check_input(aInput).direction_vector_type.value()),
      mSeed(objective_gradient_check_input(aInput).random_direction_seed.value_or(fixed_seed_value()))
{
}

auto ObjectiveGradientCheckWithoutGeometry::runCoordinateImpl(
    const library::ProcessManagerData& aProblem,
    const std::filesystem::path& aInputMeshName,
    const linear_algebra::DynamicVector<double>& aDesignVariables) const -> std::string
{
    const auto tCoordinateFunction = [&](const std::filesystem::path& aMeshFilePath) -> double
    {
        const auto tAnalysisDomainMesh = analysis::AnalysisDomainMesh{aMeshFilePath, {}};
        return aProblem.mObjective.evaluate<core::evaluation::kFunction>(tAnalysisDomainMesh);
    };
    const auto tCoordinateDerivative =
        [&](const std::filesystem::path& aMeshFilePath) -> linear_algebra::DynamicVector<double>
    {
        const auto tAnalysisDomainMesh = analysis::AnalysisDomainMesh{aMeshFilePath, {}};
        return aProblem.mObjective.evaluate<core::evaluation::kFirstDerivative>(tAnalysisDomainMesh);
    };

    const auto tGradientChecker = mesh::make_nodal_coordinate_gradient_checker(
        tCoordinateFunction, tCoordinateDerivative, aInputMeshName, mCommunicator);

    return gradientCheckTable(tGradientChecker, aDesignVariables);
}

auto ObjectiveGradientCheckWithoutGeometry::runDensityImpl(
    const library::ProcessManagerData& aProblem,
    const std::filesystem::path& aInputMeshName,
    const linear_algebra::DynamicVector<double>& aDesignVariables) const -> std::string
{
    const auto tDensityFunction = [&](const linear_algebra::DynamicVector<double>& aDesign) -> double
    {
        const auto tAnalysisDomainMesh =
            make_density_analysis_domain_mesh(aDesign.stdVector(), mDesignVariableType, aInputMeshName);
        return aProblem.mObjective.evaluate<core::evaluation::kFunction>(tAnalysisDomainMesh);
    };

    const auto tDensityDerivative = [&](const linear_algebra::DynamicVector<double>& aDesign,
                                        const linear_algebra::DynamicVector<double>& aDirectionVector) -> double
    {
        const auto tAnalysisDomainMesh =
            make_density_analysis_domain_mesh(aDesign.stdVector(), mDesignVariableType, aInputMeshName);
        return aProblem.mObjective.evaluate<core::evaluation::kFirstDerivative>(tAnalysisDomainMesh)
            .dot(aDirectionVector);
    };

    const auto tDensityGradientChecker = utilities::GradientChecker{tDensityFunction, tDensityDerivative};
    return gradientCheckTable(tDensityGradientChecker, aDesignVariables);
}

void ObjectiveGradientCheckWithoutGeometry::run(const library::ProcessManagerData& aProblem) const
{
    [[maybe_unused]] const auto tTaskLogger =
        library::run_task_log<input_parser::objective_gradient_check_without_geometry>();
    const auto tDesignVariables =
        linear_algebra::DynamicVector<double>{detail::design_variables(mInputMeshName, mDesignVariableType)};
    const auto tGradientCheck = mDesignVariableType == input_parser::DesignVariableTypes::kCoordinate
                                    ? runCoordinateImpl(aProblem, mInputMeshName, tDesignVariables)
                                    : runDensityImpl(aProblem, mInputMeshName, tDesignVariables);

    utilities::execute_on_root(mCommunicator,
                               [&tGradientCheck, tFileName = mOutputFileName]()
                               {
                                   std::ofstream tOutFile(tFileName);
                                   tOutFile << tGradientCheck;
                               });
}

namespace detail
{
auto default_starting_density() -> double { return kDefaultDensityValue; }

auto validate_output_file_name(const input_parser::objective_gradient_check_without_geometry& aInput)
    -> std::optional<std::string>
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::objective_gradient_check_without_geometry>(), aInput.output_file_name,
        "output_file_name");
}

auto validate_input_mesh_name(const input_parser::objective_gradient_check_without_geometry& aInput)
    -> std::optional<std::string>
{
    if (!aInput.input_mesh_name.has_value())
    {
        return input_validation::error_message_for_empty_parameter(
            input_parser::block_name<input_parser::objective_gradient_check_without_geometry>(), aInput.input_mesh_name,
            "input_mesh_name");
    }
    if (!std::filesystem::exists(aInput.input_mesh_name.value().mToken))
    {
        return utilities::concatenate(
            input_parser::block_name<input_parser::objective_gradient_check_without_geometry>(),
            ": Could not find input mesh with name ", aInput.input_mesh_name->mToken);
    }
    return std::nullopt;
}

auto design_variables(const std::filesystem::path& aMeshFile,
                      const input_parser::DesignVariableTypes& aDesignVariableType) -> std::vector<double>
{
    const auto tMesh = mesh::Mesh{aMeshFile};
    if (aDesignVariableType == input_parser::DesignVariableTypes::kCoordinate)
    {
        return third_party_integration::common::flatten(mesh::EntityRetrieval{tMesh}.nodalCoordinates());
    }
    const auto tSize = aDesignVariableType == input_parser::DesignVariableTypes::kElementDensity
                           ? mesh::EntityCounts{tMesh}.numberOfDesignDomainElements()
                           : mesh::EntityCounts{tMesh}.numberOfDesignDomainNodes();

    return std::vector<double>(tSize, default_starting_density());
}

auto perturbation(const input_parser::DirectionVectorTypes& aDirectionVectorType,
                  const std::size_t aSize,
                  const Seed aSeed) -> linear_algebra::DynamicVector<double>
{
    if (aDirectionVectorType == input_parser::DirectionVectorTypes::kRandom)
    {
        auto tEngine = std::default_random_engine{aSeed.mValue};
        return linear_algebra::random_perturbation_vector(aSize, tEngine);
    }
    const auto tDirection = aDirectionVectorType == input_parser::DirectionVectorTypes::kUniformPositive ? 1.0 : -1.0;
    return linear_algebra::DynamicVector<double>{std::vector<double>(aSize, tDirection)};
}

}  // namespace detail

}  // namespace plato::process_manager::extension
