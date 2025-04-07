include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)

set(LINK_LIBS PlatoFunctionalTPICommon )
create_plato_header_library(PlatoFunctionalCubit "."  "${LINK_LIBS}")
