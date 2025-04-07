#ifndef PLATO_PROCESSMANAGER_EXTENSION_ROLSENSITIVITYCHECK
#define PLATO_PROCESSMANAGER_EXTENSION_ROLSENSITIVITYCHECK

#include <filesystem>
#include <optional>
#include <vector>

#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

namespace plato::input_parser
{
struct sensitivity_check;
}

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

// clang-format off
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), new_sensitivity_check,
    (plato::input_parser::FileName, output_file_name, "Required filename to use to report the results of the sensitivity check.")
)
// clang-format on

namespace plato::process_manager::extension
{
/// @brief An implementation of a process manager that performs a gradient check of mainly the
///  mapping from design variables to a mesh, using a nodal sum as an objective.
///
/// The purpose of this is to isolate the geometry creation step from criteria evaluation for
/// gradient checks. For example, it can be useful to gradient check just ESP. This is mainly
/// useful for shape optimization since the objective is a sum of nodal coordinates.
class SensitivityCheck
{
   public:
    using ValidatedSensitivityCheckInput = core::ValidatedInputTypeWrapper<input_parser::sensitivity_check>;

    explicit SensitivityCheck(const ValidatedSensitivityCheckInput& aInput);
    explicit SensitivityCheck(const library::NewValidatedProcessManagerInput& aInput);

    void run(const library::ProcessManagerData& aProcessManagerData) const;

   private:
    std::filesystem::path mOutputFileName;
};

namespace detail
{
[[nodiscard]] std::optional<std::string> validate_output_file_name(const input_parser::sensitivity_check& aInput);
}

}  // namespace plato::process_manager::extension

#endif
