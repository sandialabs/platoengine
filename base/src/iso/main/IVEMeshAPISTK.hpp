/*
//@HEADER
// *************************************************************************
//   Plato Engine v.1.0: Copyright 2018, National Technology & Engineering
//                    Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Sandia Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact the Plato team (plato3D-help@sandia.gov)
//
// *************************************************************************
//@HEADER
*/

//-------------------------------------------------------------------------
// Filename      : IVEMeshAPISTK.hpp
//
// Description   : a IVEMeshAPISTK is an implementation of IVEMeshAPI for STK  mesh entities
//
// Creator       : Brett Clark
//
// Creation Date : 12/8/2014
//
// Owner         : Brett Clark
//-------------------------------------------------------------------------

#ifndef IVEMeshAPISTK_____HPP
#define IVEMeshAPISTK_____HPP

#include "IVEMeshAPI.hpp"
#include "IsoVector.hpp"
#include <vector>
#include <map>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/CoordinateSystems.hpp>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/Field.hpp>

struct NewNodeData
{
  stk::mesh::EntityId m_node1;
  stk::mesh::EntityId m_node2;
  stk::mesh::Entity m_new_node;
  std::vector<int> m_sharing_procs;
  std::vector<std::pair<int, stk::mesh::EntityId> > m_id_proc_pairs_from_all_procs;
  stk::mesh::BulkData *m_old_mesh;
  stk::mesh::BulkData *m_new_mesh;
  bool m_id_procs_pairs_have_been_sorted;

  NewNodeData(stk::mesh::BulkData& oldBulkData, 
              stk::mesh::BulkData& newBulkData, 
              stk::mesh::EntityId nodeA, 
              stk::mesh::EntityId nodeB, 
              [[maybe_unused]] stk::mesh::Entity **entity_place_holder=NULL)
              : m_node1(0), 
                m_node2(0), 
                m_sharing_procs(), 
                m_id_proc_pairs_from_all_procs(), 
                m_old_mesh(&oldBulkData),
                m_new_mesh(&newBulkData),
                m_id_procs_pairs_have_been_sorted(false) 
  {
    // If this represents a node that is inbetween two other nodes on an edge...
    if(nodeA != nodeB)
    {
      if ( nodeA < nodeB )
      {
        m_node1 = nodeA;
        m_node2 = nodeB;
      }
      else
      {
        m_node1 = nodeB;
        m_node2 = nodeA;
      }
    }
    // If this represents a node that will be a duplicate of an existing node...
    else
    {
      m_node1 = m_node2 = nodeA;
    }
  }
  
  void add_proc_id_pair(int proc_id, stk::mesh::EntityId id)
  {
    m_id_proc_pairs_from_all_procs.push_back(std::make_pair(proc_id, id));
  }

  void calculate_sharing_procs()
  {
    if(this->node1() != this->node2())
    {
      std::vector<int> sharingProcsA;
      stk::mesh::EntityKey keyA(stk::topology::NODE_RANK, m_node1);
      m_old_mesh->comm_shared_procs(keyA, sharingProcsA);

      std::vector<int> sharingProcsB;
      stk::mesh::EntityKey keyB(stk::topology::NODE_RANK, m_node2);
      m_old_mesh->comm_shared_procs(keyB, sharingProcsB);

      std::sort(sharingProcsA.begin(), sharingProcsA.end());
      std::sort(sharingProcsB.begin(), sharingProcsB.end());

      std::set_intersection(sharingProcsA.begin(),sharingProcsA.end(),sharingProcsB.begin(),sharingProcsB.end(),std::back_inserter(m_sharing_procs));
    }
    else
    {
      stk::mesh::EntityKey keyA(stk::topology::NODE_RANK, m_node1);
      m_old_mesh->comm_shared_procs(keyA, m_sharing_procs);
    }
  }

  size_t num_sharing_procs() const
  {
    return m_sharing_procs.size();
  }

  int sharing_proc(int index) const
  {
    return m_sharing_procs[index];
  }
  
  stk::mesh::EntityId suggested_node_id() const
  {
    return m_id_proc_pairs_from_all_procs[0].second;
  }

  stk::mesh::EntityId node1() const
  {
    return m_node1;
  }

  stk::mesh::EntityId node2() const
  {
    return m_node2;
  }

