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

#ifndef BASE_SRC_XTK_PLATO_XTK_APPLICATION_HPP_
#define BASE_SRC_XTK_PLATO_XTK_APPLICATION_HPP_

// Includes from XTK
#include <cl_XTK_Model.hpp>
#include <cl_GEN_Geometry_Engine.hpp>
#include <GEN_typedefs.hpp>
#include <cl_MTK_Mesh.hpp>
#include <cl_XTK_Enums.hpp>
#include <cl_Matrix.hpp>
#include <cl_Cell.hpp>
#include <cl_Param_List.hpp>
#include <Teuchos_ParameterList.hpp>
#include <Teuchos_XMLParameterListHelpers.hpp>

#include "Plato_Application.hpp"
#include "Plato_SharedData.hpp"
#include "Plato_XTK_SharedData.hpp"
#include "Plato_InputData.hpp"
#include "Plato_XTK_Local_Op.hpp"
#include "Plato_Exceptions.hpp"

// forward declare operators
namespace PlatoXTK
{
    class Generate_Model_Op;
    class Sensitivity;
    class Output_Mesh;
    class Load_From_HDF5;
    class Save_To_HDF5;
    class Compute_Bounds;
}

namespace moris
{
    namespace ge
    {
        class Geometry;
        class Geometry_Engine;
    }

    namespace hmr
    {
        class HMR;
    }

    namespace mtk
    {
        class Mesh_Manager;
        class Interpolation_Mesh_STK;
        class Integration_Mesh_STK;
    }

    

    
}

