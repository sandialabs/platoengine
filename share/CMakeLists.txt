include( ${CMAKE_SOURCE_DIR}/base/config/require_feature.cmake )
require_feature( LEGACY_BUILD )
install( PROGRAMS plato-cli DESTINATION etc )
install( DIRECTORY esp DESTINATION etc )

if( UNIT_TESTING )
  add_subdirectory(./esp/unittest)
endif()
