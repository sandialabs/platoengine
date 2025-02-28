#include "plato/process_manager/library/ProcessManagerFactory.hpp"

#include <iterator>

#include "plato/core/InputVariantUtilities.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::process_manager::library
{
[[nodiscard]] std::vector<ProcessManager> make_process_managers(
    const ValidatedProcessManagerInputVector& aValidatedProcessManagerInput)
{
    const auto& tRawInputVector = aValidatedProcessManagerInput.rawInput();
    auto tProcessManagerMap = std::multimap<RunStage, ProcessManager>{};
    for (const auto& tValidatedProcessInput : tRawInputVector)
    {
        const auto tProcessManagerName = core::block_name(tValidatedProcessInput);
        auto tStageAndProcess = core::create_object_from_factory<StageAndProcessManager, ValidatedProcessManagerInput>(
            tProcessManagerName, tValidatedProcessInput);
        if (tStageAndProcess.has_value())
        {
            tProcessManagerMap.insert(std::move(tStageAndProcess).value());
        }
    }
    return to_stage_ordered_vector(tProcessManagerMap);
}

auto unregistered_process_managers(const ValidatedProcessManagerInputVector& aValidatedProcessManagerInput)
    -> std::vector<std::string>
{
    auto tUnregisteredProcessManagers = std::vector<std::string>{};
    utilities::transform_if(
        aValidatedProcessManagerInput.rawInput(), std::back_inserter(tUnregisteredProcessManagers),
        [](const auto& aValidatedProcessInput) { return std::string{core::block_name(aValidatedProcessInput)}; },
        [](const auto& aValidatedProcessInput)
        {
            return !core::is_factory_function_registered<StageAndProcessManager, ValidatedProcessManagerInput>(
                core::block_name(aValidatedProcessInput));
        });
    return tUnregisteredProcessManagers;
}
}  // namespace plato::process_manager::library
