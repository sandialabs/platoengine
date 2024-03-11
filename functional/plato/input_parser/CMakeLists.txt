include(${CMAKE_UTIL_DIR}/create_plato_library.cmake)
include(${CMAKE_UTIL_DIR}/create_plato_unittester.cmake)

create_plato_library(PlatoFunctionalInputParser "." "Boost::headers;PlatoFunctionalUtilities")

if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
  # Boost will define many fusion types' member functions to be __host__ __device__
  # which ends up causing a lot of warnings. This code will never be used on a 
  # device, so suppress the warnings.
  message(STATUS "Supressing nvcc warnings for parser")
  target_compile_options(PlatoFunctionalInputParser PUBLIC 
    $<$<COMPILE_LANGUAGE:CXX>:SHELL:-Xcudafe --diag_suppress=20011 -Xcudafe --diag_suppress=20012>) 
endif()

if( UNIT_TESTING )
  create_plato_unittester(PlatoFunctionalInputParser unittest)
endif()
