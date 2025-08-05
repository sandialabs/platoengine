include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)


set(FOLDERS library 
            extension)

set(LINK_LIBS PlatoFunctionalFilter
              PlatoFunctionalSTKIO
              PlatoFunctionalKrino 
              PlatoFunctionalMesh
              PlatoFunctionalInputValidation
              PlatoFunctionalServices )

if (CUBIT_ENABLED)
  list(APPEND FOLDERS extension/cubit)
  list(APPEND LINK_LIBS "PlatoCubitInterface")
endif()

create_plato_library(PlatoFunctionalGeometry "${FOLDERS}" "${LINK_LIBS}")

if( UNIT_TESTING )

  set(FOLDERS extension/test_utilities)
  if (CUBIT_ENABLED)
    list(APPEND FOLDERS extension/cubit/test_utilities)
  endif()

  create_plato_library(PlatoFunctionalGeometryTestUtilities "${FOLDERS}" "PlatoFunctionalKrinoTestUtilities;PlatoFunctionalGeometry")

  # Build unit tester
  set(FOLDERS library/unittest 
              extension/unittest)

  set(LINK_LIBS PlatoFunctionalKrinoTestUtilities
                PlatoFunctionalSTKIOTestUtilities
                PlatoFunctionalFilterTestUtilities
                PlatoFunctionalGeometryTestUtilities)
  
  if (CUBIT_ENABLED)
    list(APPEND FOLDERS extension/cubit/unittest)
  endif()
  create_plato_unittester(PlatoFunctionalGeometry "${FOLDERS}" "${LINK_LIBS}")

  set(FOLDERS extension/parallel_unittest)
  if (CUBIT_ENABLED)
    list(APPEND FOLDERS extension/cubit/parallel_unittest)
  endif()
  set(NUM_RANKS_FOR_TEST 2)
  create_plato_parallel_unittester(PlatoFunctionalGeometry "${FOLDERS}" ${NUM_RANKS_FOR_TEST} "${LINK_LIBS}")

endif()
