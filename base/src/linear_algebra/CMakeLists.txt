set(HEADERS Plato_LinearAlgebra.hpp
            Plato_MultiVector.hpp
            Plato_StandardMultiVector.hpp
            Plato_StandardVector.hpp
            Plato_Vector.hpp
            Plato_DistributedVector.hpp
            Plato_EpetraSerialDenseVector.hpp
            Plato_SerialEpetraVectorROL.hpp
            Plato_SerialVectorROL.hpp
            Plato_DistributedVectorROL.hpp)

add_library(PlatoLinearAlgebra INTERFACE ${HEADERS} )
target_include_directories(PlatoLinearAlgebra INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

install( TARGETS PlatoLinearAlgebra EXPORT PlatoEngine
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)
target_include_directories(PlatoLinearAlgebra INTERFACE $<INSTALL_INTERFACE:include>)
install( FILES ${HEADERS} DESTINATION include)
