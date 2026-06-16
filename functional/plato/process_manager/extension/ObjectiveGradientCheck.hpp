#ifndef PLATO_PROCESSMANAGER_EXTENSION_OBJECTIVEGRADIENTCHECK
#define PLATO_PROCESSMANAGER_EXTENSION_OBJECTIVEGRADIENTCHECK

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <optional>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/process_manager/extension/DesignVariableTypes.hpp"
#include "plato/process_manager/extension/DirectionVectorTypes.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/utilities/GradientChecker.hpp"
#include "plato/utilities/MPIUtilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

// clang-format off
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), objective_gradient_check,
    (plato::input_parser::FileName, input_mesh_name, "Required filename of the mesh to use to compute the gradient check.")
    (plato::input_parser::FileName, output_file_name, "Required filename to use to report the results of the gradient check.")
    (plato::input_parser::DesignVariableTypes, design_variable_type, "Required field specifying what type of design variable to use for the gradient check. "
                                                                               "Inputs can be: 'coordinate', 'element_density', or 'nodal_density'")                                                                                                                                                       
    (unsigned int, number_of_steps, "Required field specifying the number of approximations to evaluate using a finite difference.")
    (double, initial_direction_magnitude, "Required field specifying the magnitude of the perturbation of the design controls.")
    (double, step_size_reduction_factor, "Required field specifying how much the perturbation will be reduced for each step, e.g., 0.1 for log10 step sizes.")
    (unsigned int, random_direction_seed, "Optional field specifying the seed that is used to generate the random perturbation of the controls. "
                                                    "If no seed is provided, the clock will be used to generate a seed.")
    (plato::input_parser::DirectionVectorTypes, direction_vector_type, "Required field specifying how to generate the direction vector. "
                                                                                  "Inputs can be: 'random', 'uniform_positive', or 'uniform_negative'")                                                    
)
// clang-format on

namespace plato::process_manager::extension
{
/// @brief An implementation of a process manager that performs a gradient check using
///  Plato's gradient check functionality and does it independently of the geometry.
class ObjectiveGradientCheck
{
   public:
    explicit ObjectiveGradientCheck(const library::ValidatedProcessManagerInput& aInput);

    /// @brief Take the process manager data @a aProblem and run the objective gradient check.
    void run(const library::ProcessManagerData& aProblem) const;

   private:
    /// @brief Using the process manager data @a aProblem, a given mesh @a aInputMeshName, and design variables defined
    /// on the coordinates @a aDesignVariables creates a gradient checker that perturbs the coordinates.
    [[nodiscard]] auto runCoordinateImpl(const library::ProcessManagerData& aProblem,
                                         const std::filesystem::path& aInputMeshName,
                                         const linear_algebra::DynamicVector<double>& aDesignVariables) const
        -> std::string;

    /// @brief Using the process manager data @a aProblem, a given mesh @a aInputMeshName, and design variables defined
    /// on the coordinates @a aDesignVariables creates a gradient checker that perturbs the element or node centered
    /// densities.
    [[nodiscard]] auto runDensityImpl(const library::ProcessManagerData& aProblem,
                                      const std::filesystem::path& aInputMeshName,
                                      const linear_algebra::DynamicVector<double>& aDesignVariables) const
        -> std::string;

    /// @brief Given a gradient checker type @a aGradientChecker and the design variables @a aDesignVariables generate
    /// and output the table and return as a string.
    template <typename GradientCheckerType>
    [[nodiscard]] auto gradientCheckTable(const GradientCheckerType& aGradientChecker,
                                          const linear_algebra::DynamicVector<double>& aDesignVariables) const
        -> std::string;

    constexpr static auto mDefaultRandomSeed = 42U;

    std::filesystem::path mInputMeshName;
    std::filesystem::path mOutputFileName;
    input_parser::DesignVariableTypes mDesignVariableType;
    utilities::GradientCheckParameters mGradientCheckParameters;
    input_parser::DirectionVectorTypes mDirectionVectorType;
    unsigned int mSeed = mDefaultRandomSeed;
    boost::mpi::communicator mCommunicator;
};

namespace detail
{
/// @brief Return the default starting value for nodal and element densities
[[nodiscard]] auto default_starting_density() -> double;

/// @brief Validate that an output_file_name is specified in the input @a aInput
[[nodiscard]] auto validate_output_file_name(const input_parser::objective_gradient_check& aInput)
    -> std::optional<std::string>;

/// @brief Validate that an input_mesh_name is specified in the input @a aInput and that the file is on disk.
[[nodiscard]] auto validate_input_mesh_name(const input_parser::objective_gradient_check& aInput)
    -> std::optional<std::string>;

/// @brief Generate a set of design variables for the initial configuration using the type specified in the validated
/// input @a aInput
[[nodiscard]] auto design_variables(const std::filesystem::path& aMeshFile,
                                    const input_parser::DesignVariableTypes& aDesignVariableType)
    -> std::vector<double>;

using Seed = utilities::NamedType<unsigned int, struct SeedTag>;

/// @brief Generate a perturbation vector sized @a aSize and populated based on @a aDirectionVectorType
[[nodiscard]] auto perturbation(const input_parser::DirectionVectorTypes& aDirectionVectorType,
                                std::size_t aSize,
                                Seed aSeed) -> linear_algebra::DynamicVector<double>;

}  // namespace detail

template <typename GradientCheckerType>
auto ObjectiveGradientCheck::gradientCheckTable(const GradientCheckerType& aGradientChecker,
                                                const linear_algebra::DynamicVector<double>& aDesignVariables) const
    -> std::string
{
    auto tPerturbation = linear_algebra::DynamicVector<double>{};

    utilities::execute_on_root(
        mCommunicator, [this, &aDesignVariables](linear_algebra::DynamicVector<double>& aPerturbation)
        { aPerturbation = detail::perturbation(mDirectionVectorType, aDesignVariables.size(), detail::Seed{mSeed}); },
        tPerturbation);
    const auto tRootRank = 0U;
    boost::mpi::broadcast(mCommunicator, tPerturbation, tRootRank);

    return std::format("{}\n{}", aGradientChecker.header(),
                       aGradientChecker.table(aDesignVariables, tPerturbation, mGradientCheckParameters));
}

}  // namespace plato::process_manager::extension
#endif
