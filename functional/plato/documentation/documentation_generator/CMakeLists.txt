include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)
include(${CMAKE_UTIL_DIR}/add_documentation.cmake)

create_plato_library(PlatoFunctionalDocumentationGenerator "library" PlatoFunctionalDocumentation)

if( UNIT_TESTING )
  add_documentation("${CMAKE_CURRENT_SOURCE_DIR}/library/unittest/data/")
  create_plato_unittester(PlatoFunctionalDocumentationGenerator "library/unittest")
endif()

set(SRCS DocumentationGeneratorMain.cpp )
set(EXE_NAME latex_documentation_generator)
add_executable(${EXE_NAME} ${SRCS})
target_link_libraries(${EXE_NAME} PRIVATE Boost::headers PlatoFunctional PlatoFunctionalDocumentationGenerator)

install( TARGETS ${EXE_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
