SET(LIB_SRCS PlatoKrinoApp.cpp 
         PlatoKrinoParse.cpp
         PlatoKrinoUtilities.cpp
         PlatoKrinoInterface.cpp)

SET(EXE_SRCS PlatoKrinoMain.cpp)

SET(HDRS PlatoKrinoEnums.hpp
         PlatoKrinoInterface.hpp
         PlatoKrinoApp.hpp
         PlatoKrinoParse.hpp
         PlatoKrinoUtilities.hpp)

# LIBRARY
add_library(PlatoKrinoLib SHARED ${LIB_SRCS} ${HDRS})
target_link_libraries(PlatoKrinoLib PUBLIC PlatoInterface Krino::krino_lib)
target_include_directories(PlatoKrinoLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
install(TARGETS PlatoKrinoLib EXPORT PlatoEngine DESTINATION lib)

# EXE
add_executable(PlatoKrino ${EXE_SRCS})
target_link_libraries(PlatoKrino PUBLIC PlatoKrinoLib)
install( TARGETS PlatoKrino EXPORT PlatoEngine DESTINATION bin )

if( UNIT_TESTING )
  add_subdirectory(./unit_tests)
endif()


