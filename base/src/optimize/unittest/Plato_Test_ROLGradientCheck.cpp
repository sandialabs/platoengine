#include "InterfaceTestFixture.hpp"

#include "Plato_ROLInterface.hpp"
#include "Plato_OptimizerEngineStageData.hpp"

#include "GradientCheckUtilities.hpp"

#include "ROL_Ptr.hpp"
#include "ROL_Problem.hpp"
#include "ROL_Objective.hpp"

#include "ROL_StdObjective.hpp"
#include "ROL_StdVector.hpp"

#include <memory>
#include <string>
#include <vector>
#include <numeric>

#include <gtest/gtest.h>
#include <mpi.h>

template<typename Real>
class SumOfSquaresObjective : public ROL::StdObjective<Real>
{
public:
    SumOfSquaresObjective() {};

    Real value
    (const std::vector<Real> & x, 
     Real & /*tol*/) final override 
    {
        return std::accumulate(x.cbegin(), x.cend(), 0.0, 
            [](const Real sum, const Real value)
            {return sum + value * value;});
    }

    void gradient
    (std::vector<Real> & g, 
     const std::vector<Real> & x, 
     Real & /*tol*/) final override 
    {
        for (size_t i=0; i<x.size(); i++)
            g[i] = 2*x[i];
    }
};

class PlatoTestROLGradientCheck : public Plato::InterfaceTestFixture
{
protected:
    template<typename RealType>
    std::unique_ptr<Plato::ROLInterface<RealType>>
    constructROLInterface(Plato::optimizer::algorithm_t aType)
    {
        MPI_Comm tLocalComm;
        mInterface->getLocalComm(tLocalComm);

        return std::make_unique<Plato::ROLInterface<RealType>>(mInterface.get(), tLocalComm, aType);
    }
    
    template<typename RealType>
    ROL::Ptr<ROL::Problem<RealType>>
    constructROLProblem(const std::vector<RealType> & aControlVals)
    {
        ROL::Ptr<ROL::Objective<RealType>> tObjective = ROL::makePtr<SumOfSquaresObjective<RealType>>();

        const unsigned int tNumVars = aControlVals.size();
        ROL::Ptr<ROL::StdVector<RealType>> tControl = ROL::makePtr<ROL::StdVector<RealType>>(tNumVars);
        for(unsigned int iVal=0; iVal<tNumVars; iVal++)
            (*tControl)[iVal] = aControlVals[iVal];

        return ROL::makePtr<ROL::Problem<RealType>>(tObjective, tControl);
    }
};

TEST_F(PlatoTestROLGradientCheck, CheckSumOfSquaresObjective)
{
    using RealType = double;

    ROL::Ptr<ROL::Objective<RealType>> pobj = ROL::makePtr<SumOfSquaresObjective<RealType>>();

    const unsigned int numVars = 2;
    ROL::Ptr<ROL::StdVector<RealType>> x = ROL::makePtr<ROL::StdVector<RealType>>(numVars, 1);
    (*x)[1] = 4;
    
    // test sum of squares objective value
    double tTol = 1e-10;
    const auto tValue = pobj->value(*x, tTol);
    EXPECT_EQ(tValue, 17);

    // test sum of squares gradient
    ROL::Ptr<ROL::StdVector<RealType>> tGrad = ROL::makePtr<ROL::StdVector<RealType>>(numVars, 0);
    pobj->gradient(*tGrad, *x, tTol);

    ASSERT_EQ(tGrad->dimension(), numVars);
    const std::vector<RealType> tGradGold = {2.0, 8.0};
    for(unsigned int i=0; i<numVars; i++)
    {
        EXPECT_NEAR((*tGrad)[i], tGradGold[i], tTol);
    }
}

