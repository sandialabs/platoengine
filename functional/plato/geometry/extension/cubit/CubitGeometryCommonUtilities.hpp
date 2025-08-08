#ifndef PLATO_GEOMETRY_EXTENSION_CUBITNGS_CUBITGEOMETRYCOMMONUTILITIES
#define PLATO_GEOMETRY_EXTENSION_CUBITNGS_CUBITGEOMETRYCOMMONUTILITIES

#include <fstream>
#include <optional>
#include <string>

#include "plato/geometry/extension/cubit/CubitGeometry.hpp"
#include "plato/geometry/extension/cubit/CubitWrapper.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::geometry::extension::cubit
{

///@brief Take the input specified in @a aInput and create a vector of perturbable aprepro variables. This function does
/// not affect cubit's environment.
[[nodiscard]] auto initialize_variables(const input_parser::cubit_parameterized_shape& aInput)
    -> std::vector<PerturbableApreproVariable>;

///@brief Given a list of design parameters in @a aDesignParameter, update the vector of perturbabe variables @a
/// aVariables. This function does not affect cubit's environment.
void update_variables(std::vector<PerturbableApreproVariable>& aVariables,
                      const linear_algebra::DynamicVector<double>& aDesignParameter);

///@brief Given a CubitWrapper @a aCubit, and a list of perturbable aprepro variables @a aVariables, go through and
/// update Cubit's environment to match these values.
void restore_aprepro_variables(CubitWrapper& aCubit, const std::vector<PerturbableApreproVariable>& aVariables);

///@brief Given the input in @a aInput, create an initial guess of the design parameters.
[[nodiscard]] auto initial_guess(const input_parser::cubit_parameterized_shape& aInput)
    -> linear_algebra::DynamicVector<double>;

///@brief Given the input in @a aInput, return the bounds of the design parameters.
[[nodiscard]] auto bounds(const input_parser::cubit_parameterized_shape& aInput)
    -> std::pair<std::vector<double>, std::vector<double>>;

///@brief Given the solution @a aSolution and the aprepro variables vector @a aVariables, handle the textual output
/// requested to @a aFileName.
void aprepro_variable_output(const std::filesystem::path& aFileName,
                             const linear_algebra::DynamicVector<double>& aSolution,
                             const std::vector<PerturbableApreproVariable>& aVariables);

///@brief Given an input deck @a aInput, validate that a mesh file name was specified.
[[nodiscard]] std::optional<std::string> validate_mesh_file_name(const input_parser::cubit_parameterized_shape& aInput);

///@brief Given an input deck @a aInput, validate that an output solution file name was specified.
[[nodiscard]] std::optional<std::string> validate_output_file_name(
    const input_parser::cubit_parameterized_shape& aInput);

///@brief Given an input deck @a aInput, make sure that all lower bounds are less than upper.
[[nodiscard]] std::optional<std::string> validate_lower_bounds_less_than_upper(
    const input_parser::cubit_parameterized_shape& aInput);

///@brief Given an input deck @a aInput, ensure that aprepro variables are defined.
[[nodiscard]] std::optional<std::string> validate_aprepro_variables_exists(
    const input_parser::cubit_parameterized_shape& aInput);

///@brief Given a bounded aprepro variable @a aBoundedApreproVariable compute the perturbation scale that will be used
[[nodiscard]] auto perturbation_scale(const input_parser::BoundedApreproVariable& aBoundedApreproVariable) -> double;

}  // namespace plato::geometry::extension::cubit

#endif
