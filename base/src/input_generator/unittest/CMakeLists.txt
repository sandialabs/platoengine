SET(SRCS XMLGenerator_UnitMain.cpp
          XMLGenerator_UnitTester.cpp
          UnitTesterTools_UnitTester.cpp
          Plato_Test_XMLGenerator.cpp
          XMLGeneratorLaunch_UnitTester.cpp
          PlatoGemma_UnitTester.cpp
          XMLGeneratorDefineFile_UnitTester.cpp
          XMLGeneratorDakotaDriver_UnitTester.cpp
          XMLGeneratorParseMetaData_UnitTester.cpp
          XMLGeneratorInterfaceFile_UnitTester.cpp
          XMLGeneratorLaunchScript_UnitTester.cpp
          XMLGeneratorRandomInterfaceFile_UnitTester.cpp
          XMLGeneratorPlatoMainInputDeckFile_UnitTester.cpp
          XMLGeneratorPlatoMainOperationFile_UnitTester.cpp
          XMLGeneratorPlatoAnalyzeInputFile_UnitTester.cpp
          XMLGeneratorPlatoAnalyzeOperationsFile_UnitTester.cpp
          XMLGeneratorSierraSDInputFile_UnitTester.cpp
          XMLGeneratorSierraTFOperationsFile_UnitTester.cpp
          XMLGeneratorSierraSDOperationsFile_UnitTester.cpp
          XMLGeneratorXTKInputDeck_UnitTester.cpp
          XMLGeneratorStageOperationsUtilities_UnitTester.cpp
          XMLGeneratorUtilities_UnitTester.cpp
          XMLGeneratorXTKOperationFile_UnitTester.cpp
          XMLGeneratorOptimizationParametersInput_UnitTester.cpp
          FileObject_UnitTester.cpp
          Performer_UnitTester.cpp
          SharedData_UnitTester.cpp
          Stage_UnitTester.cpp
          Criterion_UnitTester.cpp
          Operation_UnitTester.cpp
          OptimizationAlgorithm_UnitTester.cpp
          ../XMLGenerator.cpp
          ../XMLGeneratorProblem.cpp
          ../XMLGeneratorParseRun.cpp
          ../XMLGeneratorUtilities.cpp
          ../XMLGeneratorParseBlock.cpp
          ../XMLGeneratorParseLoads.cpp
          ../XMLGeneratorParseOutput.cpp
          ../XMLGeneratorRunMetadata.cpp
          ../XMLGeneratorDakotaProblem.cpp
          ../XMLGeneratorParseAssembly.cpp
          ../XMLGeneratorParseScenario.cpp
          ../XMLGeneratorParseServices.cpp
          ../XMLGeneratorParseMaterial.cpp
          ../XMLGeneratorParseCriteria.cpp
          ../XMLGeneratorOutputMetadata.cpp
          ../XMLGeneratorParseObjective.cpp
          ../XMLGeneratorOutputUtilities.cpp
          ../XMLGeneratorStagesUtilities.cpp
          ../XMLGeneratorParserUtilities.cpp
          ../XMLGeneratorParseConstraint.cpp
          ../XMLGeneratorServiceMetadata.cpp
          ../XMLGeneratorProblemUtilities.cpp
          ../XMLGeneratorServiceUtilities.cpp
          ../XMLGeneratorParseUncertainty.cpp
          ../XMLGeneratorScenarioMetadata.cpp
          ../XMLGeneratorSierraSDUtilities.cpp
          ../XMLGeneratorPlatoXTKInputFile.cpp
          ../XMLGeneratorConstraintMetadata.cpp
          ../XMLGeneratorSharedDataUtilities.cpp
          ../XMLGeneratorPerformersUtilities.cpp
          ../XMLGeneratorUncertaintyMetadata.cpp
          ../XMLGeneratorDefinesFileUtilities.cpp
          ../XMLGeneratorPlatoXTKOperationFile.cpp
          ../XMLGeneratorPlatoAnalyzeUtilities.cpp
          ../XMLGeneratorLaunchUtilities.cpp
          ../XMLGeneratorInterfaceFileUtilities.cpp
          ../XMLGeneratorDakotaInterfaceFileUtilities.cpp
          ../XMLGeneratorDakotaDriverInputFileUtilities.cpp
          ../XMLGeneratorGradBasedOptimizerOptions.cpp
          ../XMLGeneratorStagesOperationsUtilities.cpp
          ../XMLGeneratorMaterialFunctionInterface.cpp
          ../XMLGeneratorPlatoESPInputFileUtilities.cpp
          ../XMLGeneratorSierraSDInputDeckUtilities.cpp
          ../XMLGeneratorSierraTFInputDeckUtilities.cpp
          ../XMLGeneratorPlatoMainInputFileUtilities.cpp
          ../XMLGeneratorParseOptimizationParameters.cpp
          ../XMLGeneratorAnalyzeLoadFunctionInterface.cpp
          ../XMLGeneratorRandomInterfaceFileUtilities.cpp
          ../XMLGeneratorPlatoAnalyzeInputFileUtilities.cpp
          ../XMLGeneratorOptimizationParametersMetadata.cpp
          ../XMLGeneratorPlatoESPOperationsFileUtilities.cpp
          ../XMLGeneratorAnalyzePhysicsFunctionInterface.cpp
          ../XMLGeneratorPlatoMainOperationFileUtilities.cpp
          ../XMLGeneratorParseEssentialBoundaryCondition.cpp
          ../XMLGeneratorSierraSDOperationsFileUtilities.cpp
          ../XMLGeneratorSierraTFOperationsFileUtilities.cpp
          ../XMLGeneratorSierraSDOperationsFileUtilities.cpp
          ../XMLGeneratorAnalyzeLoadTagFunctionInterface.cpp
          ../XMLGeneratorParseMethodInputOptionsUtilities.cpp
          ../XMLGeneratorPostOptimizationRunFileUtilities.cpp
          ../XMLGeneratorAnalyzeAssemblyFunctionInterface.cpp
          ../XMLGeneratorPlatoAnalyzeOperationsFileUtilities.cpp
          ../XMLGeneratorAnalyzeEssentialBCFunctionInterface.cpp
          ../XMLGeneratorAnalyzeMaterialModelFunctionInterface.cpp
          ../XMLGeneratorAnalyzeEssentialBCTagFunctionInterface.cpp
          ../XMLGeneratorLaunchScriptUtilities.cpp
          ../XMLGeneratorPlatoMainConstraintValueOperationInterface.cpp
          ../XMLGeneratorPlatoMainConstraintGradientOperationInterface.cpp
          # ../PlatoAnalyzeInputDeckWriter.cpp
          # ../SalinasInputDeckWriter.cpp)
			    ../FileObject.cpp
			    ../Operation.cpp
          ../Stage.cpp
			    ../Performer.cpp
			    ../SharedData.cpp
			    ../GemmaProblem.cpp
          ../OptimizationAlgorithmFactory.cpp
          ../OptimizationAlgorithm.cpp
          )

SET(HDRS XMLGenerator_UnitTester.hpp
         XMLGenerator_UnitTester_Tools.hpp)

add_executable(PlatoXMLGenerator_UnitTester ${SRCS})
target_include_directories(PlatoXMLGenerator_UnitTester PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
target_link_libraries(PlatoXMLGenerator_UnitTester GTest::GTest PlatoParser PlatoAnalyze)
add_test(NAME PlatoXMLGenerator_UnitTester COMMAND PlatoXMLGenerator_UnitTester)
set_property(TEST PlatoXMLGenerator_UnitTester PROPERTY LABELS "small")

install( TARGETS PlatoXMLGenerator_UnitTester EXPORT PlatoEngine DESTINATION bin )
