#ifndef PLATO_PROCESSMANAGER_EXTENSION_INITIALGEOMETRYWRITER
#define PLATO_PROCESSMANAGER_EXTENSION_INITIALGEOMETRYWRITER

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

// clang-format off
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), initial_geometry_writer,
    (plato::input_parser::FileName, file_name, "Optional: do not use.")
)
// clang-format on

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

namespace plato::process_manager::extension
{
/// @brief process manager that runs the output function for the problem geometry using the initial guess.
///
/// This can be helpful to verify that an initial guess has resulted in the correct geometry.
class InitialGeometryWriter
{
   public:
    explicit InitialGeometryWriter() = default;

    /// @brief calls the output function for the geometry stored in @a aProcessManagerData using the geometry's initial
    /// guess.
    static void run(const library::ProcessManagerData& aProcessManagerData);
};

}  // namespace plato::process_manager::extension

#endif
