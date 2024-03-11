/*
 * XMLGeneratorGradBasedOptimizerOptions.cpp
 *
 *  Created on: Jan 17, 2022
 */

#include "XMLG_Macros.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorServiceUtilities.hpp"
#include "XMLGeneratorGradBasedOptimizerOptions.hpp"

#include <cmath>

namespace XMLGen
{

namespace Private
{

/******************************************************************************/
std::string get_constraint_reference_value_name
(const XMLGen::InputData& aMetaData,
 const XMLGen::Constraint &aConstraint)
{
    std::string tReturn = "";
    
    auto tCriterion = aMetaData.criterion(aConstraint.criterion());
    if(tCriterion.type() == "volume")
    {
        tReturn = "Design Volume";
    }
    return tReturn; 
}

}
// namespace Private

/******************************************************************************/
void append_grad_based_optimizer_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    const std::unordered_map<std::string, std::string> tValidOptimizers =
        { {"rol_bound_constrained", "ROL BoundConstrained"}, 
          {"rol_augmented_lagrangian", "ROL AugmentedLagrangian"}, 
          {"rol_linear_constraint", "ROL LinearConstraint"} };

    auto tLower = Plato::tolower(aMetaData.optimization_parameters().optimization_algorithm());
    auto tOptimizerItr = tValidOptimizers.find(tLower);
    if(tOptimizerItr == tValidOptimizers.end())
    {
        THROWERR(std::string("Append Optimization Algorithm Option: Optimization algorithm '")
            + aMetaData.optimization_parameters().optimization_algorithm() + "' is not supported.")
    }

    XMLGen::append_children( { "Package" }, { tOptimizerItr->second }, aParentNode);
    XMLGen::append_grad_based_optimizer_parameters(aMetaData, aParentNode);
}
// function append_grad_based_optimizer_options
/******************************************************************************/

/******************************************************************************/
void append_grad_based_optimizer_parameters
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    auto tLower = XMLGen::to_lower(aMetaData.optimization_parameters().optimization_algorithm());
    
    if(tLower.compare("rol_linear_constraint") == 0 || tLower.compare("rol_augmented_lagrangian") == 0 || tLower.compare("rol_bound_constrained") == 0)
    {
        auto tCheckGradient = aMetaData.optimization_parameters().check_gradient().empty() ? std::string("false") : XMLGen::to_lower(aMetaData.optimization_parameters().check_gradient());
        if(tCheckGradient.compare("true") == 0)
        {   
            XMLGen::append_rol_gradient_check_flags(aMetaData, aParentNode);
            XMLGen::append_rol_gradient_check_options(aMetaData, aParentNode);
        }
        XMLGen::append_optimizer_options(aMetaData, aParentNode);
        XMLGen::append_reset_algorithm_on_update_option(aMetaData, aParentNode);
        XMLGen::generate_rol_input_file(aMetaData);
        XMLGen::append_rol_input_file(aMetaData, aParentNode);
    }
    else
    {
        THROWERR(std::string("Append Optimization Algorithm Options: ") + "Optimization algorithm '"
            + aMetaData.optimization_parameters().optimization_algorithm() + "' is not supported.")
    }
}
// function append_grad_based_optimizer_parameters
/******************************************************************************/

/******************************************************************************/
void append_optimizer_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    std::vector<std::string> tKeys = {"MaxNumOuterIterations",
                                      "HessianType", 
                                      "ProblemUpdateFrequency"};
    std::vector<std::string> tValues = {aMetaData.optimization_parameters().max_iterations(),
                                        aMetaData.optimization_parameters().hessian_type(), 
                                        aMetaData.optimization_parameters().problem_update_frequency()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    auto tOptionsNode = aParentNode.child("Options");
    if(tOptionsNode.empty())
    {
        tOptionsNode = aParentNode.append_child("Options");
    }
    XMLGen::append_children(tKeys, tValues, tOptionsNode);
    auto tConvergenceNode = aParentNode.append_child("Convergence");
    XMLGen::append_children({"MaxIterations"}, {aMetaData.optimization_parameters().max_iterations()}, tConvergenceNode);
}
// function append_optimizer_options
/******************************************************************************/

