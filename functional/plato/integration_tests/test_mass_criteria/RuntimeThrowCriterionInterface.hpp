#ifndef PLATO_INTEGRATION_TESTS_TESTMASSCRITERIA_RUNTIMETHROWCRITERIONINTERFACE
#define PLATO_INTEGRATION_TESTS_TESTMASSCRITERIA_RUNTIMETHROWCRITERIONINTERFACE

#include "plato/criteria/library/CriterionInterface.hpp"

namespace plato::integration_tests::test_mass_criteria
{
/// @brief Serial version a test criterion that simply throws a runtime error. Used for checking runtime exit status.
class RuntimeThrowCriterionInterface : public criteria::library::CriterionInterface
{
   public:
    ///@brief required by the CriterionInterface, just throws a runtime error in this implementation
    [[nodiscard]] auto value(const analysis::AnalysisDomainMesh& /*unused*/) const -> double override;

    ///@brief required by the CriterionInterface, just throws a runtime error in this implementation
    [[nodiscard]] auto gradient(const analysis::AnalysisDomainMesh& /*unused*/) const -> std::vector<double> override;
};
}  // namespace plato::integration_tests::test_mass_criteria

extern "C" auto plato_create_runtime_throw_criterion(const std::vector<std::string>& /*unused*/)
    -> std::unique_ptr<::plato::criteria::library::CriterionInterface>;
#endif
