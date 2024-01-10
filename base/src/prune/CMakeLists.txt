SET(SRCS PruneAndRefineMain.cpp)

add_executable(prune_and_refine ${SRCS})

target_link_libraries(prune_and_refine PUBLIC Prune)

install( TARGETS prune_and_refine EXPORT PlatoEngine DESTINATION bin )