/******************************************************************************/
void append_rol_gradient_check_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{ 
    std::vector<std::string> tKeys = {"ROLGradientCheckPerturbationScale",
                                      "ROLGradientCheckSteps",
                                      "ROLGradientCheckSeed"};
    std::vector<std::string> tValues = {aMetaData.optimization_parameters().rol_gradient_check_perturbation_scale(),
                                        aMetaData.optimization_parameters().rol_gradient_check_steps(),
                                        aMetaData.optimization_parameters().rol_gradient_check_random_seed()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    auto tOptionsNode = aParentNode.child("Options");
    if(tOptionsNode.empty())
    {
        tOptionsNode = aParentNode.append_child("Options");
    }
    XMLGen::append_children(tKeys, tValues, tOptionsNode);
}

// function append_reset_algorithm_on_update_option
/******************************************************************************/
void append_reset_algorithm_on_update_option
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    std::vector<std::string> tKeys = {"ResetAlgorithmOnUpdate"};
    std::vector<std::string> tValues = {aMetaData.optimization_parameters().reset_algorithm_on_update()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    auto tOptionsNode = aParentNode.child("Options");
    if(tOptionsNode.empty())
    {
        tOptionsNode = aParentNode.append_child("Options");
    }
    XMLGen::append_children(tKeys, tValues, tOptionsNode);

}

/******************************************************************************/
void generate_rol_input_file(const XMLGen::InputData& aMetaData)
{
    pugi::xml_document doc;
    pugi::xml_node n1, n2, n3;
    pugi::xml_attribute a1;

    // Version entry
    n1 = doc.append_child(pugi::node_declaration);
    n1.set_name("xml");
    a1 = n1.append_attribute("version");
    a1.set_value("1.0");

    n1 = doc.append_child("ParameterList");
    n1.append_attribute("name") = "Inputs";

    n2 = n1.append_child("ParameterList");
    n2.append_attribute("name") = "General";
    addNTVParameter(n2, "Output Level", "int", "1");
    addNTVParameter(n2, "Variable Objective Function", "bool", "false");
    addNTVParameter(n2, "Scale for Epsilon Active Sets", "double", "1.0");
    addNTVParameter(n2, "Inexact Objective Function", "bool", "false");
    addNTVParameter(n2, "Inexact Gradient", "bool", "false");
    addNTVParameter(n2, "Inexact Hessian-Times-A-Vector", "bool", "false");
    addNTVParameter(n2, "Projected Gradient Criticality Measure", "bool", "false");

    n3 = n2.append_child("ParameterList");
    n3.append_attribute("name") = "Secant";
    addNTVParameter(n3, "Type", "string", "Limited-Memory BFGS");
    addNTVParameter(n3, "Use as Preconditioner", "bool", "false");
    addNTVParameter(n3, "Use as Hessian", "bool", "false");
    if(aMetaData.optimization_parameters().limited_memory_storage().length() > 0)
    {
        addNTVParameter(n3, "Maximum Storage", "int", aMetaData.optimization_parameters().limited_memory_storage());
    }
    else
    {
        addNTVParameter(n3, "Maximum Storage", "int", "10");
    }
    addNTVParameter(n3, "Barzilai-Borwein Type", "int", "1");

    n3 = n2.append_child("ParameterList");
    n3.append_attribute("name") = "Krylov";
    addNTVParameter(n3, "Type", "string", "Conjugate Gradients");
    addNTVParameter(n3, "Absolute Tolerance", "double", "1e-4");
    addNTVParameter(n3, "Relative Tolerance", "double", "1e-2");
    addNTVParameter(n3, "Iteration Limit", "int", "1");
    n3 = n2.append_child("ParameterList");
    n3.append_attribute("name") = "Polyhedral Projection";
    if(aMetaData.constraints.size() > 1)
    {
        addNTVParameter(n3, "Type", "string", "Semismooth Newton");
    }
    else 
    {
        addNTVParameter(n3, "Type", "string", "Dai-Fletcher");
    }
    addNTVParameter(n3, "Iteration Limit", "int", "1000");

    XMLGen::append_rol_step_block(aMetaData, n1); 

    n2 = n1.append_child("ParameterList");
    n2.append_attribute("name") = "Status Test";
    XMLGen::append_rol_tolerances(aMetaData, n2);

    if(aMetaData.optimization_parameters().reset_algorithm_on_update() == "true")
    {
        addNTVParameter(n2, "Iteration Limit", "int", aMetaData.optimization_parameters().problem_update_frequency());
    }
    else
    {
        addNTVParameter(n2, "Iteration Limit", "int", aMetaData.optimization_parameters().max_iterations());
    }

    // Write the file to disk
    doc.save_file("rol_inputs.xml", "  ");
}
/******************************************************************************/

/******************************************************************************/
void append_rol_input_file
(const XMLGen::InputData& /*aMetaData*/,
 pugi::xml_node& aParentNode)
{
    std::vector<std::string> tKeys = {"InputFileName"};
    std::vector<std::string> tValues = {"rol_inputs.xml"};
    auto tOptionsNode = aParentNode.child("Options");
    if(tOptionsNode.empty())
    {
        tOptionsNode = aParentNode.append_child("Options");
    }
    XMLGen::append_children(tKeys, tValues, tOptionsNode);
}
// function append_rol_input_file
/******************************************************************************/

/******************************************************************************/
void append_rol_tolerances(const XMLGen::InputData& aMetaData,
                           pugi::xml_node &aParent)
{
    addNTVParameter(aParent, "Gradient Tolerance", "double", aMetaData.optimization_parameters().rol_gradient_tolerance());
    addNTVParameter(aParent, "Constraint Tolerance", "double", aMetaData.optimization_parameters().rol_constraint_tolerance());
    addNTVParameter(aParent, "Step Tolerance", "double", aMetaData.optimization_parameters().rol_step_tolerance());
}
// function append_rol_tolerances
/******************************************************************************/

/******************************************************************************/
void append_initial_trust_region_radius(const XMLGen::InputData& aMetaData,
                           pugi::xml_node &aParent)
{
    addNTVParameter(aParent, "Initial Radius", "double", aMetaData.optimization_parameters().rol_initial_trust_region_radius());
}
// function append_initial_trust_region_radius
/******************************************************************************/

/******************************************************************************/
void append_rol_step_block(const XMLGen::InputData& aMetaData,
                           pugi::xml_node &aParent)
{
    pugi::xml_node n2, n3, n4, n5;

    n2 = aParent.append_child("ParameterList");
    n2.append_attribute("name") = "Step";
    if(aMetaData.optimization_parameters().optimization_algorithm() == "rol_augmented_lagrangian")
        addNTVParameter(n2, "Type", "string", "Augmented Lagrangian");
    else
        addNTVParameter(n2, "Type", "string", "Trust Region");

    n3 = n2.append_child("ParameterList");
    n3.append_attribute("name") = "Line Search";
    addNTVParameter(n3, "Function Evaluation Limit", "int", "20");
    addNTVParameter(n3, "Sufficient Decrease Tolerance", "double", "1e-4");
    addNTVParameter(n3, "Initial Step Size", "double", "1.0");
    addNTVParameter(n3, "User Defined Initial Step Size", "bool", "false");
    addNTVParameter(n3, "Accept Linesearch Minimizer", "bool", "false");
    addNTVParameter(n3, "Accept Last Alpha", "bool", "false");

    n4 = n3.append_child("ParameterList");
    n4.append_attribute("name") = "Descent Method";
    addNTVParameter(n4, "Type", "string", "Newton-Krylov");
    addNTVParameter(n4, "Nonlinear CG Type", "string", "Hestenes-Stiefel");

    n4 = n3.append_child("ParameterList");
    n4.append_attribute("name") = "Curvature Condition";
    addNTVParameter(n4, "Type", "string", "Strong Wolfe Conditions");
    addNTVParameter(n4, "General Parameter", "double", "0.9");
    addNTVParameter(n4, "Generalized Wolfe Parameter", "double", "0.6");

    n4 = n3.append_child("ParameterList");
    n4.append_attribute("name") = "Line-Search Method";
    addNTVParameter(n4, "Type", "string", "Cubic Interpolation");
    addNTVParameter(n4, "Backtracking Rate", "double", "0.5");
    addNTVParameter(n4, "Bracketing Tolerance", "double", "1e-8");

    n5 = n4.append_child("ParameterList");
    n5.append_attribute("name") = "Path-Based Target Level";
    addNTVParameter(n5, "Target Relaxation Parameter", "double", "1.0");
    addNTVParameter(n5, "Upper Bound on Path Length", "double", "1.0");

    // Trust region
    n3 = n2.append_child("ParameterList");
    n3.append_attribute("name") = "Trust Region";
    addNTVParameter(n3, "Subproblem Solver", "string", "Truncated CG");
    std::string tSubproblemModel = XMLGen::get_subproblem_model(aMetaData.optimization_parameters().rol_subproblem_model());
    addNTVParameter(n3, "Subproblem Model", "string", tSubproblemModel);
    XMLGen::append_initial_trust_region_radius(aMetaData, n3);
    addNTVParameter(n3, "Maximum Radius", "double", "1.0e8");
    addNTVParameter(n3, "Step Acceptance Threshold", "double", "1e-2");
    addNTVParameter(n3, "Radius Shrinking Threshold", "double", "0.05");
    addNTVParameter(n3, "Radius Growing Threshold", "double", "0.9");
    addNTVParameter(n3, "Radius Shrinking Rate (Negative rho)", "double", "0.5");
    addNTVParameter(n3, "Radius Shrinking Rate (Positive rho)", "double", "0.75");
    addNTVParameter(n3, "Radius Growing Rate", "double", "1.2");
    addNTVParameter(n3, "Safeguard Size", "double", "10");
    n4 = n3.append_child("ParameterList");
    n4.append_attribute("name") = "SPG";
    addNTVParameter(n4, "Use Nonmonotone Trust Region", "bool", "false");
    addNTVParameter(n4, "Maximum Storage Size", "int", "10");
    n5 = n4.append_child("ParameterList");
    n5.append_attribute("name") = "Solver";
    addNTVParameter(n5, "Iteration Limit", "int", "25");
    addNTVParameter(n5, "Minimum Spectral Step Size", "double", "1e-8");
    addNTVParameter(n5, "Maximum Spectral Step Size", "double", "1e8");
    addNTVParameter(n5, "Use Smallest Model Iterate", "bool", "false");
    n4 = n3.append_child("ParameterList");
    n4.append_attribute("name") = "Kelley-Sachs";
    addNTVParameter(n4, "Maximum Number of Smoothing Iterations", "int", "20");
    addNTVParameter(n4, "Sufficient Decrease Parameter", "double", "1e-4");
    addNTVParameter(n4, "Post-Smoothing Decrease Parameter", "double", ".9999");
    addNTVParameter(n4, "Binding Set Tolerance", "double", "1e-3");
    addNTVParameter(n4, "Post-Smoothing Backtracking Rate", "double", "1e-2");
    addNTVParameter(n4, "Initial Post-Smoothing Step Size", "double", "1.0");
    n4 = n3.append_child("ParameterList");
    n4.append_attribute("name") = "Lin-More";
    if(aMetaData.optimization_parameters().hessian_type() == "disabled" ||
       aMetaData.optimization_parameters().hessian_type() == "zero" ||
       aMetaData.optimization_parameters().hessian_type() == "") 
    {
        addNTVParameter(n4, "Maximum Number of Minor Iterations", "int", "0");
    }
    else
    {
        addNTVParameter(n4, "Maximum Number of Minor Iterations", "int", "1");
    }
    addNTVParameter(n4, "Sufficient Decrease Parameter", "double", "1e-2");
    n5 = n4.append_child("ParameterList");
    n5.append_attribute("name") = "Cauchy Point";
    addNTVParameter(n5, "Maximum Number of Reduction Steps", "int", "10");
    addNTVParameter(n5, "Maximum Number of Expansion Steps", "int", "10");
    addNTVParameter(n5, "Initial Step Size", "double", aMetaData.optimization_parameters().rol_lin_more_cauchy_initial_step_size());
    addNTVParameter(n5, "Normalize Initial Step Size", "bool", "false");
    addNTVParameter(n5, "Reduction Rate", "double", "0.5");
    addNTVParameter(n5, "Expansion Rate", "double", "5.0");
    addNTVParameter(n5, "Decrease Tolerance", "double", "1e-8");
    n5 = n4.append_child("ParameterList");
    n5.append_attribute("name") = "Projected Search";
    addNTVParameter(n5, "Backtracking Rate", "double", "0.5");
    addNTVParameter(n5, "Maximum Number of Steps", "int", "20");

    n4 = n3.append_child("ParameterList");
    n4.append_attribute("name") = "Inexact";
    n5 = n4.append_child("ParameterList");
    n5.append_attribute("name") = "Value";
    addNTVParameter(n5, "Tolerance Scaling", "double", "1e-1");
    addNTVParameter(n5, "Exponent", "double", "0.9");
    addNTVParameter(n5, "Forcing Sequence Initial Value", "double", "1.0");
    addNTVParameter(n5, "Forcing Sequence Update Frequency", "int", "10");
    addNTVParameter(n5, "Forcing Sequence Reduction Factor", "double", "0.1");
    n5 = n4.append_child("ParameterList");
    n5.append_attribute("name") = "Gradient";
    addNTVParameter(n5, "Tolerance Scaling", "double", "1e-1");
    addNTVParameter(n5, "Relative Tolerance", "double", "2.0");

    n3 = n2.append_child("ParameterList");
    n3.append_attribute("name") = "Primal Dual Active Set";
    addNTVParameter(n3, "Dual Scaling", "double", "1.0");
    addNTVParameter(n3, "Iteration Limit", "int", "10");
    addNTVParameter(n3, "Relative Step Tolerance", "double", "1e-8");
    addNTVParameter(n3, "Relative Gradient Tolerance", "double", "1e-6");

    n3 = n2.append_child("ParameterList");
    n3.append_attribute("name") = "Composite Step";
    addNTVParameter(n3, "Output Level", "int", "0");
    n4 = n3.append_child("ParameterList");
    n4.append_attribute("name") = "Optimality System Solver";
    addNTVParameter(n4, "Nominal Relative Tolerance", "double", "1e-8");
    addNTVParameter(n4, "Fix Tolerance", "bool", "true");
    n4 = n3.append_child("ParameterList");
    n4.append_attribute("name") = "Tangential Subproblem Solver";
    addNTVParameter(n4, "Iteration Limit", "int", "20");
    addNTVParameter(n4, "Relative Tolerance", "double", "1e-2");

    // Augmented Lagrangian
    n3 = n2.append_child("ParameterList");
    n3.append_attribute("name") = "Augmented Lagrangian";
    addNTVParameter(n3, "Initial Penalty Parameter", "double", "1.0e1");
    addNTVParameter(n3, "Penalty Parameter Growth Factor", "double", "100.0");
    addNTVParameter(n3, "Minimum Penalty Parameter Reciprocal", "double", "0.1");
    addNTVParameter(n3, "Initial Optimality Tolerance", "double", "1.0");
    addNTVParameter(n3, "Optimality Tolerance Update Exponent", "double", "1.0");
    addNTVParameter(n3, "Optimality Tolerance Decrease Exponent", "double", "1.0");
    addNTVParameter(n3, "Initial Feasibility Tolerance", "double", "1.0");
    addNTVParameter(n3, "Feasibility Tolerance Update Exponent", "double", "0.1");
    addNTVParameter(n3, "Feasibility Tolerance Decrease Exponent", "double", "0.9");
    addNTVParameter(n3, "Print Intermediate Optimization History", "bool", "false");
    addNTVParameter(n3, "Subproblem Step Type", "string", "Trust Region");
    addNTVParameter(n3, "Subproblem Iteration Limit", "int", "10");

    n3 = n2.append_child("ParameterList");
    n3.append_attribute("name") = "Moreau-Yosida Penalty";
    addNTVParameter(n3, "Initial Penalty Parameter", "double", "1e2");
    addNTVParameter(n3, "Penalty Parameter Growth Factor", "double", "1.0");
    n4 = n3.append_child("ParameterList");
    n4.append_attribute("name") = "Subproblem";
    addNTVParameter(n4, "Optimality Tolerance", "double", "1.e-12");
    addNTVParameter(n4, "Feasibility Tolerance", "double", "1.e-12");
    addNTVParameter(n4, "Print History", "bool", "false");
    addNTVParameter(n4, "Iteration Limit", "int", "200");

    n3 = n2.append_child("ParameterList");
    n3.append_attribute("name") = "Bundle";
    addNTVParameter(n3, "Initial Trust-Region Parameter", "double", "1e1");
    addNTVParameter(n3, "Maximum Trust-Region Parameter", "double", "1e8");
    addNTVParameter(n3, "Tolerance for Trust-Region Parameter", "double", "1e-4");
    addNTVParameter(n3, "Epsilon Solution Tolerance", "double", "1e-12");
    addNTVParameter(n3, "Upper Threshold for Serious Step", "double", "1e-1");
    addNTVParameter(n3, "Lower Threshold for Serious Step", "double", "2e-1");
    addNTVParameter(n3, "Upper Threshold for Null Step", "double", "9e-1");
    addNTVParameter(n3, "Distance Measure Coefficient", "double", "1.e-6");
    addNTVParameter(n3, "Maximum Bundle Size", "int", "50");
    addNTVParameter(n3, "Removal Size for Bundle Update", "int", "2");
    addNTVParameter(n3, "Cutting Plane Tolerance", "double", "1e-8");
    addNTVParameter(n3, "Cutting Plane Iteration Limit", "int", "1000");
}
// function append_rol_step_block
/******************************************************************************/

/******************************************************************************/
std::string get_subproblem_model(const std::string &aSubproblemModelString)
{
    std::string tReturn;
    if(aSubproblemModelString == "lin_more")
    {
        tReturn = "Lin-More";
    } 
    else if(aSubproblemModelString == "kelley_sachs")
    {
        tReturn = "Kelley-Sachs";
    } 
    else if(aSubproblemModelString == "spg")
    {
        tReturn = "SPG";
    } 
    else
    {
        THROWERR("Generate ROL Input File: Subproblem model not supported.")
    }
    return tReturn;
}
// function get_subproblem_model
/******************************************************************************/

/******************************************************************************/


void append_rol_gradient_check_flags
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    auto tCheckGradient = aMetaData.optimization_parameters().check_gradient().empty() ? std::string("true") : aMetaData.optimization_parameters().check_gradient();
    std::vector<std::string> tKeys = {"CheckGradient", "CheckHessian", "UseUserInitialGuess"};
    std::vector<std::string> tValues = {tCheckGradient, "False", "True"};
    XMLGen::append_children(tKeys, tValues, aParentNode);
}

