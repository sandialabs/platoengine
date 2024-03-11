add_subdirectory(./library)

set(SRCS Main.cpp )
set(EXE_NAME plato)
add_executable(${EXE_NAME} ${SRCS})
target_link_libraries(${EXE_NAME} PRIVATE Boost::headers Kokkos::kokkos PlatoFunctional)

install( TARGETS ${EXE_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
