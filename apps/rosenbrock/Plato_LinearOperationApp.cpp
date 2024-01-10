#include "Plato_LinearOperationApp.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>

namespace Plato
{
namespace
{
constexpr int kNumCriteria = 1;
constexpr int kNumControls = 2; // Hardcoded to match Rosenbrock
}

LinearOperationApp::LinearOperationApp(int aArgc, char *aArgv[], MPI_Comm& /*aLocalComm*/) :
    mWeights(kNumControls, 1.0),
    mControls(kNumControls),
    mCriterionValue(kNumCriteria),
    mGradient(kNumControls),
    mHessianTimesVector(kNumControls)
{
    // Read weights if a file was provided
    if(aArgc > 1)
    {
        std::vector<ScalarType> tFileContents;
        std::ifstream tFile(aArgv[1]);
        std::copy(std::istream_iterator<ScalarType>(tFile), std::istream_iterator<ScalarType>(), std::back_inserter(tFileContents));
        if(tFileContents.size() != kNumControls)
        {
            throw std::runtime_error("LinearOperationApp: Wrong number of weights in input file: " + std::string(aArgv[1]));
        }
        mWeights.vector() = tFileContents;
    }
}

void LinearOperationApp::finalize() 
{
}

void LinearOperationApp::initialize()
{
}

void LinearOperationApp::compute(const std::string& /*aOperationName*/) 
{
}

void LinearOperationApp::exportData(const std::string& aArgumentName, Plato::SharedData& aExportData) 
{
    if(aExportData.myLayout() == Plato::data::layout_t::SCALAR)
    {
        outputData(aArgumentName, aExportData);
    } 
    else
    {
        std::ostringstream tErrorMsg;
        tErrorMsg << "\n\n********* ERROR IN " << __FILE__ << ", FUNCTION: " << __func__ << ", LINE: " << __LINE__
                  << ", MESSAGE: DATA LAYOUT IS NOT SUPPORTED BY THE APPLICATION. ABORT! *********\n\n";
        std::perror(tErrorMsg.str().c_str());
    }
}

void LinearOperationApp::importData(const std::string & aArgumentName, const Plato::SharedData & aImportData) 
{
    if(aImportData.myLayout() == Plato::data::layout_t::SCALAR || aImportData.myLayout() == Plato::data::layout_t::SCALAR_FIELD)
    {
        inputData(aArgumentName, aImportData);
    } 
    else
    {
        std::ostringstream tErrorMsg;
        tErrorMsg << "\n\n********* ERROR IN " << __FILE__ << ", FUNCTION: " << __func__ << ", LINE: " << __LINE__
                  << ", MESSAGE: DATA LAYOUT IS NOT SUPPORTED BY THE APPLICATION. ABORT! *********\n\n";
        std::perror(tErrorMsg.str().c_str());
    }
}

void LinearOperationApp::exportDataMap(const Plato::data::layout_t & /*aDataLayout*/, std::vector<int> & /*aMyOwnedGlobalIDs*/) 
{
}

void LinearOperationApp::computeCriterionValue() 
{
    mCriterionValue[0] = mWeights.dot(mControls);
}

void LinearOperationApp::computeCriterionGradient() 
{
    mGradient.vector() = mWeights.vector();
}

void LinearOperationApp::computeCriterionHessianTimesVector() 
{
    mHessianTimesVector.fill(0.0);
}

void LinearOperationApp::inputData(const std::string & aArgumentName, const Plato::SharedData & aImportData)
{
    const auto tSetImportData = [&aImportData, this](Vector<ScalarType, OrdinalType>& aData)
    {
        assert(aData.size() == static_cast<OrdinalType>(aImportData.size()));
        Plato::StandardVector<ScalarType, OrdinalType> tInputData(aData.size());
        aImportData.getData(tInputData.vector());
        aData.update(static_cast<ScalarType>(1), tInputData, static_cast<ScalarType>(0));
    };
    applyToData(aArgumentName, tSetImportData);
}

void LinearOperationApp::outputData(const std::string & aArgumentName, Plato::SharedData & aExportData)
{
    const auto tSetExportData = [&aExportData, this](const Vector<ScalarType, OrdinalType>& aData)
    {
        assert(aData.size() == static_cast<OrdinalType>(aExportData.size()));
        Plato::StandardVector<ScalarType, OrdinalType> tOutputData(aData.size());
        tOutputData.update(static_cast<ScalarType>(1), aData, static_cast<ScalarType>(1));
        aExportData.setData(tOutputData.vector());
    };
    applyToData(aArgumentName, tSetExportData);
}

}