// function append_derivative_checker_options
/******************************************************************************/

/******************************************************************************/
void append_optimizer_update_problem_stage_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    if(!XMLGen::need_update_problem_stage(aMetaData))
    {
        return;
    }
    auto tNode = aParentNode.append_child("UpdateProblemStage");
    XMLGen::append_children({"Name"}, {"Update Problem"}, tNode);
}
// function append_optimizer_update_problem_stage_options
/******************************************************************************/

/******************************************************************************/
void append_optimizer_cache_stage_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    if(XMLGen::num_cache_states(aMetaData.services()) > 0)
    {
        auto tNode = aParentNode.append_child("CacheStage");
        XMLGen::append_children({"Name"}, {"Cache State"}, tNode);
    }
}
// function append_optimizer_cache_stage_options
/******************************************************************************/

/******************************************************************************/
void append_grad_based_optimizer_output_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    if(aMetaData.mOutputMetaData.size() > 0 && 
       aMetaData.mOutputMetaData[0].isOutputDisabled())
    {
        return;
    }
    auto tNode = aParentNode.append_child("Output");
    append_children({"OutputStage"}, {"Output To File"}, tNode);
}
// function append_grad_based_optimizer_output_options
/******************************************************************************/

/******************************************************************************/
void append_grad_based_optimizer_variables_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    std::vector<std::string> tKeys =
        {"ValueName", "InitializationStage", "FilteredName", "LowerBoundValueName", "LowerBoundVectorName",
         "UpperBoundValueName", "UpperBoundVectorName", "SetLowerBoundsStage", "SetUpperBoundsStage"};
    std::vector<std::string> tValues;
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        tValues =
            {"Control", "Initial Guess", "Topology", "Lower Bound Value", "Lower Bound Vector",
             "Upper Bound Value", "Upper Bound Vector", "Set Lower Bounds", "Set Upper Bounds"};
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        tValues =
            {"Design Parameters", "Initialize Design Parameters", "Topology", "Lower Bound Value", "Lower Bound Vector",
             "Upper Bound Value", "Upper Bound Vector", "Set Lower Bounds", "Set Upper Bounds"};
    }
    auto tNode = aParentNode.append_child("OptimizationVariables");
    XMLGen::append_children(tKeys, tValues, tNode);
}
// function append_grad_based_optimizer_variables_options
/******************************************************************************/

