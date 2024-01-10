###############################################################################
# Sources:
###############################################################################
SET(SRCS UnitMain.cpp
         Plato_StkMeshUtils.cpp
         Plato_PlatoProxyApp.cpp
         Plato_PhysicsProxyApp.cpp
         Plato_StructuralTopologyOptimizationProxyGoldResults.cpp
         Plato_Test_Proxy.cpp
         Plato_Test_SimpleRocket.cpp
         Plato_Test_InputData.cpp
         Plato_Test_LinearAlgebra.cpp
         Plato_Test_AugmentedLagrangian.cpp
         Plato_Test_Bounds.cpp
         Plato_Test_SimpleTopOpt.cpp
         Plato_Test_OptimizationTestFunctions.cpp
         Plato_Test_UniqueCounter.cpp
         Plato_Test_SimpleRocketOptimization.cpp
         Plato_Test_LocalStatisticsOperations.cpp
         Plato_Test_WriteParameterStudyData.cpp
         Plato_Test_FreeFunctions.cpp
         Plato_Test_ROL.cpp
         Plato_Test_TimersTree.cpp
         PSL_Test_Triangle.cpp  
         PSL_Test_OverhangFilter.cpp  
         PSL_Test_KernelThenHeavisideFilter.cpp  
         PSL_Test_Standardization.cpp
         PSL_Test_Random.cpp
         PSL_Test_ParameterData.cpp
         PSL_Test_ParallelExchanger.cpp
         PSL_Test_NearestNeighborSearches.cpp
         PSL_Test_MpiWrapperInterface.cpp
         PSL_Test_MpiWrapperImplementation.cpp
         PSL_Test_MeshScaleAgent.cpp
         PSL_Test_Mesh.cpp
         PSL_Test_KernelFilter.cpp
         PSL_Test_GradientCheck.cpp
         PSL_Test_GlobalUtilities.cpp
         PSL_Test_FreeHelpers.cpp
         PSL_Test_FixedRadiusNearestNeighborsSearches.cpp
         PSL_Test_DenseMatrixImplementation.cpp
         PSL_Test_DenseMatrixBuilder.cpp
         PSL_Test_ConjugateGradient.cpp
         PSL_Test_CompressedRowSparseMatrixImplementation.cpp
         PSL_Test_CompressedRowSparseMatrixBuilder.cpp
         PSL_Test_BoundingBoxSearches.cpp
         PSL_Test_BoundingBoxMortonHierarchy.cpp
         PSL_Test_BoundingBox.cpp
         PSL_Test_BasicDenseVectorOperations.cpp
         PSL_Test_AbstractAuthority.cpp
         PSL_Test_Point.cpp
         PSL_Test_Vector.cpp
         PSL_Test_OrthogonalGridUtilities.cpp
         PSL_Test_RegularHex8.cpp
         )

if( AMFILTER_ENABLED )
  list(APPEND SRCS PSL_Test_TetMeshUtilities.cpp PSL_Test_AMFilterUtilities.cpp)
endif()

IF( DAKOTADRIVER )
  list(APPEND SRCS Plato_Test_PlatoDakotaDriver.cpp)
  add_compile_definitions(${Dakota_DEFINES})
ENDIF()

###############################################################################
# Headers:
###############################################################################
SET(HDRS Plato_StructuralTopologyOptimizationProxyGoldResults.hpp
         ParseUnitTestStrings.hpp
         Plato_UnitTestUtils.hpp
         Plato_StkMeshUtils.hpp
         )

# Copy Input files from source to binary dir
SET( testFiles "rol_unit_test_inputs.xml")
Plato_add_test_files("${testFiles}")


add_executable(PlatoMainUnitTester ${SRCS})
target_link_libraries(PlatoMainUnitTester PlatoApp PlatoPSLFilter PlatoPSLAbstractInterface PlatoPSLHelper PlatoProxy PlatoOptimize ${NLOPT_LIBRARY} GTest::GTest ${PYTHON_LIBRARY} ${PYTHON_LINK_LIBS})
if( DAKOTADRIVER )
  target_include_directories(PlatoMainUnitTester PUBLIC ${Dakota_INCLUDE_DIRS})
  target_link_libraries(PlatoMainUnitTester ${Dakota_LIBRARIES} ${Boost_LIBRARIES})
endif()

add_test(NAME PlatoMainUnitTester COMMAND PlatoMainUnitTester)
set_property(TEST PlatoMainUnitTester PROPERTY LABELS "large")

install( TARGETS PlatoMainUnitTester EXPORT PlatoEngine DESTINATION bin )
