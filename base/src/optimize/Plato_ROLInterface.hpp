/*
//@HEADER
// *************************************************************************
//   Plato Engine v.1.0: Copyright 2018, National Technology & Engineering
//                    Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Sandia Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact the Plato team (plato3D-help@sandia.gov)
//
// *************************************************************************
//@HEADER
*/

#pragma once

#include "Plato_OptimizerInterface.hpp"

#include "ROL_Bounds.hpp"
#include "ROL_Solver.hpp"

#include "Plato_ReducedObjectiveROL.hpp"
#include "Plato_ReducedConstraintROL.hpp"
#include "Plato_DistributedVectorROL.hpp"

#include "GradientCheckUtilities.hpp"

#include <vector>

#include <mpi.h>

namespace Plato
{

template<typename ScalarType, typename OrdinalType = size_t>
class ROLInterface : public OptimizerInterface<ScalarType,OrdinalType>
{
public:
    ROLInterface(Plato::Interface* aInterface, const MPI_Comm & aComm, const Plato::optimizer::algorithm_t& aType):
        OptimizerInterface<ScalarType,OrdinalType>(aInterface,aComm),
        mAlgorithmType(aType)
    {
    }

    void initialize() override 
    {
        OptimizerInterface<ScalarType,OrdinalType>::initialize();
        mOutputBuffer = getOutputBuffer();
    }

    Plato::optimizer::algorithm_t algorithm() const override
    {
        return mAlgorithmType;
    }

    void run() override
    {
        this->initialize();
        constexpr OrdinalType tCONTROL_VECTOR_INDEX = 0;
        std::string tControlName = this->mInputData.getControlName(tCONTROL_VECTOR_INDEX);
        const OrdinalType tNumControls = this->mInterface->size(tControlName);
        auto tControlBoundsMng = this->setControlBounds(tNumControls);
        
        Teuchos::RCP<Plato::DistributedVectorROL<ScalarType>> tControls =
                Teuchos::rcp(new Plato::DistributedVectorROL<ScalarType>(this->mComm, tNumControls));
        this->setInitialGuess(tControlName, *tControls);
        
        solve(tControls, tControlBoundsMng);

        this->finalize();
    }

    void solve(Teuchos::RCP<Plato::DistributedVectorROL<ScalarType>> &aControls,
               Teuchos::RCP<ROL::BoundConstraint<ScalarType>> &aControlBoundsMng)
    {
        if(this->mInputData.getCheckGradient())
        {
            runChecks(aControls, aControlBoundsMng);
        }
        else
        {
            solveOptimizationProblem(aControls, aControlBoundsMng);
        }
    }

    void runChecks(Teuchos::RCP<Plato::DistributedVectorROL<ScalarType>> &aControls,
                   Teuchos::RCP<ROL::BoundConstraint<ScalarType>> &aControlBoundsMng)
    {
        Teuchos::RCP<ROL::Objective<ScalarType>> tObjective = makeObjective();
        ROL::Ptr<ROL::Problem<ScalarType>> tOptimizationProblem = makeOptimizationProblem(tObjective, aControls, aControlBoundsMng);
        auto tGradCheckOutput = this->checkGradient(tOptimizationProblem, this->mInputData);
        this->checkConstraint(tOptimizationProblem);
    }

    void solveOptimizationProblem(Teuchos::RCP<Plato::DistributedVectorROL<ScalarType>> &aControls,
                                  Teuchos::RCP<ROL::BoundConstraint<ScalarType>> &aControlBoundsMng)
    {
        int tNumSolves=1;
        if(this->mInputData.getResetAlgorithmOnUpdate())
        {
            // The approach here assumes the max number of iterations in the rol_inputs.xml file has been
            // set to the this->mInputData.getProblemUpdateFrequency() value
            tNumSolves = this->mInputData.getMaxNumIterations()/this->mInputData.getProblemUpdateFrequency();
        }

        for(int i=0; i<tNumSolves; ++i)
        {
            auto tParameterList = this->updateParameterListFromRolInputsFile();
            if(i>0)
            {
                updateControl(aControls);
                setSavedOptimizerValues(tParameterList);
            }
            Teuchos::RCP<ROL::Objective<ScalarType>> tObjective = makeObjective();
            ROL::Ptr<ROL::Problem<ScalarType>> tOptimizationProblem = makeOptimizationProblem(tObjective, aControls, aControlBoundsMng);
            ROL::Solver<ScalarType> tOptimizer(tOptimizationProblem, *tParameterList);
            std::ostream outputStream(this->mOutputBuffer);
            tOptimizer.solve(outputStream);
             
            saveOptimizerValues(tOptimizer);

            outputStream.flush();
            this->printControl(tOptimizationProblem);
        }
    }

