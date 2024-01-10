/*
 * XMLGeneratorPlatoAnalyzeInputFile_UnitTester.cpp
 *
 *  Created on: Jun 8, 2020
 */

#include <gtest/gtest.h>
#include "XMLGeneratorUtilities.hpp"
#include "XMLGenerator_UnitTester_Tools.hpp"
#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorPlatoXTKInputFile.hpp"
#include <fstream>

TEST(PlatoTestXMLGenerator, XMLGXTKInputFile)
{
    XMLGen::InputData tMetaData;

    // Create services
    XMLGen::Service tService;
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("3");
    tService.code("xtk");
    tMetaData.append(tService);

    // mesh information
    tMetaData.mesh.run_name = "xmlg_xtk_run_mesh.exo";
    tMetaData.mesh.name = "xmlg_xtk_mesh.exo";
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("max_iterations", "10");
    tOptimizationParameters.append("filter_radius_scale", "2.0");
    tOptimizationParameters.append("discretization", "levelset");
    tOptimizationParameters.append("levelset_initialization_method", "swiss_cheese");
    tOptimizationParameters.append("levelset_sphere_packing_factor", "8 6 2");
    tOptimizationParameters.append("levelset_sphere_radius", "0.211 0.211 4.11");
    tOptimizationParameters.append("levelset_material_box_min", "0.0 -1.5 -0.25");
    tOptimizationParameters.append("levelset_material_box_max", "4.25 1.5 0.25");

    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tOptimizationParameters.filterInEngine(true);
    tOptimizationParameters.normalizeInAggregator(true);
    tMetaData.set(tOptimizationParameters);

    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("location_name", "ss_1");
    tBC.property("location_type", "sideset");
    tBC.property("type", "fixed_value");
    tMetaData.ebcs.push_back(tBC);

    XMLGen::Load tLoad1;
    tLoad1.type("traction");
    tLoad1.location_type("sideset");
    tLoad1.id("1");
    tLoad1.location_name("ss_2");
    std::vector<std::string> tValues = {"0.0", "-3e3", "0"};
    tLoad1.load_values(tValues);
    tMetaData.loads.push_back(tLoad1);

    // expect throw here since we need block information in XTK
    EXPECT_THROW(write_xtk_input_deck_file(tMetaData), std::exception);

    XMLGen::Block tBlock1;
    tBlock1.name = "block_1";
    tBlock1.material_id = "1";
    tMetaData.blocks.push_back(tBlock1);

    XMLGen::Block tBlock2;
    tBlock2.name = "block_2";
    tBlock2.material_id = "2";
    tMetaData.blocks.push_back(tBlock2);
    EXPECT_NO_THROW(write_xtk_input_deck_file(tMetaData));

    auto tReadData = XMLGen::read_data_from_file("plato_xtk_input_deck.xml");
    std::string tGoldString = std::string("<?xmlversion=\"1.0\"?><XTK_Parameters><workflow>STK</workflow><output_file>xmlg_xtk_run_mesh.exo</output_file><output_path>./</output_path><union_side_sets>ss_1_n_p1,ss_1_c_p1;ss_2_n_p1,ss_2_c_p1</union_side_sets><union_side_set_names>ss_1;ss_2</union_side_set_names><union_side_set_colors>1;1</union_side_set_colors><deactivate_all_but_side_sets>ss_1,ss_2</deactivate_all_but_side_sets><union_blocks>block_1_n_p1,block_1_c_p1,block_2_n_p1,block_2_c_p1;block_1_n_p0,block_1_c_p0,block_2_n_p0,block_2_c_p0</union_blocks><union_block_names>block_1;block_2</union_block_names><union_block_colors>1;1</union_block_colors><deactivate_all_but_blocks>block_1,block_2</deactivate_all_but_blocks></XTK_Parameters><GEN_Parameters><IQI_types>Strain_Energy,Volume</IQI_types><isocontour_threshold>0.0</isocontour_threshold><isocontour_tolerance>1e-12</isocontour_tolerance><num_phases>2</num_phases><Geometry><design>true</design><type>swiss_cheese_slice</type><lower_bounds>0.0,-1.5,-0.25</lower_bounds><upper_bounds>4.25,1.5,0.25</upper_bounds><semidiameters>0.211,0.211,4.11</semidiameters><num_holes>8,6,2</num_holes><superellipse_exponent>2</superellipse_exponent><superellipse_scaling>0.1</superellipse_scaling><superellipse_regularization>0.0</superellipse_regularization></Geometry></GEN_Parameters><STK_Parameters><mesh_file>xmlg_xtk_mesh.exo</mesh_file></STK_Parameters>");

    ASSERT_STREQ(tGoldString.c_str(), tReadData.str().c_str());
    Plato::system("rm plato_xtk_input_deck");
}
