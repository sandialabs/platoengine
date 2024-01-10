
#include <gtest/gtest.h>

#include "pugixml.hpp"
#include "XMLGeneratorGradBasedOptimizerOptions.hpp"
#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorOptimizationParametersMetadata.hpp"
#include "XMLGeneratorParseMethodInputOptionsUtilities.hpp"
#include "XMLGenerator_UnitTester_Tools.hpp"

namespace PlatoTestXMLGenerator
{
TEST(PlatoTestXMLGenerator, InsertLevelsetBasedShapeOptimizationInputs)
{
    XMLGen::MetaDataTags tTags;
    XMLGen::insert_plato_levelset_input_options(tTags);
    EXPECT_EQ(7u, tTags.size());

    std::unordered_map<std::string, std::string> tGoldValues = { {"levelset_nodesets",""}, {"levelset_sphere_radius",""}, 
        {"create_levelset_spheres", ""}, {"levelset_material_box_min", ""}, {"levelset_material_box_max", ""},
        {"levelset_sphere_packing_factor", ""}, {"levelset_initialization_method", ""} };
    for(auto& tPair : tTags)
    {
        // TEST INPUT KEYWORDS
        auto tGoldItr = tGoldValues.find(tPair.first);
        ASSERT_FALSE(tGoldItr == tGoldValues.end());
        EXPECT_STREQ(tPair.first.c_str(), tGoldItr->first.c_str());

        // TEST DEFAULT VALUES
        EXPECT_STREQ(tPair.second.second.c_str(), tGoldItr->second.c_str());
    }
}

TEST(PlatoTestXMLGenerator, InsertProjectionFilterInputs)
{
    XMLGen::MetaDataTags tTags;
    XMLGen::insert_plato_projection_filter_input_options(tTags);
    EXPECT_EQ(8u, tTags.size());

    std::unordered_map<std::string, std::string> tGoldValues = { {"projection_type", ""}, {"filter_heaviside_min",""}, 
        {"filter_heaviside_max",""}, {"filter_heaviside_scale", ""}, {"filter_heaviside_update", ""}, {"filter_use_additive_continuation", ""},
        {"filter_projection_start_iteration", ""}, {"filter_projection_update_interval", ""} };
    for(auto& tPair : tTags)
    {
        // TEST INPUT KEYWORDS
        auto tGoldItr = tGoldValues.find(tPair.first);
        ASSERT_FALSE(tGoldItr == tGoldValues.end());
        EXPECT_STREQ(tPair.first.c_str(), tGoldItr->first.c_str());

        // TEST DEFAULT VALUES
        EXPECT_STREQ(tPair.second.second.c_str(), tGoldItr->second.c_str());
    }
}

TEST(PlatoTestXMLGenerator, InsertSymmetryFilterInputs)
{
    XMLGen::MetaDataTags tTags;
    XMLGen::insert_plato_symmetry_filter_input_options(tTags);
    EXPECT_EQ(5u, tTags.size());

    std::unordered_map<std::string, std::string> tGoldValues = { {"symmetry_plane_normal",""}, {"symmetry_plane_origin",""}, 
        {"mesh_map_filter_radius", ""}, {"filter_before_symmetry_enforcement", ""}, {"mesh_map_search_tolerance", ""} };
    for(auto& tPair : tTags)
    {
        // TEST INPUT KEYWORDS
        auto tGoldItr = tGoldValues.find(tPair.first);
        ASSERT_FALSE(tGoldItr == tGoldValues.end());
        EXPECT_STREQ(tPair.first.c_str(), tGoldItr->first.c_str());

        // TEST DEFAULT VALUES
        EXPECT_STREQ(tPair.second.second.c_str(), tGoldItr->second.c_str());
    }
}

TEST(PlatoTestXMLGenerator, InsertPlatoFilterInputs)
{
    XMLGen::MetaDataTags tTags;
    XMLGen::insert_plato_filter_input_options(tTags);
    EXPECT_EQ(12u, tTags.size());

    std::unordered_map<std::string, std::string> tGoldValues = { {"filter_type","kernel"}, {"filter_service",""}, 
        {"filter_in_engine", "true"}, {"filter_radius_scale", "2.0"}, {"filter_radius_absolute", ""},
        {"filter_power", "1"}, {"filter_type_identity_generator_name", "identity"}, {"filter_type_kernel_generator_name", "kernel"},
        {"filter_type_kernel_then_heaviside_generator_name", "kernel_then_heaviside"}, 
        {"filter_type_kernel_then_tanh_generator_name", "kernel_then_tanh"}, {"boundary_sticking_penalty", "1.0"}, 
        {"symmetry_plane_location_names", ""} };
    for(auto& tPair : tTags)
    {
        // TEST INPUT KEYWORDS
        auto tGoldItr = tGoldValues.find(tPair.first);
        ASSERT_FALSE(tGoldItr == tGoldValues.end());
        EXPECT_STREQ(tPair.first.c_str(), tGoldItr->first.c_str());

        // TEST DEFAULT VALUES
        EXPECT_STREQ(tPair.second.second.c_str(), tGoldItr->second.c_str());
    }
}

TEST(PlatoTestXMLGenerator, InsertFixedBlocksInputs)
{
    XMLGen::MetaDataTags tTags;
    XMLGen::insert_fixed_blocks_input_options(tTags);
    EXPECT_EQ(6u, tTags.size());

    std::unordered_map<std::string, std::string> tGoldValues = { {"fixed_block_ids",""}, {"fixed_sideset_ids",""}, 
        {"fixed_nodeset_ids", ""}, {"fixed_block_domain_values", ""}, {"fixed_block_boundary_values", ""},
        {"fixed_block_material_states", ""} };
    for(auto& tPair : tTags)
    {
        // TEST INPUT KEYWORDS
        auto tGoldItr = tGoldValues.find(tPair.first);
        ASSERT_FALSE(tGoldItr == tGoldValues.end());
        EXPECT_STREQ(tPair.first.c_str(), tGoldItr->first.c_str());

        // TEST DEFAULT VALUES
        EXPECT_STREQ(tPair.second.second.c_str(), tGoldItr->second.c_str());
    }
}

TEST(PlatoTestXMLGenerator, InsertPruneAndRefineInputs)
{
    XMLGen::MetaDataTags tTags;
    XMLGen::insert_prune_and_refine_input_options(tTags);
    EXPECT_EQ(6u, tTags.size());

    std::unordered_map<std::string, std::string> tGoldValues = { {"prune_mesh",""}, {"prune_threshold",""},{"number_refines",""}, 
        {"number_buffer_layers", ""}, {"prune_and_refine_path", ""}, {"number_prune_and_refine_processors", ""} };
    for(auto& tPair : tTags)
    {
        // TEST INPUT KEYWORDS
        auto tGoldItr = tGoldValues.find(tPair.first);
        ASSERT_FALSE(tGoldItr == tGoldValues.end());
        EXPECT_STREQ(tPair.first.c_str(), tGoldItr->first.c_str());

        // TEST DEFAULT VALUES
        EXPECT_STREQ(tPair.second.second.c_str(), tGoldItr->second.c_str());
    }
}

TEST(PlatoTestXMLGenerator, InsertRestartInputs)
{
    XMLGen::MetaDataTags tTags;
    XMLGen::insert_restart_input_options(tTags);
    EXPECT_EQ(4u, tTags.size());

    std::unordered_map<std::string, std::string> tGoldValues = { {"restart_iteration","0"}, 
        {"write_restart_file","false"}, {"initial_guess_file_name", ""}, {"initial_guess_field_name", ""} };
    for(auto& tPair : tTags)
    {
        // TEST INPUT KEYWORDS
        auto tGoldItr = tGoldValues.find(tPair.first);
        ASSERT_FALSE(tGoldItr == tGoldValues.end());
        EXPECT_STREQ(tPair.first.c_str(), tGoldItr->first.c_str());

        // TEST DEFAULT VALUES
        EXPECT_STREQ(tPair.second.second.c_str(), tGoldItr->second.c_str());
    }
}

TEST(PlatoTestXMLGenerator, InsertRolInputs)
{
    XMLGen::MetaDataTags tTags;
    XMLGen::insert_rol_input_options(tTags);
    EXPECT_EQ(10u, tTags.size());

    std::unordered_map<std::string, std::string> tGoldValues = { {"rol_subproblem_model", ""}, 
                                                                 {"reset_algorithm_on_update","false"}, 
                                                                 {"rol_lin_more_cauchy_initial_step_size", "3.0"} ,
                                                                 {"rol_gradient_check_perturbation_scale", "1.0"} ,
                                                                 {"rol_gradient_check_steps", "12"},
                                                                 {"rol_gradient_check_random_seed", ""},
                                                                 {"rol_initial_trust_region_radius", "1.5e1"},
                                                                 {"rol_gradient_tolerance", "1e-10"},
                                                                 {"rol_constraint_tolerance", "1e-10"},
                                                                 {"rol_step_tolerance", "1e-14"}
                                                                 };
    for(auto& tPair : tTags)
    {
        // TEST INPUT KEYWORDS
        auto tGoldItr = tGoldValues.find(tPair.first);
        ASSERT_FALSE(tGoldItr == tGoldValues.end());
        EXPECT_STREQ(tPair.first.c_str(), tGoldItr->first.c_str());

        // TEST DEFAULT VALUES
        EXPECT_STREQ(tPair.second.second.c_str(), tGoldItr->second.c_str());
    }
}

TEST(PlatoTestXMLGenerator, AppendROLInitialTrustRegionRadius)
{
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("rol_initial_trust_region_radius", "3e-2");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_initial_trust_region_radius(tMetaData, tDocument);

    auto tNode = tDocument.child("Parameter");
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Initial Radius", "double", "3e-2"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tNode);
}

TEST(PlatoTestXMLGenerator, TestROLInputMetadataGetters)
{
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("rol_initial_trust_region_radius", "3e-2");
    tOptimizationParameters.append("rol_gradient_tolerance", "1.2e-3");
    tOptimizationParameters.append("rol_constraint_tolerance", "4.7e-9");
    tOptimizationParameters.append("rol_step_tolerance", "7.1e-14");
    EXPECT_STREQ("3e-2", tOptimizationParameters.rol_initial_trust_region_radius().c_str());
    EXPECT_STREQ("1.2e-3", tOptimizationParameters.rol_gradient_tolerance().c_str());
    EXPECT_STREQ("4.7e-9", tOptimizationParameters.rol_constraint_tolerance().c_str());
    EXPECT_STREQ("7.1e-14", tOptimizationParameters.rol_step_tolerance().c_str());
}

TEST(PlatoTestXMLGenerator, AppendROLTolerances)
{
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("rol_gradient_tolerance", "1.2e-3");
    tOptimizationParameters.append("rol_constraint_tolerance", "4.7e-9");
    tOptimizationParameters.append("rol_step_tolerance", "7.1e-14");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_rol_tolerances(tMetaData, tDocument);

    auto tNode = tDocument.child("Parameter");
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Gradient Tolerance", "double", "1.2e-3"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tNode);

    tNode = tNode.next_sibling("Parameter");
    tValues = {"Constraint Tolerance", "double", "4.7e-9"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tNode);

    tNode = tNode.next_sibling("Parameter");
    tValues = {"Step Tolerance", "double", "7.1e-14"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tNode);
}

TEST(PlatoTestXMLGenerator, InsertDerivativeCheckerInputs)
{
    XMLGen::MetaDataTags tTags;
    XMLGen::insert_derivative_checker_input_options(tTags);
    EXPECT_EQ(2u, tTags.size());

    std::unordered_map<std::string, std::string> tGoldValues = { {"check_hessian","false"}, {"check_gradient","false"} };
    for(auto& tPair : tTags)
    {
        // TEST INPUT KEYWORDS
        auto tGoldItr = tGoldValues.find(tPair.first);
        ASSERT_FALSE(tGoldItr == tGoldValues.end());
        EXPECT_STREQ(tPair.first.c_str(), tGoldItr->first.c_str());

        // TEST DEFAULT VALUES
        EXPECT_STREQ(tPair.second.second.c_str(), tGoldItr->second.c_str());
    }
}

TEST(PlatoTestXMLGenerator, InsertGeneralOptimizationInputs)
{
    XMLGen::MetaDataTags tTags;
    XMLGen::insert_general_optimization_input_options(tTags);
    EXPECT_EQ(14u, tTags.size());

    std::unordered_map<std::string, std::string> tGoldValues = { {"max_iterations",""}, {"verbose", "false"}, {"output_method", "epu"},
        {"output_frequency", "5"}, {"optimization_type", "topology"}, {"optimization_algorithm", "oc"}, 
        {"normalize_in_aggregator", ""}, {"problem_update_frequency", "5"}, {"objective_number_standard_deviations", ""},
        {"descriptors", ""}, {"lower_bounds", ""}, {"upper_bounds", ""}, {"hessian_type", ""}, {"limited_memory_storage", "8"} };
    for(auto& tPair : tTags)
    {
        // TEST INPUT KEYWORDS
        auto tGoldItr = tGoldValues.find(tPair.first);
        ASSERT_FALSE(tGoldItr == tGoldValues.end());
        EXPECT_STREQ(tPair.first.c_str(), tGoldItr->first.c_str());

        // TEST DEFAULT VALUES
        EXPECT_STREQ(tPair.second.second.c_str(), tGoldItr->second.c_str());
    }
}

TEST(PlatoTestXMLGenerator, InsertShapeOptimizationInputs)
{
    XMLGen::MetaDataTags tTags;
    XMLGen::insert_shape_optimization_input_options(tTags);
    EXPECT_EQ(4u, tTags.size());

    std::unordered_map<std::string, std::string> tGoldValues = { {"csm_file", ""}, {"num_shape_design_variables", ""}, {"esp_workflow", "aflr4_aflr3"}, {"mesh_morph", "false"} };
    for(auto& tPair : tTags)
    {
        // TEST INPUT KEYWORDS
        auto tGoldItr = tGoldValues.find(tPair.first);
        ASSERT_FALSE(tGoldItr == tGoldValues.end());
        EXPECT_STREQ(tPair.first.c_str(), tGoldItr->first.c_str());

        // TEST DEFAULT VALUES
        EXPECT_STREQ(tPair.second.second.c_str(), tGoldItr->second.c_str());
    }
}

TEST(PlatoTestXMLGenerator, InsertTopologyOptimizationInputs)
{
    XMLGen::MetaDataTags tTags;
    XMLGen::insert_topology_optimization_input_options(tTags);
    EXPECT_EQ(3u, tTags.size());

    std::unordered_map<std::string, std::string> tGoldValues = { {"enforce_bounds", "false"}, 
        {"discretization", "density"}, {"initial_density_value", "0.5"} };
    for(auto& tPair : tTags)
    {
        // TEST INPUT KEYWORDS
        auto tGoldItr = tGoldValues.find(tPair.first);
        ASSERT_FALSE(tGoldItr == tGoldValues.end());
        EXPECT_STREQ(tPair.first.c_str(), tGoldItr->first.c_str());

        // TEST DEFAULT VALUES
        EXPECT_STREQ(tPair.second.second.c_str(), tGoldItr->second.c_str());
    }
}
}
