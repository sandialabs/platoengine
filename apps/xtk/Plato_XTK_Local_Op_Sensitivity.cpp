/*
 * Plato_XTK_Local_Op_Sensitivity.cpp
 *
 *  Created on: Jul 3, 2019
 *      Author: doble
 */


#include "Plato_XTK_Local_Op_Sensitivity.hpp"
#include <functional>
#include "Plato_Parser.hpp"
#include "Plato_OperationsUtilities.hpp"
#include "Plato_XTK_SharedData.hpp"
#include <cl_XTK_Background_Mesh.hpp>
#include <cl_Cell.hpp>
#include <assert.hpp>
#include <fn_norm.hpp>
#include <cl_GEN_Geometry_Engine.hpp>

#include <limits>
namespace PlatoXTK
{
/******************************************************************************/
Sensitivity::Sensitivity(Plato::XTKApplication*        aXTKApp,
                         Plato::InputData &            aNode):
                         Local_Op(aXTKApp)
{
    // Criteria Shared Data

    // get data names
    mGradientX = Plato::Get::String(aNode, "GradientX");
    mGradientY = Plato::Get::String(aNode, "GradientY");
    mGradientZ = Plato::Get::String(aNode, "GradientZ");

    // register shared data
    mXTKApp->registerSharedData(mGradientX,Plato::data::SCALAR_FIELD);
    mXTKApp->registerSharedData(mGradientY,Plato::data::SCALAR_FIELD);
    mXTKApp->registerSharedData(mGradientZ,Plato::data::SCALAR_FIELD);

    // output
    Plato::InputData tOutputNode = Plato::Get::InputData(aNode, "Output");
    mOutputSharedDataName = Plato::Get::String(tOutputNode,"SharedDataName");

    // sparse or dense
    mSparse = Plato::Get::Bool(aNode,"Sparse",true);

    mOutputStartFieldIndex = mXTKApp->mCriteriaSensitivityFieldName.size();
    mOutputEndFieldIndex   = mOutputStartFieldIndex + 2;

    std::string tVecFieldBase  = Plato::Get::String(aNode, "vec_field_base");

    moris::Cell<std::string> tVecBase = {{"X"},{"Y"},{"Z"}};

    for(int i = 0 ; i < 3; i++)
    {
        mXTKApp->mCriteriaSensitivity.push_back(moris::Matrix<DDRMat>(0,0));
        mXTKApp->mCriteriaSensitivityFieldName.push_back(tVecFieldBase + tVecBase(i));
    }

            
    // layout
    mOutputLayout = Plato::getLayout(tOutputNode, Plato::data::layout_t::SCALAR);
    int tOutputSize = Plato::Get::Int(tOutputNode, "Size");
    // register shared data in application
    mXTKApp->registerSharedData(mOutputSharedDataName,mOutputLayout,tOutputSize);
}
/******************************************************************************/
Sensitivity::~Sensitivity()
{

}
/******************************************************************************/
void Sensitivity::operator()()
{

        // mXTKApp->mCurrentGEN->communicate_requested_IQIs();
        moris::Cell<moris::Matrix<moris::DDRMat>*> tdCriteriadPDV(1);

        // background mesh
        moris::mtk::Interpolation_Mesh& tInterpolationMesh = mXTKApp->mCurrentXTK->get_background_mesh();
        xtk::Background_Mesh tBackgroundMesh(&tInterpolationMesh);

        // number of ig nodes
        moris::uint tNumNodes = tBackgroundMesh.get_num_entities(EntityRank::NODE);

        // allocate a map
        moris::Matrix<moris::DDSMat> tMap(1,tNumNodes);
        
        moris_index tDesignGeomIndex = 0;

        Plato::XTKSharedData* tdQI_dx = nullptr;
        mXTKApp->getSharedData(mGradientX,Plato::data::SCALAR_FIELD,tdQI_dx);

        Plato::XTKSharedData* tdQI_dy = nullptr;
        mXTKApp->getSharedData(mGradientY,Plato::data::SCALAR_FIELD,tdQI_dy);

        Plato::XTKSharedData* tdQI_dz = nullptr;
        if(mXTKApp->mCurrentXTK->get_spatial_dim() == 3)
        {
            mXTKApp->getSharedData(mGradientZ,Plato::data::SCALAR_FIELD,tdQI_dz);
        }
        tdCriteriadPDV(0) = new Matrix<DDRMat>(tNumNodes,mXTKApp->mCurrentXTK->get_spatial_dim(),0.0);
        tdCriteriadPDV(0)->fill(0.0);

        // std::cout<<"tdQI_dx size ="<<tdQI_dx->size()<<std::endl;
        // std::cout<<"tdQI_dy size ="<<tdQI_dy->size()<<std::endl;
        // std::cout<<"tdQI_dz size ="<<tdQI_dz->size()<<std::endl;

        
        // std::cout<<"tdQI_dx max = "<<*std::max_element(std::begin(tdQI_dx->get_data()),std::end(tdQI_dx->get_data()))<<std::endl;
        // std::cout<<"tdQI_dx min = "<<*std::min_element(std::begin(tdQI_dx->get_data()),std::end(tdQI_dx->get_data()))<<std::endl;

        // std::cout<<"tdQI_dy max = "<<*std::max_element(std::begin(tdQI_dy->get_data()),std::end(tdQI_dy->get_data()))<<std::endl;
        // std::cout<<"tdQI_dy min = "<<*std::min_element(std::begin(tdQI_dy->get_data()),std::end(tdQI_dy->get_data()))<<std::endl;

        // std::cout<<"tdQI_dz max = "<<*std::max_element(std::begin(tdQI_dz->get_data()),std::end(tdQI_dz->get_data()))<<std::endl;
        // std::cout<<"tdQI_dz min = "<<*std::min_element(std::begin(tdQI_dz->get_data()),std::end(tdQI_dz->get_data()))<<std::endl;


        for(uint i = 0; i < tNumNodes; i++)
        {   
            if(tBackgroundMesh.is_interface_node(static_cast<moris_index>(i), tDesignGeomIndex) /*&& !mXTKApp->mCurrentXTK->is_vertex_fixed_to_bulk_phase(i)*/)
            {

                (*tdCriteriadPDV(0))(i,0) = (*tdQI_dx)(i);
                (*tdCriteriadPDV(0))(i,1) = (*tdQI_dy)(i);
                if(mXTKApp->mCurrentXTK->get_spatial_dim() == 3)
                {
                    (*tdCriteriadPDV(0))(i,2) = (*tdQI_dz)(i);
                }
        
            }
        }

        mXTKApp->mCurrentGEN->set_dQIdp(tdCriteriadPDV,&tMap);

        Matrix<DDRMat> tDCriteriaDAdv = mXTKApp->mCurrentGEN->get_dcriteria_dadv();

        if (par_rank() == 0)
        {
            std::cout<< "--------------------------------------------------------------------------------\n";
            std::cout<< "Gradients of design criteria wrt ADVs:\n";

            for (uint i=0;i<tDCriteriaDAdv.n_rows();++i)
            {
                Matrix<DDRMat> tDIQIDAdv = tDCriteriaDAdv.get_row(i);

                auto tItrMin = std::min_element(tDIQIDAdv.data(),tDIQIDAdv.data()+tDIQIDAdv.numel());
                auto tIndMin = std::distance(tDIQIDAdv.data(),tItrMin);

                auto tItrMax = std::max_element(tDIQIDAdv.data(),tDIQIDAdv.data()+tDIQIDAdv.numel());
                auto tIndMax = std::distance(tDIQIDAdv.data(),tItrMax);

                std::cout<<"Criteria("<<i<<"): norm ="<<norm(tDIQIDAdv)<< " | min = " <<tDIQIDAdv.min()<< " | max = " <<tDIQIDAdv.max()<<std::endl;

            }

            std::cout<<  "--------------------------------------------------------------------------------"<<std::endl;
        }

        // commit to shared data
        Plato::XTKSharedData* tDcriteriaDAdvSD = nullptr;
        mXTKApp->getSharedData(mOutputSharedDataName,mOutputLayout,tDcriteriaDAdvSD);
        tDcriteriaDAdvSD->resize(tDCriteriaDAdv.numel());
        tDcriteriaDAdvSD->setData(tDCriteriaDAdv);

        
        for(int i = 0 ; i < (int)tdCriteriadPDV(0)->n_cols(); i++)
        {
            mXTKApp->mCriteriaSensitivity(mOutputStartFieldIndex + i) =  tdCriteriadPDV(0)->get_column(i);
        }


        mXTKApp->mCriteriaSensitivity(mOutputStartFieldIndex + 2) =  moris::Matrix<moris::DDRMat>( tdCriteriadPDV(0)->n_rows(), 1,0);


        delete tdCriteriadPDV(0);

}
/******************************************************************************/

std::string Sensitivity::op_name() const
{
    return "Sensitivity";
}
}
