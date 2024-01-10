/*
 * Plato_XTK_Local_Op_Decompose.cpp
 *
 *  Created on: June 1, 2021
 *      Author: doble
 */

#include "Plato_XTK_Local_Op_Compute_Bounds.hpp"

#include "Plato_Parser.hpp"
#include "Plato_OperationsUtilities.hpp"
#include <cl_MTK_Mesh_Manager.hpp>

namespace PlatoXTK
{
/******************************************************************************/
Compute_Bounds::Compute_Bounds(Plato::XTKApplication* aXTKApp,
                               Plato::InputData &     aNode):
                               Local_Op(aXTKApp)
{
    Plato::InputData tOutputNode = Plato::Get::InputData(aNode, "Output");
    mOutputSharedDataName = Plato::Get::String(tOutputNode,"SharedDataName");
    mOutputLayout         = Plato::getLayout(tOutputNode, Plato::data::layout_t::SCALAR);
    mOutputSize           = Plato::Get::Int(tOutputNode,"Size");

    if(mOutputLayout == Plato::data::SCALAR_FIELD)
    {
        mOutputSize = mXTKApp->mCurrentBGMTK->get_interpolation_mesh(0)->get_num_entities(EntityRank::NODE);
    }

    Plato::InputData tOptionNode = Plato::Get::InputData(aNode, "Options");
    
    mValue = Plato::Get::Double(tOptionNode,"val");

    mXTKApp->registerSharedData(mOutputSharedDataName,mOutputLayout,mOutputSize);
    
}
/******************************************************************************/
Compute_Bounds::~Compute_Bounds()
{

}
/******************************************************************************/
void Compute_Bounds::operator()()
{
    Plato::XTKSharedData* tOutputData = nullptr;
    mXTKApp->getSharedData(mOutputSharedDataName,mOutputLayout,tOutputData);

    std::vector<double> tBounds(mOutputSize);

    for(int i = 0; i < (int)tBounds.size(); i++)
    {
        tBounds[i] = mValue;
    }

    tOutputData->setData(tBounds);

}

std::string Compute_Bounds::op_name() const
{
    return "Compute_Bounds";
}

}
