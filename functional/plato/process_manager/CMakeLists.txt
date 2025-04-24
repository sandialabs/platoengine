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
  # Build unit tester
  set(DIRECTORIES library/unittest extension/unittest)
  if (SNOPT_ENABLED)
    list(APPEND DIRECTORIES extension/snopt/unittest)
  else()
    # Tests to check that errors are issued if snopt is missing, but requested in input
    list(APPEND DIRECTORIES extension/snopt/unittest_missing_snopt)
  endif()
  create_plato_unittester(PlatoFunctionalProcessManager "${DIRECTORIES}" PlatoFunctionalGeometryTestUtilities)
endif()
