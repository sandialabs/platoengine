set(HEADERS Plato_BoundsBase.hpp
            Plato_RestartFileUtilities.hpp
            Plato_OptimizerParser.hpp
            Plato_DriverFactory.hpp
            Plato_CommWrapper.hpp
            Plato_HostBounds.hpp
            Plato_Himmelblau.hpp
            Plato_KokkosTypes.hpp
            Plato_ErrorChecks.hpp
            Plato_GoldsteinPrice.hpp
            Plato_EngineObjective.hpp
            Plato_EngineConstraint.hpp
            Plato_Radius.hpp
            Plato_Rosenbrock.hpp
            Plato_Criterion.hpp
            Plato_DriverInterface.hpp
            Plato_SOParameterStudies.hpp
            Plato_SOParameterStudiesInterface.hpp
            Plato_DakotaFactory.hpp
            Plato_OptimizerFactory.hpp
            Plato_OptimizerInterface.hpp
            Plato_OptimizerUtilities.hpp
            Plato_ROLInterface.hpp
            Plato_StochasticROLInterface.hpp
            Plato_ReducedConstraintROL.hpp
            Plato_ReducedObjectiveROL.hpp
            Plato_ReducedStochasticObjectiveROL.hpp
            Plato_ConstraintSimOptROL.hpp
            Plato_ObjectiveSimOptROL.hpp
            Plato_ProxyComplianceROL.hpp
            Plato_ProxyVolumeROL.hpp
            GradientCheckUtilities.hpp
            )

add_library(PlatoOptimize INTERFACE ${HEADERS} )
target_include_directories(PlatoOptimize INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoOptimize INTERFACE PlatoAnalyze PlatoLinearAlgebra PlatoStochastic)
if(DAKOTADRIVER)
  target_link_libraries(PlatoOptimize INTERFACE PlatoDakotaDriver)
endif()

install( TARGETS PlatoOptimize EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoOptimize INTERFACE $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

if( UNIT_TESTING )
  add_subdirectory(./unittest)
endif()
