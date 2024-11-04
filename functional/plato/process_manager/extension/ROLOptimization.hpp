#ifndef PLATO_PROCESSMANAGER_EXTENSION_ROLOPTIMIZATION
#define PLATO_PROCESSMANAGER_EXTENSION_ROLOPTIMIZATION

#include <boost/mpi/communicator.hpp>
#include <optional>

#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/third_party_integration/rol/OptimizationParameters.hpp"

namespace plato::input_parser
{
struct rol_optimization;
}

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

namespace plato::process_manager::extension
{
/// @brief An implementation of a process manager that performs optimization  using ROL.
class ROLOptimization
{
   public:
    using ValidatedOptimizationParameters = core::ValidatedInputTypeWrapper<input_parser::rol_optimization>;

    explicit ROLOptimization(const ValidatedOptimizationParameters& aInput);

    void run(const library::ProcessManagerData& aProcessManagerData) const;

   private:
    third_party_integration::rol::OptimizationParameters mROLOptions;
    boost::mpi::communicator mCommunicator{};
};

namespace detail
{
[[nodiscard]] std::optional<std::string> validate_rol_max_iterations(const input_parser::rol_optimization& aInput);
[[nodiscard]] std::optional<std::string> validate_step_tolerance(const input_parser::rol_optimization& aInput);
[[nodiscard]] std::optional<std::string> validate_gradient_tolerance(const input_parser::rol_optimization& aInput);
[[nodiscard]] std::optional<std::string> validate_initial_search_radius(const input_parser::rol_optimization& aInput);
[[nodiscard]] std::optional<std::string> validate_unique_output_name(const input_parser::rol_optimization& aInput);

}  // namespace detail

}  // namespace plato::process_manager::extension

#endif
