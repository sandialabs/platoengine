SET(SRCS SupportStructureMain.cpp)
SET(EVAL_SRCS SupportStructureEvalMain.cpp)

add_executable(create_support_structure ${SRCS})
add_executable(support_structure_eval ${EVAL_SRCS})
target_link_libraries(create_support_structure PUBLIC SupportStructureLib TeuchosCore::teuchoscore)
target_link_libraries(support_structure_eval PUBLIC STKIO::stk_io STKMesh::stk_mesh_base SEACASIoss::Ioss TeuchosCore::teuchoscore)

install( TARGETS create_support_structure EXPORT PlatoEngine DESTINATION bin )
install( TARGETS support_structure_eval EXPORT PlatoEngine DESTINATION bin )

if( UNIT_TESTING )
#  add_subdirectory(./unittest)
endif()

