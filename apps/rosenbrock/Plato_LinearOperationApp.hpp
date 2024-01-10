#ifndef PLATO_LINEAROPERATION_HPP_
#define PLATO_LINEAROPERATION_HPP_

#include "Plato_SharedData.hpp"
#include "Plato_CriterionApplication.hpp"
#include "Plato_StandardVector.hpp"
#include "Plato_SerializationHeaders.hpp"

#include <mpi.h>

#include <sstream>
#include <string>

namespace Plato
{

class LinearOperationApp : public CriterionApplication
{
public:
    LinearOperationApp(int aArgc, char *aArgv[], MPI_Comm& aLocalComm);

    LinearOperationApp(const LinearOperationApp& aRhs) = delete;
    LinearOperationApp& operator=(const LinearOperationApp& aRhs) = delete;
    LinearOperationApp(const LinearOperationApp&& aRhs) = delete;
    LinearOperationApp& operator=(LinearOperationApp&& aRhs) = delete;

    void finalize() override;
    void initialize() override;
    void compute(const std::string & aOperationName) override;
    void exportData(const std::string & aArgumentName, Plato::SharedData & aExportData) override;
    void importData(const std::string & aArgumentName, const Plato::SharedData & aImportData) override;
    void exportDataMap(const Plato::data::layout_t & aDataLayout, std::vector<int> & aMyOwnedGlobalIDs) override;
    void computeCriterionValue() override;
    void computeCriterionGradient() override;
    void computeCriterionHessianTimesVector() override;

    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
        aArchive & boost::serialization::make_nvp("Application", boost::serialization::base_object<Application>(*this));
    }

private:
    void inputData(const std::string & aArgumentName, const Plato::SharedData & aImportData);
    void outputData(const std::string & aArgumentName, Plato::SharedData & aExportData);

    template<typename F>
    void applyToData(const std::string& aArgumentName, const F& aF);

private:
    using ScalarType = double;
    using OrdinalType = int;

    StandardVector<ScalarType, OrdinalType> mWeights;

    StandardVector<ScalarType, OrdinalType> mControls;
    StandardVector<ScalarType, OrdinalType> mCriterionValue;
    StandardVector<ScalarType, OrdinalType> mGradient;
    StandardVector<ScalarType, OrdinalType> mHessianTimesVector;
};

template<typename F>
void LinearOperationApp::applyToData(const std::string& aArgumentName, const F& aF)
{
    constexpr auto kControlName = "Controls";
    constexpr auto kDirectionName = "Direction";
    constexpr auto kValueName = "CriterionValue";
    constexpr auto kGradientName = "CriterionGradient";
    constexpr auto kHessianTimesVectorName = "CriterionHessianTimesVector";

    if(aArgumentName == std::string(kValueName))
    {
        aF(mCriterionValue);
    }
    else if(aArgumentName == std::string(kControlName))
    {
        aF(mControls);
    } 
    else if(aArgumentName == std::string(kGradientName))
    {
        aF(mGradient);
    }
    else if(aArgumentName == std::string(kHessianTimesVectorName))
    {
        aF(mHessianTimesVector);
    }
    else if(aArgumentName == std::string(kDirectionName))
    {
        // No-op, Hessian is 0
    }
    else
    {
        std::ostringstream tErrorMsg;
        tErrorMsg << "\n\n********* ERROR IN " << __FILE__ << ", FUNCTION: " << __func__ << ", LINE: " << __LINE__
                << ", MESSAGE: ARGUMENT NAME = '" << aArgumentName
                << "' IS NOT DEFINED IN THE DATA MAP. ABORT! *********\n\n";
        std::perror(tErrorMsg.str().c_str());
    }
}

}

#endif