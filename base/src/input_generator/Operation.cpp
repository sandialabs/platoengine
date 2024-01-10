/*
 * Operation.cpp
 *
 *  Created on: April 12, 2022
 */

#include "Operation.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorPlatoMainOperationFileUtilities.hpp"
#include <regex>
#include <fstream>

using namespace XMLGen;
namespace director
{
    
Operation::Operation
(const std::string& aName,
 const std::string& aFunction,
 std::shared_ptr<Performer> aPerformer,
 int aConcurrentEvaluations) :
 FileObject(aName,aConcurrentEvaluations),
 mFunction(aFunction),
 mPerformer(aPerformer)
{
    if( evaluations() == 0 )
        mChDir = false;
    else
        mChDir = true;
}

void Operation::appendCommonChildren
(pugi::xml_node& aOperationNode,
 std::string aEvaluationString)
{
    addChild(aOperationNode, "Function", mFunction);
    addChild(aOperationNode, "Name", name(aEvaluationString));
}

pugi::xml_node Operation::forNode
(pugi::xml_node& aNode, 
 const std::string& aXMLLoopVectorName)
{
    if(evaluations() == 0)
        return aNode; 
    else
    {
        auto tOperation = aNode.append_child("Operation");
        auto tForNode = tOperation.append_child("For");
        tForNode.append_attribute("var") = loopVar().c_str();
        tForNode.append_attribute("in") = aXMLLoopVectorName.c_str();
        return tForNode;
    }
}

OperationGemmaMPISystemCall::OperationGemmaMPISystemCall
(const std::string& aInputDeck, 
 const std::string& aPath,
 const std::string& aNumRanks, 
 std::shared_ptr<Performer> aPerformer,
 int aConcurrentEvaluations) :
 Operation("gemma", "SystemCallMPI",aPerformer, aConcurrentEvaluations),
 mCommand("gemma"),
 mArgument(aInputDeck),
 mNumRanks(aNumRanks),
 mPath(aPath)
{
    mOnChange = true;
}

 void OperationGemmaMPISystemCall::write_definition
 (pugi::xml_document& aDocument, 
  std::string aEvaluationString)
 {
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);
    addChild(tOperationNode, "Command", mPath+mCommand);
    if(mChDir)
        addChild(tOperationNode, "ChDir", std::string("evaluations_") + tag(aEvaluationString));
    addChild(tOperationNode, "OnChange", (mOnChange ? "true" : "false"));
    addChild(tOperationNode, "NumRanks", mNumRanks );
    addChild(tOperationNode, "Argument", mArgument );
    addChild(tOperationNode, "AppendInput", "false" );
 }

 void OperationGemmaMPISystemCall::write_interface
 (pugi::xml_node& aNode, 
  std::string aEvaluationString)
{
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
}

OperationAprepro::OperationAprepro
(const std::string& aInputDeck, 
 const std::vector<std::string>& aOptions, 
 std::shared_ptr<SharedData> aSharedData,
 std::shared_ptr<Performer> aPerformer, 
 int aConcurrentEvaluations) :
 Operation("aprepro", "SystemCall", aPerformer, aConcurrentEvaluations),
 mCommand("aprepro")
{
    mArgument.push_back("-q");
    mArgument.push_back(aInputDeck + ".template");
    mArgument.push_back(aInputDeck);
    mOptions = aOptions;

    mInput.mLayout = "scalar";
    mInput.mSize = std::to_string(mOptions.size());
    mInput.mSharedData = aSharedData;

    mOnChange = true;
}

void OperationAprepro::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);
    addChild(tOperationNode, "Command", mCommand);

    if(mChDir)
        addChild(tOperationNode, "ChDir", std::string("evaluations_") + tag(aEvaluationString) );
    addChild(tOperationNode, "OnChange", (mOnChange ? "true" : "false"));
    
    for( auto tA : mArgument )
        addChild(tOperationNode, "Argument", tA );
    
    addChild(tOperationNode, "AppendInput", "true" );

    for( auto tO : mOptions )
        addChild(tOperationNode, "Option", tO + std::string("=") );
    
    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", "parameters");
    addChild(tInputNode, "Layout", mInput.mLayout);
    addChild(tInputNode, "Size", mInput.mSize);
}

