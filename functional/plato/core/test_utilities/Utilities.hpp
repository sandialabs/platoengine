#ifndef PLATO_CORE_TEST_UTILITIES_UTILITIES
#define PLATO_CORE_TEST_UTILITIES_UTILITIES

#include "plato/core/Function.hpp"
#include "plato/test_utilities/Penalty.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::core::test_utilities
{
[[nodiscard]] inline auto make_rosenbrock_function(const plato::test_utilities::Rosenbrock& aRosenbrock)
{
    namespace pftu = plato::test_utilities;
    return make_function([aRosenbrock](const pftu::TwoDVector& aX) { return aRosenbrock.f(aX(0), aX(1)); },
                         [aRosenbrock](const pftu::TwoDVector& aX) { return aRosenbrock.df(aX(0), aX(1)); });
}

[[nodiscard]] inline auto make_penalty_function(const plato::test_utilities::Penalty& aPenalty)
{
    namespace pftu = plato::test_utilities;
    return make_function([aPenalty](const pftu::TwoDVector& aX) { return aPenalty.f(aX(0), aX(1)); },
                         [aPenalty](const pftu::TwoDVector& aX) { return aPenalty.df(aX(0), aX(1)); });
}

}  // namespace plato::core::test_utilities

#endif
