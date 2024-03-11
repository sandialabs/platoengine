#ifndef PLATO_MAIN_LIBRARY_OPTIMIZATIONPROBLEM
#define PLATO_MAIN_LIBRARY_OPTIMIZATIONPROBLEM

#include <ROL_Problem.hpp>
#include <boost/mpi/communicator.hpp>
#include <string_view>

#include "plato/core/ValidationRegistration.hpp"
#include "plato/main/library/PlatoProblem.hpp"

namespace plato::main::library
{
class OptimizationProblem
{
   public:
    ///@brief Construct a new Optimization Problem object
    ///@param aInputFile filename of the block structured plato input file
    explicit OptimizationProblem(const std::string_view aInputFile);

    ///@brief Run gradient check on the parsed/defined ROL problem
    void gradientCheck() const;

    ///@brief Run constraint check on the parsed/defined ROL problem
    void constraintCheck() const;

    ///@brief Run a gradient check using a linear pseudo-objective so that errors in the parameter sensitivities might
    /// be evident
    void sensitivityCheck() const;

    ///@brief Run the optimization problem
    void optimize();

    /// @brief The dimension of the design parameters being optimized.
    [[nodiscard]] int dimension() const;

   private:
    void outputResult() const;

   private:
    PlatoProblem mProblem;
    ROL::Ptr<ROL::Problem<double>> mROLProblem;
    ROL::Solver<double> mROLSolver;
    boost::mpi::communicator mCommunicator{};
};

}  // namespace plato::main::library

#endif
