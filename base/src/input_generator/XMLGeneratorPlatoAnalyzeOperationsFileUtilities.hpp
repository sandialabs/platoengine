/*
 * XMLGeneratorPlatoAnalyzeOperationsFileUtilities.hpp
 *
 *  Created on: Jun 4, 2020
 */

#pragma once

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorAnalyzeFunctionMapTypes.hpp"

namespace XMLGen
{

namespace Analyze
{
/*!< information used to identify materials, \n
 * i.e. pair<material_category, vector<pair<material_property_argument_name_tag, material_property_tag> > > */
using MaterialPropertyTags = std::tuple<std::string, std::string, std::vector<std::pair<std::string, std::string>>>;
//using MaterialPropertyTags = std::pair<std::string, std::vector<std::pair<std::string, std::string>>>;

/*!< map from element block identification number to material metadata, i.e. map< block_id, MaterialInfo > */
using MaterialPropertyMetadata = std::unordered_map<std::string, XMLGen::Analyze::MaterialPropertyTags>;
}
// namespace Analyze

/******************************************************************************//**
 * \fn append_compute_objective_value_to_plato_analyze_operation
 * \brief Append compute objective value operation to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_objective_value_to_plato_analyze_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_objective_value_operation_for_topology_problem
 * \brief Append compute objective value operation for topology optimizatino problems
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_objective_value_operation_for_topology_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_objective_value_operation_for_shape_problem
 * \brief Append compute objective value operation for shape optimizatino problems
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_objective_value_operation_for_shape_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_constraint_value_operation_for_topology_problem
 * \brief Append compute constraint value operation for topology optimizatino problems
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_constraint_value_operation_for_topology_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_constraint_value_operation_for_shape_problem
 * \brief Append compute constraint value operation for shape optimizatino problems
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_constraint_value_operation_for_shape_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_objective_gradient_to_plato_analyze_operation
 * \brief Append compute objective gradient operation to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_objective_gradient_to_plato_analyze_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_objective_gradient_operation_for_topology_problem
 * \brief Append compute objective gradient operation for topology optimization problems to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_objective_gradient_operation_for_topology_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_objective_gradient_operation_for_shape_problem
 * \brief Append compute objective gradient operation for shape optimization problems to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_objective_gradient_operation_for_shape_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_constraint_gradient_operation_for_topology_problem
 * \brief Append compute constraint gradient operation for topology optimization problems to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_constraint_gradient_operation_for_topology_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_constraint_gradient_operation_for_shape_problem
 * \brief Append compute constraint gradient operation for shape optimization problems to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_constraint_gradient_operation_for_shape_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_criterion_value_operations_for_dakota_problem
 * \brief Append compute criterion value operations for dakota problems to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_criterion_value_operations_for_dakota_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_objective_criterion_value_operations_for_dakota_problem
 * \brief Append compute criterion value operation for objectives in dakota problems to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_objective_criterion_value_operations_for_dakota_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_criterion_value_operation_for_dakota_problem
 * \brief Append compute criterion value operation for dakota problems to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_criterion_value_operation_for_dakota_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument,
 const std::string& aCriterionId,
 const std::string& aIdentifierString);

/******************************************************************************//**
 * \fn append_compute_constraint_criterion_value_operations_for_dakota_problem
 * \brief Append compute criterion value operation for constraints in dakota problems to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_constraint_criterion_value_operations_for_dakota_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_constraint_value_to_plato_analyze_operation
 * \brief Append compute constraint value operation to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_constraint_value_to_plato_analyze_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_constraint_gradient_to_plato_analyze_operation
 * \brief Append compute constraint gradient operation to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_constraint_gradient_to_plato_analyze_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_reinit_on_change_data
 * \brief Append reinitialize on change operation
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
 * \param [in]     aName       operation name
 * \param [in]     aSharedData shared data name if needed
**********************************************************************************/
void append_reinit_on_change_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument,
 const std::string& aName,
 const std::string& aSharedData);

/******************************************************************************//**
 * \fn append_update_problem_to_plato_analyze_operation
 * \brief Append update problem operation to PUGI XML document. The update problem \n
 * operation is used to enable safe non-Plato parameter, e.g. physics-only parameters \n
 * updates during optimization.
 * \param [in]     aMetaData Plato problem input data
 * \param [in/out] aDocument    pugi::xml_document
**********************************************************************************/
void append_update_problem_to_plato_analyze_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_compute_solution_to_plato_analyze_operation
 * \brief Append compute solution operation to PUGI XML document.
 * \param [in]     aMetaData Plato problem input data
 * \param [in/out] aDocument    pugi::xml_document
