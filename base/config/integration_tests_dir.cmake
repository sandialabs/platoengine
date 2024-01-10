macro ( integration_tests_dir )

  if( NOT EXISTS ${INTEGRATION_TESTS_DIR} )
    set(INTEGRATION_TESTS_DIR ${PROJECT_SOURCE_DIR}/../plato-integration-tests )
  endif()
  if( NOT EXISTS ${INTEGRATION_TESTS_DIR} )
    message( WARNING "Cound not find plato-integration-tests directory, integration tests may not function." )
    message( WARNING "The directory can be set with the INTEGRATION_TESTS_DIR cmake variable." )
  else()
    message( STATUS "Integration tests directory: ${INTEGRATION_TESTS_DIR}")
  endif()

endmacro()

macro ( add_integration_test test_dir )
    add_subdirectory(${INTEGRATION_TESTS_DIR}/${test_dir} ${PROJECT_BINARY_DIR}/integration_tests/${test_dir})
endmacro()