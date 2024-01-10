/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/
//-------------------------------------------------------------------------
// Filename      : MeshWrapper.hpp
//
// Description   : a MeshWrapper is a wrapper around stk::mesh
//
// Creator       : Brett Clark
//
// Creation Date : 10/10/2018
//
// Owner         : Brett Clark
//-------------------------------------------------------------------------

#ifndef MeshWrapper_____HPP
#define MeshWrapper_____HPP

#include <vector>
#include <map>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/CoordinateSystems.hpp>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/Field.hpp>

#define FIELD_DENSITY 0
#define FIELD_SUPPORT 1

namespace plato
{

class MeshWrapper
{

private:
    std::shared_ptr<stk::mesh::BulkData> mBulkData;
    std::shared_ptr<stk::mesh::MetaData> mMetaData;
    stk::io::StkMeshIoBroker *mIoBroker;
    stk::ParallelMachine *mComm;
    bool mLocallyOwnedBulk;
    bool mLocallyOwnedMeta;
    stk::mesh::Field<double> *mCoordsField;
    std::vector<stk::mesh::Field<double> *> mIsoFields;
    stk::mesh::Field<double> *mSupportStructureField;
    std::vector<stk::mesh::Field<double>*> mNodalFields;
    std::vector<stk::mesh::Field<double>*> mElementFields;
    int mTimeStep;

public:

    // Constructors
    MeshWrapper(stk::ParallelMachine* comm);

    // Destructor
    ~MeshWrapper();
    bool prepare_as_source();
    void initialize();
    bool read_exodus_mesh(std::string &meshfile, std::string &fieldname,
                          std::string &outputFieldsString,
                          int input_file_is_spread, int time_step);
    stk::mesh::MetaData* meta_data() { return mMetaData.get(); }
    stk::mesh::BulkData* bulk_data() { return mBulkData.get(); }
    void get_output_fields(std::string &outputFieldsString);
    void export_my_mesh();
    void get_shared_boundary_nodes(std::set<uint64_t> &shared_boundary_nodes);
    stk::mesh::Entity getStkEntity(const uint64_t &handle) const;
    double getMaxNodalIsoFieldVariable(uint64_t node) const;
    double getFieldValue(const stk::mesh::Entity &aNode, int aFieldType);
    void nodeCoordinates(const stk::mesh::Entity &aNode, double aCoords[3]);
    void setSupportStructureFieldValue(uint64_t nodeLocalId, double value);
    double getSupportStructureFieldValue(uint64_t nodeLocalId);
    void write_exodus_mesh(std::string &meshfile, int concatenate);
};

} // namespace plato

#endif // MeshWrapper_____HPP



