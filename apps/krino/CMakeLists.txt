SET(EXE_SRCS PlatoKrinoMain.cpp)

# EXE
add_executable(PlatoKrino ${EXE_SRCS})
target_link_libraries(PlatoKrino PUBLIC PlatoKrinoLib)
install( TARGETS PlatoKrino EXPORT PlatoEngine DESTINATION bin )