void OperationAprepro::write_interface
(pugi::xml_node& aNode, 
 std::string aEvaluationString)
{
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", "parameters");
    addChild(tInputNode, "SharedDataName", mInput.mSharedData->name(aEvaluationString));
}

OperationHarvestDataFunction::OperationHarvestDataFunction
(const std::string& aFile,
 const std::string& aMathOperation,
 const std::string& aDataColumn,
 std::shared_ptr<SharedData> aSharedData,
 std::shared_ptr<Performer> aPerformer,
 int aConcurrentEvaluations) : 
 Operation("harvest_data", "HarvestDataFromFile", aPerformer, aConcurrentEvaluations),
 mFile(aFile),
 mOperation(aMathOperation),
 mColumn(aDataColumn)
{
    mOutput.mLayout = "scalar";
    mOutput.mSize = "1";
    mOutput.mSharedData = aSharedData;
    
    mOnChange = true;
}

void OperationHarvestDataFunction::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);

    if(mChDir)
        addChild(tOperationNode, "File", std::string("./evaluations_") + tag(aEvaluationString) + std::string("/") + mFile);
    else
        addChild(tOperationNode, "File", mFile);

    addChild(tOperationNode, "Operation", mOperation);
    addChild(tOperationNode, "Column", mColumn);

    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", "criterion value");
    addChild(tOutputNode, "Layout", mOutput.mLayout);
    addChild(tOutputNode, "Size", mOutput.mSize);
}

void OperationHarvestDataFunction::write_interface
(pugi::xml_node& aNode, 
 std::string aEvaluationString)
{
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", "criterion value");
    addChild(tOutputNode, "SharedDataName", mOutput.mSharedData->name(aEvaluationString));
}

OperationCubitTet10Conversion::OperationCubitTet10Conversion
(const std::string& aExodusFileName, 
 const std::vector<std::string>& aBlockList, 
 std::shared_ptr<SharedData> aSharedData,
 std::shared_ptr<Performer> aPerformer, 
 int aConcurrentEvaluations) :
 Operation("cubit_convert_to_tet10", "SystemCall", aPerformer, aConcurrentEvaluations),
 mCommand("cubit")
{
    mArgument.push_back(" -input toTet10.jou -batch -nographics -nogui -noecho -nojournal -nobanner -information off");
    mInput.mSharedData = aSharedData;
    mOnChange = true;

    writeTet10ConversionFile(aExodusFileName,aBlockList);
}

void OperationCubitTet10Conversion::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);
    addChild(tOperationNode, "Command", mCommand);

    if(mChDir)
        addChild(tOperationNode, "ChDir", std::string("evaluations_") + tag(aEvaluationString) );
    addChild(tOperationNode, "OnChange", (mOnChange ? "true" : "false"));
    
    for( auto tA : mArgument )
        addChild(tOperationNode, "Argument", tA );
    
    addChild(tOperationNode, "AppendInput", "false" );

    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", "parameters");
}

void OperationCubitTet10Conversion::write_interface
(pugi::xml_node& aNode, 
 std::string aEvaluationString)
{
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", "parameters");
    addChild(tInputNode, "SharedDataName", mInput.mSharedData->name(aEvaluationString));
}

void OperationCubitTet10Conversion::writeTet10ConversionFile
(std::string aExodusFileName,
 std::vector<std::string> aBlockList)
{
    std::ofstream tOutFile("toTet10.jou");
    tOutFile<<"import mesh \""<<aExodusFileName<<"\" no_geom\n";
    for( auto tB : aBlockList )
        tOutFile<<"block "<<tB<<" element type tetra10\n";
    tOutFile<<"set exodus netcdf4 off\n";
    tOutFile<<"set large exodus file on\n";
    tOutFile<<"export mesh \""<<aExodusFileName<<"\" overwrite\n";
    tOutFile.close();

    for( int iEvaluation = 0 ; iEvaluation < evaluations(); ++iEvaluation)
    {  
        auto tCommand = std::string("mkdir -p ") + "evaluations_" + std::to_string(iEvaluation);
        Plato::system(tCommand.c_str());
        tCommand = std::string("cp ") + "toTet10.jou" + " " + "evaluations_" + std::to_string(iEvaluation);
        Plato::system(tCommand.c_str());
    }
}

