
/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#include "Kokkos_Core.hpp"
#include <mpi.h>

#include <iostream>

#include "STKExtract.hpp"
using namespace iso;

int main(int argc,  char **argv)
{
  stk::parallel_machine_init(&argc,&argv);
  Kokkos::initialize(argc, argv);

  {
    STKExtract ex(MPI_COMM_WORLD);
    if(ex.create_mesh_apis_stand_alone(argc, argv, "", "", "LSD", "", 1e-5,
                      0.0, 0, 1, 0, 0))
    {
        ex.run_stand_alone();
    }
  }

  Kokkos::finalize();
  stk::parallel_machine_finalize();

  return 0;
}
