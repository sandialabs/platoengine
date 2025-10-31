#ifndef PLATO_INTEGRATION_TESTS_TESTMASSCRITERIA_MASSPROPERTIESVECTORINTERFACE
#define PLATO_INTEGRATION_TESTS_TESTMASSCRITERIA_MASSPROPERTIESVECTORINTERFACE

#include "plato/criteria/library/VectorCriterionInterface.hpp"

namespace plato::integration_tests::test_mass_criteria
{
/// @brief A made-up vector interface for testing the named vector component features.
/// @note This does not actually compute any mass properties!
class MassPropertiesInterface : public criteria::library::VectorCriterionInterface
{
   public:
    [[nodiscard]] auto value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
        -> std::vector<double> override;

    [[nodiscard]] auto rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                              const std::vector<double>& aDirectionVector) const
        -> std::vector<double> override;

    [[nodiscard]] auto rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const std::vector<double>& aDualVector) const
        -> std::vector<double> override;

   private:
    std::vector<std::size_t> mComponentIndices;
};
}  // namespace plato::integration_tests::test_mass_criteria

extern "C" auto plato_create_mass_properties_vector_criterion(const std::vector<std::string>&)
    -> std::unique_ptr<::plato::criteria::library::VectorCriterionInterface>;

#endif
