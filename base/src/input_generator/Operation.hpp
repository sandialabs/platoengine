/*
 * Operation.hpp
 *
 *  Created on: April 12, 2022
 */
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <regex>

#include "pugixml.hpp"

#include "SharedData.hpp"
#include "Performer.hpp"

namespace director
{
enum returnVariable {value, gradient};
enum upperLower {upper , lower};
enum copyVariable {copyvalue , copyfield};

struct InputOutput
{
    std::string mLayout;
    std::string mSize;
    std::shared_ptr<SharedData> mSharedData;
};

class Operation : public FileObject
{

protected:
    std::string mFunction;
    std::shared_ptr<Performer> mPerformer;
    bool mChDir;
    bool mOnChange;
    
    void appendCommonChildren(pugi::xml_node &aOperationNode,
                              std::string aEvaluationString);

public:
    Operation(const std::string& aName,
                          const std::string& aFunction,
                          std::shared_ptr<Performer> aPerformer,
                          int aConcurrentEvaluations);
    virtual void write_definition(pugi::xml_document& aDocument, 
                                  std::string aEvaluationString = "") = 0;
    virtual void write_interface(pugi::xml_node& aNode, 
                                 std::string aEvaluationString = "") = 0;

    pugi::xml_node forNode(pugi::xml_node& aNode, 
                           const std::string& aXMLLoopVectorName);
};

class OperationGemmaMPISystemCall : public Operation
{
private:
    std::string mCommand; 
    std::string mArgument;
    std::string mNumRanks;
    std::string mPath;
    
public:
    OperationGemmaMPISystemCall(const std::string& aInputDeck,
                                            const std::string& aPath,
                                            const std::string& aNumRanks, 
                                            std::shared_ptr<Performer> aPerformer,
                                            int aConcurrentEvaluations);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationAprepro : public Operation
{
private:
    std::string mCommand; 
    std::vector<std::string> mArgument;
    std::vector<std::string> mOptions;
    InputOutput mInput;

public:
    OperationAprepro(const std::string& aInputDeck,
                                 const std::vector<std::string>& aOptions, 
                                 std::shared_ptr<SharedData> aSharedData, 
                                 std::shared_ptr<Performer> aPerformer,
                                 int aConcurrentEvaluations);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationHarvestDataFunction : public Operation
{
private:
    std::string mFile;
    std::string mOperation;
    std::string mColumn;
    InputOutput mOutput;
public:
    OperationHarvestDataFunction(const std::string& aFile,
                                             const std::string& aMathOperation,
                                             const std::string& aDataColumn,
                                             std::shared_ptr<SharedData> aSharedData,
                                             std::shared_ptr<Performer> aPerformer,
                                             int aConcurrentEvaluations);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationCubitTet10Conversion : public Operation
{
private:
    std::string mCommand; 
    std::vector<std::string> mArgument;
    std::vector<std::string> mOptions;
    InputOutput mInput;

    void writeTet10ConversionFile(std::string aExodusFileName,
                                  std::vector<std::string> aBlockList);
public:
    OperationCubitTet10Conversion(const std::string& aExodusFileName,
                                              const std::vector<std::string>& aBlockList, 
                                              std::shared_ptr<SharedData> aSharedData, 
                                              std::shared_ptr<Performer> aPerformer,
                                              int aConcurrentEvaluations);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationCubitSubBlock : public Operation
{
private:
    std::string mCommand; 
    std::vector<std::string> mArgument;
    std::vector<std::string> mOptions;
    InputOutput mInput;

    void writeSubBlockFile(std::string aExodusFileName,
                           std::vector<std::string> aBoundingBox);
public:
    OperationCubitSubBlock(const std::string& aExodusFileName,
                                       const std::vector<std::string>& aBoundingBox, 
                                       std::shared_ptr<SharedData> aSharedData, 
                                       std::shared_ptr<Performer> aPerformer,
                                       int aConcurrentEvaluations);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationDecomp : public Operation
{
private:
    std::string mCommand; 
    std::vector<std::string> mArgument;
    
public:
    OperationDecomp(const std::string& aExodusFileName,
                                int aNumberOfPartitions, 
                                std::shared_ptr<Performer> aPerformer,
                                int aConcurrentEvaluations);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationSetBounds : public Operation
{
private:
    InputOutput mInput;
    InputOutput mOutput;
    std::string mUseCase;
    std::string mDiscretization;
    
public:
    OperationSetBounds(const std::string& aName,
                       upperLower aUpperLower,
                       const std::string& aUseCase,
                       const std::string& aDiscretization,
                       std::shared_ptr<SharedData> aInputSharedData,
                       std::shared_ptr<SharedData> aOutputSharedData,
                       std::shared_ptr<Performer> aPerformer);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationFilter : public Operation
{
private:
    InputOutput mInput;
    InputOutput mOutput;
    returnVariable mReturnVar;
public:
    OperationFilter(const std::string& aName,
                       returnVariable aReturnVar,
                       std::shared_ptr<SharedData> aInputSharedData,
                       std::shared_ptr<SharedData> aOutputSharedData,
                       std::shared_ptr<Performer> aPerformer);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationInitializeUniformField : public Operation
{
private:
    InputOutput mOutput;
    double mInitialValue;
public:
    OperationInitializeUniformField(const std::string& aName,
                       double aInitialValue,
                       std::shared_ptr<SharedData> aOutputSharedData,
                       std::shared_ptr<Performer> aPerformer);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationCopy : public Operation
{
private:
    InputOutput mInput;
    InputOutput mOutput;
public:
    OperationCopy(const std::string& aName,
                       copyVariable aCopyVar,
                       std::shared_ptr<SharedData> aInputSharedData,
                       std::shared_ptr<SharedData> aOutputSharedData,
                       std::shared_ptr<Performer> aPerformer);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationComputeCriterion : public Operation
{
private:
    InputOutput mInput;
    InputOutput mOutput;
    returnVariable mReturnVariable;
    std::string mCriterion;
    double mTarget;

public:
    OperationComputeCriterion(const std::string& aName,
                       returnVariable aReturnVariable,
                       std::string aCriterion, 
                       std::shared_ptr<SharedData> aInputSharedData,
                       std::shared_ptr<SharedData> aOutputSharedData,
                       std::shared_ptr<Performer> aPerformer,
                       double aTarget = 0.0 );
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationDesignVolume : public Operation
{
private:
    InputOutput mOutput;
public:
    OperationDesignVolume(const std::string& aName,
                       std::shared_ptr<SharedData> aOutputSharedData,
                       std::shared_ptr<Performer> aPerformer);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationComputeVolumeSIMP : public Operation
{
private:
    InputOutput mInput;
    std::vector<InputOutput> mOutputs;
    
public:
    OperationComputeVolumeSIMP(const std::string& aName,
                       std::shared_ptr<SharedData> aInputSharedData,
                       std::vector<std::shared_ptr<SharedData> > aOutputSharedData,
                       std::shared_ptr<Performer> aPerformer);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

class OperationWriteOutput : public Operation
{
private:
    std::vector<InputOutput> mOutputs;
    
public:
    OperationWriteOutput(const std::string& aName,
                       std::vector<std::shared_ptr<SharedData> > aOutputSharedData,
                       std::shared_ptr<Performer> aPerformer);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};

enum layout {elementvalue, nodalfield};
struct Aggregate
{
   layout mLayout;
   std::vector<std::shared_ptr<SharedData> > mInput;
   std::shared_ptr<SharedData> mOutput;
};

class OperationAggregator : public Operation
{
private:
    std::vector<Aggregate> mAggregates;
    
public:
    OperationAggregator(const std::string& aName,
                       std::vector<std::shared_ptr<SharedData> > aOutputSharedData,
                       std::shared_ptr<Performer> aPerformer);
    void write_definition(pugi::xml_document& aDocument, 
                          std::string aEvaluationString = "") override;
    void write_interface(pugi::xml_node& aNode, 
                         std::string aEvaluationString = "") override;
};


}
