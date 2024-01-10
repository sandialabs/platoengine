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

/*
 * XTK_Interface.cpp
 *
 *  Created on: Jan 22, 2019
 *      Author: ktdoble
 */

#include "Plato_XTK_Application.hpp"
#include "Plato_SharedField.hpp"
#include "Plato_Parser.hpp"
#include "Plato_Exceptions.hpp"

// local op includes
#include "Plato_XTK_Local_Op_Compute_Bounds.hpp"
#include "Plato_XTK_Local_Op_Generate_Model.hpp"
#include "Plato_XTK_Local_Op_Load_From_Hdf5.hpp"
#include "Plato_XTK_Local_Op_Save_To_Hdf5.hpp"

// parameter files
#include <fn_PRM_HMR_Parameters.hpp>
#include <fn_PRM_GEN_Parameters.hpp>
#include <fn_PRM_XTK_Parameters.hpp>
#include <fn_PRM_STK_Parameters.hpp>
#include <fn_PRM_print.hpp>

// XTK includes
#include <cl_HMR.hpp>
#include <cl_GEN_Geometry.hpp>
#include <cl_MTK_Mesh_Data_Input.hpp>
#include <cl_MTK_Scalar_Field_Info.hpp>
#include <cl_GEN_Geometry_Engine.hpp>
#include <cl_XTK_Background_Mesh.hpp>
#include <cl_XTK_Hole_Seeder.hpp>
#include <cl_MTK_Writer_Exodus.hpp>
#include <cl_MTK_Integration_Mesh_STK.hpp>
#include <cl_MTK_Interpolation_Mesh_STK.hpp>
#include <fn_norm.hpp>
#include <limits>
#include <op_minus.hpp>

// MORIS Globals to control communication and logging
#include "assert.hpp"
#include <cl_Logger.hpp>                // MRS/IOS/src
#include "cl_Communication_Manager.hpp" // COM/src
#include "Plato_XTK_Local_Op_Sensitivity.hpp"

moris::Comm_Manager gMorisComm;
moris::Logger gLogger;

using namespace moris;

namespace Plato
{
    //-------------------------------------------------------------------------------------
    XTKApplication::XTKApplication()
    {
    }

    //-------------------------------------------------------------------------------------

    XTKApplication::XTKApplication(int aArgc, char **aArgv, MPI_Comm &aLocalComm)
    {
        gMorisComm = moris::Comm_Manager(aLocalComm);

        // parse the xtk input file
        const char *input_char = aArgv[1];

        std::cout << "input_char = " << *input_char << std::endl;

        Plato::Parser *parser = new Plato::PugiParser();
        mInputfileData = parser->parseFile(input_char);

        input_char = getenv("PLATO_APP_FILE");

        // if no app file then standalone execution
        if (input_char)
        {
            mAppfileData = parser->parseFile(input_char);
            mStandaloneExecution = false;
        }
        else
        {
            mStandaloneExecution = true;
        }

        this->setup_parameters();

        gMorisComm = moris::Comm_Manager(aLocalComm);

        // Severity level 0 - all outputs
        gLogger.initialize(2);

        delete parser;
    }
    //-------------------------------------------------------------------------------------

    XTKApplication::XTKApplication(
        const std::string &aInputXML,
        const std::string &aApplicationXML,
        MPI_Comm &aLocalComm) : mOutputMeshIndex(0)
    {
        // new parser
        gMorisComm = moris::Comm_Manager(aLocalComm);

        Plato::Parser *parser = new Plato::PugiParser();
        mInputfileData = parser->parseFile(aInputXML);

        // if no app file then standalone execution
        if (!aApplicationXML.empty())
        {
            mAppfileData = parser->parseFile(aApplicationXML);
            mStandaloneExecution = false;
        }
        else
        {
            mStandaloneExecution = true;
        }

        this->setup_parameters();

        gMorisComm = moris::Comm_Manager(aLocalComm);

        // Severity level 0 - all outputs
        gLogger.initialize(2);

        delete parser;
    }

    //-------------------------------------------------------------------------------------
    XTKApplication::~XTKApplication()
    {
        for (auto p : mNodeSharedDataMap)
        {
            delete p.second;
        }
        mNodeSharedDataMap.clear();

        for (auto p : mScalarSharedDataMap)
        {
            delete p.second;
        }
        mScalarSharedDataMap.clear();

        for (auto p : mOperationMap)
        {
            delete p.second;
        }
        mOperationMap.clear();

    }

    //-------------------------------------------------------------------------------------

