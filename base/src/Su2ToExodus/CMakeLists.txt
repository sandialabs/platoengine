SET(SRCS Su2ToExodus_Main.cpp Su2ToExodus.cpp)
SET(HDRS Su2ToExodus.hpp)

add_executable(Su2ToExodus ${SRCS})
target_link_libraries(Su2ToExodus SEACASExodus::exodus)

install( TARGETS Su2ToExodus EXPORT PlatoEngine DESTINATION bin )

if( UNIT_TESTING )
  add_subdirectory(./unittest)
endif()