**********************************************************************************/
void append_compute_solution_to_plato_analyze_operation
(pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn return_random_material_metadata_for_plato_analyze_operation_xml_file
 * \brief Return material metadata needed to write the plato analyze operation xml file.
 * \param [in] aRandomMetaData random samples metadata
 * \return map from element block identification number to material property tags, \n
 * i.e. map< block_id, pair< material_category, vector< pair<material_property_argument_name_tag, material_property_tag> > > >
**********************************************************************************/
XMLGen::Analyze::MaterialPropertyMetadata
return_random_material_metadata_for_plato_analyze_operation_xml_file
(const XMLGen::RandomMetaData& aRandomMetaData);

/******************************************************************************//**
 * \fn append_random_material_properties_to_plato_analyze_operation
 * \brief Append material properties to the plato analyze operation xml file.
 * \param [in]     aMetaData Plato problem input data
 * \param [in/out] aParentNode  pugi::xml_node
**********************************************************************************/
void append_random_material_properties_to_plato_analyze_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_random_traction_vector_to_plato_analyze_operation
 * \brief Append random traction vector to the plato analyze operation xml file.
 * \param [in]     aMetaData Plato problem input data
 * \param [in/out] aParentNode  pugi::xml_node
**********************************************************************************/
void append_random_traction_vector_to_plato_analyze_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_write_output_to_plato_analyze_operation
 * \brief Append random traction vector to the plato analyze operation xml file.
 * \param [in]     aMetaData Plato problem input data
 * \param [in/out] aParentNode  pugi::xml_node
**********************************************************************************/
void append_write_output_to_plato_analyze_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_filter_control_to_plato_analyze_helmholtz_operation
 * \brief Append filter control to the plato analyze operation xml file for helmholtz
 * \param [in]     aXMLMetaData Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_filter_control_to_plato_analyze_helmholtz_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_filter_gradient_to_plato_analyze_helmholtz_operation
 * \brief Append filter gradient to the plato analyze operation xml file for helmholtz
 * \param [in]     aXMLMetaData Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_filter_gradient_to_plato_analyze_helmholtz_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn write_plato_analyze_operation_xml_file
 * \brief Write Plato Analyze operations to plato_analyze_operation.xml file.
 * \param [in] aMetaData Plato problem input data
**********************************************************************************/
void write_plato_analyze_operation_xml_file
(const XMLGen::InputData& aXMLMetaData);

/******************************************************************************//**
 * \fn write_plato_analyze_operation_xml_file_dakota_problem
 * \brief Write Plato Analyze operations for dakota problem to plato_analyze_operation.xml file.
 * \param [in] aMetaData Plato problem input data
**********************************************************************************/
void write_plato_analyze_operation_xml_file_dakota_problem
(const XMLGen::InputData& aXMLMetaData);

/******************************************************************************//**
 * \fn write_plato_analyze_helmholtz_operation_xml_file
 * \brief Write Plato Analyze operations to plato_analyze_operation.xml file.
 * Specialized for Plato Analyze instance that is a helmholtz filter.
 * \param [in] aMetaData Plato problem input data
**********************************************************************************/
void write_plato_analyze_helmholtz_operation_xml_file
(const XMLGen::InputData& aXMLMetaData);

/******************************************************************************//**
 * \fn write_amgx_input_file
 * \brief Write AMGX input .json file. This file is used to assign values for the \n
 * linear solver parameters. Interested readers can find more information on AMGX \n
 * in \see{https://github.com/NVIDIA/AMGX}.
 * \param [in]     aMetaData Plato problem input data
 * \param [in/out] aDocument    pugi::xml_document
**********************************************************************************/
void write_amgx_input_file(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn write_default_amgx_input_file
 * \brief Write AMGX input .json file. This file is used to assign values for the \n
 * linear solver parameters. Interested readers can find more information on AMGX \n
 * in \see{https://github.com/NVIDIA/AMGX}.
 * \param [in]     aMetaData Plato problem input data
 * \param [in/out] aDocument    pugi::xml_document
**********************************************************************************/
void write_default_amgx_input_file(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn write_amgx_input_file_for_plasticity
 * \brief Write AMGX input .json file. This file is used to assign values for the \n
 * linear solver parameters. Interested readers can find more information on AMGX \n
 * in \see{https://github.com/NVIDIA/AMGX}.
 * \param [in]     aMetaData Plato problem input data
 * \param [in/out] aDocument    pugi::xml_document
**********************************************************************************/
void write_amgx_input_file_for_plasticity(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn write_amgx_input_file_for_thermoplasticity
 * \brief Write AMGX input .json file. This file is used to assign values for the \n
 * linear solver parameters. Interested readers can find more information on AMGX \n
 * in \see{https://github.com/NVIDIA/AMGX}.
 * \param [in]     aMetaData Plato problem input data
 * \param [in/out] aDocument    pugi::xml_document
**********************************************************************************/
void write_amgx_input_file_for_thermoplasticity(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn append_mesh_map_data
 * \brief Append mesh map data plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_mesh_map_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_visualization_to_plato_analyze_operation
 * \brief Append output related info to plato_analyze_operation.xml.
 * \param [in]     aMetaData   Plato problem input data
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_visualization_to_plato_analyze_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_compute_map_criterion_gradient_operation_for_shape_problem
 * \brief Append the operation for concatenating dF/dX and dX/dP in PA
 * \param [in] aCriterionName  name of the current criterion
 * \param [in/out] aDocument   pugi::xml_document
**********************************************************************************/
void append_compute_map_criterion_gradient_operation_for_shape_problem(const std::string &aCriterionName, 
                                                                        pugi::xml_document &aDocument);
}
// namespace XMLGen
