/*
 * XMLGeneratorXTKOperationFile_UnitTester.cpp  
 * 
 *  Created on: Oct  07, 2021 
 *      Author: Keenan Doble
 */

#include <gtest/gtest.h>
#include "XMLGeneratorUtilities.hpp"
#include "XMLGenerator_UnitTester_Tools.hpp"
#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorPlatoXTKOperationFile.hpp"
#include "pugixml.hpp"
#include <fstream>

TEST(PlatoTestXMLGenerator, XMLGXTKOperations)
{
    Plato::system("rm plato_xtk_operations.xml");

    XMLGen::InputData tMetaData;

    // Since the meta data uses a density discretization by default, this operation should not output a file
    EXPECT_NO_THROW(write_xtk_operations_file(tMetaData));
    // std::stringstream tReadData = XMLGen::read_data_from_file("plato_xtk_operations.xml");
    // std::cout << "first tReadData.str() = " << tReadData.str() << std::endl;
    // ASSERT_TRUE(tReadData.str().empty());

    // // specify the meta data
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "levelset");
    tOptimizationParameters.append("optimization_type", "topology");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    tMetaData.mesh.run_name = "xmlg_xtk_run_mesh.exo";
    tMetaData.mesh.name = "xmlg_xtk_mesh.exo";

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

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;
    
    XMLGen::Constraint tConstraint;
    tConstraint.id("1");
    tConstraint.criterion("3");
    tConstraint.service("1");
    tMetaData.constraints.push_back(tConstraint);


    EXPECT_NO_THROW(write_xtk_operations_file(tMetaData));

    std::stringstream tReadData = XMLGen::read_data_from_file("plato_xtk_operations.xml");
    ASSERT_TRUE(!tReadData.str().empty());

    std::cout << "Second tReadData.str() = " << tReadData.str() << std::endl;

    // std::string tGoldString = std::string("<?xmlversion=\"1.0\"?><XTK_Parameters><workflow>STK</workflow><output_file>xmlg_xtk_run_mesh.exo</output_file><output_path>./</output_path><union_side_sets>ss_1_n_p1,ss_1_c_p1;ss_2_n_p1,ss_2_c_p1</union_side_sets><union_side_set_names>ss_1;ss_2</union_side_set_names><union_side_set_colors>1;1</union_side_set_colors><deactivate_all_but_side_sets>ss_1,ss_2</deactivate_all_but_side_sets><union_blocks>block_1_n_p1,block_1_c_p1,block_2_n_p1,block_2_c_p1;block_1_n_p0,block_1_c_p0,block_2_n_p0,block_2_c_p0</union_blocks><union_block_names>block_1;block_2</union_block_names><union_block_colors>1;1</union_block_colors><deactivate_all_but_blocks>block_1,block_2</deactivate_all_but_blocks></XTK_Parameters><GEN_Parameters><IQI_types>Strain_Energy,Volume</IQI_types><isocontour_threshold>0.0</isocontour_threshold><isocontour_tolerance>1e-12</isocontour_tolerance><num_phases>2</num_phases><Geometry><design>true</design><type>swiss_cheese_slice</type><lower_bounds>0.0,-1.5,-0.25</lower_bounds><upper_bounds>4.25,1.5,0.25</upper_bounds><semidiameters>0.211,0.211,4.11</semidiameters><num_holes>8,6,2</num_holes><superellipse_exponent>2</superellipse_exponent><superellipse_scaling>0.1</superellipse_scaling><superellipse_regularization>0.0</superellipse_regularization></Geometry></GEN_Parameters><STK_Parameters><mesh_file>xmlg_xtk_mesh.exo</mesh_file></STK_Parameters>");

    // ASSERT_STREQ(tGoldString.c_str(), tReadData.str().c_str());
    // Plato::system("rm plato_xtk_operations.xml");
}