    void updateControl(Teuchos::RCP<Plato::DistributedVectorROL<ScalarType>> &aControls)
    {
        const std::string tControlResetStageName = this->mInputData.getControlResetStageName();
        if(!tControlResetStageName.empty())
        {
            Teuchos::ParameterList tControlResetStageParameterList;
            constexpr OrdinalType tCONTROL_VECTOR_INDEX = 0;
            const std::string tControlName = this->mInputData.getControlName(tCONTROL_VECTOR_INDEX);
            tControlResetStageParameterList.set(tControlName, aControls->vector().data());
            this->mInterface->compute(tControlResetStageName, tControlResetStageParameterList);
        }
    }
  
    void setSavedOptimizerValues(Teuchos::RCP<Teuchos::ParameterList> &aParameterList)
    {
        if(mAlgorithmType == Plato::optimizer::algorithm_t::ROL_BOUND_CONSTRAINED)
        {
            aParameterList->sublist("Step").sublist("Trust Region").set("Initial Radius", mTrustRegionRadiusSize);
        }
        else if(mAlgorithmType == Plato::optimizer::algorithm_t::ROL_AUGMENTED_LAGRANGIAN)
        {
         // Below is how you would reset the penalty parameter if you wanted to. We need to investigate this more to know 
         // if we want to set it or not.
         //   aParameterList->sublist("Step").sublist("Augmented Lagrangian").set("Initial Penalty Parameter", mAugmentedLagrangianPenalty);
        }
    }
    
    void saveOptimizerValues(const ROL::Solver<ScalarType> &aOptimizer)
    {
        if(mAlgorithmType == Plato::optimizer::algorithm_t::ROL_BOUND_CONSTRAINED)
        {
            ROL::Ptr<const ROL::TypeB::AlgorithmState<ScalarType>> tAlgorithmState =
                    ROL::staticPtrCast<const ROL::TypeB::AlgorithmState<ScalarType>>(aOptimizer.getAlgorithmState());
            mTrustRegionRadiusSize = tAlgorithmState->searchSize;
        }
        else if(mAlgorithmType == Plato::optimizer::algorithm_t::ROL_AUGMENTED_LAGRANGIAN)
        {
            ROL::Ptr<const ROL::TypeG::AlgorithmState<ScalarType>> tAlgorithmState =
                    ROL::staticPtrCast<const ROL::TypeG::AlgorithmState<ScalarType>>(aOptimizer.getAlgorithmState());
            mAugmentedLagrangianPenalty = tAlgorithmState->searchSize;
        }
    }

    std::vector<std::vector<ScalarType>> checkGradient
    (const ROL::Ptr<ROL::Problem<ScalarType>> & aOptimizationProblem,
     const Plato::OptimizerEngineStageData & aInputData)
    {
        std::cout << "Checking gradient..." << std::endl;

        const auto tPerturbationScale = aInputData.getROLPerturbationScale();
        const auto tCheckGradientSeed = aInputData.getROLCheckGradientSeed();

        auto tControl = aOptimizationProblem->getPrimalOptimizationVector();
        auto tPerturbation = Plato::GradientCheck::perturb_control(tControl, tPerturbationScale, tCheckGradientSeed);

        std::ofstream tOutputFile;
        tOutputFile.open("ROL_gradient_check_output.txt");

        const auto tNumSteps = aInputData.getROLCheckGradientSteps();
        const auto tStepSize = aInputData.getROLCheckGradientStepSize();
        const std::vector<ScalarType> tSteps = Plato::GradientCheck::generate_steps<ScalarType>(tNumSteps, tStepSize);

        auto tObjective = aOptimizationProblem->getObjective();
	    auto tGradCheckOut = tObjective->checkGradient(*tControl, *tPerturbation, tSteps, true, tOutputFile);

        tOutputFile.close();

        return tGradCheckOut;
    }

protected:
    virtual Teuchos::RCP<ROL::Objective<ScalarType>> makeObjective() const
    {
        return Teuchos::rcp(new Plato::ReducedObjectiveROL<ScalarType>(this->mInputData, this->mInterface));
    }

