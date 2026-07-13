include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_interface.cmake)
include(${CMAKE_UTIL_DIR}/add_documentation.cmake)

set(DIRECTORIES library
                extension)
set(LINK_LIBS PlatoFunctionalCore
              PlatoFunctionalInputParser 
              PlatoFunctionalLinearAlgebra
              PlatoFunctionalServices
              PlatoFunctionalMesh
              PlatoFunctionalKrino
              PlatoFunctionalInputParser)
create_plato_library(PlatoFunctionalCriteria "${DIRECTORIES}" "${LINK_LIBS}")
add_documentation(${CMAKE_CURRENT_SOURCE_DIR}/library/documentation)

set(HEADERS library/CriterionInterface.hpp
            library/VectorCriterionInterface.hpp)
create_plato_interface(PlatoCriteriaInterface "${HEADERS}"  PlatoAnalysisInterface)

if( UNIT_TESTING )
  # Build test utilities lib
  set(DIRECTORIES library/test_utilities
                  extension/test_utilities)
  create_plato_library(PlatoFunctionalCriteriaTestUtilities "${DIRECTORIES}" PlatoFunctionalCriteria)

  set(DIRECTORIES library/unittest
                  extension/unittest)
  set(LINK_LIBS PlatoFunctionalCriteriaTestUtilities 
                PlatoFunctionalMesh
                PlatoFunctionalSTKIOTestUtilities)
  create_plato_unittester(PlatoFunctionalCriteria "${DIRECTORIES}" "${LINK_LIBS}")

  set(DIRECTORIES library/parallel_unittest)
  set(NUM_RANKS_FOR_TEST 3)
  create_plato_parallel_unittester(PlatoFunctionalCriteria "${DIRECTORIES}" ${NUM_RANKS_FOR_TEST})
endif()
