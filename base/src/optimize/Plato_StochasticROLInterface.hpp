#pragma once

#include "Plato_ROLInterface.hpp"
#include "Plato_ReducedStochasticObjectiveROL.hpp"
#include "Plato_SampleValidation.hpp"
#include "Plato_SampleParameterMap.hpp"

#include "ROL_BatchManager.hpp"
#include "ROL_MonteCarloGenerator.hpp"
#include "ROL_StochasticProblem.hpp"
#include "ROL_Uniform.hpp"

namespace Plato
{

template<typename ScalarType, typename OrdinalType = size_t>
class StochasticROLInterface: public ROLInterface<ScalarType,OrdinalType>
{
public:
    StochasticROLInterface(Plato::Interface* aInterface, const MPI_Comm & aComm, const Plato::optimizer::algorithm_t& aType):
        ROLInterface<ScalarType, OrdinalType>(aInterface, aComm, aType)
    {
    }

    void initialize() override 
    {
        ROLInterface<ScalarType, OrdinalType>::initialize();
        bool tValid = this->validateDistributionsDefined();
        tValid &= this->validateDistributionsConsistent();
        tValid &= this->validateDataLayer();
        if(tValid)
        {
            this->initializeDistributions();
        }
    }

protected:
    Teuchos::RCP<ROL::Objective<ScalarType>> makeObjective() const override
    {
        return Teuchos::rcp(new Plato::ReducedStochasticObjectiveROL<ScalarType>(
            this->mInputData, 
            this->mInterface,
            mDistributionIndexMap,
            mSampler));
    }

    ROL::Ptr<ROL::Problem<ScalarType>> updateProblem(ROL::Ptr<ROL::Problem<ScalarType>>&& aOptimizationProblem) const override
    {
        ROL::Ptr<ROL::StochasticProblem<ScalarType>> tStochasticOptimizationProblem 
            = ROL::makePtr<ROL::StochasticProblem<ScalarType>>(*aOptimizationProblem);
        auto tParameterList = this->updateParameterListFromRolInputsFile();
        tStochasticOptimizationProblem->makeObjectiveStochastic(*tParameterList, mSampler);
        return tStochasticOptimizationProblem;
    }

private:
    /// Checks that each stochastic parameter has a matching distribution defined in the distributions
    /// input file. If a parameter does not have a distribution, an exception is registered.
    bool validateDistributionsDefined()
    {
        const std::string& tDistributionsFile = this->mInputData.getROLStochasticDistributionsFile();
        const Teuchos::RCP<Teuchos::ParameterList> tDistributionParameters = 
            Teuchos::getParametersFromXmlFile(tDistributionsFile);
        const std::vector<Plato::StochasticSampleSharedDataNames>& tSampleData = this->mInputData.getStochasticSampleSharedDataNames();
        if(const auto tUndefinedDistribution = Plato::undefined_distributions(tSampleData, *tDistributionParameters); 
            tUndefinedDistribution)
        {   
            this->mInterface->registerException(
                ParsingException("Could not find distribution with name " 
                + tUndefinedDistribution.value() + ". Check input file " + tDistributionsFile));
            return false;
        }
        return true;
    }

    bool validateDistributionsConsistent()
    {
        if(const auto tErrorMessage = are_distributions_consistent_for_all_samples(this->mInputData.getStochasticSampleSharedDataNames());
            tErrorMessage)
        {   
            this->mInterface->registerException(
                ParsingException("Inconsistent usage of distributions in stochastic samples. "
                                + tErrorMessage.value()));
            return false;
        }
        return true;
    }

    bool validateDataLayer()
    {
        const std::vector<Plato::StochasticSampleSharedDataNames>& tSampleData = this->mInputData.getStochasticSampleSharedDataNames();
        if(const auto tUndefinedParameter = Plato::undefined_parameters(tSampleData, *this->mInterface); 
            tUndefinedParameter)
        {
            this->mInterface->registerException(ParsingException(
                R"(" Could not find Parameter ")" + tUndefinedParameter.value() 
                + R"(" in data layer. Please check the interface file.)"));
            return false;
        }
        return true;
    }

    /// @brief Construct all probability distributions defined in an input file and construct the
    ///  random sampler from those distributions.
    /// @pre validateDistributionsDefined should be called first. Otherwise, non-existent distributions 
    ///  default to uniform on [0, 1], which may give unexpected results.
    void initializeDistributions()
    {
        mNumSamples = this->mInputData.getROLStochasticNumberOfSamples();
        const std::string& tDistributionsFile = this->mInputData.getROLStochasticDistributionsFile();
        const Teuchos::RCP<Teuchos::ParameterList> tDistributionParameters = 
            Teuchos::getParametersFromXmlFile(tDistributionsFile);
        const std::vector<Plato::StochasticSampleSharedDataNames>& tSampleData = 
            this->mInputData.getStochasticSampleSharedDataNames();
        mDistributionIndexMap = Plato::distribution_map(tSampleData.front());
        mSampleDistributions.resize(mDistributionIndexMap.size());
        for(const auto& [tDistributionName, tIndex] : mDistributionIndexMap)
        {
            Teuchos::ParameterList tDistributionForParameter;
            tDistributionForParameter.sublist("SOL").sublist("Distribution") = 
                tDistributionParameters->sublist(tDistributionName);
            mSampleDistributions[tIndex] = ROL::DistributionFactory<ScalarType>(tDistributionForParameter);
        }
        // Default values from ROL
        constexpr bool tUseSA = false;
        constexpr bool tAdaptive = false;
        constexpr int tNumNewSamples = 0;
        mSampler = ROL::makePtr<ROL::MonteCarloGenerator<ScalarType>>(mNumSamples, mSampleDistributions, mBatchManager, 
            tUseSA, tAdaptive, tNumNewSamples, this->mInputData.getROLStochasticSamplerSeed());
    }

private:
    int mNumSamples = 1;
    std::vector<ROL::Ptr<ROL::Distribution<ScalarType>>> mSampleDistributions;
    std::unordered_map<std::string, unsigned int> mDistributionIndexMap;
    ROL::Ptr<ROL::BatchManager<ScalarType>> mBatchManager = ROL::makePtr<ROL::BatchManager<ScalarType>>();
    ROL::Ptr<ROL::SampleGenerator<ScalarType>> mSampler;
};

} // namespace Plato