    void
    XTKApplication::initialize()
    {

        // initialize the background mesh
        // this happens here so we can export the scalar field data map
        mCurrentBGMTK = std::make_shared<mtk::Mesh_Manager>();
        if (mWorkflowType == "HMR")
        {
            // Set performer to HMR
            mCurrentHMR->set_performer(mCurrentBGMTK);

            // uniform initial refinement
            mCurrentHMR->perform_initial_refinement();

            // HMR finalize
            mCurrentHMR->perform();
        }
        else if (mWorkflowType == "STK")
        {
            std::string tMeshFile = mSTKParameters(0)(0).get<std::string>("input_file");

            // construct the stk
            mSTKIpMesh = std::make_shared<mtk::Interpolation_Mesh_STK>(tMeshFile, nullptr);
            mSTKIgMesh = std::make_shared<mtk::Integration_Mesh_STK>(*mSTKIpMesh, nullptr);

            mCurrentBGMTK->register_mesh_pair(mSTKIpMesh.get(), mSTKIgMesh.get());
        }

        else
        {
            MORIS_ERROR(0, "Invalid workflow type: STK or HMR");
        }

        // setup operations
        this->setup_local_operations();

        // if(mVerbose)
        // {
        this->print_operation_map();
        this->print_shared_data_map();
        // }
    }

    //-------------------------------------------------------------------------------------

    void
    XTKApplication::finalize()
    {
    }

    //-------------------------------------------------------------------------------------
    void
    XTKApplication::compute(const std::string &aOperationName)
    {
        auto it = mOperationMap.find(aOperationName);
        if (it == mOperationMap.end())
        {
            std::stringstream ss;
            ss << "Request for operation ('" << aOperationName << "') that doesn't exist.";
            throw Plato::LogicException(ss.str());
        }

        (*mOperationMap[aOperationName])();
    }

    void
    XTKApplication::print_operation_map()
    {
        std::cout << "XTK Application Operation Map: \n";

        for (auto tIter = mOperationMap.begin(); tIter != mOperationMap.end(); tIter++)
        {
            std::cout << "    User Operation Name: " << std::left << std::setw(38) << tIter->first << " | Function/Op_Class Name: " << std::left << std::setw(20) << tIter->second->op_name() << std::endl;
        }
    }

    void
    XTKApplication::print_shared_data_map()
    {
        std::cout << "XTK Application Nodal Shared Data Map: \n";

        for (auto tIter = mNodeSharedDataMap.begin(); tIter != mNodeSharedDataMap.end(); tIter++)
        {
            std::cout << "    Shared Data Name: " << std::left << std::setw(20) << tIter->first << std::endl;
        }
        std::cout << "XTK Application Scalar Shared Data Map: \n";

        for (auto tIter = mScalarSharedDataMap.begin(); tIter != mScalarSharedDataMap.end(); tIter++)
        {
            std::cout << "    Shared Data Name: " << std::left << std::setw(20) << tIter->first << std::endl;
        }
    }

    Plato::data::layout_t XTKApplication::string_to_layout(std::string const &aLayoutStr)
    {
        MORIS_ASSERT(aLayoutStr == "Node", "Only node layout currently supported");

        return Plato::data::SCALAR_FIELD;
    }

    void
    XTKApplication::importData(const std::string &aArgumentName,
                               const Plato::SharedData &aImportData)
    {
        Plato::XTKSharedData *tSharedData = nullptr;
        this->getSharedData(aArgumentName, aImportData.myLayout(), tSharedData);
        std::vector<double> tImportedVector(aImportData.size());
        aImportData.getData(tImportedVector);
        tSharedData->setData(tImportedVector);

        if (mNodeSharedDataMap.find(aArgumentName) != mNodeSharedDataMap.end())
        {
            //        Plato::XTKSharedData* tSharedData = getNodeField(aArgumentName);
        }

        else
        {
            std::stringstream ss;
            ss << "Invalid aArgumentName specified in importData ('" << aArgumentName << "'). \n"
               << __FILE__ << ":" << __LINE__;
            throw Plato::ParsingException(ss.str());
        }
    }

    //-------------------------------------------------------------------------------------
    void
    XTKApplication::exportData(const std::string &aArgumentName,
                               Plato::SharedData &aExportData)
    {
        Plato::XTKSharedData *tSharedData = nullptr;
        Plato::data::layout_t tLayout = aExportData.myLayout();

        this->getSharedData(aArgumentName, tLayout, tSharedData);

        aExportData.setData(tSharedData->get_data());
    }

