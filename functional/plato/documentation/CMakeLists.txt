include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS Boost::headers
              PlatoFunctionalComponents
              PlatoFunctionalUtilities)

create_plato_library(PlatoFunctionalDocumentation "." "${LINK_LIBS}")

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalDocumentation unittest PlatoFunctionalInputParserTestUtilities)
endif()

add_subdirectory(./documentation_generator)
