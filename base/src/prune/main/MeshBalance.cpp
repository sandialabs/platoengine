/*
 * MeshBalance.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: tzirkle
 */

#include "MeshBalance.hpp"
#include <percept/stk_rebalance/Rebalance.hpp>
#include <percept/stk_rebalance/ZoltanPartition.hpp>

    MeshBalance::MeshBalance()
    {

    }

    void MeshBalance::balance_mesh(MeshManager &tMeshManager)
    {
        int spatial_dimension = 3;

              typedef stk::mesh::Field<double> VectorField;
              const stk::mesh::EntityRank rank_to_rebalance    = stk::topology::ELEMENT_RANK;

              Teuchos::ParameterList emptyList;
              stk::rebalance::Zoltan zoltan_partition(tMeshManager.get_output_bulk_data(), tMeshManager.get_u_communicator(), spatial_dimension, emptyList);

              stk::mesh::MetaData & fem_meta = tMeshManager.get_output_meta_data();

              VectorField & coord_field = fem_meta.declare_field<double>( stk::topology::NODE_RANK,  "coordinates" ) ;
              stk::mesh::Selector selector(fem_meta.universal_part());

              stk::rebalance::Rebalance rb;
              rb.rebalance(tMeshManager.get_output_bulk_data(), selector, &coord_field, NULL, zoltan_partition,rank_to_rebalance);
    }



