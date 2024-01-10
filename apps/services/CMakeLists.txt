
add_compile_definitions(${Dakota_DEFINES})

add_executable(PlatoMain Plato_Main.cpp PlatoApp.cpp PlatoApp.hpp)
target_link_libraries(PlatoMain PUBLIC PlatoInterface PlatoFilters PlatoPenaltyModels PlatoAnalyze PlatoOptimize Boost::serialization)

install( TARGETS PlatoMain EXPORT PlatoEngine DESTINATION bin )

if( ENABLE_PLATO_SERVICES )
  add_executable(PlatoEngineServices Plato_Services.cpp PlatoApp.cpp PlatoApp.hpp)
  target_link_libraries(PlatoEngineServices PlatoInterface PlatoFilters PlatoPenaltyModels PlatoAnalyze)
  install( TARGETS PlatoEngineServices EXPORT PlatoEngine DESTINATION bin )
endif()

if (PLATO_ENABLE_SERVICES_PYTHON)
  find_package(Python3 COMPONENTS Development Interpreter)
  add_library( PlatoServices SHARED ServicesPythonModule.cpp PlatoApp.cpp PlatoApp.hpp)
  target_include_directories(PlatoServices SYSTEM PRIVATE ${Python3_INCLUDE_DIRS} )
  target_link_libraries( PlatoServices PlatoInterface PlatoFilters PlatoPenaltyModels PlatoAnalyze ${Python3_LIBRARIES} Boost::serialization)
  set_target_properties( PlatoServices PROPERTIES PREFIX "" )
  install( TARGETS PlatoServices EXPORT PlatoEngine DESTINATION lib )
endif()


###############################################################################
###############################################################################

add_library(PlatoApp PlatoApp.cpp PlatoApp.hpp)
target_include_directories(PlatoApp PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoApp PUBLIC PlatoAnalyze PlatoInterface PRIVATE PlatoFilters PlatoPenaltyModels)
install(TARGETS PlatoApp EXPORT PlatoEngine DESTINATION lib)
target_include_directories(PlatoApp PUBLIC $<INSTALL_INTERFACE:include>)

if( UNIT_TESTING )
  add_subdirectory(./unittest)
endif()
