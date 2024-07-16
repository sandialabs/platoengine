include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalUtilities STKSearch::stk_search PlatoFunctionalTPICommon)
create_plato_library(PlatoFunctionalSTKSearch "." "${LINK_LIBS}")
if( BUILD_WITH_SANITIZER_FLAGS )
  target_compile_definitions(PlatoFunctionalSTKSearch PRIVATE BUILD_WITH_SANITIZER_FLAGS=1)
else()
  target_compile_definitions(PlatoFunctionalSTKSearch PRIVATE BUILD_WITH_SANITIZER_FLAGS=0)
endif()

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalSTKSearch unittest)
endif()
