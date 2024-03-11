/*
 * XMLGeneratorStagesOperationsUtilities.hpp
 *
 *  Created on: Jan 14, 2022
 */

#pragma once

#include "pugixml.hpp"
#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn get_objective_value_operation_name
 * \brief Return objective function value operation name based on the service \n
          provider used to evaluate the objective function.
 * \param [in]  aService service input metadata 
 * \return objective function value name (string)
**********************************************************************************/
std::string get_objective_value_operation_name(XMLGen::Service &aService);

/******************************************************************************//**
 * \fn get_objective_value_operation_output_name
 * \brief Return output name for compute objective value operation based on the \n
          service provider used to evaluate the objective function. 
 * \param [in] aService The service provider
 * \return objective function value output name (string)
**********************************************************************************/
std::string get_objective_value_operation_output_name(XMLGen::Service &aService);

/******************************************************************************//**
 * \fn get_objective_gradient_operation_name
 * \brief Get the name of the Compute Objective Gradient operation based on the service provider
 * \param [in] aService The service provider
**********************************************************************************/
std::string get_objective_gradient_operation_name(XMLGen::Service &aService);

/******************************************************************************//**
 * \fn get_objective_gradient_operation_output_name
 * \brief Get the name of the Compute Objective Gradient operation output based on the service provider
 * \param [in] aService The service provider
**********************************************************************************/
std::string get_objective_gradient_operation_output_name(XMLGen::Service &aService);

