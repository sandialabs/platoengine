include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/copy_rol_data.cmake)

set(FUNCTIONAL_ROL_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data/)  

copy_rol_data()

set(LINK_LIBS ROL::rol
              PlatoFunctionalCriteria)

create_plato_library(PlatoFunctionalROL "." "${LINK_LIBS}")

if(UNIT_TESTING)
    create_plato_unittester(PlatoFunctionalROL unittest)
endif()