OperationCubitSubBlock::OperationCubitSubBlock
(const std::string& aExodusFileName, 
 const std::vector<std::string>& aBoundingBox, 
 std::shared_ptr<SharedData> aSharedData,
 std::shared_ptr<Performer> aPerformer, 
 int aConcurrentEvaluations) :
 Operation("create_sub_block", "SystemCall", aPerformer, aConcurrentEvaluations),
 mCommand("cubit")
{
    mArgument.push_back(" -input subBlock.jou -batch -nographics -nogui -noecho -nojournal -nobanner -information off");
    mInput.mSharedData = aSharedData;
    mOnChange = true;

    writeSubBlockFile(aExodusFileName,aBoundingBox);
}

void OperationCubitSubBlock::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);
    addChild(tOperationNode, "Command", mCommand);

    if(mChDir)
        addChild(tOperationNode, "ChDir", std::string("evaluations_") + tag(aEvaluationString) );
    addChild(tOperationNode, "OnChange", (mOnChange ? "true" : "false"));
    
    for( auto tA : mArgument )
        addChild(tOperationNode, "Argument", tA );
    
    addChild(tOperationNode, "AppendInput", "false" );

    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", mInput.mSharedData->name(aEvaluationString));
}

void OperationCubitSubBlock::write_interface
(pugi::xml_node& aNode, 
 std::string aEvaluationString)
{
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", mInput.mSharedData->name(aEvaluationString));
    addChild(tInputNode, "SharedDataName", mInput.mSharedData->name(aEvaluationString));
}

void OperationCubitSubBlock::writeSubBlockFile
(std::string aExodusFileName,
 std::vector<std::string> aBoundingBox)
{
    std::vector<std::string> tCoords = {"x_coord >= ", "and y_coord >= ", "and z_coord >= ", "and x_coord <= ", "and y_coord <= ", "and z_coord <= " };
    std::ofstream tOutFile("subBlock.jou");
    tOutFile<<"import mesh \""<<aExodusFileName<<"\" no_geom\n";
    tOutFile<<"delete block all\n";
    tOutFile<<"block 2 tet with ";
    for( unsigned int iB = 0; iB < aBoundingBox.size(); ++iB )
        tOutFile<<tCoords[iB]<<aBoundingBox[iB];
    tOutFile<<"\n";
    tOutFile<<"block 1 tet all\n";
    tOutFile<<"set exodus netcdf4 off\n";
    tOutFile<<"set large exodus file on\n";
    tOutFile<<"export mesh \""<<aExodusFileName<<"\" overwrite\n";
    tOutFile.close();

    for( int iEvaluation = 0 ; iEvaluation < evaluations(); ++iEvaluation)
    {  
        auto tCommand = std::string("mkdir -p ") + "evaluations_" + std::to_string(iEvaluation);
        Plato::system(tCommand.c_str());
        tCommand = std::string("cp ") + "subBlock.jou" + " " + "evaluations_" + std::to_string(iEvaluation);
        Plato::system(tCommand.c_str());
    }
}

OperationDecomp::OperationDecomp
(const std::string& aExodusFileName, 
 int aNumberOfPartitions,
 std::shared_ptr<Performer> aPerformer, 
 int aConcurrentEvaluations) :
 Operation("decomp_mesh", "SystemCall", aPerformer, aConcurrentEvaluations),
 mCommand("decomp")
{
    mArgument.push_back(" -p ");
    mArgument.push_back(std::to_string(aNumberOfPartitions));
    mArgument.push_back(" ");
    mArgument.push_back(aExodusFileName);
    mOnChange = false;
}

void OperationDecomp::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);
    addChild(tOperationNode, "Command", mCommand);

    if(mChDir)
        addChild(tOperationNode, "ChDir", std::string("evaluations_") + tag(aEvaluationString) );
    
    for( auto tA : mArgument )
        addChild(tOperationNode, "Argument", tA );    
}

