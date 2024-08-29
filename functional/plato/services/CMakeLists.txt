include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(CONFIGURED_SERVICES_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/configured_services)
configure_file(${FUNCTIONAL_BASE_DIR}/plato/cmake/PluginDirectoryPath.cpp ${CONFIGURED_SERVICES_SOURCE}/PluginDirectoryPath.cpp)
set(LINK_LIBS PlatoFunctionalUtilities)

create_plato_library(PlatoFunctionalServices ".;${CONFIGURED_SERVICES_SOURCE}" "${LINK_LIBS}")
if( BUILD_WITH_SANITIZER_FLAGS )
  target_compile_definitions(PlatoFunctionalServices PRIVATE BUILD_WITH_SANITIZER_FLAGS=1)
else()
  target_compile_definitions(PlatoFunctionalServices PRIVATE BUILD_WITH_SANITIZER_FLAGS=0)
endif()

if( UNIT_TESTING )
  create_plato_library(PlatoServicesTestSharedLib test_shared_lib "")
  create_plato_unittester(PlatoFunctionalServices unittest)
endif()