namespace Plato
{
/******************************************************************************/
class XTKApplication: public Application
/******************************************************************************/
{
public:
    /******************************************************************************/
    /*!
     * @param[in] aCommunicator   - MPI communicator to use in XTK
     * @param[in] aInputMeshFile  - Mesh file to use for background mesh
     * @param[in] aFieldName      - Node field on background mesh to refine against
     * @param[in] aOutputMeshFile - Name of output mesh file
     */
    XTKApplication();
    /******************************************************************************/

    /******************************************************************************/
    /*!
     * General inputs
     */
    XTKApplication(int aArgc, char **aArgv, MPI_Comm& aLocalComm);
    /******************************************************************************/

    /******************************************************************************//**
     * @brief Constructor
     * @param [in] aInputXML Inputs for XTK
     * @param [in] aApplicationXML Application XML File
     * @param [in] aLocalComm local communicator
    **********************************************************************************/
    XTKApplication(const std::string    & aInputXML,
                   const std::string    & aApplicationXML,
                   MPI_Comm & aLocalComm);

    /******************************************************************************/
    /*!
     * Deconstructor that deletes meshes created via factor
     */
    ~XTKApplication();
    /******************************************************************************/

    /******************************************************************************/
    void initialize();
    /******************************************************************************/

    /******************************************************************************/
    void finalize();
    /******************************************************************************/

    /******************************************************************************/
    void initialize(MPI_Comm                       aCommunicator, 
                    Teuchos::ParameterList const & aParameters);
    /******************************************************************************/

    /******************************************************************************/
    /*!
     * @brief Perform XTK operations
     */
    void
    compute(const std::string & aOperationName);
    /******************************************************************************/

    /******************************************************************************/
    /*!
     * @brief Export Data from XTK.
     * Valid: aArgumentName:
     *  - "control"                  - export the control field (which was seeded with call to compute("seed_field"))
     *  - "topo"                     - export the topology field
     *  - "constraint grad wrt topo" - export the constraint gradient with respect to topology field
     *  - "objective grad wrt topo"  - export the objective gradient with respect to topology field
     *  - "dx1dp0"                   - Shape sensitivity in x direction to design variable 0
     *  - "dx2dp0"                   - Shape sensitivity in y direction to design variable 0
     *  - "dx3dp0"                   - Shape sensitivity in z direction to design variable 0
     *  - "dx1dp1"                   - Shape sensitivity in x direction to design variable 1
     *  - "dx2dp1"                   - Shape sensitivity in y direction to design variable 1
     *  - "dx3dp1"                   - Shape sensitivity in z direction to design variable 1
     *  - "NumDesVar"                - Number of design variables a  node depends on (should be 0 or 2)
     *  - "DesVar0"                  - Design variable 0
     *  - "DesVar1"                  - Design variable 1
     *
     * @param aArgumentName - Data name to export
     * @param aExportData   - Exported data
     */
    void
    exportData(const std::string & aArgumentName, Plato::SharedData & aExportData);
    /******************************************************************************/

    /******************************************************************************/
    /*!
     * Import data (specifically import the design field)
     * Valid argument names:
     * - topo - import the topology field
     * - dfdx- import the modal spatial gradient of f
     * - dhdx- import the nodal spatial gradient of h
     * @param aArgumentName
     * @param aImportData
     */
    void
    importData(const std::string       & aArgumentName, const Plato::SharedData & aImportData);
    /******************************************************************************/

    /******************************************************************************/
    /*!
     *
     * @param aDataLayout
     * @param aMyOwnedGlobalIDs
     */
    void exportDataMap(const Plato::data::layout_t & aDataLayout,  std::vector<int> & aMyOwnedGlobalIDs);
    /******************************************************************************/

    /******************************************************************************/
    void getSharedData(std::string aSharedDataName, const Plato::data::layout_t & aDataLayout, XTKSharedData* & aSharedData);
    /******************************************************************************/

    /******************************************************************************/
    /*!
     * Adds shared data to the shared data map. If the shared data already exists, nothing happens
     */
    void registerSharedData(std::string aSharedDataName, const Plato::data::layout_t & aDataLayout, int aSize = 0);
    /******************************************************************************/

    //-------------------------------------------------------------------------------------
    // Access of member data
    //-------------------------------------------------------------------------------------

    // give local operators friend status
    friend class PlatoXTK::Generate_Model_Op;
    friend class PlatoXTK::Sensitivity;
    friend class PlatoXTK::Output_Mesh;
    friend class PlatoXTK::Load_From_HDF5;
    friend class PlatoXTK::Save_To_HDF5;
    friend class PlatoXTK::Compute_Bounds;

    //-------------------------------------------------------------------------------------
    // Printing functions
    //-------------------------------------------------------------------------------------
    /******************************************************************************/
    void print_operation_map();
    /******************************************************************************/

    /******************************************************************************/
    void print_shared_data_map();
    /******************************************************************************/

    //-------------------------------------------------------------------------------------
    // Parsing functions
    //-------------------------------------------------------------------------------------
    /******************************************************************************/
    Plato::data::layout_t string_to_layout(std::string const & aLayoutStr);
    /******************************************************************************/


protected:
    bool mVerbose;
    bool mStandaloneExecution;
    std::string mWorkflowType;

    // Problem Parameters
    moris::Cell< moris::Cell< moris::ParameterList > > mXTKParameters;
    moris::Cell< moris::Cell< moris::ParameterList > > mGENParameters;
    moris::Cell< moris::Cell< moris::ParameterList > > mHMRParameters;
    moris::Cell< moris::Cell< moris::ParameterList > > mSTKParameters;

public:
    // Current Performers
    std::shared_ptr< moris::hmr::HMR >            mCurrentHMR;
    std::shared_ptr< moris::ge::Geometry_Engine > mCurrentGEN;
    std::shared_ptr< mtk::Mesh_Manager >          mCurrentBGMTK;
    std::shared_ptr< mtk::Mesh_Manager >          mCurrentOutputMTK;
    std::shared_ptr< xtk::Model >                 mCurrentXTK;
    std::shared_ptr<mtk::Interpolation_Mesh_STK>  mSTKIpMesh;
    std::shared_ptr<mtk::Integration_Mesh_STK>    mSTKIgMesh;

protected:

    // phase table member data
    int              mNumPhases;
    int              mDefaultPhase;


    // input parameters
    moris::uint                     mStep = 0;

    // 
    uint mOutputMeshIndex;

    // Cell Mesh Fields
    moris::Cell<moris::Matrix<DDRMat>> mCriteriaSensitivity;
    moris::Cell<std::string> mCriteriaSensitivityFieldName;




    // // shape ad setup
    // moris::Cell<std::string>                    mAdDesignVarNames;
    // moris::Cell<Sacado::Fad::DFad<moris::real>> mADDesignVars;
    // moris::Cell<xtk::Geometry_AD*>              mADGeometryReps;
    // std::unordered_map<std::string,int>         mAdDesignVarNameToIndex;


    // seeded field data
    std::string                   mOutputSeededMeshFile;
    bool                          mSeeded = false;
    std::string                   mSeededOpName;
    moris::Matrix<moris::DDRMat>  mSeededField;

    // fixed design variables
    moris::Matrix<moris::IndexMat> mFixedBGNodesGD;


    // Shared data - import
    Plato::XTKSharedData mConstraintGradX1;
    Plato::XTKSharedData mConstraintGradX2;
    Plato::XTKSharedData mConstraintGradX3;
    Plato::XTKSharedData mObjectiveGradX1;
    Plato::XTKSharedData mObjectiveGradX2;
    Plato::XTKSharedData mObjectiveGradX3;

    // Shared data - export
    Plato::XTKSharedData mConstraintGradWrtTopo;
    Plato::XTKSharedData mObjectiveGradWrtTopo;
    Plato::XTKSharedData mSeededControl;

    // interface file
    Plato::InputData mAppfileData;
    Plato::InputData mInputfileData;

    // local operators
    std::map<std::string, PlatoXTK::Local_Op*> mOperationMap;

    // data map
    std::map<std::string, Plato::XTKSharedData*> mNodeSharedDataMap;
    std::map<std::string, Plato::XTKSharedData*> mScalarSharedDataMap;

    // Fields on Cut Mesh
    std::vector<std::string> mNodeFieldsOnCutMesh;

    // Fields on BG Mesh
    std::vector<std::string> mNodeFieldsOnBGMesh;


    void
    setup_parameters();

    void
    setup_local_operations();

    void
    setup_phase_table();


    /******************************************************************************/
    template<typename ValueType>
    void throwParsingException(const std::string & aName, const std::map<std::string, ValueType> & aValueMap)
    {
        std::stringstream tMessage;
        tMessage << "Cannot find specified Argument: " << aName.c_str() << std::endl;
        tMessage << "Available Arguments: " << std::endl;
        for(auto tIterator : aValueMap)
        {
            tMessage << tIterator.first << std::endl;
        }
        Plato::ParsingException tParsingException(tMessage.str());
        throw tParsingException;
    }
    /******************************************************************************/





};
}

#endif /* BASE_SRC_XTK_PLATO_XTK_APPLICATION_HPP_ */