void OperationDecomp::write_interface
(pugi::xml_node& aNode, 
 std::string aEvaluationString)
{
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
}

OperationSetBounds::OperationSetBounds
(const std::string& aName,
upperLower aUpperLower,
const std::string& aUseCase,
const std::string& aDiscretization,
std::shared_ptr<SharedData> aInputSharedData,
std::shared_ptr<SharedData> aOutputSharedData,
std::shared_ptr<Performer> aPerformer) :
Operation(aName, (aUpperLower==lower?"SetLowerBounds":"SetUpperBounds"), aPerformer, 0),
mUseCase(aUseCase),
mDiscretization(aDiscretization)
{
    mInput.mSharedData = aInputSharedData;
    mOutput.mSharedData = aOutputSharedData;
}

void OperationSetBounds::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);
    addChild(tOperationNode, "UseCase",mUseCase);
    addChild(tOperationNode, "Discretization",mDiscretization);
    
    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", mInput.mSharedData->name(aEvaluationString));
    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", mOutput.mSharedData->name(aEvaluationString));
}

void OperationSetBounds::write_interface
(pugi::xml_node& aNode, 
 std::string aEvaluationString)
{
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
    
    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", mInput.mSharedData->name(aEvaluationString));
    addChild(tInputNode, "SharedDataName", mInput.mSharedData->name(aEvaluationString));

    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", mOutput.mSharedData->name(aEvaluationString));
    addChild(tOutputNode, "SharedDataName", mOutput.mSharedData->name(aEvaluationString));

}

OperationFilter::OperationFilter
(const std::string& aName,
returnVariable aReturnVar,
std::shared_ptr<SharedData> aInputSharedData,
std::shared_ptr<SharedData> aOutputSharedData,
std::shared_ptr<Performer> aPerformer) :
Operation(aName, "Filter", aPerformer, 0),
mReturnVar(aReturnVar)
{
    mInput.mSharedData = aInputSharedData;
    mOutput.mSharedData = aOutputSharedData;
}

void OperationFilter::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);
    addChild(tOperationNode, "Gradient",(mReturnVar==gradient?"True":"False"));
    
    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", mInput.mSharedData->name(aEvaluationString));
    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", mOutput.mSharedData->name(aEvaluationString));
}

void OperationFilter::write_interface
(pugi::xml_node& aNode, 
 std::string aEvaluationString)
{
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
    
    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", mInput.mSharedData->name(aEvaluationString));
    addChild(tInputNode, "SharedDataName", mInput.mSharedData->name(aEvaluationString));

    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", mOutput.mSharedData->name(aEvaluationString));
    addChild(tOutputNode, "SharedDataName", mOutput.mSharedData->name(aEvaluationString));

}


OperationInitializeUniformField::OperationInitializeUniformField
(const std::string& aName,
double aInitialValue,
std::shared_ptr<SharedData> aOutputSharedData,
std::shared_ptr<Performer> aPerformer) :
Operation(aName, "InitializeField", aPerformer, 0)
{
    mOutput.mSharedData = aOutputSharedData;
    mInitialValue = aInitialValue;
}

void OperationInitializeUniformField::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);
    addChild(tOperationNode, "Method","Uniform");
    auto tUniformNode = tOperationNode.append_child("Uniform");
    addChild(tUniformNode,"Value",std::to_string(mInitialValue));
    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", mOutput.mSharedData->name(aEvaluationString));
}

void OperationInitializeUniformField::write_interface
(pugi::xml_node& aNode, 
 std::string aEvaluationString)
{
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
    
    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", mOutput.mSharedData->name(aEvaluationString));
    addChild(tOutputNode, "SharedDataName", mOutput.mSharedData->name(aEvaluationString));

}

OperationCopy::OperationCopy
(const std::string& aName,
copyVariable aCopyVar,
std::shared_ptr<SharedData> aInputSharedData,
std::shared_ptr<SharedData> aOutputSharedData,
std::shared_ptr<Performer> aPerformer):
Operation(aName, (aCopyVar == copyvalue ?"CopyValue":"CopyField"), aPerformer, 0)
{
    mInput.mSharedData = aInputSharedData;
    mOutput.mSharedData = aOutputSharedData;
}

