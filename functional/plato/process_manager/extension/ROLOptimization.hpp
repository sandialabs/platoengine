#ifndef PLATO_PROCESSMANAGER_EXTENSION_ROLOPTIMIZATION
#define PLATO_PROCESSMANAGER_EXTENSION_ROLOPTIMIZATION

#include <boost/mpi/communicator.hpp>
#include <optional>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/third_party_integration/rol/OptimizationParameters.hpp"

namespace plato::input_parser
{
struct rol_optimization;
}

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

// clang-format off
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), rol_optimization,
    (plato::input_parser::FileName, input_file_name, "Optional filename of the XML file specifying the ROL input parameters.")
    (plato::input_parser::FileName, export_settings_file_name, "Optional filename to trigger an exporting of all the set parameters of ROL.")
    (unsigned int, max_iterations, "Optional command to override the maximum number of outer iterations given in an input file.")
    (double, step_tolerance, "Optional command to override the step tolerance given in an input file. The step tolerance is the norm of the change in all of the controls from one step to the next. This number should scale with the problem size.")
    (double, gradient_tolerance, "Optional command to override the gradient tolerance given in an input file. The gradient tolerance is the norm of the gradient which at an optimal solution should be zero. This number should scale with the problem size.")
    (double, initial_search_radius, "Optional command to override the starting search radius for the controls. This number should increase with the problem size.")
    (bool, verbose_output, "Optional command to increase the verbosity of the ROL algorithms to their maximum.")
    (bool, approximate_hessian, "Optional command to override the default zero-Hessian behavior. The Hessian will be approximated with a secant method.")
    (bool, output_design_history, "Optional command to write the entire design history to output file. Default is to only write the final design.")
)
// clang-format on

namespace plato::process_manager::extension
{
/// @brief An implementation of a process manager that performs optimization  using ROL.
class ROLOptimization
{
   public:
    explicit ROLOptimization(const library::ValidatedProcessManagerInput& aInput);

    void run(const library::ProcessManagerData& aProcessManagerData) const;

   private:
    third_party_integration::rol::OptimizationParameters mROLOptions;
    boost::mpi::communicator mCommunicator{};
};

namespace detail
{
[[nodiscard]] auto validate_rol_max_iterations(const input_parser::rol_optimization& aInput)
    -> std::optional<std::string>;
[[nodiscard]] auto validate_step_tolerance(const input_parser::rol_optimization& aInput) -> std::optional<std::string>;
[[nodiscard]] auto validate_gradient_tolerance(const input_parser::rol_optimization& aInput)
    -> std::optional<std::string>;
[[nodiscard]] auto validate_initial_search_radius(const input_parser::rol_optimization& aInput)
    -> std::optional<std::string>;
[[nodiscard]] auto validate_unique_output_name(const input_parser::rol_optimization& aInput)
    -> std::optional<std::string>;

}  // namespace detail

}  // namespace plato::process_manager::extension

#endif