/******************************************************************************//**
 * \fn append_design_volume_operation
 * \brief Append upper bound operation to stage in interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_design_volume_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_initial_field_operation
 * \brief Append initial field operation to stage in interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_initial_field_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_compute_normalization_factor_operation
 * \brief Append operation for calculating normalization factor to stage in interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_compute_normalization_factor_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_initial_values_operation
 * \brief Append initialize values operation for shape optimization problems to \n
          stage in interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_initial_values_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_update_geometry_on_change_operation
 * \brief Append operation for updating geometry for shape optimization to stage \n
          in interface xml file.
 * \param [in]  aFirstPlatoMainPerformer PlatoMain performer name 
 * \param [out] aParentNode              parent xml node
**********************************************************************************/
void append_update_geometry_on_change_operation 
(const std::string &aFirstPlatoMainPerformer, 
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_initialize_geometry_operation
 * \brief Append initialize geometry operation for shape optimization problems to \n
          stage in interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_initialize_geometry_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

 /******************************************************************************//**
 * \fn append_join_mesh_operation
 * \brief Append operation for joining auxiliary and primary mesh.
 * \param [in]  aFirstPlatoMainPerformer Plato main performer id
 * \param [out] aParentNode              parent xml node
**********************************************************************************/
void append_join_mesh_operation
(const std::string &aFirstPlatoMainPerformer, 
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_rename_mesh_operation
 * \brief Append operation for overwriting ESP mesh with joined mesh.
 * \param [in]  aFirstPlatoMainPerformer Plato main performer id
 * \param [out] aParentNode              parent xml node
**********************************************************************************/
void append_rename_mesh_operation
(const std::string &aFirstPlatoMainPerformer, 
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_tet10_conversion_operation
 * \brief Append operation for converting to tet10
 * \param [in]  aFirstPlatoMainPerformer Plato main performer id
 * \param [out] aParentNode              parent xml node
**********************************************************************************/
void append_tet10_conversion_operation
(const std::string &aFirstPlatoMainPerformer, 
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_filter_control_operation
 * \brief Append filter control operation to stage in interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_filter_control_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_enforce_bounds_operation
 * \brief Append enforce bounds operation to interface xml file
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_enforce_bounds_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_objective_value_operation
 * \brief Append calculate objective value operation to stage in interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
 * \param [in]  aCalculatingNormalizationFactor will normalization factor for the objective value be computed
**********************************************************************************/
void append_objective_value_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode,
 bool aCalculatingNormalizationFactor);

/******************************************************************************//**
 * \fn append_objective_value_operation_for_multi_load_case
 * \brief Append the objective value operation for multi-load case problems
 * \param [in] aMetaData  Plato problem input data
 * \param [in] aParentNode Parent to add data to
 * \param [in] aCalculatingNormalizationFactor Flag telling if we are calculating the objective normalization factor
**********************************************************************************/
void append_objective_value_operation_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode,
 bool aCalculatingNormalizationFactor);

/******************************************************************************//**
 * \fn append_objective_value_operation_for_non_multi_load_case
 * \brief Append the objective value operation for non multi-load case problems
 * \param [in] aMetaData  Plato problem input data
 * \param [in] aParentNode Parent to add data to
 * \param [in] aCalculatingNormalizationFactor Flag telling if we are calculating the objective normalization factor
**********************************************************************************/
void append_objective_value_operation_for_non_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode,
 bool aCalculatingNormalizationFactor);

/******************************************************************************//**
 * \fn append_lower_bound_operation
 * \brief Append lower bound operation to interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_lower_bound_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_upper_bound_operation
 * \brief Append upper bound stage to interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_upper_bound_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_write_output_operation
 * \brief Append write output operation to output stage.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_write_output_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_visualization_operation
 * \brief Append enable visualization operation to output stage.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
 void append_visualization_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_write_output_operation_nondeterministic_usecase
 * \brief Append write output operation for nondeterministic usse cases.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_write_output_operation_nondeterministic_usecase
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_write_output_operation_deterministic_usecase
 * \brief Append write output operation for deterministic usse cases.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_write_output_operation_deterministic_usecase
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_compute_qoi_statistics_operation
 * \brief Append compute QoI statistics operation to output stage.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_compute_qoi_statistics_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_output_operation_to_interface_file
 * \brief Append plato main output operation to ouput stage in interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_output_operation_to_interface_file
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_copy_value_operation
 * \brief Append operation copying a value from one shared data to another
 * \param [in]  aPlatoMainPerformer performer doing the copy
 * \param [in]  aInputSharedDataName name of input shared data
 * \param [in]  aOutputSharedDataName name of output shared data
 * \param [out] aParentNode  parent xml node
**********************************************************************************/
void append_copy_value_operation
(const std::string &aPlatoMainPerformer,
 const std::string &aInputSharedDataName,
 const std::string &aOutputSharedDataName,
 pugi::xml_node &aParentNode);

/******************************************************************************//**
 * \fn append_negate_value_operation
 * \brief Append operation negating a value
 * \param [in]  aPlatoMainPerformer performer doing the copy
 * \param [in]  aSharedDataName name of shared data to be negated
 * \param [out] aParentNode  parent xml node
**********************************************************************************/
void append_negate_value_operation
(const std::string &aPlatoMainPerformer,
 const std::string &aSharedDataName,
 pugi::xml_node &aParentNode);

/******************************************************************************//**
 * \fn append_negate_field_operation
 * \brief Append operation negating a field
 * \param [in]  aPlatoMainPerformer performer doing the copy
 * \param [in]  aSharedDataName name of shared data to be negated
 * \param [out] aParentNode  parent xml node
**********************************************************************************/
void append_negate_field_operation
(const std::string &aPlatoMainPerformer,
 const std::string &aSharedDataName,
 pugi::xml_node &aParentNode);

/******************************************************************************//**
 * \fn append_reinitialize_on_change_operation
 * \brief Append operation for reinitializing performer after geometry and mesh are updated.
 * \param [in]  aPerformer Performer name
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_reinitialize_on_change_operation 
(const std::string &aPerformer, 
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_helmholtz_filter_criterion_gradient_operation
 * \brief Append helmholtz filter criterion gradient operation to interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [in]  aInputSharedDataName criterion gradient shared data name
 * \param [in]  aOutputSharedDataName output criterion gradient shared data name (after filter is applied)
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_helmholtz_filter_criterion_gradient_operation
(const XMLGen::InputData& aMetaData,
 const std::string& aInputSharedDataName,
 const std::string& aOutputSharedDataName,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_copy_field_operation
 * \brief Append operation copying a field from one shared data to another
 * \param [in]  aPlatoMainPerformer performer doing the copy
 * \param [in]  aInputSharedDataName name of input shared data
 * \param [in]  aOutputSharedDataName name of output shared data
 * \param [out] aParentNode  parent xml node
**********************************************************************************/
void append_copy_field_operation
(const std::string &aPlatoMainPerformer,
 const std::string &aInputSharedDataName,
 const std::string &aOutputSharedDataName,
 pugi::xml_node &aParentNode);

/******************************************************************************//**
 * \fn append_filter_criterion_gradient_operation
 * \brief Append filter criterion gradient operation to PUGI XML document.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [in]  aInputSharedDataName name of input shared data
 * \param [in]  aOutputSharedDataName name of output shared data
 * \param [out] aParentNode  parent xml node
**********************************************************************************/
void append_filter_criterion_gradient_operation
(const XMLGen::InputData& aMetaData,
 const std::string& aInputSharedDataName,
 const std::string& aOutputSharedDataName,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_compute_constraint_gradient_operation
 * \brief Append constaint gradient operation for shape optimization problems.
 * \param [in]  aConstraint input constraint metadata
 * \param [in]  aPerformer  name of performer calculating the constraint gradient
 * \param [in]  aIdentifierString  string identifying the constraint criterion being evaluated
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_compute_constraint_gradient_operation
(const XMLGen::Constraint &aConstraint,
 const std::string &aPerformer,
 const std::string &aIdentifierString,
 pugi::xml_node &aParentNode);

/******************************************************************************//**
 * \fn append_compute_shape_sensitivity_on_change_operation
 * \brief Append compute shape sensitivity on change operation for shape optimization problems.
 * \param [out] aParentNode  parent xml node
**********************************************************************************/
void append_compute_shape_sensitivity_on_change_operation
(pugi::xml_node &aParentNode);

/******************************************************************************//**
 * \fn append_aggregate_objective_value_operation
 * \brief Append aggregate objective value operation to interface xml file.
 * \param [in]  aMetaData Plato problem input data
 * \param [out] aParentNode  parent xml node
**********************************************************************************/
void append_aggregate_objective_value_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_aggregate_objective_value_operation_for_multi_load_case
 * \brief Append the aggregate objective value operation for multi-load case problems
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_aggregate_objective_value_operation_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode);

/******************************************************************************//**
 * \fn append_aggregate_objective_value_operation_for_non_multi_load_case
 * \brief Append the aggregate objective value operation for non multi-load case problems
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_aggregate_objective_value_operation_for_non_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode);

/******************************************************************************//**
 * \fn append_compute_criterion_sensitivity_operation
 * \brief Append criterion sensitivity operation for shape optimization problem
 * \param [in]  aMetaData  Plato problem input data
 * \param [in]  aService Current service
 * \param [in]  aSharedDataName Criterion identifier string
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_compute_criterion_sensitivity_operation
(const XMLGen::InputData& aMetaData,
 const XMLGen::Service &aService,
 const std::string &aIdentifierString,
 pugi::xml_node &aParentNode);

/******************************************************************************//**
 * \fn append_aggregate_objective_gradient_operation
 * \brief Append aggregate objective gradient operation to interface xml file.
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_aggregate_objective_gradient_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_aggregate_objective_gradient_operation_for_multi_load_case
 * \brief Append the aggregate objective gradient operation for multi-load case problems.
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
 * \param [in]  aType type of data being aggregated
**********************************************************************************/
void append_aggregate_objective_gradient_operation_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode,
 std::string &aType);

/******************************************************************************//**
 * \fn append_aggregate_objective_gradient_operation_for_non_multi_load_case
 * \brief Append the aggregate objective gradient operation for non multi-load case problems
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
 * \param [in]  aType type of data being aggregated
**********************************************************************************/
void append_aggregate_objective_gradient_operation_for_non_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode,
 std::string &aType);

/******************************************************************************//**
 * \fn append_objective_gradient_operation
 * \brief Append objective gradient operation to interface xml file.
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_objective_gradient_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode);

/******************************************************************************//**
 * \fn append_objective_gradient_operation_for_multi_load_case
 * \brief Append objective gradient operation for multi-load use case to interface xml file.
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_objective_gradient_operation_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode);

/******************************************************************************//**
 * \fn append_objective_gradient_operation_for_non_multi_load_case
 * \brief Append objective gradient operation for single-load use case to interface xml file.
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_objective_gradient_operation_for_non_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode);

}
// namespace XMLGen
