#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTINTERFACE
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTINTERFACE

#include <filesystem>
#include <utility>
#include <vector>

#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/third_party_integration/snopt/SNOPTTypes.hpp"

namespace plato::third_party_integration::snopt
{
/// @brief Solve optimization problem with SNOPT and return resulting design variables
/// @pre The size of @a aInitialGuess must be the same as the sizes of both vectors in @a aBoundConstraints.
[[nodiscard]] auto run_snopt_problem(const std::vector<double>& aInitialGuess,
                                     const SNOPTBounds& aBoundConstraints,
                                     ObjectiveType aObjective,
                                     ConstraintVectorType aConstraints,
                                     const std::filesystem::path& aLogFilePath) -> std::vector<double>;
}  // namespace plato::third_party_integration::snopt

#endif