    /// Override this function to perform any additional setup for the optimization problem.
    /// For example, the stochastic optimization methods need to specify that the objective
    /// and or constraint are stochastic.
    virtual ROL::Ptr<ROL::Problem<ScalarType>> updateProblem(ROL::Ptr<ROL::Problem<ScalarType>>&& aOptimizationProblem) const
    {
        return std::move(aOptimizationProblem);
    }

    std::streambuf *getOutputBuffer() 
    {
        int tMyRank = -1;
        MPI_Comm_rank(this->mComm, &tMyRank);
        assert(tMyRank >= 0);
        if(tMyRank == 0)
        {
            if (this->mInputData.getOutputDiagnosticsToFile()) {
                mOutputFile.open("ROL_output.txt", std::ofstream::out);
                return mOutputFile.rdbuf();
            }
        }
        return std::cout.rdbuf();
    }

    void createOptimizationProblemLinearConstraint(ROL::Problem<ScalarType>& aOptimizationProblem)
    {
        const OrdinalType tNumConstraints = this->mInputData.getNumConstraints();
        for(OrdinalType tConstraintIndex = 0; tConstraintIndex < tNumConstraints; ++tConstraintIndex)
        {
            Teuchos::RCP<Plato::SerialVectorROL<ScalarType>> tDual = Teuchos::rcp(new Plato::SerialVectorROL<ScalarType>(1));
            Teuchos::RCP<ROL::Constraint<ScalarType>> tEquality = Teuchos::rcp(new Plato::ReducedConstraintROL<ScalarType>(this->mInputData, this->mInterface, tConstraintIndex));
            const std::string tConstraintName = "Equality Linear Constraint " + std::to_string(tConstraintIndex);
            aOptimizationProblem.addLinearConstraint(tConstraintName, tEquality, tDual);
        }

        auto tParameterList = this->updateParameterListFromRolInputsFile();
        aOptimizationProblem.setProjectionAlgorithm(*tParameterList);   
    }

    void createOptimizationProblemAugmentedLagrangian(ROL::Problem<ScalarType>& aOptimizationProblem)   
    {
        const OrdinalType tNumConstraints = this->mInputData.getNumConstraints();
        
        for(OrdinalType tConstraintIndex = 0; tConstraintIndex < tNumConstraints; ++tConstraintIndex)
        {
            Teuchos::RCP<Plato::SerialVectorROL<ScalarType>> tDual = Teuchos::rcp(new Plato::SerialVectorROL<ScalarType>(1));

            Teuchos::RCP<ROL::Constraint<ScalarType>> tEquality = 
                         Teuchos::rcp(new Plato::ReducedConstraintROL<ScalarType>(this->mInputData, 
                         this->mInterface, tConstraintIndex));
            const std::string tConstraintName = "Equality Constraint " + std::to_string(tConstraintIndex);
            aOptimizationProblem.addConstraint(tConstraintName, tEquality, tDual);
        }
    }    

    void printControl(const ROL::Ptr<ROL::Problem<ScalarType>> & aOptimizationProblem)
    {
        int tMyRank = -1;
        MPI_Comm_rank(this->mComm, &tMyRank);
        assert(tMyRank >= static_cast<int>(0));
        if(tMyRank == static_cast<int>(0))
        {
            const bool tOutputControlToFile = this->mInputData.getOutputControlToFile();
            if(tOutputControlToFile == true)
            {
                std::ofstream tOutputFile;
                tOutputFile.open("ROL_control_output.txt");
                ROL::Ptr<ROL::Vector<ScalarType>> tSolutionPtr = aOptimizationProblem->getPrimalOptimizationVector();
                Plato::DistributedVectorROL<ScalarType> & tSolution =
                        dynamic_cast<Plato::DistributedVectorROL<ScalarType>&>(tSolutionPtr.operator*());
                std::vector<ScalarType> & tData = tSolution.vector();
                for(OrdinalType tIndex = 0; tIndex < tData.size(); tIndex++)
                    tOutputFile << tData[tIndex] << "\n";
                tOutputFile.close();
            }
        }
    }

