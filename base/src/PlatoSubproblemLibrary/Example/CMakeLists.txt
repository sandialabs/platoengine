set(SOURCES 
    PSL_Implementation_CompressedRowSparseMatrix.cpp
    PSL_Implementation_DenseMatrix.cpp
    PSL_Implementation_MeshModular.cpp
    PSL_Implementation_MeshMonolithic.cpp
    PSL_Implementation_MpiWrapper.cpp
    PSL_Interface_BasicDenseVectorOperations.cpp
    PSL_Interface_BasicGlobalUtilities.cpp
    PSL_Interface_ConjugateGradient.cpp
    PSL_Interface_DenseMatrixBuilder.cpp
    PSL_Interface_DenseMatrix.cpp
    PSL_Interface_Kernel_StructParameterDataBuilder.cpp
    PSL_Interface_MeshModular.cpp
    PSL_Interface_MeshMonolithic.cpp
    PSL_Interface_MpiWrapper.cpp
    PSL_Interface_NeuralNetwork_StructParameterDataBuilder.cpp
    PSL_Interface_ParallelExchanger_global.cpp
    PSL_Interface_ParallelExchanger_localAndNonlocal.cpp
    PSL_Interface_ParallelExchanger_ownershipAndGlobal.cpp
    PSL_Interface_ParallelExchanger_ownershipFlag.cpp
    PSL_Interface_ParallelVector.cpp
    PSL_Interface_PointCloud.cpp
    PSL_Interface_SparseMatrixBuilder.cpp
    )

set(HEADERS 
    PSL_Implementation_CompressedRowSparseMatrix.hpp
    PSL_Implementation_DenseMatrix.hpp
    PSL_Implementation_Kernel_StructParameterData.hpp
    PSL_Implementation_MeshModular.hpp
    PSL_Implementation_MeshMonolithic.hpp
    PSL_Implementation_MpiWrapper.hpp
    PSL_Implementation_NeuralNetwork_StructParameterData.hpp
    PSL_Interface_BasicDenseVectorOperations.hpp
    PSL_Interface_BasicGlobalUtilities.hpp
    PSL_Interface_ConjugateGradient.hpp
    PSL_Interface_DenseMatrixBuilder.hpp
    PSL_Interface_DenseMatrix.hpp
    PSL_Interface_Kernel_StructParameterDataBuilder.hpp
    PSL_Interface_MeshModular.hpp
    PSL_Interface_MeshMonolithic.hpp
    PSL_Interface_MpiWrapper.hpp
    PSL_Interface_NeuralNetwork_StructParameterDataBuilder.hpp
    PSL_Interface_ParallelExchanger_global.hpp
    PSL_Interface_ParallelExchanger_localAndNonlocal.hpp
    PSL_Interface_ParallelExchanger_ownershipAndGlobal.hpp
    PSL_Interface_ParallelExchanger_ownershipFlag.hpp
    PSL_Interface_ParallelVector.hpp
    PSL_Interface_PointCloud.hpp
    PSL_Interface_SparseMatrixBuilder.hpp
    )

add_library(PlatoPSLExample ${SOURCES} ${HEADERS} )
target_include_directories(PlatoPSLExample PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(PlatoPSLExample PUBLIC PlatoPSLAbstractInterface PlatoPSLSpatialSearching)

install( TARGETS PlatoPSLExample EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoPSLExample PUBLIC $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)

