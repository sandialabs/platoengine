include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS Boost::headers
              PlatoFunctionalComponents
              PlatoFunctionalUtilities)

create_plato_library(PlatoFunctionalDocumentation "." "${LINK_LIBS}")

set(DOCUMENTATION_MARKDOWN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/markdown")
file(GLOB COPY_FILES ${DOCUMENTATION_MARKDOWN_DIR}/*.md)
file(COPY ${COPY_FILES} DESTINATION ${FUNCTIONAL_DATA_BUILD_PATH})
file(COPY ${COPY_FILES} DESTINATION ${FUNCTIONAL_DATA_INSTALL_PATH})

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalDocumentation unittest PlatoFunctionalInputParserTestUtilities)
endif()

add_subdirectory(./documentation_generator)
