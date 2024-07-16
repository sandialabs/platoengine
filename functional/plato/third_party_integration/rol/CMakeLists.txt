include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

set(LINK_LIBS ROL::rol
              PlatoFunctionalCriteria)

create_plato_library(PlatoFunctionalROL "." "${LINK_LIBS}")

if(UNIT_TESTING)
    create_plato_unittester(PlatoFunctionalROL unittest)
endif()
