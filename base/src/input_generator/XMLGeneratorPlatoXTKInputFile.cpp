/*
 * XMLGeneratorPlatoXTKInputFile.cpp
 *
 *  Created on: July 21, 2021
 */

#include "pugixml.hpp"
#include "exception_handling.hpp"

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorPlatoXTKInputFile.hpp"
#include "XMLGeneratorPlatoAnalyzeInputFileUtilities.hpp"

namespace XMLGen
{

std::string
format_input_string_for_xtk_xml( std::string aInputDeckString )
{
    std::string tOutputString = aInputDeckString;
    for ( unsigned int i = 0; i < tOutputString.length(); i++ )
    {
        if ( tOutputString[i] == ' ' )
        {
            tOutputString[i] = ',';
        }
    }

    return tOutputString;
}

void
write_xtk_input_deck_file( const XMLGen::InputData &aInputData )
{
    if ( aInputData.optimization_parameters().discretization() == "levelset" )
    {

        pugi::xml_document doc;
        pugi::xml_node     tNode1 = doc.append_child( pugi::node_declaration );
        tNode1.set_name( "xml" );
        pugi::xml_attribute tmp_att = tNode1.append_attribute( "version" );
        tmp_att.set_value( "1.0" );

        // XTK specific parameters
        tNode1 = doc.append_child( "XTK_Parameters" );
        addChild( tNode1, "workflow", "STK" );// file that goes to analyze
        addChild( tNode1, "output_file", aInputData.mesh.run_name );// file that goes to analyze
        addChild( tNode1, "output_path", "./" );// file that goes to analyze

        setup_union_set_parameters( aInputData, tNode1 );

        // Geometry Parameters
        tNode1 = doc.append_child( "GEN_Parameters" );
        addChild( tNode1, "IQI_types", "Strain_Energy,Volume" );// dummy iqis
        addChild( tNode1, "isocontour_threshold", "0.0" );
        addChild( tNode1, "isocontour_tolerance", "1e-12" );
        addChild( tNode1, "num_phases", "2" );

        if ( aInputData.optimization_parameters().levelset_initialization_method() != "swiss_cheese" )
        {
            THROWERR( std::string( "Only swiss_cheese level set initialization method currently supported." ) );
        }

        auto tGeomNode = tNode1.append_child( "Geometry" );
        addChild( tGeomNode, "design", "true" );
        addChild( tGeomNode, "type", "swiss_cheese_slice" );
        addChild( tGeomNode, "lower_bounds", XMLGen::format_input_string_for_xtk_xml( aInputData.optimization_parameters().levelset_material_box_min() ) );
        addChild( tGeomNode, "upper_bounds", XMLGen::format_input_string_for_xtk_xml( aInputData.optimization_parameters().levelset_material_box_max() ) );
        addChild( tGeomNode, "semidiameters", XMLGen::format_input_string_for_xtk_xml( aInputData.optimization_parameters().levelset_sphere_radius() ) );
        addChild( tGeomNode, "num_holes", XMLGen::format_input_string_for_xtk_xml( aInputData.optimization_parameters().levelset_sphere_packing_factor() ) );
        addChild( tGeomNode, "superellipse_exponent", "2" );
        addChild( tGeomNode, "superellipse_scaling", "0.1" );
        addChild( tGeomNode, "superellipse_regularization", "0.0" );

        tNode1 = doc.append_child( "STK_Parameters" );
        addChild( tNode1, "mesh_file", aInputData.mesh.name );

        doc.save_file( "plato_xtk_input_deck.xml", "  " );
    }
}

void
setup_union_set_parameters( const XMLGen::InputData &aInputData,
    pugi::xml_node &                                 aXTKParamNodes )
{

    std::string tUnionSSStr      = "";
    std::string tUnionSSColorStr = "";
    std::string tUnionSSNamesStr = "";// semicolon deliminated
    std::string tKeepSSStr       = "";// comma deliminated
    for ( auto &tEBC : aInputData.ebcs )
    {
        if ( tEBC.location_type() != "sideset" )
        {
            THROWERR( std::string( "Invalid essential boundary condition location_type: must be sideset for levelset discretization" ) );
        }
        std::cout << "location_name = " << tEBC.location_name() << std::endl;
        std::string tLocationName = tEBC.location_name();

        tUnionSSStr.append( tLocationName + "_n_p1," + tLocationName + "_c_p1;" );
        tUnionSSColorStr.append( "1;" );
        tUnionSSNamesStr.append( tLocationName + ";" );
        tKeepSSStr.append( tLocationName + "," );
    }

    for ( auto &tLoads : aInputData.loads )
    {
        if ( tLoads.location_type() != "sideset" )
        {
            THROWERR( std::string( "Invalid load location_type: must be sideset for levelset discretization" ) );
        }
        std::string tLocationName = tLoads.location_name();

        tUnionSSStr.append( tLocationName + "_n_p1," + tLocationName + "_c_p1;" );
        tUnionSSColorStr.append( "1;" );
        tUnionSSNamesStr.append( tLocationName + ";" );
        tKeepSSStr.append( tLocationName + "," );
    }

    // remove the comma from last entries
    tUnionSSStr.pop_back();
    tUnionSSNamesStr.pop_back();
    tUnionSSColorStr.pop_back();
    tKeepSSStr.pop_back();

    addChild( aXTKParamNodes, "union_side_sets", tUnionSSStr );
    addChild( aXTKParamNodes, "union_side_set_names", tUnionSSNamesStr );
    addChild( aXTKParamNodes, "union_side_set_colors", tUnionSSColorStr );
    addChild( aXTKParamNodes, "deactivate_all_but_side_sets", tKeepSSStr );

    std::string tUnionMaterialBlockStr = "";
    std::string tUnionVoidBlockStr     = "";
    std::string tUnionBlockColorStr    = "1;1";
    std::string tUnionBlockNamesStr    = "block_1;block_2";// semicolon deliminated
    std::string tKeepBlockStr          = "block_1,block_2";// comma deliminated

    for ( auto &tBlock : aInputData.blocks )
    {
        std::string tBlockName = tBlock.name;

        std::cout << "tBlockName = " << tBlockName << std::endl;

        tUnionMaterialBlockStr.append( tBlockName + "_n_p1," + tBlockName + "_c_p1," );
        tUnionVoidBlockStr.append( tBlockName + "_n_p0," + tBlockName + "_c_p0," );
    }

    // remove the comma from last entries
    tUnionMaterialBlockStr.pop_back();
    tUnionVoidBlockStr.pop_back();

    if ( tUnionMaterialBlockStr.empty() || tUnionVoidBlockStr.empty() )
    {
        THROWERR( "Empty string detected. Levelset discretization requires at least one block to be present in the input file" );
    }

    // combine
    std::string tUnionBlockStr = std::string( tUnionMaterialBlockStr + ";" + tUnionVoidBlockStr );

    addChild( aXTKParamNodes, "union_blocks", tUnionBlockStr );
    addChild( aXTKParamNodes, "union_block_names", tUnionBlockNamesStr );
    addChild( aXTKParamNodes, "union_block_colors", tUnionBlockColorStr );
    addChild( aXTKParamNodes, "deactivate_all_but_blocks", tKeepBlockStr );
}

}// namespace XMLGen
