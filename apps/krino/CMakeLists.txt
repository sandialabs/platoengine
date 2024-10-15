SET(EXE_SRCS PlatoKrinoMain.cpp
             PlatoKrinoApp.cpp
             PlatoKrinoAppUtils.cpp)

# EXE
add_executable(PlatoKrino ${EXE_SRCS})
target_link_libraries(PlatoKrino PUBLIC PlatoInterface PlatoFunctionalKrinoLib)
install( TARGETS PlatoKrino EXPORT PlatoEngine DESTINATION bin )


