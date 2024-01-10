/*
 * Plato_HarvestDataFromFile.cpp
 *
 *  Created on: Feb 09, 2022
 */

#include "PlatoApp.hpp"
#include "Plato_Parser.hpp"
#include "Plato_Macros.hpp"
#include "Plato_InputData.hpp"
#include "Plato_OperationsUtilities.hpp"
#include "Plato_HarvestDataFromFile.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
BOOST_CLASS_EXPORT_IMPLEMENT(Plato::HarvestDataFromFile)

namespace Plato
{

double compute(Plato::Table& aTable)
{
    auto tOutput = 0.0;
    std::string tLowerOp = aTable.mOperation;
    std::transform(tLowerOp.begin(), tLowerOp.end(), tLowerOp.begin(), ::tolower);

    if(tLowerOp == "max")
    {
        auto tResult = std::numeric_limits<double>::epsilon();
        for(auto& tRow : aTable.mData)
        {
            tResult = std::max(tRow[aTable.mCol], tResult);
        }
        tOutput = tResult;
    }
    if(tLowerOp == "min")
    {
        auto tResult = std::numeric_limits<double>::max();
        for(auto& tRow : aTable.mData)
        {
            tResult = std::min(tRow[aTable.mCol], tResult);
        }
        tOutput = tResult;
    }
    if(tLowerOp == "sum")
    {
        auto tResult = 0.0;
        for(auto& tRow : aTable.mData)
        {
            tResult += tRow[aTable.mCol];
        }
        tOutput = tResult;
    }
    return tOutput;
}
// function compute

HarvestDataFromFile::HarvestDataFromFile(PlatoApp* aPlatoApp, Plato::InputData& aNode) : 
    Plato::LocalOp(aPlatoApp),
    mInputData(aNode)
{
    this->initialize();
    this->checkInputs(); 
}

void HarvestDataFromFile::operator()()
{
    // read table from disk
    std::ofstream out_file("debug.dat");
    out_file<<"trying to read file"<<std::endl;
    Plato::Table tTable;
    auto tFileName = mInputStrKeyValuePairs.at("File").front();
    Plato::read_table(tFileName, tTable.mData);

    out_file<<"after reading file"<<std::endl;
    // evaluate operation
    tTable.mOperation = mInputStrKeyValuePairs.at("Operation").front();
    tTable.mCol = std::stoi(mInputStrKeyValuePairs.at("Column").front());
    mLocalOutputValue = Plato::compute(tTable);
    out_file<<"after compute"<<std::endl;
    // set output shared data
    if(!mUnitTest)
    {
        auto tOutputArgumentName = mInputStrKeyValuePairs.at("Output").front();
        std::vector<double>* tOutput = mPlatoApp->getValue(tOutputArgumentName);
        (*tOutput)[0] = mLocalOutputValue;
    }
    out_file<<"finished reading file"<<std::endl;
    out_file.close();
}

void HarvestDataFromFile::getArguments(std::vector<Plato::LocalArg>& aLocalArgs)
{
    this->getInputArguments(aLocalArgs);  
    this->getOutputArguments(aLocalArgs);  
}

std::string HarvestDataFromFile::name() const
{
    return (mInputStrKeyValuePairs.at("Name").front());
}

std::string HarvestDataFromFile::file() const
{
    return (mInputStrKeyValuePairs.at("File").front());
}

std::string HarvestDataFromFile::column() const
{
    return (mInputStrKeyValuePairs.at("Column").front());
}

std::string HarvestDataFromFile::function() const
{
    return (mInputStrKeyValuePairs.at("Function").front());
}

std::string HarvestDataFromFile::operation() const
{
    return (mInputStrKeyValuePairs.at("Operation").front());
}

double HarvestDataFromFile::result() const
{
    return mLocalOutputValue;
}

void HarvestDataFromFile::unittest(const bool aUnitTest)
{
    mUnitTest = aUnitTest;
}

void HarvestDataFromFile::checkInputs()
{
    this->checkInput("File");
    this->checkInput("Column");
    this->checkInput("Function");
    this->checkInput("Operation");
}

void HarvestDataFromFile::checkInput(const std::string& aKeyword)
{
    auto tItr = mInputStrKeyValuePairs.find(aKeyword);
    if(tItr->second.front().empty())
    {
        THROWERR(std::string("Input keyword '") + aKeyword + "' is empty. A value must be provided for this keyword.")
    }
}

void HarvestDataFromFile::initialize()
{
    // parse single arguments
    mInputStrKeyValuePairs["File"].push_back(Plato::Get::String(mInputData, "File"));
    mInputStrKeyValuePairs["Name"].push_back(Plato::Get::String(mInputData, "Name"));
    mInputStrKeyValuePairs["Column"].push_back(Plato::Get::String(mInputData, "Column"));
    mInputStrKeyValuePairs["Function"].push_back(Plato::Get::String(mInputData, "Function"));
    mInputStrKeyValuePairs["Operation"].push_back(Plato::Get::String(mInputData, "Operation"));

    // parse inputs
    for(Plato::InputData& tNode : mInputData.getByName<Plato::InputData>("Input"))
    {
        mInputStrKeyValuePairs["Input"].push_back(Plato::Get::String(tNode, "ArgumentName"));
    }

    // parse outputs
    for(Plato::InputData& tNode : mInputData.getByName<Plato::InputData>("Output"))
    {
        mInputStrKeyValuePairs["Output"].push_back(Plato::Get::String(tNode, "ArgumentName"));
    }
}

void HarvestDataFromFile::getInputArguments(std::vector<Plato::LocalArg>& aLocalArgs)
{
    auto tItr = mInputStrKeyValuePairs.find("Input");
    if(tItr != mInputStrKeyValuePairs.end())
    {
        for(auto& tName : tItr->second)
        {
            aLocalArgs.push_back(Plato::LocalArg(Plato::data::layout_t::SCALAR, tName));
        }
    }
}

void HarvestDataFromFile::getOutputArguments(std::vector<Plato::LocalArg>& aLocalArgs)
{
    auto tItr = mInputStrKeyValuePairs.find("Output");
    if(tItr != mInputStrKeyValuePairs.end())
    {
        for(auto& tName : tItr->second) 
        {
            aLocalArgs.push_back(Plato::LocalArg(Plato::data::layout_t::SCALAR, tName, 1 /*length*/));
        }
    }
    else
    {
        THROWERR("The 'HarvestDataFromFile' operation must have an output argument.")
    }
}

}
// namespace Plato