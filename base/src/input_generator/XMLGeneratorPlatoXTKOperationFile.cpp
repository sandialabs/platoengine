#include "pugixml.hpp"

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorPlatoXTKOperationFile.hpp"
#include "Plato_Exceptions.hpp"

namespace XMLGen
{
    void write_xtk_operations_file(const XMLGen::InputData &aInputData)
    {
        if (aInputData.optimization_parameters().optimization_type() == "topology" && aInputData.optimization_parameters().discretization() == "levelset")
        {
            pugi::xml_document tDoc;

            pugi::xml_node tNode1 = tDoc.append_child(pugi::node_declaration);
            tNode1.set_name("xml");
            pugi::xml_attribute tAtt = tNode1.append_attribute("version");
            tAtt.set_value("1.0");

            // generate model
            tNode1 = tDoc.append_child("Operation");
            addChild(tNode1, "Name", "Update Problem");
            addChild(tNode1, "Function", "Generate_Model_Op");

            auto tOptions = tNode1.append_child("Operation");
            addChild(tOptions, "compute_sens", "true");
            addChild(tOptions, "output_mesh", "true");
            addChild(tOptions, "output_mesh_file", aInputData.mesh.run_name);
            addChild(tOptions, "verbose", "false");

            auto tInput = tNode1.append_child("Input");
            addChild(tInput, "SharedDataName", "Topology");
            addChild(tInput, "ArgumentName", "Topology");
            addChild(tInput, "Layout", "Scalar Field");

            auto tOutput = tNode1.append_child("Output");
            addChild(tOutput, "SharedDataName", "Initial Control");
            addChild(tOutput, "Layout", "Scalar Field");

            tNode1 = tDoc.append_child("Operation");
            addChild(tNode1, "Name", "Compute Upper Bounds");
            addChild(tNode1, "Function", "Compute_Bounds");

            tOptions = tNode1.append_child("Options");
            addChild(tOptions, "val", "0.75");

            tOutput = tNode1.append_child("Output");
            addChild(tOutput, "SharedDataName", "Upper Bound Vector");
            addChild(tOutput, "Layout", "Scalar Field");

            tNode1 = tDoc.append_child("Operation");
            addChild(tNode1, "Name", "Compute Lower Bounds");
            addChild(tNode1, "Function", "Compute_Bounds");

            tOptions = tNode1.append_child("Options");
            addChild(tOptions, "val", "-0.75");

            tOutput = tNode1.append_child("Output");
            addChild(tOutput, "SharedDataName", "Lower Bound Vector");
            addChild(tOutput, "Layout", "Scalar Field");

            // Objective Sensitivity Operation
            tNode1 = tDoc.append_child("Operation");
            addChild(tNode1, "Name", "Compute Objective Gradient XTK");
            addChild(tNode1, "Function", "Objective_Sensitivity");
            addChild(tNode1, "GradientX", "Objective GradientX X");
            addChild(tNode1, "GradientY", "Objective GradientX Y");
            addChild(tNode1, "GradientZ", "Objective GradientX Z");
            addChild(tNode1, "vec_field_base", "dfdx");

            tOutput = tNode1.append_child("Output");
            addChild(tOutput, "SharedDataName", "TODO - Criterion Gradient - criterion_1_service_2_scenario_1");
            addChild(tOutput, "Layout", "Scalar Field");

            if (aInputData.constraints.size() != 1)
            {
                throw Plato::ParsingException("Implementation Currently only excepts a single constraint");
            }

            // Objective Sensitivity Operation
            tNode1 = tDoc.append_child("Operation");
            addChild(tNode1, "Name", "Compute Constraint Gradient 1 XTK");
            addChild(tNode1, "Function", "Constraint_Sensitivity");
            addChild(tNode1, "GradientX", "Constraint GradientX X");
            addChild(tNode1, "GradientY", "Constraint GradientX Y");
            addChild(tNode1, "GradientZ", "Constraint GradientX Z");
            addChild(tNode1, "vec_field_base", "dhdx");
            tOutput = tNode1.append_child("Output");

            std::string tCriterionID = aInputData.constraints[0].criterion();
            std::string tServiceID = aInputData.constraints[0].service();
            std::string tScenarioID = aInputData.constraints[0].scenario();
            ConcretizedCriterion tConcretizedCriterion(tCriterionID, tServiceID, tScenarioID);
            auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
            auto tSharedDataName = std::string("Criterion Gradient - ") + tIdentifierString;
            addChild(tOutput, "SharedDataName", tSharedDataName);
            addChild(tOutput, "Layout", "Scalar Field");

            // load sensitivities from platoanalyze via hdf
            tNode1 = tDoc.append_child("Operation");
            addChild(tNode1, "Name", "Load Objective GradientX From HDF5");
            addChild(tNode1, "Function", "Load_From_HDF5");
            addChild(tNode1, "FileName", "./Objective_Gradx.hdf");
            tInput = tNode1.append_child("Input");
            addChild(tInput, "SharedDataName", "Objective GradientX X");
            addChild(tInput, "Layout", "Node");
            tInput = tNode1.append_child("Input");
            addChild(tInput, "SharedDataName", "Objective GradientX Y");
            addChild(tInput, "Layout", "Node");
            tInput = tNode1.append_child("Input");
            addChild(tInput, "SharedDataName", "Objective GradientX Z");
            addChild(tInput, "Layout", "Node");

            // load sensitivities from platoanalyze via hdf
            std::string tFileName = "./Constraint_Gradx_id_" + aInputData.constraints[0].id() + ".hdf";
            tNode1 = tDoc.append_child("Operation");
            addChild(tNode1, "Name", "Load Constraint GradientX From HDF5");
            addChild(tNode1, "Function", "Load_From_HDF5");
            addChild(tNode1, "FileName", tFileName);
            tInput = tNode1.append_child("Input");
            addChild(tInput, "SharedDataName", "Constraint GradientX X");
            addChild(tInput, "Layout", "Node");
            tInput = tNode1.append_child("Input");
            addChild(tInput, "SharedDataName", "Constraint GradientX Y");
            addChild(tInput, "Layout", "Node");
            tInput = tNode1.append_child("Input");
            addChild(tInput, "SharedDataName", "Constraint GradientX Z");
            addChild(tInput, "Layout", "Node");

            tDoc.save_file("plato_xtk_operations.xml", "  ");
        }
    }
}
// namespace XMLGen