/******************************************************************************/
void append_grad_based_optimizer_objective_options
(const XMLGen::InputData& /*aMetaData*/,
 pugi::xml_node& aParentNode)
{
    std::map<std::string, std::string> tKeyToValueMap =
        { {"ValueName", ""}, {"ValueStageName", ""}, {"GradientName", ""}, {"GradientStageName", ""} };

    std::string tValueNameString = "Objective Value";

    tKeyToValueMap.find("ValueName")->second = tValueNameString;
    tKeyToValueMap.find("ValueStageName")->second = std::string("Compute Objective Value");
    tKeyToValueMap.find("GradientName")->second = std::string("Objective Gradient");
    tKeyToValueMap.find("GradientStageName")->second = std::string("Compute Objective Gradient");

    auto tKeys = XMLGen::transform_key_tokens(tKeyToValueMap);
    auto tValues = XMLGen::transform_value_tokens(tKeyToValueMap);
    auto tNode = aParentNode.append_child("Objective");
    XMLGen::append_children(tKeys, tValues, tNode);
}
// function append_grad_based_optimizer_objective_options
/******************************************************************************/

/******************************************************************************/
void append_grad_based_optimizer_constraint_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    for (auto &tConstraint : aMetaData.constraints)
    {
        std::map<std::string, std::string> tKeyToValueMap;
        if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY  && aMetaData.optimization_parameters().discretization() == "density")
        {
            tKeyToValueMap =
            { {"ValueName", ""}, {"ValueStageName", ""}, {"GradientName", ""}, {"GradientStageName", ""},
                 {"ReferenceValueName", XMLGen::Private::get_constraint_reference_value_name(aMetaData, tConstraint)} };
        }
        else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
        {
            tKeyToValueMap =
            { {"ValueName", ""}, {"ValueStageName", ""}, {"GradientName", ""}, {"GradientStageName", ""} };
        }

        tKeyToValueMap.find("ValueName")->second = std::string("Constraint Value ") + tConstraint.id();
        tKeyToValueMap.find("ValueStageName")->second = std::string("Compute Constraint Value ") + tConstraint.id();
        tKeyToValueMap.find("GradientName")->second = std::string("Constraint Gradient ") + tConstraint.id();
        tKeyToValueMap.find("GradientStageName")->second = std::string("Compute Constraint Gradient ") + tConstraint.id();

        XMLGen::generate_target_value_entries(aMetaData, tConstraint, tKeyToValueMap);

        auto tKeys = XMLGen::transform_key_tokens(tKeyToValueMap);
        auto tValues = XMLGen::transform_value_tokens(tKeyToValueMap);
        auto tNode = aParentNode.append_child("Constraint");
        XMLGen::append_children(tKeys, tValues, tNode);
    }
}
// function append_grad_based_optimizer_constraint_options
/******************************************************************************/