    void setBounds(const std::vector<ScalarType> & aInputs, Plato::DistributedVectorROL<ScalarType> & aBounds)
    {
        assert(aInputs.empty() == false);
        if(aInputs.size() == static_cast<size_t>(1))
        {
            const ScalarType tValue = aInputs[0];
            aBounds.fill(tValue);
        }
        else
        {
            assert(aInputs.size() == static_cast<size_t>(aBounds.dimension()));
            aBounds.setVector(aInputs);
        }
    }

    void setInitialGuess(const std::string & aMyName, Plato::DistributedVectorROL<ScalarType> & aControl)
    {
        std::string tInitializationStageName = this->mInputData.getInitializationStageName();
        if(!tInitializationStageName.empty())
        {
            // Use user-defined stage to compute initial guess
            Teuchos::ParameterList tPlatoInitializationStageParameterList;
            tPlatoInitializationStageParameterList.set(aMyName, aControl.vector().data());
            this->mInterface->compute(tInitializationStageName, tPlatoInitializationStageParameterList);
        }
        else
        {
            // Use user-defined values to compute initial guess. Hence, a stage was not defined by the user.
            std::vector<ScalarType> tInitialGuess = this->mInputData.getInitialGuess();
            assert(tInitialGuess.empty() == false);
            if(tInitialGuess.size() == static_cast<size_t>(1))
            {
                const ScalarType tValue = tInitialGuess[0];
                aControl.fill(tValue);
            }
            else
            {
                assert(tInitialGuess.size() == static_cast<size_t>(aControl.dimension()));
                aControl.setVector(tInitialGuess);
            }
        }
    }

    Teuchos::RCP<Teuchos::ParameterList> updateParameterListFromRolInputsFile() const
    {
        const std::string tFileName = this->mInputData.getInputFileName();
        Teuchos::RCP<Teuchos::ParameterList> tParameterList = Teuchos::rcp(new Teuchos::ParameterList);
        Teuchos::updateParametersFromXmlFile(tFileName, tParameterList.ptr());
        return tParameterList;
    }
    
    void checkConstraint(const ROL::Ptr<ROL::Problem<ScalarType>> & aOptimizationProblem)
    {
        std::cout<<"Checking constraint..."<<std::endl;
        const auto tPerturbationScale = this->mInputData.getROLPerturbationScale();
        const auto tCheckGradientSeed = this->mInputData.getROLCheckGradientSeed();
        if(tCheckGradientSeed !=0)
        {
            std::srand((unsigned int)tCheckGradientSeed);
            std::cout<<"Setting seed to: "<<(unsigned int)tCheckGradientSeed<<std::endl;
        }
        auto tx = aOptimizationProblem->getPrimalOptimizationVector();
        tx->randomize(0, tPerturbationScale);
        auto tv = aOptimizationProblem->getPrimalOptimizationVector();
        tv->randomize(0, tPerturbationScale);
        std::ofstream tOutputFile;
        if(mAlgorithmType == Plato::optimizer::algorithm_t::ROL_AUGMENTED_LAGRANGIAN)
        {
            auto tg = aOptimizationProblem->getDualOptimizationVector();
            tg->randomize(0, tPerturbationScale);
            auto tc = aOptimizationProblem->getResidualVector();
            tc->randomize(-tPerturbationScale, tPerturbationScale);
            auto tw = aOptimizationProblem->getMultiplierVector();
            tw->randomize(-tPerturbationScale, tPerturbationScale);
            
            auto tConstraint = aOptimizationProblem->getConstraint();
            
            tOutputFile.open("ROL_constraint_check_output.txt");
            tConstraint->checkApplyJacobian(*tx, *tv, *tc, true, tOutputFile);
            tOutputFile.close();

            tOutputFile.open("ROL_adjoint_consistency_output.txt");
            tConstraint->checkAdjointConsistencyJacobian(*tw, *tv, *tx, true, tOutputFile);
            tOutputFile.close();   
        }
        else if(mAlgorithmType == Plato::optimizer::algorithm_t::ROL_LINEAR_CONSTRAINT)
        {
            tOutputFile.open("ROL_linearity_check_output.txt");
            aOptimizationProblem->checkLinearity(true, tOutputFile);
            tOutputFile.close();
        }
        else
        {
            std::cout<<"Nothing to check on ROL_Bound_Constrained Problem"<<std::endl;
        }
    }

