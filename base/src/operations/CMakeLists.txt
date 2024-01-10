set(SOURCES Plato_Filter.cpp
            Plato_CopyField.cpp
            Plato_CopyValue.cpp
            Plato_ChainRule.cpp
            Plato_Roughness.cpp
            Plato_SystemCall.cpp
            Plato_SystemCallOperation.cpp
            Plato_Aggregator.cpp
            Plato_DesignVolume.cpp
            Plato_Reinitialize.cpp
            Plato_EnforceBounds.cpp
            Plato_UpdateProblem.cpp
            Plato_CSMMeshOutput.cpp
            Plato_ComputeVolume.cpp
            Plato_SetUpperBounds.cpp
            Plato_SetLowerBounds.cpp
            Plato_PlatoMainOutput.cpp
            Plato_InitializeField.cpp
            Plato_InitializeValues.cpp
            Plato_WriteGlobalValue.cpp
            Plato_CSMParameterOutput.cpp
            Plato_OperationsUtilities.cpp
            Plato_HarvestDataFromFile.cpp
            Plato_NormalizeObjectiveValue.cpp
            Plato_MeanPlusVarianceMeasure.cpp
            Plato_MeanPlusVarianceGradient.cpp
            Plato_ReciprocateObjectiveValue.cpp
            Plato_NormalizeObjectiveGradient.cpp
            Plato_OutputNodalFieldSharedData.cpp
            Plato_ReciprocateObjectiveGradient.cpp
            Plato_StatisticsOperationsUtilities.cpp
            )
                        
set(HEADERS Plato_Filter.hpp
            Plato_CopyField.hpp
            Plato_CopyValue.hpp
            Plato_ChainRule.hpp
            Plato_Roughness.hpp
            Plato_SystemCall.hpp
            Plato_SystemCallOperation.hpp
            Plato_Aggregator.hpp
            Plato_DesignVolume.hpp
            Plato_Reinitialize.hpp
            Plato_EnforceBounds.hpp
            Plato_UpdateProblem.hpp
            Plato_CSMMeshOutput.hpp
            Plato_ComputeVolume.hpp
            Plato_SetUpperBounds.hpp
            Plato_SetLowerBounds.hpp
            Plato_LocalOperation.hpp
            Plato_PlatoMainOutput.hpp
            Plato_InitializeField.hpp
            Plato_InitializeValues.hpp
            Plato_WriteGlobalValue.hpp
            Plato_CSMParameterOutput.hpp
            Plato_HarvestDataFromFile.hpp
            Plato_OperationsUtilities.hpp
            Plato_NormalizeObjectiveValue.hpp
            Plato_MeanPlusVarianceMeasure.hpp
            Plato_MeanPlusVarianceGradient.hpp
            Plato_OutputNodalFieldSharedData.hpp
            Plato_NormalizeObjectiveGradient.hpp
            Plato_ReciprocateObjectiveGradient.hpp
            Plato_StatisticsOperationsUtilities.hpp
            )
                        
add_library(PlatoOperations ${SOURCES} ${HEADERS} )
target_link_libraries(PlatoOperations PUBLIC PlatoDataLayer PlatoParser PlatoPenaltyModels PRIVATE PlatoFilters PlatoApp)
target_include_directories(PlatoOperations PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

if( ENABLE_ISO )
  target_link_libraries(PlatoOperations PRIVATE IsoExtract)
  target_compile_definitions(PlatoOperations PRIVATE ENABLE_ISO)
endif()

target_link_libraries(PlatoOperations PRIVATE Boost::serialization)

install( TARGETS PlatoOperations EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoOperations PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

if( UNIT_TESTING )
  add_subdirectory(./unittest)
endif()
