// Boost will define many fusion types' member functions to be __host__ __device__
// which ends up causing a lot of warnings. This code will never be used on a 
// device, so suppress the warnings.
//
// This file should be included at the bottom of any file using boost and 
// Plato_SuppressNvccWarnings.hpp should be included at the top of the file.
#if defined __CUDACC__
  #pragma nv_diag_default 20011
  #pragma nv_diag_default 20012
#endif