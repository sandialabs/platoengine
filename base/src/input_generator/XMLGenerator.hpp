#ifndef SRC_XMLGENERATOR_HPP_
#define SRC_XMLGENERATOR_HPP_

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <tuple>

#include "Plato_Parser.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

class XMLGenerator
{

public:
    explicit XMLGenerator(const std::string &input_filename = "", bool use_launch = false, const XMLGen::Arch& arch = XMLGen::Arch::CEE);
    virtual ~XMLGenerator() = default;
    void generate();
    void generate(XMLGen::InputData& aInputData);
    const XMLGen::InputData& getInputData(){ return m_InputData; }

protected:

    bool parseLoads(std::istream &fin);
    void getTokensFromLine(std::istream &fin, std::vector<std::string>& tokens);
    void parseBoundaryConditions(std::istream &fin);
    bool parseAssemblies(std::istream &fin);
    void setNumPerformers(XMLGen::InputData& aInputData);
    void parseInputFile();
    bool parseMesh(std::istream &fin);
    void parseOutput(std::istream &fin);
    void parseMaterials(std::istream &fin);
    void parseCriteria(std::istream &fin);
    bool parseBlocks(std::istream &fin);
    void parseObjective(std::istream &fin);
    void parseScenarios(std::istream &fin);
    void parseServices(std::istream &fin);
    void parseRuns(std::istream &fin);
    bool parseConstraints(std::istream &fin);
    bool parseOptimizationParameters(std::istream &fin);
    void parseUncertainties(std::istream &fin);

    void finalize(XMLGen::InputData& aInputData);
    void finalizeScenarios(XMLGen::InputData& aInputData);
    void finalizeMeshMetaData(XMLGen::InputData& aInputData);
    
    std::string toLower(const std::string &s);
    std::string toUpper(const std::string &s);
    bool parseSingleValue(const std::vector<std::string> &aTokens,
                          const std::vector<std::string> &aInputStrings,
                          std::string &aReturnStringValue);

    bool parseSingleUnLoweredValue(const std::vector<std::string> &aTokens,
                                   const std::vector<std::string> &aUnLoweredTokens,
                                   const std::vector<std::string> &aInputStrings,
                                   std::string &aReturnStringValue);

    void preProcessInputMetaData(XMLGen::InputData& aInputData);

    std::string m_InputFilename;
    XMLGen::InputData m_InputData;
    XMLGen::InputData m_InputDataWithExpandedEBCs;
    std::vector<XMLGen::InputData> m_PreProcessedInputData;
private:

    void determineIfPlatoEngineFilteringIsNeeded();
    void expandEssentialBoundaryConditions(XMLGen::InputData& aInputData);
    void updateScenariosWithExpandedBoundaryConditions(std::map<int, std::vector<int> > aOldIDToNewIDMap);

    void setupHelmholtzFilterService(XMLGen::InputData& aInputData);

    void loadOutputData
    (XMLGen::InputData &aNewInputData,
     const XMLGen::InputData &aOriginalInputData,
     const std::string &aServiceID);

    void loadMaterialData
    (XMLGen::InputData &aNewInputData,
     const XMLGen::InputData &aOriginalInputData,
     const std::string &aScenarioID);

    void loadObjectiveData
    (XMLGen::InputData &aNewInputData,
     const XMLGen::InputData &aOriginalInputData,
     const std::string &aScenarioID,
     const std::string &aServiceID,
     const std::string &aShapeServiceID);

    void loadConstraintData
    (XMLGen::InputData &aNewInputData,
     const XMLGen::InputData &aOriginalInputData,
     const std::string &aScenarioID,
     const std::string &aServiceID);

    void clearInputDataLists(XMLGen::InputData &aInputData);
    void createCopiesForMultiLoadCase();

    void createCopiesForMultiPerformerCase
    (XMLGen::InputData& aInputData,
     std::set<std::tuple<std::string,std::string,std::string>> &aScenarioServiceTuples);

    void createCopiesForPerformerCreation(XMLGen::InputData& aInputData);

    void findObjectiveScenarioServiceTuples
    (XMLGen::InputData& aInputData,
     std::set<std::tuple<std::string,std::string,std::string>>& aObjectiveScenarioServiceTuples);

    void findConstraintScenarioServiceTuples
    (XMLGen::InputData& aInputData,
     std::set<std::tuple<std::string,std::string,std::string>>& aConstraintScenarioServiceTuples);

    void removeDuplicateTuplesFromConstraintList(
         std::set<std::tuple<std::string,std::string,std::string>>& aObjectiveScenarioServiceTuples, 
         std::set<std::tuple<std::string,std::string,std::string>>& aConstraintScenarioServiceTuples);

    void createInputDataCopiesForObjectivePerformers
    (XMLGen::InputData& aInputData,
     std::set<std::tuple<std::string,std::string,std::string>>& aObjectiveScenarioServiceTuples);

    void createInputDataCopiesForConstraintPerformers
    (XMLGen::InputData& aInputData,
    std::set<std::tuple<std::string,std::string,std::string>>& aConstraintScenarioServiceTuples);

    void createObjectiveCopiesForMultiLoadCase
    (XMLGen::InputData& aInputData,
     std::set<std::tuple<std::string,std::string,std::string>>& aObjectiveScenarioServiceTuples);

    void verifyAllServicesAreTheSame(XMLGen::InputData& aInputData);

    bool serviceExists(std::vector<XMLGen::Service> &aServiceList, XMLGen::Service &aService);

    void writeInputFiles(XMLGen::InputData& aInputData);
    void checkForProblemSetupErrors(XMLGen::InputData& aInputData);
    void checkForShapeOptimizationSetupErrors(XMLGen::InputData& aInputData);
};

}

#endif /* SRC_XMLGENERATOR_HPP_ */
