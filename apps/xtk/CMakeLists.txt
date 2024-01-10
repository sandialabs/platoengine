message( STATUS "Compiling with XTK at path ${XTK_INSTALL}\n")

find_package( moris COMPONENTS XTK INT MTK REQUIRED HINTS ${XTK_INSTALL} $ENV{XTK_INSTALL})

message(STATUS "Moris libs: ${ALL_MRS_LIBS}")

# find_package(PETSc)
set(moris_DEFINITIONS "-DF77ADD_;-DMORIS_PERFORM_CHECK;-DUSE_XTK;-DXTK_USE_MORIS;-DMORIS_USE_32BIT;-DMORIS_USE_ARMA;-DARMA_NO_DEBUG;-DDEBUG")

add_definitions(${moris_DEFINITIONS})

###############################################################################
# Sources:
###############################################################################
set(XTKMAIN_SOURCES  Plato_XTK_Application.cpp 
                     Plato_XTK_Local_Op_Compute_Bounds.cpp
					 Plato_XTK_Local_Op_Generate_Model.cpp
					 Plato_XTK_Local_Op_Sensitivity.cpp
					 Plato_XTK_Local_Op_Load_From_Hdf5.cpp
					 Plato_XTK_Local_Op_Save_To_Hdf5.cpp)
				     
set(XTKMAIN_HEADERS  Plato_XTK_Application.hpp
					 Plato_XTK_SharedData.hpp
					 Plato_XTK_Local_Op.hpp
                     Plato_XTK_Local_Op_Compute_Bounds.hpp
					 Plato_XTK_Local_Op_Generate_Model.hpp
					 Plato_XTK_Local_Op_Sensitivity.hpp
					 Plato_XTK_Local_Op_Load_From_Hdf5.hpp
					 Plato_XTK_Local_Op_Save_To_Hdf5.hpp
					 Plato_XTK_Local_Op_Generate_Model.hpp
					 )

include_directories(SYSTEM ${XTK_INCLUDES})

# actual target:
set(REQUIRED_MORIS_LIBS 
	XTK-lib
	INT-lib
	MTK-lib
  )
# dependency seems missing in moris, add here to resolve possible link errors
target_link_libraries(DLA-lib INTERFACE ML::ml Epetra::epetra)

set(XTKMAIN_LIBS
  PlatoInterface
  PlatoOperations
  PlatoAnalyze
	${REQUIRED_MORIS_LIBS}
  Amesos::all_libs
  Anasazi::all_libs
  Belos::all_libs
  ML::ml
  Ifpack::all_libs
  Epetra::epetra
  STKIO::stk_io
  STKMesh::stk_mesh_base
  SEACASIoss::Ioss
  SEACASNemesis::nemesis
  SEACASExodus::exodus
  )

# MPMD executable
add_executable(XTKMain ${XTKMAIN_SOURCES} XTK_Main.cpp)
target_link_libraries(XTKMain ${XTKMAIN_LIBS})

add_library(PlatoXTK ${XTKMAIN_SOURCES} ${XTKMAIN_HEADERS} )
target_link_libraries(PlatoXTK ${XTKMAIN_LIBS})
target_include_directories(PlatoXTK PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

add_executable(XTK ${XTKMAIN_SOURCES} XTK_Main_Standalone.cpp)
target_link_libraries(XTK ${XTKMAIN_LIBS})

install( TARGETS XTKMain EXPORT PlatoEngine DESTINATION bin )


if( UNIT_TESTING )
  add_subdirectory(./unittest)
endif()
