/*
 * Plato_XTK_Local_Op_Decompose.cpp
 *
 *  Created on: Jul 3, 2019
 *      Author: doble
 */

#include "Plato_XTK_Local_Op_Generate_Model.hpp"

#include "cl_MTK_Mesh.hpp"
#include <cl_MTK_Mesh_Manager.hpp>
#include <cl_XTK_Model.hpp>
#include <cl_XTK_Enriched_Integration_Mesh.hpp>
#include <cl_MTK_Integration_Mesh_STK.hpp>
#include <cl_MTK_Interpolation_Mesh_STK.hpp>
#include <cl_HMR.hpp>
#include <cl_GEN_Geometry_Engine.hpp>
#include "Plato_Parser.hpp"
#include "Plato_OperationsUtilities.hpp"
#include "fn_compute_xtk_model_volumes.hpp"
#include "fn_PRM_GEN_Parameters.hpp"
#include <fn_norm.hpp>
#include <cl_MTK_Writer_Exodus.hpp>

namespace PlatoXTK
{
/******************************************************************************/
Generate_Model_Op::Generate_Model_Op(Plato::XTKApplication* aXTKApp,
                                     Plato::InputData &     aNode):
                                             Local_Op(aXTKApp),
                                             mInitialized(false)
{
    if(!aXTKApp->mStandaloneExecution)
    {
        Plato::InputData tOutputNode = Plato::Get::InputData(aNode, "Output");
        mOutputSharedDataName = Plato::Get::String(tOutputNode,"SharedDataName");
        int tInitialADVOutputSize = Plato::Get::Int(tOutputNode,"Size");
        mOutputLayout = Plato::getLayout(tOutputNode, Plato::data::layout_t::SCALAR);

        mXTKApp->registerSharedData(mOutputSharedDataName,mOutputLayout,tInitialADVOutputSize);

        Plato::InputData tInputNode = Plato::Get::InputData(aNode, "Input");
        mADVSharedDataName = Plato::Get::String(tInputNode,"SharedDataName");
        int tADVSharedDataSize = Plato::Get::Int(tInputNode,"Size");
        mADVLayout = Plato::getLayout(tInputNode, Plato::data::layout_t::SCALAR);

        mXTKApp->registerSharedData(mADVSharedDataName,mADVLayout,tADVSharedDataSize);
    }
    
}
/******************************************************************************/
Generate_Model_Op::~Generate_Model_Op()
{

}
/******************************************************************************/
void Generate_Model_Op::operator()()
{
    // do some setup the first time this is called. usually this is in the initialize control call
    if(!mInitialized)
    {

        // Geometry engine initialize
        mXTKApp->mCurrentGEN = std::make_shared< ge::Geometry_Engine >( mXTKApp->mGENParameters, nullptr );

        // Initialize  Background Mesh Mesh
        mXTKApp->mCurrentBGMTK = std::make_shared< mtk::Mesh_Manager >();

        // Initialize output mesh from XTK
        mXTKApp->mCurrentOutputMTK = std::make_shared< mtk::Mesh_Manager >();

        // Set performer to HMR
        if(mXTKApp->mWorkflowType == "HMR")
        {
            // hmr performer
            mXTKApp->mCurrentHMR = std::make_shared< hmr::HMR >( mXTKApp->mHMRParameters( 0 )( 0 ) );
            mXTKApp->mCurrentHMR->set_performer( mXTKApp->mCurrentBGMTK );

            // uniform initial refinement
            mXTKApp->mCurrentHMR->perform_initial_refinement();

            // HMR finalize
            mXTKApp->mCurrentHMR->perform();
        }
        else if(mXTKApp->mWorkflowType == "STK")
        {
            std::string tMeshFile = mXTKApp->mSTKParameters(0)(0).get<std::string>("input_file");

            // construct the stk
            mXTKApp->mSTKIpMesh = std::make_shared<mtk::Interpolation_Mesh_STK>( tMeshFile ,nullptr);
            mXTKApp->mSTKIgMesh = std::make_shared<mtk::Integration_Mesh_STK> ( *mXTKApp->mSTKIpMesh.get(), nullptr );

            mXTKApp->mCurrentBGMTK->register_mesh_pair(mXTKApp->mSTKIpMesh.get(),mXTKApp->mSTKIgMesh.get());
        }

        
        

        //
        mXTKApp->mCurrentGEN ->distribute_advs( mXTKApp->mCurrentBGMTK->get_mesh_pair( 0 ),{}, EntityRank::NODE );
        
        // Get ADVs
        Matrix<DDRMat> tADVs        = mXTKApp->mCurrentGEN->get_advs();
        Matrix<DDRMat> tLowerBounds = mXTKApp->mCurrentGEN->get_lower_bounds();
        Matrix<DDRMat> tUpperBounds = mXTKApp->mCurrentGEN->get_upper_bounds();

        // moris::print(tADVs,"Initial ADVS");
        std::cout<<"Initial Design Varaiable Len:  "<<tADVs.numel()<<std::endl;
        std::cout<<"Initial Design Varaiable Min:  "<<tADVs.min()<<std::endl;
        std::cout<<"Initial Design Varaiable Max:  "<<tADVs.max()<<std::endl;
        std::cout<<"Initial Design Varaiable Norm: "<<moris::norm(tADVs)<<std::endl;

        mInitialized = true;

        if(!mXTKApp->mStandaloneExecution)
        {
            mXTKApp->registerSharedData(mOutputSharedDataName,mOutputLayout,tADVs.numel());

            Plato::XTKSharedData* tInitialControlSharedData = nullptr;
            mXTKApp->getSharedData(mOutputSharedDataName,mOutputLayout,tInitialControlSharedData);

            tInitialControlSharedData->setData(tADVs);
        }

        
    }

    else
    {
        // add criteria sensitivity fields to mesh (for visualization)
        xtk::Enriched_Integration_Mesh & tEnrIGMesh = mXTKApp->mCurrentXTK->get_enriched_integ_mesh(0);
        for(int i = 0; i < (int)mXTKApp->mCriteriaSensitivity.size(); i++)
        {
            moris::moris_index tFieldIndex = tEnrIGMesh.create_field(mXTKApp->mCriteriaSensitivityFieldName(i),EntityRank::NODE);
            tEnrIGMesh.add_field_data(tFieldIndex,EntityRank::NODE,mXTKApp->mCriteriaSensitivity(i));
        }

        // write the mesh 
        moris::mtk::Writer_Exodus tWriter( mXTKApp->mCurrentOutputMTK->get_integration_mesh(0) );

        // pass nodal field names to writer
        // mWriter( aVisMeshIndex )->set_nodal_fields( tNodalFieldNames );

        std::string tOptIterStrg = std::to_string(mXTKApp->mOutputMeshIndex);
        std::string tMeshFile = "./xtk_evolve.e-s." + std::string(4-tOptIterStrg.length(),'0') + tOptIterStrg;

        tWriter.write_mesh(
                    "", tMeshFile, 
                    "", "./xtk_temp2." + std::to_string( mXTKApp->mOutputMeshIndex ) + ".exo" );

        tWriter.set_time(mXTKApp->mOutputMeshIndex);

        tWriter.set_nodal_fields( mXTKApp->mCriteriaSensitivityFieldName );

        for(int i = 0; i < (int)mXTKApp->mCriteriaSensitivity.size(); i++)
        {
            if(mXTKApp->mCriteriaSensitivity(i).numel() > 0)
            {
                tWriter.write_nodal_field(mXTKApp->mCriteriaSensitivityFieldName(i), mXTKApp->mCriteriaSensitivity(i));
            }
        }


        mXTKApp->mOutputMeshIndex++;

        tWriter.close_file();


        Plato::XTKSharedData* tADVSharedData = nullptr;
        mXTKApp->getSharedData(mADVSharedDataName,mADVLayout,tADVSharedData);

        Matrix<DDRMat> tNewADVs(1,tADVSharedData->size());

        std::vector<double> const & tData = tADVSharedData->get_data();

        for(int i = 0; i < tADVSharedData->size(); i++)
        {
            tNewADVs(i) = tData[i];
        }

        std::cout<<"Design Varaiable Len:  "<<tNewADVs.numel()<<std::endl;
        std::cout<<"Design Varaiable Min:  "<<tNewADVs.min()<<std::endl;
        std::cout<<"Design Varaiable Max:  "<<tNewADVs.max()<<std::endl;
        std::cout<<"Design Varaiable Norm: "<<moris::norm(tNewADVs)<<std::endl;
        mXTKApp->mCurrentGEN->set_advs(tNewADVs);

    }


    // XTK initialize
    mXTKApp->mCurrentXTK = std::make_shared< xtk::Model >( mXTKApp->mXTKParameters( 0 )( 0 ) );    

    mXTKApp->mCurrentOutputMTK = std::make_shared< mtk::Mesh_Manager >();

    // Set XTK Cooperations
    mXTKApp->mCurrentXTK->set_geometry_engine(  mXTKApp->mCurrentGEN.get() );
    mXTKApp->mCurrentXTK->set_input_performer(  mXTKApp->mCurrentBGMTK );
    mXTKApp->mCurrentXTK->set_output_performer( mXTKApp->mCurrentOutputMTK );

    mXTKApp->mCurrentGEN->output_fields(mXTKApp->mCurrentBGMTK->get_interpolation_mesh( 0 ));

    mXTKApp->mCurrentGEN->reset_mesh_information(mXTKApp->mCurrentBGMTK->get_interpolation_mesh(0) );

    // XTK perform - decompose - enrich - ghost - multigrid
    mXTKApp->mCurrentXTK->perform();

    // Assign PDVs
    mXTKApp->mCurrentGEN->create_pdvs( mXTKApp->mCurrentOutputMTK->get_mesh_pair(0) );

    std::cout<<"END GENERATE_MODEL XTK"<<std::endl;

}

std::string Generate_Model_Op::op_name() const
{
    return "Generate_Model_Op";
}

}
