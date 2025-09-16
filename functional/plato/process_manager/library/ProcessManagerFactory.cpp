#include "plato/process_manager/library/ProcessManagerFactory.hpp"

#include <cstdint>
#include <iterator>

#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::process_manager::library
{
namespace
{
enum struct RegistrationStatus : std::uint8_t
{
    kRegistered,
    kUnregistered
};

struct CheckRegistrationStatus
{
    RegistrationStatus mRegistrationStatus;

    template <typename T>
    auto operator()(const T& aValidatedProcessInput) const
    {
        const auto tIsRegistered =
            core::is_factory_function_registered<StageAndProcessManager, ValidatedProcessManagerInput>(
                aValidatedProcessInput.rawInput().mBlockName);
        return mRegistrationStatus == RegistrationStatus::kRegistered ? tIsRegistered : !tIsRegistered;
    }
};

}  // namespace

auto make_process_managers(const ValidatedProcessManagers& aValidatedProcessManagerInput) -> std::vector<ProcessManager>
{
    auto tProcessManagerMap = std::multimap<RunStage, ProcessManager>{};
    utilities::transform_if(
        aValidatedProcessManagerInput.rawInput(), std::inserter(tProcessManagerMap, tProcessManagerMap.begin()),
        [](const auto& aValidatedProcessInput)
        {
            return core::create_object_from_factory<StageAndProcessManager, ValidatedProcessManagerInput>(
                       aValidatedProcessInput.rawInput().mBlockName, aValidatedProcessInput)
                .value();
        },
        CheckRegistrationStatus{RegistrationStatus::kRegistered});

    return to_stage_ordered_vector(tProcessManagerMap);
}

}  // namespace plato::process_manager::library
