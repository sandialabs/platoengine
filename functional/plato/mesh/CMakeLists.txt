include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS PlatoFunctionalUtilities
              PlatoFunctionalAnalysis
              PlatoFunctionalSTKIO
              PlatoFunctionalKrino)

create_plato_library(PlatoFunctionalMesh "." "${LINK_LIBS}")

if( UNIT_TESTING )

  set(LINK_LIBS PlatoFunctionalMesh
              PlatoFunctionalSTKIOTestUtilities 
              PlatoFunctionalLinearAlgebra)
  create_plato_library(PlatoFunctionalMeshTestUtilities "test_utilities" "${LINK_LIBS}")

  set(LINK_LIBS PlatoFunctionalMeshTestUtilities)
  create_plato_unittester(PlatoFunctionalMesh unittest "${LINK_LIBS}")

  set(NUM_RANKS_FOR_TEST 4)
  create_plato_parallel_unittester(PlatoFunctionalMesh "parallel_unittest" ${NUM_RANKS_FOR_TEST} "${LINK_LIBS}")
endif()
