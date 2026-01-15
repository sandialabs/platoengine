include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS Boost::headers
              PlatoFunctionalComponents
              PlatoFunctionalUtilities)

create_plato_library(PlatoFunctionalInputParser "." "${LINK_LIBS}")

# The header boost/spirit/include/qi.hpp results in many uninitialized usage warnings.
target_compile_options(PlatoFunctionalInputParser PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-Wno-maybe-uninitialized>)

if( UNIT_TESTING )

  create_plato_header_library(PlatoFunctionalInputParserTestUtilities "test_utilities" "PlatoFunctionalInputParser")
  create_plato_unittester(PlatoFunctionalInputParser unittest PlatoFunctionalInputParserTestUtilities)
endif()

add_subdirectory(./documentation_generator)
