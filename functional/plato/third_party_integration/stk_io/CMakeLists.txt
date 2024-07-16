include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalUtilities STKIO::stk_io PlatoFunctionalTPICommon)
create_plato_library(PlatoFunctionalSTKIO "." "${LINK_LIBS}")
if( BUILD_WITH_SANITIZER_FLAGS )
  target_compile_definitions(PlatoFunctionalSTKIO PRIVATE BUILD_WITH_SANITIZER_FLAGS=1)
else()
  target_compile_definitions(PlatoFunctionalSTKIO PRIVATE BUILD_WITH_SANITIZER_FLAGS=0)
endif()

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalSTKIO unittest)
endif()
