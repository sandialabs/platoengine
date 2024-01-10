
set(SOURCES 
    PSL_AbstractAuthority.cpp
    PSL_Abstract_DenseMatrixBuilder.cpp
    PSL_Abstract_DenseMatrix.cpp
    PSL_Abstract_DenseVectorOperations.cpp
    PSL_Abstract_FixedRadiusNearestNeighborsSearcher.cpp
    PSL_Abstract_GlobalUtilities.cpp
    PSL_Abstract_Mesh.cpp
    PSL_Abstract_MpiWrapper.cpp
    PSL_Abstract_NearestNeighborSearcher.cpp
    PSL_Abstract_OptimizationMesh.cpp
    PSL_Abstract_OverlapSearcher.cpp
    PSL_Abstract_ParallelExchanger.cpp
    PSL_Abstract_ParallelExchanger_Managed.cpp
    PSL_Abstract_ParallelVector.cpp
    PSL_Abstract_ParameterDataBuilder.cpp
    PSL_Abstract_PointCloud.cpp
    PSL_Abstract_PositiveDefiniteLinearSolver.cpp
    PSL_Abstract_SparseMatrixBuilder.cpp
    PSL_Abstract_SparseMatrix.cpp
    )

set(HEADERS 
    PSL_AbstractAuthority.hpp
    PSL_Abstract_DenseMatrixBuilder.hpp
    PSL_Abstract_DenseMatrix.hpp
    PSL_Abstract_DenseVectorOperations.hpp
    PSL_Abstract_FixedRadiusNearestNeighborsSearcher.hpp
    PSL_Abstract_GlobalUtilities.hpp
    PSL_Abstract_Mesh.hpp
    PSL_Abstract_MpiWrapper.hpp
    PSL_Abstract_NearestNeighborSearcher.hpp
    PSL_Abstract_OptimizationMesh.hpp
    PSL_Abstract_OverlapSearcher.hpp
    PSL_Abstract_ParallelExchanger.hpp
    PSL_Abstract_ParallelExchanger_Managed.hpp
    PSL_Abstract_ParallelVector.hpp
    PSL_Abstract_ParameterDataBuilder.hpp
    PSL_Abstract_PointCloud.hpp
    PSL_Abstract_PositiveDefiniteLinearSolver.hpp
    PSL_Abstract_SparseMatrixBuilder.hpp
    PSL_Abstract_SparseMatrix.hpp
    )

add_library(PlatoPSLAbstractInterface ${SOURCES} ${HEADERS})
target_include_directories(PlatoPSLAbstractInterface PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoPSLAbstractInterface PUBLIC PlatoPSLHelper PlatoPSLExample)

install( TARGETS PlatoPSLAbstractInterface EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoPSLAbstractInterface PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

