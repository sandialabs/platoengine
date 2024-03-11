SET(XMLGenerator_SRCS XMLGenerator_Main.cpp 
                      XMLGenerator.cpp
                      XMLGeneratorProblem.cpp
                      XMLGeneratorParseBlock.cpp
                      XMLGeneratorParseLoads.cpp
                      XMLGeneratorParseOutput.cpp
                      XMLGeneratorParseScenario.cpp
                      XMLGeneratorParseServices.cpp
                      XMLGeneratorParseMaterial.cpp
                      XMLGeneratorParseAssembly.cpp
                      XMLGeneratorParseCriteria.cpp
                      XMLGeneratorDakotaProblem.cpp
                      XMLGeneratorOutputMetadata.cpp 
                      XMLGeneratorParseObjective.cpp
                      XMLGeneratorOutputUtilities.cpp
                      XMLGeneratorStagesUtilities.cpp
                      XMLGeneratorParseConstraint.cpp
                      XMLGeneratorParserUtilities.cpp
                      XMLGeneratorServiceMetadata.cpp
                      XMLGeneratorProblemUtilities.cpp
                      XMLGeneratorServiceUtilities.cpp
                      XMLGeneratorScenarioMetadata.cpp
                      XMLGeneratorParseUncertainty.cpp
                      XMLGeneratorSierraSDUtilities.cpp
                      XMLGeneratorConstraintMetadata.cpp
                      XMLGeneratorSharedDataUtilities.cpp
                      XMLGeneratorPerformersUtilities.cpp
                      XMLGeneratorUncertaintyMetadata.cpp
                      XMLGeneratorDefinesFileUtilities.cpp
                      XMLGeneratorPlatoAnalyzeUtilities.cpp
                      XMLGeneratorInterfaceFileUtilities.cpp
                      XMLGeneratorDakotaInterfaceFileUtilities.cpp
                      XMLGeneratorDakotaDriverInputFileUtilities.cpp 
                      XMLGeneratorGradBasedOptimizerOptions.cpp
                      XMLGeneratorStagesOperationsUtilities.cpp
                      XMLGeneratorParseOptimizationParameters.cpp
                      XMLGeneratorOptimizationParametersMetadata.cpp
                      XMLGeneratorParseEssentialBoundaryCondition.cpp
                      XMLGeneratorParseMethodInputOptionsUtilities.cpp
                      XMLGeneratorPostOptimizationRunFileUtilities.cpp
                      XMLGeneratorAnalyzePhysicsFunctionInterface.cpp
                      XMLGeneratorMaterialFunctionInterface.cpp
                      XMLGeneratorRandomInterfaceFileUtilities.cpp
                      XMLGeneratorPlatoMainInputFileUtilities.cpp
                      XMLGeneratorPlatoESPInputFileUtilities.cpp
                      XMLGeneratorPlatoESPOperationsFileUtilities.cpp
                      XMLGeneratorPlatoAnalyzeInputFileUtilities.cpp
                      XMLGeneratorPlatoMainOperationFileUtilities.cpp
                      XMLGeneratorPlatoAnalyzeOperationsFileUtilities.cpp
                      XMLGeneratorSierraTFOperationsFileUtilities.cpp
                      XMLGeneratorSierraSDOperationsFileUtilities.cpp
                      XMLGeneratorSierraSDInputDeckUtilities.cpp
                      XMLGeneratorSierraTFInputDeckUtilities.cpp
                      XMLGeneratorLaunchUtilities.cpp
                      XMLGeneratorAnalyzeLoadFunctionInterface.cpp
                      XMLGeneratorAnalyzeLoadTagFunctionInterface.cpp
                      XMLGeneratorAnalyzeEssentialBCFunctionInterface.cpp
                      XMLGeneratorAnalyzeEssentialBCTagFunctionInterface.cpp
                      XMLGeneratorAnalyzeAssemblyFunctionInterface.cpp
                      XMLGeneratorAnalyzeMaterialModelFunctionInterface.cpp
                      XMLGeneratorPlatoMainConstraintValueOperationInterface.cpp
                      XMLGeneratorPlatoMainConstraintGradientOperationInterface.cpp
                      XMLGeneratorLaunchScriptUtilities.cpp
                      XMLGeneratorRunMetadata.cpp
                      XMLGeneratorUtilities.cpp
                      XMLGeneratorParseRun.cpp
                      )
                      
