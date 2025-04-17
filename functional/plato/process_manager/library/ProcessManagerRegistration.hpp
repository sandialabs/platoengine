#ifndef PLATO_PROCESSMANAGER_LIBRARY_PROCESSMANAGERREGISTRATION
#define PLATO_PROCESSMANAGER_LIBRARY_PROCESSMANAGERREGISTRATION

#include <type_traits>
#include <variant>

#include "plato/core/FactoryRegistration.hpp"
#include "plato/core/InputVariantUtilities.hpp"
#include "plato/core/VariantInputBuilder.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

namespace plato::process_manager::library
{
/// @brief ProcessManager is the object that is used to peform some operation on ProcessManagerData.
///  This could be a full optimization run or a gradient check.
using ProcessManager = std::function<void(const ProcessManagerData&)>;

/// @brief This associates a ProcessManager with a stage in which to run it.
/// This is the return type of the factory so that each ProcessManager can be run in the correct order.
using StageAndProcessManager = std::pair<RunStage, ProcessManager>;

/// @brief The input for a single ProcessManager, which is a variant with alternatives corresponding to
/// input blocks satisfying IsProcessManagerInput.
using ProcessManagerInput = core::InputVariant<input_parser::ParsedInput, input_parser::IsProcessManagerInput>;

/// @brief Fully validated ProcessManager input vector
using ValidatedProcessManagerInputVector = core::ValidatedInputTypeWrapper<
    std::vector<core::ValidatedInputVariant<input_parser::ParsedInput, input_parser::IsProcessManagerInput>>>;
using NewValidatedProcessManagers =
    input_validation::ValidatedComponentType<input_parser::ComponentType::kProcessManager>;

/// @brief A single validated ProcessManager input variant
using ValidatedProcessManagerInput = typename ValidatedProcessManagerInputVector::RawInputType::value_type;

/// @brief Factory registration type
using ProcessManagerRegistration = core::FactoryRegistration<StageAndProcessManager, ValidatedProcessManagerInput>;

/// @brief A single validated ProcessManager input variant
using NewValidatedProcessManagerInput =
    input_validation::ValidatedInputDataBlock<input_parser::ComponentType::kProcessManager>;
/// @brief Factory registration type
using NewProcessManagerRegistration =
    core::FactoryRegistration<StageAndProcessManager, NewValidatedProcessManagerInput>;

/// @brief Checks if a ProcessManager creation function is registered with name @a aFunctionName.
[[nodiscard]] bool is_process_manager_function_registered(const std::string_view aFunctionName);

/// @brief Checks if a ProcessManager creation function is registered with name @a aFunctionName.
[[nodiscard]] auto is_new_process_manager_function_registered(std::string_view aFunctionName) -> bool;

}  // namespace plato::process_manager::library
#endif
