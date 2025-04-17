#include "plato/process_manager/library/ProcessManagerFactory.hpp"

#include <iterator>

#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::process_manager::library
{
namespace
{
enum struct RegistrationStatus
{
    kRegistered,
    kUnregistered
};

struct NewCheckRegistrationStatus
{
    RegistrationStatus mRegistrationStatus;

    template <typename T>
    auto operator()(const T& aValidatedProcessInput) const
    {
        const auto tIsRegistered =
            core::is_factory_function_registered<StageAndProcessManager, NewValidatedProcessManagerInput>(
                aValidatedProcessInput.rawInput().mBlockName);
        return mRegistrationStatus == RegistrationStatus::kRegistered ? tIsRegistered : !tIsRegistered;
    }
};

}  // namespace

auto make_process_managers(const NewValidatedProcessManagers& aValidatedProcessManagerInput)
    -> std::vector<ProcessManager>
{
    auto tProcessManagerMap = std::multimap<RunStage, ProcessManager>{};
    utilities::transform_if(
        aValidatedProcessManagerInput.rawInput(), std::inserter(tProcessManagerMap, tProcessManagerMap.begin()),
        [](const auto& aValidatedProcessInput)
        {
            return core::create_object_from_factory<StageAndProcessManager, NewValidatedProcessManagerInput>(
                       aValidatedProcessInput.rawInput().mBlockName, aValidatedProcessInput)
                .value();
        },
        NewCheckRegistrationStatus{RegistrationStatus::kRegistered});

    return to_stage_ordered_vector(tProcessManagerMap);
}

}  // namespace plato::process_manager::library