SET(XMLGenerator_HDRS XMLGenerator.hpp 
                      XMLGeneratorProblem.hpp 
                      XMLGeneratorDataStruct.hpp
                      XMLGeneratorDakotaProblem.hpp
                      XMLGeneratorValidInputKeys.hpp
                      XMLGeneratorRandomMetadata.hpp
                      XMLGeneratorOutputMetadata.hpp 
                      XMLGeneratorStagesUtilities.hpp
                      XMLGeneratorParserUtilities.hpp 
                      XMLGeneratorOutputUtilities.hpp
                      XMLGeneratorServiceMetadata.hpp
                      XMLGeneratorProblemUtilities.hpp
                      XMLGeneratorServiceUtilities.hpp
                      XMLGeneratorScenarioMetadata.hpp
                      XMLGeneratorAssemblyMetadata.hpp
                      XMLGeneratorBoundaryMetadata.hpp
                      XMLGeneratorMaterialMetadata.hpp 
                      XMLGeneratorSierraSDUtilities.hpp
                      XMLGeneratorCriterionMetadata.hpp 
                      XMLGeneratorConstraintMetadata.hpp
                      XMLGeneratorSharedDataUtilities.hpp
                      XMLGeneratorPerformersUtilities.hpp
                      XMLGeneratorUncertaintyMetadata.hpp
                      XMLGeneratorDefinesFileUtilities.hpp 
                      XMLGeneratorPlatoAnalyzeUtilities.hpp 
                      XMLGeneratorInterfaceFileUtilities.hpp
                      XMLGeneratorDakotaInterfaceFileUtilities.hpp 
                      XMLGeneratorDakotaDriverInputFileUtilities.hpp 
                      XMLGeneratorGradBasedOptimizerOptions.hpp
                      XMLGeneratorStagesOperationsUtilities.hpp
                      XMLGeneratorOptimizationParametersMetadata.hpp
                      XMLGeneratorParseMethodInputOptionsUtilities.hpp
                      XMLGeneratorPostOptimizationRunFileUtilities.hpp
                      XMLGeneratorAnalyzePhysicsFunctionInterface.hpp
                      XMLGeneratorMaterialFunctionInterface.hpp
                      XMLGeneratorPlatoMainInputFileUtilities.hpp
                      XMLGeneratorPlatoESPInputFileUtilities.hpp
                      XMLGeneratorPlatoESPOperationsFileUtilities.hpp
                      XMLGeneratorPlatoAnalyzeInputFileUtilities.hpp
                      XMLGeneratorPlatoMainOperationFileUtilities.hpp
                      XMLGeneratorPlatoAnalyzeOperationsFileUtilities.hpp
                      XMLGeneratorSierraTFOperationsFileUtilities.hpp
                      XMLGeneratorSierraSDOperationsFileUtilities.hpp
                      XMLGeneratorSierraSDInputDeckUtilities.hpp
                      XMLGeneratorSierraTFInputDeckUtilities.hpp
                      XMLGeneratorLaunchUtilities.hpp
                      # XMLGeneratorAnalyzeCriterionUtilities.hpp
                      XMLGeneratorAnalyzeLoadFunctionInterface.hpp
                      XMLGeneratorAnalyzeLoadTagFunctionInterface.hpp
                      XMLGeneratorAnalyzeEssentialBCFunctionInterface.hpp
                      XMLGeneratorAnalyzeEssentiallBCTagFunctionInterface.hpp
                      XMLGeneratorAnalyzeAssemblyFunctionInterface.hpp
                      # XMLGeneratorAnalyzeMaterialModelFunctionInterface.hpp
                      # XMLGeneratorAnalyzeAppendCriterionFunctionInterface.hpp
                      XMLGeneratorPlatoMainConstraintValueOperationInterface.hpp
                      XMLGeneratorPlatoMainConstraintGradientOperationInterface.hpp
                      XMLGeneratorLaunchScriptUtilities.hpp
                      # XMLGeneratorRandomInterfaceFileUtilities.hpp 
                      # XMLGeneratorPlatoMainFunctionMapTypes.hpp
                      # XMLGeneratorDataStruct.hpp
                      XMLGeneratorUtilities.hpp
                      XMLGeneratorParseOutput.hpp 
                      XMLGeneratorParseMaterial.hpp 
                      XMLGeneratorParseAssembly.hpp 
                      XMLGeneratorParseEssentialBoundaryCondition.hpp 
                      XMLGeneratorParseCriteria.hpp 
                      XMLGeneratorParseObjective.hpp 
                      XMLGeneratorParseBlock.hpp
                      # XMLGeneratorParseMetadata.hpp 
                      XMLGeneratorParseScenario.hpp 
                      XMLGeneratorParseOptimizationParameters.hpp 
                      XMLGeneratorParseServices.hpp 
                      XMLGeneratorParseConstraint.hpp 
                      XMLGeneratorParseUncertainty.hpp
                      XMLGeneratorRunMetadata.hpp
                      XMLGeneratorParseRun.hpp
                      )

add_executable(XMLGenerator ${XMLGenerator_SRCS})
target_link_libraries(XMLGenerator PUBLIC PlatoParser PlatoAnalyze)

install( TARGETS XMLGenerator EXPORT PlatoEngine DESTINATION bin )

if( UNIT_TESTING )
  add_subdirectory(./unittest)
endif()