    Teuchos::RCP<ROL::BoundConstraint<ScalarType>> setControlBounds(const OrdinalType& aNumControls)
    {
        std::vector<ScalarType> tInputBoundsData(aNumControls);

        // ********* GET LOWER BOUNDS INFORMATION *********
        Plato::getLowerBoundsInputData(this->mInputData, this->mInterface, tInputBoundsData);

        // ********* SET LOWER BOUNDS FOR OPTIMIZER *********
        Teuchos::RCP<Plato::DistributedVectorROL<ScalarType>> tControlLowerBounds =
                Teuchos::rcp(new Plato::DistributedVectorROL<ScalarType>(this->mComm, aNumControls));
        this->setBounds(tInputBoundsData, tControlLowerBounds.operator*());

        // ********* GET UPPER BOUNDS INFORMATION *********
        Plato::getUpperBoundsInputData(this->mInputData, this->mInterface, tInputBoundsData);

        // ********* SET UPPER BOUNDS FOR OPTIMIZER *********
        Teuchos::RCP<Plato::DistributedVectorROL<ScalarType>> tControlUpperBounds =
                Teuchos::rcp(new Plato::DistributedVectorROL<ScalarType>(this->mComm, aNumControls));
        this->setBounds(tInputBoundsData, tControlUpperBounds.operator*());

        // ********* CREATE BOUND CONSTRAINT FOR OPTIMIZER *********
        Teuchos::RCP<ROL::BoundConstraint<ScalarType>> tControlBoundsMng =
                Teuchos::rcp(new ROL::Bounds<ScalarType>(tControlLowerBounds, tControlUpperBounds));
        return tControlBoundsMng;
    }

    /******************************************************************************//**
     * @brief All optimizing is done so do any optional final
     * stages. Called only once from the interface.
    **********************************************************************************/
    void finalize() override
    {
        this->mInterface->finalize(this->mInputData.getFinalizationStageName());
    }

    ROL::Ptr<ROL::Problem<ScalarType>> makeOptimizationProblem(Teuchos::RCP<ROL::Objective<ScalarType>> &aObjective,
                                                               Teuchos::RCP<Plato::DistributedVectorROL<ScalarType>> &aControls,
                                                               Teuchos::RCP<ROL::BoundConstraint<ScalarType>> &aControlBoundsMng)
    {
        ROL::Ptr<ROL::Problem<ScalarType>> tOptimizationProblem = ROL::makePtr<ROL::Problem<ScalarType>>(aObjective, aControls);
                
        tOptimizationProblem->addBoundConstraint(aControlBoundsMng);
        if(mAlgorithmType == Plato::optimizer::algorithm_t::ROL_LINEAR_CONSTRAINT)
        {
            createOptimizationProblemLinearConstraint(*tOptimizationProblem);
        }
        else if(mAlgorithmType == Plato::optimizer::algorithm_t::ROL_AUGMENTED_LAGRANGIAN)
        {
            createOptimizationProblemAugmentedLagrangian(*tOptimizationProblem);
        }
        
        tOptimizationProblem = updateProblem(std::move(tOptimizationProblem));

        const bool tLumpConstraints = ( mAlgorithmType == Plato::optimizer::algorithm_t::ROL_LINEAR_CONSTRAINT ? false : true );
        constexpr bool tPrintToStream = true;

        tOptimizationProblem->finalize(tLumpConstraints, tPrintToStream, mOutputFile);
        return tOptimizationProblem;
    }

protected:
    std::ofstream mOutputFile;

private:
    std::streambuf *mOutputBuffer;
    Plato::optimizer::algorithm_t mAlgorithmType;
    double mTrustRegionRadiusSize=0.0;
    double mAugmentedLagrangianPenalty=1.0;
};

} // namespace Plato
