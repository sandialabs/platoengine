#include "plato/integration_tests/test_mass_criteria/RuntimeThrowCriterionInterface.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/CriterionInterface.hpp"

namespace plato::integration_tests::test_mass_criteria
{
auto RuntimeThrowCriterionInterface::value(const analysis::AnalysisDomainMesh& /*unused*/) const -> double
{
    throw std::runtime_error("criterion value throws");
    return 0;
}

auto RuntimeThrowCriterionInterface::gradient(const analysis::AnalysisDomainMesh& /*unused*/) const
    -> std::vector<double>
{
    throw std::runtime_error("criterion gradient throws");
    return {};
}
}  // namespace plato::integration_tests::test_mass_criteria

auto plato_create_runtime_throw_criterion(const std::vector<std::string>& /*unused*/)
    -> std::unique_ptr<::plato::criteria::library::CriterionInterface>
{
    return std::make_unique<::plato::integration_tests::test_mass_criteria::RuntimeThrowCriterionInterface>();
}
