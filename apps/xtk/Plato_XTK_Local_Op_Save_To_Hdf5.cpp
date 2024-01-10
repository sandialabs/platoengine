/*
 * Plato_XTK_Local_Op_Save_To_HDF5.cpp
 *
 *  Created on: Jul 26, 2019
 *      Author: doble
 */

#include "Plato_XTK_Local_Op_Save_To_Hdf5.hpp"
#include <HDF5_Tools.hpp>
#include <cl_Matrix.hpp>

#include "Plato_Parser.hpp"

namespace PlatoXTK
{
/******************************************************************************/
Save_To_HDF5::Save_To_HDF5(Plato::XTKApplication* aXTKApp,
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
Save_To_HDF5::~Save_To_HDF5()
{

}
/******************************************************************************/
void Save_To_HDF5::Save_To_HDF5::operator()()
{
    hid_t tFileId = moris::create_hdf5_file( mHdfFileName,true );

    // iterate through shared data and add to file
    for(moris::uint i =0 ; i < mSharedDataName.size(); i++)
    {
        // get the shared data
        Plato::XTKSharedData* tSharedData = nullptr;
        mXTKApp->getSharedData(mSharedDataName[i],mFieldLayout[i],tSharedData);

        // allocate vector for data
        std::vector<double> tVector(tSharedData->size());
        tSharedData->getData(tVector);

        // save to hdf5
        herr_t tErrCode;
        moris::save_vector_to_hdf5_file(tFileId, mSharedDataName[i], tVector,tErrCode);
    }

    // close the file
     moris::close_hdf5_file( tFileId );
}
/******************************************************************************/
std::string Save_To_HDF5::Save_To_HDF5::op_name() const
{
    return "Save_To_HDF5";
}
/******************************************************************************/

}