  void sort_id_proc_pairs()
  {
    m_id_procs_pairs_have_been_sorted = true;
    std::sort(m_id_proc_pairs_from_all_procs.begin(), m_id_proc_pairs_from_all_procs.end());
  }

  stk::mesh::EntityId get_id_for_node() const
  {
    return m_id_proc_pairs_from_all_procs[0].second;
  }

  void set_node_entity(IsoVector &pos,
                       stk::mesh::Field<double> *field)
  {
    this->sort_id_proc_pairs();
    stk::mesh::EntityId id_for_node = this->get_id_for_node();
    m_new_node = m_new_mesh->declare_node(id_for_node);
    double* node_coords = stk::mesh::field_data(*field, m_new_node);
    node_coords[0] = pos.x();
    node_coords[1] = pos.y();
    node_coords[2] = pos.z();
    for (size_t i=0;i<m_id_proc_pairs_from_all_procs.size();++i)
    {
      if ( m_id_proc_pairs_from_all_procs[i].first != m_new_mesh->parallel_rank() )
      {
        m_new_mesh->add_node_sharing(m_new_node, m_id_proc_pairs_from_all_procs[i].first);
      }
    }
  }

  bool operator==(const NewNodeData& otherNodeData) const
  {
    if(this->node1() != this->node2())
    {
      if ( this->node1() == otherNodeData.node1() &&
           this->node2() == otherNodeData.node2() )
      {
        return true;
      }
    }
    else
    {
      if ( this->node1() == otherNodeData.node1())
        return true;
    }
    return false;
  }
};

struct NodeDataSorter
{
  bool operator()(const NewNodeData& data1, const NewNodeData& data2) const
  {
    if(data1.node1() != data1.node2()) // if this is between two nodes on an edge
    {
      if ( data1.node1() != data2.node1() )
      {
        return data1.node1() < data2.node1();
      }
      else
      {
        return data1.node2() < data2.node2();
      }
    }
    else // this is just a new node being created with the same location as an existing node
    {
      return data1.node1() < data2.node1();
    }
  }
};

class IVEMeshAPISTK : public IVEMeshAPI
{
  
private:
  std::shared_ptr<stk::mesh::BulkData> mBulkData;
  std::shared_ptr<stk::mesh::MetaData> mMetaData;
  stk::io::StkMeshIoBroker *mIoBroker;
  stk::mesh::Part *mFixedTriPart;
  stk::mesh::Part *mOptimizedTriPart;
  stk::ParallelMachine *mComm;
  bool mLocallyOwnedBulk;
  bool mLocallyOwnedMeta;
  stk::mesh::Field<double> *mCoordsField;
  stk::mesh::Field<double> *mIsoField;
  std::vector<stk::mesh::Field<double>*> mNodalFields;
  std::vector<stk::mesh::Field<double>*> mElementFields;
  std::vector<std::string> mFixedBlocks;
  stk::mesh::Field<double> *mFixedTriMap, *mOptimizedTriMap;
  std::vector<stk::mesh::EntityId> mAvailableElemIds;
  std::vector<stk::mesh::EntityId> mAvailableNodeIds;
  size_t mNextAvailableNodeIndex;
  size_t mNextAvailableElemIndex;
  int mTimeStep;

public:

  // Constructors
  IVEMeshAPISTK(stk::ParallelMachine* comm,
                stk::mesh::BulkData* bulk_data,
                stk::mesh::MetaData* meta_data,
                std::string fieldname);
  IVEMeshAPISTK(stk::ParallelMachine* comm);

  // Destructor
  ~IVEMeshAPISTK() override;

