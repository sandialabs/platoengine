/*
 * Plato_XTK_Local_Op_Load_From_Hdf5.cpp
 *
 *  Created on: Jul 26, 2019
 *      Author: doble
 */

#include "Plato_XTK_Local_Op_Load_From_Hdf5.hpp"
#include <HDF5_Tools.hpp>
#include <cl_Matrix.hpp>

#include "Plato_Parser.hpp"

namespace PlatoXTK
{
/******************************************************************************/
Load_From_HDF5::Load_From_HDF5(Plato::XTKApplication* aXTKApp,
                               Plato::InputData &     aNode):
                                  Local_Op(aXTKApp)
{
    mHdfFileName = Plato::Get::String(aNode,"FileName");

    for(auto tInputNode : aNode.getByName<Plato::InputData>("Input"))
    {
        std::string tName = Plato::Get::String(tInputNode, "SharedDataName");
        std::string tLayoutStr= Plato::Get::String(tInputNode,"Layout");

        Plato::data::layout_t  tLayout = mXTKApp->string_to_layout(tLayoutStr);

        // register shared data in application
        mXTKApp->registerSharedData(tName,tLayout);

        // add to operation member data
        mSharedDataName.push_back(tName);
        mFieldLayout.push_back(tLayout);
    }

}
/******************************************************************************/
Load_From_HDF5::~Load_From_HDF5()
{

}
/******************************************************************************/
void Load_From_HDF5::Load_From_HDF5::operator()()
{
    // create file
    hid_t tFileId = moris::open_hdf5_file( mHdfFileName );

    // iterate through shared data and add to file
    for(moris::uint i =0 ; i < mSharedDataName.size(); i++)
    {
        std::vector<double> tVector;

        // load from hdf5
        herr_t tErrCode;
        moris::load_vector_from_hdf5_file(tFileId, "data"+ std::to_string(i),tVector,tErrCode);

        // get the shared data
        Plato::XTKSharedData* tSharedData = nullptr;
        mXTKApp->getSharedData(mSharedDataName[i],mFieldLayout[i],tSharedData);

        // resize the shared data
        tSharedData->resize(tVector.size());

        // add the data from hdf5 file to shared data
        tSharedData->setData(tVector);
    }

    // close the file
     moris::close_hdf5_file( tFileId );
}
/******************************************************************************/
std::string Load_From_HDF5::Load_From_HDF5::op_name() const
{
    return "Load_From_HDF5";
}
/******************************************************************************/

}