/******************************************************************************/
void generate_target_value_entries
(const XMLGen::InputData& aXMLMetaData,
 const XMLGen::Constraint &aConstraint,
 std::map<std::string, std::string> &aKeyToValueMap)
{
    auto tCriterion = aXMLMetaData.criterion(aConstraint.criterion());
    std::string tCriterionType = tCriterion.type();
    if(tCriterionType == "volume")
    {
        if(aConstraint.absoluteTarget().length() > 0)
        {
            double tAbsoluteTargetValue = std::atof(aConstraint.absoluteTarget().c_str());
            if(fabs(tAbsoluteTargetValue) < 1e-16)
            {
                THROWERR("You must specify a non-zero volume constraint value.")
            }
            double tReferenceValue = std::atof(aConstraint.absoluteTarget().c_str());
            tReferenceValue *= std::atof(aConstraint.divisor().c_str());
            aKeyToValueMap["AbsoluteTargetValue"] = aConstraint.absoluteTarget();
            aKeyToValueMap["ReferenceValue"] = std::to_string(tReferenceValue);
        }
        else if(aConstraint.relativeTarget().length() > 0)
        {
            double tRelativeTargetValue = std::atof(aConstraint.relativeTarget().c_str());
            if(fabs(tRelativeTargetValue) < 1e-16)
            {
                THROWERR("You must specify a non-zero volume constraint value.")
            }
            aKeyToValueMap["NormalizedTargetValue"] = aConstraint.relativeTarget();
        }
        else
            THROWERR("Append Optimization Constraint Options: Constraint target was not set.")
    }
    else
    {
        if(aConstraint.absoluteTarget().length() == 0)
        {
            THROWERR("You must use absolute target values with non-volume constraints.")
        }
        double tAbsoluteTargetValue = std::atof(aConstraint.absoluteTarget().c_str());
        aKeyToValueMap["AbsoluteTargetValue"] = aConstraint.absoluteTarget();
        double tReferenceValue = fabs(tAbsoluteTargetValue) < 1e-16 ? 1.0 : std::atof(aConstraint.absoluteTarget().c_str());
        tReferenceValue *= std::atof(aConstraint.divisor().c_str());
        aKeyToValueMap["ReferenceValue"] = std::to_string(tReferenceValue);
        if(aConstraint.greater_than())
        {
            aKeyToValueMap["AbsoluteTargetValue"].insert(0,1,'-');
        }
    }
}
// function generate_target_value_entries
/******************************************************************************/

/******************************************************************************/
void append_bound_constraints_options_topology_optimization
(const std::vector<std::string>& aValues,
 pugi::xml_node& aParentNode)
{
    std::vector<std::string> tKeys = {"Upper", "Lower"};
    auto tNode = aParentNode.append_child("BoundConstraint");
    XMLGen::append_children(tKeys, aValues, tNode);
}
// function append_bound_constraints_options_topology_optimization
/******************************************************************************/

}
//  namespace XMLGen
