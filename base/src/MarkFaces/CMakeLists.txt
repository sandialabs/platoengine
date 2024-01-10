SET(SRCS MarkFaces_Main.cpp MarkFaces.cpp)
SET(HDRS MarkFaces.hpp)

add_executable(MarkFaces ${SRCS} ${HDRS})
target_link_libraries(MarkFaces SEACASExodus::exodus)

install( TARGETS MarkFaces EXPORT PlatoEngine DESTINATION bin )

#if( UNIT_TESTING )
#  add_subdirectory(./unittest)
#endif()

