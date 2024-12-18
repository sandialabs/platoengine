include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

create_plato_library(PlatoFunctionalDocumentationGenerator "library" PlatoFunctionalUtilities)

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalDocumentationGenerator "library/unittest")
endif()

set(SRCS DocumentationGeneratorMain.cpp )
set(EXE_NAME latex_documentation_generator)
add_executable(${EXE_NAME} ${SRCS})
target_link_libraries(${EXE_NAME} PRIVATE Boost::headers PlatoFunctional PlatoFunctionalDocumentationGenerator)

install( TARGETS ${EXE_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
