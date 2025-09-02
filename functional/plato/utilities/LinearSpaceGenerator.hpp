#ifndef PLATO_UTILITIES_LINEARSPACEGENERATOR
#define PLATO_UTILITIES_LINEARSPACEGENERATOR

#include <string>
#include <string_view>
#include <vector>

namespace plato::utilities
{
/// @brief The purpose of this is to generate a set of linearly spaced values.
/// @pre mStartingValue < mEndingValue
/// @note If mStep doesn't event divide the range, the last value will be less than mEndingValue
/// @sa ParameterSweep.
struct LinearSpaceGenerator
{
    double mStartingValue = 1;
    double mStep = 1;
    double mEndingValue = 10;

    ///@brief Compute the vector of steps for the linear space
    [[nodiscard]] auto steps() const -> std::vector<double>;
};

}  // namespace plato::utilities

#endif
