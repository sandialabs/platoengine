#ifndef PLATO_UTILITIES_LOGSPACEGENERATOR
#define PLATO_UTILITIES_LOGSPACEGENERATOR

#include <vector>

namespace plato::utilities
{
/// @brief The purpose of this is to generate a set of logarithmically spaced values.
///
/// The main use is in the gradient checker for generating finite difference step sizes.
struct LogSpaceGenerator
{
    double mStartingMagnitude = 1;
    double mStepRatio = 1.0;
    unsigned int mNumberOfSteps = 1;

    [[nodiscard]] std::vector<double> steps() const;
};
}  // namespace plato::utilities

#endif
