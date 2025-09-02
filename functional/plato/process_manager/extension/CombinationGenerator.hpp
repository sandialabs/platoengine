#ifndef PLATO_PROCESSMANAGER_EXTENSION_COMBINATIONGENERATOR
#define PLATO_PROCESSMANAGER_EXTENSION_COMBINATIONGENERATOR

#include <vector>

#include "plato/utilities/CartesianProduct.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::process_manager::extension
{
using StepValues = std::vector<std::vector<double>>;
using IndexSizeVector = utilities::NamedType<std::vector<std::size_t>, struct IndexSizeVectorTag>;
using SubIndexVector = utilities::NamedType<std::vector<std::size_t>, struct SubIndexVectorTag>;
using IndexStateVector = utilities::NamedType<std::vector<std::size_t>, struct IndexStateVectorTag>;

///@brief CombinationGenerator is a class that can iterate through combinations of vector parameters and appropriately
/// populate a larger space vector
/// eg InitialValues {1,2,3,4}, StepValues {{0,1} , {1,2}} linked with indices {0, 2} would iterate through the
/// combinations: {0,2,1,4} {1,2,1,4} {0,2,2,4} {1,2,2,4}
/// Can be used with std::transform like: std::transform(tCombinationGenerator.begin(), tCombinationGenerator.end(),
/// std::back_inserter(tStates),[](const auto aCombination) { return aCombination; });
class CombinationGenerator
{
   public:
    ///@brief Create a combination generator from the step values @a aStepValues, applying them to the list of
    /// subindices @a SubIndexVector in the initial state @a aInitialValues
    ///@pre StepValues must be of equal size to sub index vector checked with assert.
    CombinationGenerator(const StepValues& aStepValues,
                         const SubIndexVector& aSubIndexVector,
                         const std::vector<double>& aInitialValues);

    [[nodiscard]] auto begin() -> CombinationGenerator;
    [[nodiscard]] auto end() -> CombinationGenerator;
    [[nodiscard]] auto operator*() const -> const std::vector<double>&;
    [[nodiscard]] auto operator++() -> CombinationGenerator&;
    [[nodiscard]] auto operator<=>(const CombinationGenerator& aOther) const -> bool = default;

   private:
    utilities::CartesianProduct mCartesianProduct;
    utilities::CartesianProductIterator mCartesianProductIterator;
    StepValues mStepValues;
    std::vector<std::size_t> mSubIndices;
    std::vector<double> mCurrentStateValues;
};

namespace detail
{

///@brief Given a StepValues @a aStepValues vector of range vectors, determine the sizes of each.
[[nodiscard]] auto sizes_vector(const StepValues& aStepValues) -> IndexSizeVector;

///@brief Given the a vector of all the values @a aCurrentValues, replace values at the subindices listed in @a
/// aSubIndexVector with the current state of the indices @a aIndexStateVector into the step value ranges in @a
/// aStepValues
[[nodiscard]] auto state_values(const std::vector<double>& aCurrentValues,
                                const StepValues& aStepValues,
                                const SubIndexVector& aSubIndexVector,
                                const IndexStateVector& aIndexStateVector) -> std::vector<double>;

}  // namespace detail

}  // namespace plato::process_manager::extension
#endif
