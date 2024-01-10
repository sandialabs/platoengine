install( PROGRAMS plato-cli DESTINATION etc )
install( DIRECTORY esp DESTINATION etc )

if( UNIT_TESTING )
  add_subdirectory(./esp/unittest)
endif()
