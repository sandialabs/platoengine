#ifndef PLATO_PROCESS_MANAGER_EXTENSION_INITIALGEOMETRYWRITER
#define PLATO_PROCESS_MANAGER_EXTENSION_INITIALGEOMETRYWRITER

#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

// clang-format off
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), initial_geometry_writer,
    (plato::input_parser::CrossReference<plato::components::ComponentType::kFilter>, filter, "Name of the filter to apply. "
                                                                                               "Only required if more than one filter is specified.")
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
    void run(const library::ProcessManagerData& aProcessManagerData) const;
};

}  // namespace plato::process_manager::extension

#endif