void OperationCopy::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);    
    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", mInput.mSharedData->name(aEvaluationString));
    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", mOutput.mSharedData->name(aEvaluationString));
}

void OperationCopy::write_interface
(pugi::xml_node& aNode, 
std::string aEvaluationString)
{
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
    
    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", mInput.mSharedData->name(aEvaluationString));
    addChild(tInputNode, "SharedDataName", mInput.mSharedData->name(aEvaluationString));

    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", mOutput.mSharedData->name(aEvaluationString));
    addChild(tOutputNode, "SharedDataName", mOutput.mSharedData->name(aEvaluationString));
}

OperationComputeCriterion::OperationComputeCriterion
(const std::string& aName,
returnVariable aReturnVariable,
std::string aCriterion, 
std::shared_ptr<SharedData> aInputSharedData,
std::shared_ptr<SharedData> aOutputSharedData,
std::shared_ptr<Performer> aPerformer,
double aTarget):
Operation(aName, (aReturnVariable==value?"ComputeCriterionValue":"ComputeCriterionGradient"), aPerformer, 0),
mReturnVariable(aReturnVariable),
mCriterion(aCriterion),
mTarget(aTarget)
{
    mInput.mSharedData = aInputSharedData;
    mOutput.mSharedData = aOutputSharedData;
}

void OperationComputeCriterion::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);
    addChild(tOperationNode, "Criterion",mCriterion);
    if(mReturnVariable==value)
        addChild(tOperationNode, "Target",std::to_string(mTarget));

    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", mInput.mSharedData->name(aEvaluationString));
    auto tOutputNode = tOperationNode.append_child("Output");
    if(mReturnVariable==value)
        addChild(tOutputNode, "Argument", "Value");
    else
        addChild(tOutputNode, "Argument", "Gradient");
    addChild(tOutputNode, "ArgumentName", mOutput.mSharedData->name(aEvaluationString));
}

void OperationComputeCriterion::write_interface
(pugi::xml_node& aNode, 
 std::string aEvaluationString)
{
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
    
    auto tInputNode = tOperationNode.append_child("Input");
    addChild(tInputNode, "ArgumentName", mInput.mSharedData->name(aEvaluationString));
    addChild(tInputNode, "SharedDataName", mInput.mSharedData->name(aEvaluationString));

    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", mOutput.mSharedData->name(aEvaluationString));
    addChild(tOutputNode, "SharedDataName", mOutput.mSharedData->name(aEvaluationString));

}

OperationDesignVolume::OperationDesignVolume
(const std::string& aName,
std::shared_ptr<SharedData> aOutputSharedData,
std::shared_ptr<Performer> aPerformer):
Operation(aName, "DesignVolume", aPerformer, 0)
{
    mOutput.mSharedData = aOutputSharedData;
}

void OperationDesignVolume::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);    
    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", mOutput.mSharedData->name(aEvaluationString));
}

void OperationDesignVolume::write_interface
(pugi::xml_node& aNode, 
std::string aEvaluationString)
{
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
    
    auto tOutputNode = tOperationNode.append_child("Output");
    addChild(tOutputNode, "ArgumentName", mOutput.mSharedData->name(aEvaluationString));
    addChild(tOutputNode, "SharedDataName", mOutput.mSharedData->name(aEvaluationString));
}

OperationComputeVolumeSIMP::OperationComputeVolumeSIMP
(const std::string& aName,
 std::shared_ptr<SharedData> aInputSharedData,
 std::vector<std::shared_ptr<SharedData> > aOutputSharedData,
 std::shared_ptr<Performer> aPerformer):
Operation(aName, "ComputeVolume", aPerformer, 0)
{
    mInput.mSharedData = aInputSharedData;
    for( auto iSd : aOutputSharedData )
    {
        InputOutput tOut;
        tOut.mSharedData = iSd;
        mOutputs.push_back(tOut);
    }
}

