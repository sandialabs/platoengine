include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_interface.cmake)

set(DIRECTORIES library
                extension)
set(LINK_LIBS PlatoFunctionalCore
              PlatoFunctionalLinearAlgebra
              PlatoFunctionalServices
              PlatoFunctionalMesh)
create_plato_library(PlatoFunctionalCriteria "${DIRECTORIES}" "${LINK_LIBS}")
create_plato_interface(PlatoCriteriaInterface library/CriterionInterface.hpp  PlatoAnalysisInterface)

if( UNIT_TESTING )
  # Build test utilities lib
  set(DIRECTORIES library/test_utilities)
  create_plato_library(PlatoFunctionalCriteriaTestUtilities "${DIRECTORIES}" PlatoFunctionalCriteria)

  set(DIRECTORIES library/unittest
                  extension/unittest)
  set(LINK_LIBS PlatoFunctionalCriteriaTestUtilities 
                PlatoFunctionalSTKIOTestUtilities)
  create_plato_unittester(PlatoFunctionalCriteria "${DIRECTORIES}" "${LINK_LIBS}")

  set(DIRECTORIES library/parallel_unittest)
  set(NUM_RANKS_FOR_TEST 3)
  create_plato_parallel_unittester(PlatoFunctionalCriteria "${DIRECTORIES}" ${NUM_RANKS_FOR_TEST})
endif()
