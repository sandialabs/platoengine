#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/core/Function.hpp"
#include "plato/core/ParallelFunction.hpp"
#include "plato/core/test_utilities/Utilities.hpp"
#include "plato/test_utilities/ParallelFunction.hpp"
#include "plato/test_utilities/ParallelTestWrapper.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"
#include "plato/test_utilities/TwoDTestTypesSerialization.hpp"

namespace plato::core::parallel_unittest
{
TEST(ParallelFunction, AdaptRosenbrock)
{
    namespace ptu = plato::test_utilities;

    const auto tA = double{1.0};
    const auto tB = double{10.0};
    const auto tRosenbrock = ptu::Rosenbrock{tA, tB};
    // Serial
    const auto tF = [tRosenbrock](const ptu::TwoDVector& aVector)
    { return tRosenbrock.f(aVector.mData[0], aVector.mData[1]); };
    const auto tDF = [tRosenbrock](const ptu::TwoDVector& aVector)
    { return tRosenbrock.df(aVector.mData[0], aVector.mData[1]); };
    const auto tSerialFunction = make_function_with_first_derivative(tF, tDF);

    // Parallel
    const auto tComm = boost::mpi::communicator{};
    const auto tParallelFunction = ptu::make_parallel_function(
        ptu::ParallelTestFunctionWrapper<double, const ptu::TwoDVector&>{tF},
        ptu::ParallelTestFunctionWrapper<ptu::TwoDVector, const ptu::TwoDVector&>{tDF}, tComm);

    const auto tAdaptedParallelFunction = core::adapt_parallel_function(tParallelFunction, tComm);

    EXPECT_GT(tComm.size(), 1);

    const auto tControl = ptu::TwoDVector{1.0, -2.0};
    if (tComm.rank() == 0)
    {
        EXPECT_EQ(tSerialFunction.evaluate<evaluation::kFunction>(tControl),
                  tAdaptedParallelFunction.evaluate<evaluation::kFunction>(tControl));
        EXPECT_EQ(tSerialFunction.evaluate<evaluation::kFirstDerivative>(tControl),
                  tAdaptedParallelFunction.evaluate<evaluation::kFirstDerivative>(tControl));
    }
    else
    {
        EXPECT_EQ(0.0, tAdaptedParallelFunction.evaluate<evaluation::kFunction>(tControl));
        EXPECT_EQ(0.0 * tSerialFunction.evaluate<evaluation::kFirstDerivative>(tControl),
                  tAdaptedParallelFunction.evaluate<evaluation::kFirstDerivative>(tControl));
    }
}

TEST(ParallelFunction, BroadCastFromRoot)
{
    namespace ptu = plato::test_utilities;

    const auto tF = [](const double aArg) { return aArg * aArg; };
    const auto tDF = [](const double aArg) { return 2.0 * aArg; };

    const auto tComm = boost::mpi::communicator{};
    EXPECT_GT(tComm.size(), 1);

    const auto tAdaptedParallelFunction = core::adapt_parallel_function(
        ptu::make_parallel_function(ptu::ParallelTestFunctionWrapper<double, double>{tF},
                                    ptu::ParallelTestFunctionWrapper<double, double>{tDF}, tComm),
        tComm);

    const auto tArg{5.6};
    if (tComm.rank() == 0)
    {
        EXPECT_EQ(tAdaptedParallelFunction.evaluate<evaluation::kFunction>(tArg), tArg * tArg);
    }
    else
    {
        EXPECT_EQ(tAdaptedParallelFunction.evaluate<evaluation::kFunction>(tArg), 0.0);
    }

    EXPECT_EQ(broadcast_from_root(tComm, tAdaptedParallelFunction.evaluate<evaluation::kFunction>(tArg)), tArg * tArg);
}

}  // namespace plato::core::parallel_unittest