TEST_F(PlatoTestROLGradientCheck, ConstructROLInterface_TestAlgorithmType)
{
    const std::vector<std::string> tFileContents = {
        "<Performer> \n",
        "  <Name>PlatoMain</Name> \n",
        "  <Code>Plato_Main</Code> \n",
        "  <PerformerID>0</PerformerID> \n",
        "</Performer> \n"
    };
    writeInterfaceFile(tFileContents);

    Plato::optimizer::algorithm_t tType = Plato::optimizer::algorithm_t::ROL_BOUND_CONSTRAINED;
    auto tOptimizer = constructROLInterface<double>(tType);

    EXPECT_EQ(tOptimizer->algorithm(), tType);
}

TEST_F(PlatoTestROLGradientCheck, PerturbControls_SeedProvided)
{
    const std::vector<double> tControlVals = {88.0, 88.0};
    auto tOptimizationProblem = constructROLProblem(tControlVals);
     
    auto tControl = tOptimizationProblem->getPrimalOptimizationVector();

    const double tPerturbationScale = 2.0;
    const unsigned int tCheckGradientSeed = 123;
    auto tPerturbation = Plato::GradientCheck::perturb_control(tControl, tPerturbationScale, tCheckGradientSeed);

    const ROL::StdVector<double>& tPerturbationVector =
            dynamic_cast<const ROL::StdVector<double>&>(*tPerturbation);

    size_t tPerturbationSize = tPerturbationVector.dimension();
    ASSERT_EQ(tPerturbationSize, 2);

    const std::vector<double> tPerturbationGold = {-1.7597944, 1.1532739};
    for(unsigned int iPerturbed=0; iPerturbed<tPerturbationSize; iPerturbed++)
    {
        EXPECT_NEAR(tPerturbationVector[iPerturbed], tPerturbationGold[iPerturbed], 1.0e-6);
    }
}

TEST_F(PlatoTestROLGradientCheck, ComputeSteps_DefaultSizeIs10)
{
    const unsigned int tNumSteps = 5;

    const std::vector<double> tSteps = Plato::GradientCheck::generate_steps<double>(tNumSteps);

    std::vector<double> tStepsGold = {1.0, 1.0e-1, 1.0e-2, 1.0e-3, 1.0e-4};
    for(unsigned int iStep = 0; iStep<tNumSteps; iStep++)
    {
        EXPECT_NEAR(tSteps[iStep], tStepsGold[iStep], 1.0e-10);
    }
}

TEST_F(PlatoTestROLGradientCheck, ComputeSteps_SpecifiedSizeIs2)
{
    const unsigned int tNumSteps = 5;
    const unsigned int tStepSize = 2;

    const std::vector<double> tSteps = Plato::GradientCheck::generate_steps<double>(tNumSteps, tStepSize);

    std::vector<double> tStepsGold = {1.0, 0.5, 0.25, 0.125, 0.0625};
    for(unsigned int iStep = 0; iStep<tNumSteps; iStep++)
    {
        EXPECT_NEAR(tSteps[iStep], tStepsGold[iStep], 1.0e-10);
    }
}

