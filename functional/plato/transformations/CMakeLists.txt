include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(DIRECTORIES library extension)
set(LINK_LIBS PlatoFunctionalAnalysis
              PlatoFunctionalUtilities
              PlatoFunctionalTPICommon 
              PlatoFunctionalSTKIO)

create_plato_library(PlatoFunctionalTransformations "." "${LINK_LIBS}")

if( UNIT_TESTING )
  set(DIRECTORIES unittest)
  set(LINK_LIBS "")
  create_plato_unittester(PlatoFunctionalTransformations "${DIRECTORIES}" "${LINK_LIBS}")
endif()