    void
    XTKApplication::exportDataMap(const Plato::data::layout_t &aDataLayout,
                                  std::vector<int> &aMyOwnedGlobalIDs)
    {

        if (aDataLayout == Plato::data::layout_t::SCALAR_FIELD)
        {
            moris::uint tNumLocalVals = mCurrentBGMTK->get_interpolation_mesh(0)->get_num_entities(moris::EntityRank::NODE);
            aMyOwnedGlobalIDs.resize(tNumLocalVals);
            for (moris::uint tLocalID = 0; tLocalID < tNumLocalVals; tLocalID++)
            {
                aMyOwnedGlobalIDs[tLocalID] = mCurrentBGMTK->get_interpolation_mesh(0)->get_glb_entity_id_from_entity_loc_index((moris::moris_index)tLocalID, moris::EntityRank::NODE);
            }
        }
        else if (aDataLayout == Plato::data::layout_t::ELEMENT_FIELD)
        {
            moris::uint tNumLocalVals = mCurrentBGMTK->get_interpolation_mesh(0)->get_num_entities(moris::EntityRank::ELEMENT);
            aMyOwnedGlobalIDs.resize(tNumLocalVals);
            for (moris::uint tLocalID = 0; tLocalID < tNumLocalVals; tLocalID++)
            {
                aMyOwnedGlobalIDs[tLocalID] = mCurrentBGMTK->get_interpolation_mesh(0)->get_glb_entity_id_from_entity_loc_index((moris::moris_index)tLocalID, moris::EntityRank::ELEMENT);
            }
        }
        else
        {
            Plato::ParsingException tParsingException("XTKApp currently only supports SCALAR_FIELD data layout");
            throw tParsingException;
        }
    }

    void XTKApplication::getSharedData(std::string aSharedDataName, const Plato::data::layout_t &aDataLayout, XTKSharedData *&aSharedData)
    {
        if (aDataLayout == Plato::data::layout_t::SCALAR_FIELD)
        {
            MORIS_ASSERT(mNodeSharedDataMap.find(aSharedDataName) != mNodeSharedDataMap.end(), "Scalar Field Shared Data Not Found.");

            aSharedData = mNodeSharedDataMap[aSharedDataName];
        }
        else if (aDataLayout == Plato::data::layout_t::SCALAR)
        {
            MORIS_ASSERT(mScalarSharedDataMap.find(aSharedDataName) != mScalarSharedDataMap.end(), "Scalar Field Shared Data Not Found.");
            aSharedData = mScalarSharedDataMap[aSharedDataName];
        }
        else if (aDataLayout == Plato::data::layout_t::ELEMENT_FIELD)
        {
            Plato::ParsingException tParsingException("XTKApp currently only supports SCALAR_FIELD data layout");
            throw tParsingException;
        }
        else
        {
            Plato::ParsingException tParsingException("XTKApp currently only supports SCALAR_FIELD data layout");
            throw tParsingException;
        }
    }

    void XTKApplication::registerSharedData(std::string aSharedDataName, const Plato::data::layout_t &aDataLayout, int aSize)
    {
        if (aDataLayout == Plato::data::layout_t::SCALAR_FIELD)
        {
            auto tIter = mNodeSharedDataMap.find(aSharedDataName);

            if (tIter == mNodeSharedDataMap.end())
            {
                mNodeSharedDataMap[aSharedDataName] = new Plato::XTKSharedData(aSize, aSharedDataName, Plato::data::SCALAR_FIELD);
            }
        }
        else if (aDataLayout == Plato::data::layout_t::ELEMENT_FIELD)
        {
            Plato::ParsingException tParsingException("XTKApp currently only supports SCALAR_FIELD data layout");
            throw tParsingException;
        }
        else if (aDataLayout == Plato::data::layout_t::SCALAR)
        {
            auto tIter = mScalarSharedDataMap.find(aSharedDataName);

            if (tIter == mScalarSharedDataMap.end())
            {
                mScalarSharedDataMap[aSharedDataName] = new Plato::XTKSharedData(aSize, aSharedDataName, Plato::data::SCALAR);
            }
        }
        else
        {
            Plato::ParsingException tParsingException("XTKApp currently only supports SCALAR_FIELD, ELEMENT_FIELD, SCALAR data layout");
            throw tParsingException;
        }
    }


    //-------------------------------------------------------------------------------------

