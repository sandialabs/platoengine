#include "plato/process_manager/library/ProcessManagerFactory.hpp"

#include <iterator>

#include "plato/core/InputVariantUtilities.hpp"
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
struct CheckRegistrationStatus
{
    RegistrationStatus mRegistrationStatus;

    template <typename T>
    auto operator()(const T& aValidatedProcessInput) const
    {
        const auto tIsRegistered =
            core::is_factory_function_registered<StageAndProcessManager, ValidatedProcessManagerInput>(
                core::block_name(aValidatedProcessInput));
        return mRegistrationStatus == RegistrationStatus::kRegistered ? tIsRegistered : !tIsRegistered;
    }
};

}  // namespace

[[nodiscard]] std::vector<ProcessManager> make_process_managers(
    const ValidatedProcessManagerInputVector& aValidatedProcessManagerInput)
{
    auto tProcessManagerMap = std::multimap<RunStage, ProcessManager>{};
    utilities::transform_if(
        aValidatedProcessManagerInput.rawInput(), std::inserter(tProcessManagerMap, tProcessManagerMap.begin()),
        [](const auto& aValidatedProcessInput)
        {
            return core::create_object_from_factory<StageAndProcessManager, ValidatedProcessManagerInput>(
                       core::block_name(aValidatedProcessInput), aValidatedProcessInput)
                .value();
        },
        CheckRegistrationStatus{RegistrationStatus::kRegistered});

    return to_stage_ordered_vector(tProcessManagerMap);
}

auto unregistered_process_managers(const ValidatedProcessManagerInputVector& aValidatedProcessManagerInput)
    -> std::vector<std::string>
{
    auto tUnregisteredProcessManagers = std::vector<std::string>{};
    utilities::transform_if(
        aValidatedProcessManagerInput.rawInput(), std::back_inserter(tUnregisteredProcessManagers),
        [](const auto& aValidatedProcessInput) { return std::string{core::block_name(aValidatedProcessInput)}; },
        CheckRegistrationStatus{RegistrationStatus::kUnregistered});
    return tUnregisteredProcessManagers;
}
}  // namespace plato::process_manager::library
