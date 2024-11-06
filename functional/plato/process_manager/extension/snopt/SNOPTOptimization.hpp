#ifndef PLATO_PROCESSMANAGER_EXTENSION_SNOPT_SNOPTOPTIMIZATION
#define PLATO_PROCESSMANAGER_EXTENSION_SNOPT_SNOPTOPTIMIZATION

#include <filesystem>
#include <optional>
#include <string>

#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/third_party_integration/snopt/SNOPTInterface.hpp"
#include "plato/third_party_integration/snopt/SNOPTTypes.hpp"

namespace plato::input_parser
{
struct snopt_optimization;
}

namespace plato::process_manager::library
{
struct ProcessManagerData;
}

namespace plato::process_manager::extension::snopt
{
/// @brief An implementation of a process manager that performs optimization using SNOPT.
class SNOPTOptimization
{
   public:
    using ValidatedOptimizationParameters = core::ValidatedInputTypeWrapper<input_parser::snopt_optimization>;

    explicit SNOPTOptimization(const ValidatedOptimizationParameters& aInput);

    void run(const library::ProcessManagerData& aProcessManagerData) const;

   private:
    std::filesystem::path mOutputLogFileName;
    third_party_integration::snopt::SNOPTOptions mOptions;
};

namespace detail
{

[[nodiscard]] auto make_constraints(const library::ProcessManagerData& aProcessManagerData)
    -> third_party_integration::snopt::InterfaceConstraintVectorType;

[[nodiscard]] std::optional<std::string> validate_time_limit_in_minutes(const input_parser::snopt_optimization& aInput);

}  // namespace detail

}  // namespace plato::process_manager::extension::snopt

#endif