  // functions from base class
  void transfer_output_fields(IVEHandle n1, IVEHandle n2, IVEHandle new_node,
                   double mu, IVEMeshAPI *output_mesh_api) override;
  void copy_node_output_fields(IVEHandle n1, IVEHandle new_node, IVEMeshAPI *output_mesh_api) override;
  void copy_element_output_fields(IVEHandle e1, IVEHandle e2, IVEMeshAPI *output_mesh_api) override;
  IVEHandle new_node( IsoVector &coordinates ) override;
  void get_fixed_block_nodes(std::vector<IVEHandle> &fixed_block_nodes) override;
  IVEHandle get_new_node_id() override;
  IVEHandle new_tri( IVEHandle n1, IVEHandle n2, IVEHandle n3, bool is_fixed, IVEHandle source_elem ) override;
  int element_nodes(IVEHandle elem, IVEHandle nodes[8]) const override;
  void hex_nodes(IVEHandle hex, IVEHandle nodes[8]) const override;
  void tet_nodes(IVEHandle tet, IVEHandle nodes[4]) const override;
  void hex_quad_nodes(IVEHandle hex, int index, IVEHandle nodes[4]) const override;
  IVEHandle get_connected_hex(IVEHandle hex, IVEHandle n1, IVEHandle n2, IVEHandle n3, IVEHandle n4) const override;
  IVEHandle get_connected_tet(IVEHandle tet, IVEHandle n1, IVEHandle n2, IVEHandle n3) const override;
  IsoVector node_coordinates( IVEHandle node ) const override;
  double get_nodal_iso_field_variable(IVEHandle node) const override;
  void store_tri_to_tet_map_entry(const IVEHandle &tri, const IVEHandle &tet) override;
  void store_tet_to_tri_map_entry(const IVEHandle &tet, const IVEHandle &tri) override;
  void get_shared_boundary_nodes(std::set<IVEHandle> &shared_boundary_nodes) override;
  void get_attached_elements(const std::set<IVEHandle> &nodes,
                                     std::vector<IVEHandle> &attached_elements) override;
  void batch_create_edge_boundary_nodes(std::vector<BoundaryNodeInfo> &boundary_info,
                                          IVEMeshAPI *existing_mesh) override;
  void batch_create_duplicate_nodes(std::vector<DuplicateNodeInfo> &dup_node_infos, 
                                            IVEMeshAPI *existing_mesh) override;
  void print_boundary_node_info(std::vector<BoundaryNodeInfo> &bni) override;
  void reserve_new_node_ids(uint64_t num_requested) override;
  void reserve_new_tri_ids(uint64_t num_requested) override;
  void calculate_average_edge_length_and_bbox(const std::vector<IVEHandle> &elem_list,
          double &minx, double &miny, double &minz, double &maxx, double &maxy, double &maxz, double &ave_length) override;
  void set_min_node_id(BoundaryNodeInfo &bni, const IVEHandle &n) override;
  void set_max_node_id(BoundaryNodeInfo &bni, const IVEHandle &n) override;
  void set_existing_node_id(DuplicateNodeInfo &dni, const IVEHandle &n) override;
  
  // functions local to this derived class
  void get_element_buckets(stk::mesh::Selector& sel,
       stk::mesh::BucketVector& buckets);
  void set_fixed_block_ids(std::string fixed_block_string);
  bool prepare_as_source();
  bool prepare_as_destination();
  void initialize();
  void export_my_mesh();
  stk::mesh::EntityId get_next_entity_id(stk::topology::rank_t rank);
  void prepare_to_create_tris();
  void get_output_fields(std::string &outputFieldsString);
  void prepare_field_data(std::string &outputFieldsString, IVEMeshAPISTK *outAPI);
  stk::mesh::MetaData* meta_data() { return mMetaData.get(); }
  stk::mesh::BulkData* bulk_data() { return mBulkData.get(); }
  bool read_exodus_mesh(std::string &meshfile, std::string &fieldname, 
                        std::string &outputFieldsString,
                        int input_file_is_spread, int time_step);
  void write_exodus_mesh(std::string &meshfile, int output_method, int iso_only);
  void set_comm(stk::ParallelMachine* comm) { mComm = comm; }
  stk::ParallelMachine* get_comm() { return mComm; }
  void set_bulk_data_ptr(std::shared_ptr<stk::mesh::BulkData> bp) { mBulkData = bp; }
  void set_meta_data_ptr(std::shared_ptr<stk::mesh::MetaData> mp) { mMetaData = mp; }
  IVEHandle get_handle(const stk::mesh::Entity &entity) const;
  stk::mesh::Entity get_stk_entity(const IVEHandle &handle) const;
  int time_step() { return mTimeStep; };
  void time_step(int val) { mTimeStep = val; };
  void add_nodal_field(std::string &name);
  void add_element_field(std::string &name);
  stk::mesh::Field<double> *get_nodal_field(std::string &name);
  stk::mesh::Field<double> *get_element_field(std::string &name);
};

#endif // IVEMeshAPISTK_____HPP