TEST_F(PlatoTestROLGradientCheck, GradientCheckSumOfSquaresObjective_BaseTen)
{
    using RealType = double;

    const std::vector<std::string> tFileContents = {
        "<Performer> \n",
        "  <Name>PlatoMain</Name> \n",
        "  <Code>Plato_Main</Code> \n",
        "  <PerformerID>0</PerformerID> \n",
        "</Performer> \n",
        "<Optimizer> \n",
        "  <Options>\n",
        "    <ROLGradientCheckPerturbationScale>0.1</ROLGradientCheckPerturbationScale>\n",
        "    <ROLGradientCheckSeed>123</ROLGradientCheckSeed>\n",
        "    <ROLGradientCheckSteps>4</ROLGradientCheckSteps>\n",
        "  </Options>\n",
        "</Optimizer> \n"
    };
    writeInterfaceFile(tFileContents);

    const std::vector<RealType> tControlVals = {1.0, 4.0};
    auto tOptimizationProblem = constructROLProblem(tControlVals);

    auto tEngineData = constructOptimizerData();

    Plato::optimizer::algorithm_t tType = Plato::optimizer::algorithm_t::ROL_BOUND_CONSTRAINED;
    auto tROLInterface = constructROLInterface<RealType>(tType);

	auto tGradCheckOut = tROLInterface->checkGradient(tOptimizationProblem, tEngineData);

    // check output
    const auto tGradCheckSteps = tEngineData.getROLCheckGradientSteps();
    ASSERT_EQ(tGradCheckOut.size(), tGradCheckSteps);
    ASSERT_EQ(tGradCheckOut[0].size(), 4);

    const std::vector<RealType> tStepsGold = {1.0, 1.0e-1, 1.0e-2, 1.0e-3};
    for(int i=0; i<tGradCheckSteps; i++)
    {
        EXPECT_EQ(tGradCheckOut[i][0], tStepsGold[i]);
    }

    const RealType tGradGold = 2.85330103377e-01;
    for(int i=0; i<tGradCheckSteps; i++)
    {
        EXPECT_NEAR(tGradCheckOut[i][1], tGradGold, 1.0e-12);
    }

    const std::vector<RealType> tFDGold = {2.96397395808e-01, 2.86436832621e-01, 2.85440776302e-01, 2.85341170670e-01};
    for(int i=0; i<tGradCheckSteps; i++)
    {
        EXPECT_NEAR(tGradCheckOut[i][2], tFDGold[i], 1.0e-12);
    }
}

TEST_F(PlatoTestROLGradientCheck, GradientCheckSumOfSquaresObjective_BaseTwo)
{
    using RealType = double;

    const std::vector<std::string> tFileContents = {
        "<Performer> \n",
        "  <Name>PlatoMain</Name> \n",
        "  <Code>Plato_Main</Code> \n",
        "  <PerformerID>0</PerformerID> \n",
        "</Performer> \n",
        "<Optimizer> \n",
        "  <Options>\n",
        "    <ROLGradientCheckPerturbationScale>0.1</ROLGradientCheckPerturbationScale>\n",
        "    <ROLGradientCheckSeed>123</ROLGradientCheckSeed>\n",
        "    <ROLGradientCheckSteps>4</ROLGradientCheckSteps>\n",
        "    <ROLGradientCheckStepSize>2</ROLGradientCheckStepSize>\n",
        "  </Options>\n",
        "</Optimizer> \n"
    };
    writeInterfaceFile(tFileContents);

    const std::vector<RealType> tControlVals = {1.0, 4.0};
    auto tOptimizationProblem = constructROLProblem(tControlVals);

    auto tEngineData = constructOptimizerData();

    Plato::optimizer::algorithm_t tType = Plato::optimizer::algorithm_t::ROL_BOUND_CONSTRAINED;
    auto tROLInterface = constructROLInterface<RealType>(tType);

	auto tGradCheckOut = tROLInterface->checkGradient(tOptimizationProblem, tEngineData);

    // check output
    const auto tGradCheckSteps = tEngineData.getROLCheckGradientSteps();
    ASSERT_EQ(tGradCheckOut.size(), tGradCheckSteps);
    ASSERT_EQ(tGradCheckOut[0].size(), 4);

    const std::vector<RealType> tStepsGold = {1.0, 0.5, 0.25, 0.125};
    for(int i=0; i<tGradCheckSteps; i++)
    {
        EXPECT_EQ(tGradCheckOut[i][0], tStepsGold[i]);
    }

    const RealType tGradGold = 2.85330103377e-01;
    for(int i=0; i<tGradCheckSteps; i++)
    {
        EXPECT_NEAR(tGradCheckOut[i][1], tGradGold, 1.0e-12);
    }

    const std::vector<RealType> tFDGold = {2.96397395808e-01, 2.90863749593e-01, 2.88096926485e-01, 2.86713514931e-01};
    for(int i=0; i<tGradCheckSteps; i++)
    {
        EXPECT_NEAR(tGradCheckOut[i][2], tFDGold[i], 1.0e-12);
    }
}

