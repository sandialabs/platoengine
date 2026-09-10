#include "plato/third_party_integration/snopt/test_utilities/TestUtilities.hpp"

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <utility>
#include <vector>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/output/OutputInfo.hpp"
#include "plato/output/OutputManager.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"
#include "plato/third_party_integration/snopt/SNOPTInterface.hpp"
#include "plato/third_party_integration/snopt/SNOPTTypes.hpp"

namespace plato::third_party_integration::snopt::test_utilities
{

auto to_dynamic_vector(const plato::test_utilities::TwoDVector& aX) -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{std::vector{aX(0), aX(1)}};
}

auto run_snopt_write_output_counter_test(const std::filesystem::path& aLogFilePath,
                                         const boost::mpi::communicator& aCommunicator) -> int
{
    const auto tInitialGuess = std::vector{1.2, 0.8};
    const auto tBounds =
        std::pair{std::vector{-kSNOPTUnbounded, -kSNOPTUnbounded}, std::vector{kSNOPTUnbounded, kSNOPTUnbounded}};

    int tOutputCount = 1;
    auto tOutputFunction = [&tOutputCount](const linear_algebra::DynamicVector<double>&, const output::OutputInfo&)
    { tOutputCount++; };
    auto tOutputManager = output::OutputManager{tOutputFunction, output::OutputMode::kEveryIterationAppend};

    const auto tSolution = run_snopt_problem(
        tInitialGuess, tBounds, test_utilities::rosenbrock_dynamic_vector_function(plato::test_utilities::Rosenbrock{}),
        {}, std::move(tOutputManager), aLogFilePath, SNOPTOptions{}, aCommunicator);
    /* SNOPT Output:
    No. of iterations                  19   Objective            5.7675691061E-17
    No. of major iterations            17   Linear    obj. term  0.0000000000E+00
                                            Nonlinear obj. term  5.7675691061E-17
    User function calls (total)        21
    No. of superbasics                  2   No. of basic nonlinears             0
    No. of degenerate steps             0   Percentage                       0.00
    Max x                       2 1.0E+00   Max pi                      1 1.0E+00
    Max Primal infeas           0 0.0E+00   Max Dual infeas             1 2.8E-07*/

    return tOutputCount;
}

}  // namespace plato::third_party_integration::snopt::test_utilities
