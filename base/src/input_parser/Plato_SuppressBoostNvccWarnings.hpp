// Boost will define many fusion types' member functions to be __host__ __device__
// which ends up causing a lot of warnings. This code will never be used on a 
// device, so suppress the warnings.
//
// This file should be included at the top of any file using boost and 
// Plato_RestoreNvccWarnings.hpp should be included at the end of the file.
#if defined __CUDACC__
  #pragma nv_diag_suppress 20011
  #pragma nv_diag_suppress 20012
#endif
