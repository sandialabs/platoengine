include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(DIRECTORIES library extension)
set(LINK_LIBS PlatoFunctionalGeometry 
              PlatoFunctionalInputValidation
              PlatoFunctionalROL)

if (SNOPT_ENABLED)
  list(APPEND DIRECTORIES extension/snopt)
  list(APPEND LINK_LIBS PlatoFunctionalSNOPT)
endif()

create_plato_library(PlatoFunctionalProcessManager "${DIRECTORIES}" "${LINK_LIBS}")

if( UNIT_TESTING )
  set(LINK_LIBS GTest::GTest
                PlatoFunctionalProcessManager
                PlatoFunctionalTestUtilities)
  create_plato_library(PlatoFunctionalProcessManagerTestUtilities "extension/test_utilities" "${LINK_LIBS}")

  # Build unit tester
  set(DIRECTORIES library/unittest extension/unittest)
  if (SNOPT_ENABLED)
    list(APPEND DIRECTORIES extension/snopt/unittest)
  endif()
  set(LINK_LIBS PlatoFunctionalGeometryTestUtilities 
                PlatoFunctionalFilterTestUtilities 
                PlatoFunctionalCriteriaTestUtilities
                PlatoFunctionalProcessManagerTestUtilities
                PlatoFunctionalSTKIOTestUtilities )
  create_plato_unittester(PlatoFunctionalProcessManager "${DIRECTORIES}" "${LINK_LIBS}")

  set(NUM_RANKS_FOR_TEST 4)
  create_plato_parallel_unittester(PlatoFunctionalProcessManager "extension/parallel_unittest" ${NUM_RANKS_FOR_TEST} "${LINK_LIBS}")

endif()