void OperationComputeVolumeSIMP::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);
    addChild(tOperationNode, "PenaltyModel","SIMP");
    
    auto tIONode = tOperationNode.append_child("Input");
    addChild(tIONode, "ArgumentName", mInput.mSharedData->name(aEvaluationString));
    
    for(auto iOutput : mOutputs)
    {   
        tIONode = tOperationNode.append_child("Output");
        addChild(tIONode, "ArgumentName", iOutput.mSharedData->name(aEvaluationString));
    }
    
    auto tSIMPNode = tOperationNode.append_child("SIMP");
    addChild(tSIMPNode, "PenaltyExponent","1.0");
    addChild(tSIMPNode, "MinimumValue","0.0");
}

void OperationComputeVolumeSIMP::write_interface
(pugi::xml_node& aNode, 
 std::string aEvaluationString)
 {
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
    
    auto tIONode = tOperationNode.append_child("Input");
    addChild(tIONode, "ArgumentName", mInput.mSharedData->name(aEvaluationString));
    addChild(tIONode, "SharedDataName", mInput.mSharedData->name(aEvaluationString));

    for(auto iOutput : mOutputs)
    {   
        tIONode = tOperationNode.append_child("Output");
        addChild(tIONode, "ArgumentName", iOutput.mSharedData->name(aEvaluationString));
        addChild(tIONode, "SharedDataName", iOutput.mSharedData->name(aEvaluationString));
    }

 }

OperationWriteOutput::OperationWriteOutput
(const std::string& aName,
 std::vector<std::shared_ptr<SharedData> > aOutputSharedData,
 std::shared_ptr<Performer> aPerformer):
Operation(aName, "WriteOutput", aPerformer, 0)
{
    for( auto iSd : aOutputSharedData )
    {
        InputOutput tOut;
        tOut.mSharedData = iSd;
        mOutputs.push_back(tOut);
    }
}

void OperationWriteOutput::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);
    
    if(mOutputs.size()>0)
    {
        auto tIONode = tOperationNode.append_child("Output");
        addChild(tIONode, "ArgumentName", mOutputs[0].mSharedData->name(aEvaluationString));
    
        for(unsigned int iOut = 1; iOut < mOutputs.size(); ++iOut)
        {   
            tIONode = tOperationNode.append_child("Output");
            addChild(tIONode, "ArgumentName", mOutputs[iOut].mSharedData->name(aEvaluationString));
        }
    }
    
}

void OperationWriteOutput::write_interface
(pugi::xml_node& aNode, 
 std::string aEvaluationString)
 {
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));

    if(mOutputs.size()>0)
    {
        auto tIONode = tOperationNode.append_child("Output");  
        addChild(tIONode, "ArgumentName", mOutputs[0].mSharedData->name(aEvaluationString));
        addChild(tIONode, "SharedDataName", mOutputs[0].mSharedData->name(aEvaluationString));

        for(unsigned int iOut = 1; iOut < mOutputs.size(); ++iOut)
        {   
            tIONode = tOperationNode.append_child("Output");
            addChild(tIONode, "ArgumentName", mOutputs[iOut].mSharedData->name(aEvaluationString));
            addChild(tIONode, "SharedDataName", mOutputs[iOut].mSharedData->name(aEvaluationString));
        }
    }
 }

OperationAggregator::OperationAggregator
(const std::string& aName,
 std::vector<std::shared_ptr<SharedData> > /*aOutputSharedData*/,
 std::shared_ptr<Performer> aPerformer):
Operation(aName, "Aggregator", aPerformer, 0)
{
   
}

void OperationAggregator::write_definition
(pugi::xml_document& aDocument, 
 std::string aEvaluationString)
{
    auto tOperationNode = aDocument.append_child("Operation");
    appendCommonChildren(tOperationNode,aEvaluationString);
}

void OperationAggregator::write_interface
(pugi::xml_node& aNode, 
 std::string aEvaluationString)
 {
    auto tOperationNode = aNode.append_child("Operation");
    addChild(tOperationNode, "Name", name(aEvaluationString));
    addChild(tOperationNode, "PerformerName",  mPerformer->name(aEvaluationString));
 }


} //namespace