    void
    XTKApplication::setup_parameters()
    {
        // allocate default parameters list
        mXTKParameters.resize(1);
        mXTKParameters(0).resize(1);
        mXTKParameters(0)(0) = moris::prm::create_xtk_parameter_list();
        auto tXTKNode = mInputfileData.getByName<Plato::InputData>("XTK_Parameters")[0];

        mVerbose = Plato::Get::Bool(tXTKNode, "verbose", false);

        mWorkflowType = Plato::Get::String(tXTKNode, "workflow");
        mXTKParameters(0)(0).set("output_file", Plato::Get::String(tXTKNode, "output_file"));
        mXTKParameters(0)(0).set("output_path", Plato::Get::String(tXTKNode, "output_path"));
        mXTKParameters(0)(0).set("union_blocks", Plato::Get::String(tXTKNode, "union_blocks"));
        mXTKParameters(0)(0).set("union_block_names", Plato::Get::String(tXTKNode, "union_block_names"));
        mXTKParameters(0)(0).set("union_block_colors", Plato::Get::String(tXTKNode, "union_block_colors"));
        mXTKParameters(0)(0).set("union_side_sets", Plato::Get::String(tXTKNode, "union_side_sets"));
        mXTKParameters(0)(0).set("union_side_set_names", Plato::Get::String(tXTKNode, "union_side_set_names"));
        mXTKParameters(0)(0).set("union_side_set_colors", Plato::Get::String(tXTKNode, "union_side_set_colors"));
        mXTKParameters(0)(0).set("deactivate_all_but_blocks", Plato::Get::String(tXTKNode, "deactivate_all_but_blocks"));
        mXTKParameters(0)(0).set("deactivate_all_but_side_sets", Plato::Get::String(tXTKNode, "deactivate_all_but_side_sets"));
        mXTKParameters(0)(0).set("decompose", true);
        mXTKParameters(0)(0).set("decomposition_type", "conformal");
        mXTKParameters(0)(0).set("enrich", true);

        std::string tBasisRank = "";
        std::string tEnrMeshIndices = "";
        if (mWorkflowType == "HMR")
        {
            tBasisRank = "bspline";
            tEnrMeshIndices = "0,1";
        }
        else if (mWorkflowType == "STK")
        {
            tBasisRank = "node";
            tEnrMeshIndices = "0";
        }

        mXTKParameters(0)(0).set("basis_rank", tBasisRank);
        mXTKParameters(0)(0).set("enrich_mesh_indices", tEnrMeshIndices);
        mXTKParameters(0)(0).set("ghost_stab", false);
        mXTKParameters(0)(0).set("multigrid", false);
        mXTKParameters(0)(0).set("verbose", mVerbose);
        mXTKParameters(0)(0).set("print_enriched_ig_mesh", false);
        mXTKParameters(0)(0).set("exodus_output_XTK_ig_mesh", true);
        mXTKParameters(0)(0).set("triangulate_all", false);
        mXTKParameters(0)(0).insert("fixed_blocks", Plato::Get::String(tXTKNode, "fixed_blocks"));
        mXTKParameters(0)(0).insert("fixed_block_phase", Plato::Get::String(tXTKNode, "fixed_block_phase"));

        if (mVerbose)
        {
            moris::prm::print(mXTKParameters(0)(0));
        }

        // allocate default hmr parameter list
        if (mWorkflowType == "HMR")
        {
            mHMRParameters.resize(1);
            mHMRParameters(0).resize(1);
            mHMRParameters(0)(0) = moris::prm::create_hmr_parameter_list();
            auto tNode = mInputfileData.getByName<Plato::InputData>("HMR_Parameters")[0];

            std::string tNumElemsStr = Plato::Get::String(tNode, "number_of_elements_per_dimension");
            std::string tDomSizeStr = Plato::Get::String(tNode, "domain_dimensions");
            std::string tDomOffsetStr = Plato::Get::String(tNode, "domain_offset");
            std::string tSideSetStr = Plato::Get::String(tNode, "domain_sidesets");
            std::string tStateRefinement = Plato::Get::String(tNode, "state_refinement_level");
            std::string tGeomRefinementLev = Plato::Get::String(tNode, "geom_refinement_level");

            MORIS_ERROR(tStateRefinement >= tGeomRefinementLev, "condition state_refinement_level >= geom_refinement_level refinement not met");

            std::string tLagrangeOrder = "1";
            std::string tBsplineOrder = "2,1";
            std::string tInitialRef = tGeomRefinementLev + "," + tStateRefinement;
            std::string tLagrangePattern = "1";
            mHMRParameters(0)(0).set("number_of_elements_per_dimension", tNumElemsStr);
            mHMRParameters(0)(0).set("domain_dimensions", tDomSizeStr);
            mHMRParameters(0)(0).set("domain_offset", tDomOffsetStr);
            mHMRParameters(0)(0).set("domain_sidesets", tSideSetStr);
            mHMRParameters(0)(0).set("lagrange_output_meshes", std::string("0"));
            mHMRParameters(0)(0).set("lagrange_orders", tLagrangeOrder);
            mHMRParameters(0)(0).set("lagrange_pattern", tLagrangePattern);
            mHMRParameters(0)(0).set("bspline_orders", tBsplineOrder);
            mHMRParameters(0)(0).set("bspline_pattern", std::string("0,1"));
            mHMRParameters(0)(0).set("lagrange_to_bspline", "0,1");
            mHMRParameters(0)(0).set("truncate_bsplines", 1);
            mHMRParameters(0)(0).set("refinement_buffer", 1);
            mHMRParameters(0)(0).set("staircase_buffer", 1);
            mHMRParameters(0)(0).set("initial_refinement", tInitialRef);
            mHMRParameters(0)(0).set("initial_refinement_pattern", "0,1");
            mHMRParameters(0)(0).set("use_number_aura", 1);
            mHMRParameters(0)(0).set("use_multigrid", 0);
            mHMRParameters(0)(0).set("severity_level", 0);
            mHMRParameters(0)(0).set("write_lagrange_output_mesh", "./HMR_Output_Lagrange.e");
            mHMRParameters(0)(0).set("use_advanced_T_matrix_scheme", 1);

            if (mVerbose)
            {
                moris::prm::print(mHMRParameters(0)(0));
            }
            mCurrentHMR = std::make_shared<hmr::HMR>(mHMRParameters(0)(0));
        }

        else if (mWorkflowType == "STK")
        {
            mSTKParameters.resize(1);
            mSTKParameters(0).resize(1);
            auto tNode = mInputfileData.getByName<Plato::InputData>("STK_Parameters")[0];
            mSTKParameters(0)(0) = prm::create_stk_parameter_list();
            mSTKParameters(0)(0).set("input_file", Plato::Get::String(tNode, "mesh_file"));
        }

        else
        {
            MORIS_ERROR(0, "Invalid workflow specified, valid keys: HMR, STK");
        }

        // figure out how many geometries we have
        mGENParameters.resize(3);
        mGENParameters(0).resize(1);
        mGENParameters(0)(0) = moris::prm::create_gen_parameter_list();
        for (auto tNode : mInputfileData.getByName<Plato::InputData>("GEN_Parameters"))
        {
            mGENParameters(0)(0).set("IQI_types", Plato::Get::String(tNode, "IQI_types"));
            mGENParameters(0)(0).set("isocontour_threshold", Plato::Get::Double(tNode, "isocontour_threshold")); // Level-set isocontour level
            mGENParameters(0)(0).set("isocontour_tolerance", Plato::Get::Double(tNode, "isocontour_tolerance")); // Level-set isocontour level
            mGENParameters(0)(0).set("output_mesh_file", "./gen_fields.exo");
            if (mVerbose)
            {
                moris::prm::print(mGENParameters(0)(0));
            }

            mNumPhases = Plato::Get::Int(tNode, "num_phases", -1);
            mDefaultPhase = Plato::Get::Int(tNode, "default_phase", -1);

            if (mNumPhases == -1)
            {
                Plato::ParsingException tParsingException("No num_phases parameter provided");
            }

            if (mDefaultPhase == -1)
            {
                Plato::ParsingException tParsingException("No default_phase parameter provided");
            }

            std::vector<Plato::InputData> tGeometryData = tNode.getByName<Plato::InputData>("Geometry");

            int tGeoCount = 0;
            for (auto tGeom : tGeometryData)
            {
                std::string tGeomType = Plato::Get::String(tGeom, "type");

                bool tDesign = Plato::Get::Bool(tGeom, "design", false);

                if (tGeomType == "swiss_cheese_slice")
                {
                    // initialize swiss cheese geometry
                    mGENParameters(1).push_back(prm::create_swiss_cheese_slice_parameter_list());

                    mGENParameters(1)(tGeoCount).set("left_bound", Plato::Get::Double(tGeom, "left_bound"));
                    mGENParameters(1)(tGeoCount).set("right_bound", Plato::Get::Double(tGeom, "right_bound"));
                    mGENParameters(1)(tGeoCount).set("bottom_bound", Plato::Get::Double(tGeom, "bottom_bound"));
                    mGENParameters(1)(tGeoCount).set("top_bound", Plato::Get::Double(tGeom, "top_bound"));
                    mGENParameters(1)(tGeoCount).set("hole_x_semidiameter", Plato::Get::Double(tGeom, "hole_x_semidiameter"));
                    mGENParameters(1)(tGeoCount).set("hole_y_semidiameter", Plato::Get::Double(tGeom, "hole_y_semidiameter"));
                    mGENParameters(1)(tGeoCount).insert("hole_z_semidiameter", Plato::Get::Double(tGeom, "hole_z_semidiameter"));
                    mGENParameters(1)(tGeoCount).set("number_of_x_holes", Plato::Get::Int(tGeom, "number_of_x_holes"));
                    mGENParameters(1)(tGeoCount).set("number_of_y_holes", Plato::Get::Int(tGeom, "number_of_y_holes"));
                    mGENParameters(1)(tGeoCount).insert("number_of_z_holes", Plato::Get::Int(tGeom, "number_of_z_holes"));
                    mGENParameters(1)(tGeoCount).set("superellipse_exponent", Plato::Get::Double(tGeom, "superellipse_exponent"));
                    mGENParameters(1)(tGeoCount).set("superellipse_scaling", Plato::Get::Double(tGeom, "superellipse_scaling"));
                    mGENParameters(1)(tGeoCount).set("superellipse_regularization", Plato::Get::Double(tGeom, "superellipse_regularization"));
                }
                else
                {
                    mGENParameters(1).push_back(moris::prm::create_geometry_parameter_list());
                    mGENParameters(1)(tGeoCount).set("type", Plato::Get::String(tGeom, "type"));
                    mGENParameters(1)(tGeoCount).set("constant_parameters", Plato::Get::String(tGeom, "constant_parameters"));
                    if (mVerbose)
                    {
                        moris::prm::print(mGENParameters(1)(tGeoCount));
                    }
                }

                if (tDesign)
                {
                    mGENParameters(1)(tGeoCount).set("discretization_mesh_index", 0);     // Index of B-spline mesh to create level set field on (-1 = none)
                    mGENParameters(1)(tGeoCount).set("discretization_lower_bound", -3.0); // Lower bound of level set field (if bspline_mesh_index >= 0)
                    mGENParameters(1)(tGeoCount).set("discretization_upper_bound", 3.0);  // Upper bound of level set field (if bspline_mesh_index >= 0)
                    if (mVerbose)
                    {
                        moris::prm::print(mGENParameters(1)(tGeoCount));
                    }
                }

                tGeoCount++;
            }
        }
    }
    void
    XTKApplication::setup_local_operations()
    {
        // Application File Parsing
        MORIS_ERROR(!mAppfileData.empty(), "Empty Application file provided to XTK");
        for (auto tNode : mAppfileData.getByName<Plato::InputData>("Operation"))
        {
            std::string tStrName = Plato::Get::String(tNode, "Name");
            std::string tStrFunction = Plato::Get::String(tNode, "Function");

            std::vector<std::string> tFunctions;

            tFunctions.push_back("Generate_Model_Op");
            if (tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new PlatoXTK::Generate_Model_Op(this, tNode);
                continue;
            }

            tFunctions.push_back("Objective_Sensitivity");
            if (tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new PlatoXTK::Sensitivity(this, tNode);
                continue;
            }

            tFunctions.push_back("Constraint_Sensitivity");
            if (tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new PlatoXTK::Sensitivity(this, tNode);
                continue;
            }

            tFunctions.push_back("Load_From_HDF5");
            if (tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new PlatoXTK::Load_From_HDF5(this, tNode);
                continue;
            }
            tFunctions.push_back("Save_To_HDF5");
            if (tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new PlatoXTK::Save_To_HDF5(this, tNode);
                continue;
            }

            tFunctions.push_back("Compute_Bounds");
            if (tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new PlatoXTK::Compute_Bounds(this, tNode);
                continue;
            }

            std::stringstream tMessage;
            tMessage << "Cannot find specified Function: " << tStrFunction << std::endl;
            tMessage << "Available Functions: " << std::endl;
            for (auto tMyFunction : tFunctions)
            {
                tMessage << tMyFunction << std::endl;
            }
            Plato::ParsingException tParsingException(tMessage.str());
            throw tParsingException;
        }
    }

    //-------------------------------------------------------------------------------------

}
