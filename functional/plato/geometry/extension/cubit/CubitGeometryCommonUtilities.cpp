#include "plato/geometry/extension/cubit/CubitGeometryCommonUtilities.hpp"

#include <algorithm>
#include <iterator>

#include "plato/geometry/extension/cubit/BracedOutput.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/utilities/StringUtilities.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::geometry::extension::cubit
{

namespace
{

constexpr auto kPerturbationScale = 1e-4;
constexpr auto kPerturbationACISFloor = 1e-6;

}  // namespace

auto initialize_variables(const input_parser::cubit_parameterized_shape& aInput)
    -> std::vector<PerturbableApreproVariable>
{
    std::vector<PerturbableApreproVariable> tVariables;
    tVariables.reserve(aInput.aprepro_variables.value().mList.size());

    std::ranges::transform(aInput.aprepro_variables.value().mList, std::back_inserter(tVariables),
                           [](const auto aBoundedApreproVariable)
                           {
                               return PerturbableApreproVariable{
                                   aBoundedApreproVariable.mName.mToken, Value{aBoundedApreproVariable.mInitialValue},
                                   PerturbationScale{perturbation_scale(aBoundedApreproVariable)}};
                           });
    return tVariables;
}

void update_variables(std::vector<PerturbableApreproVariable>& aVariables,
                      const linear_algebra::DynamicVector<double>& aDesignParameter)
{
    assert(aVariables.size() == aDesignParameter.size());
    std::transform(aDesignParameter.stdVector().begin(), aDesignParameter.stdVector().end(), aVariables.begin(),
                   aVariables.begin(),
                   [](const double aValue, PerturbableApreproVariable& aVariable)
                   {
                       aVariable.mValue.mValue = aValue;
                       return aVariable;
                   });
}

void restore_aprepro_variables(CubitWrapper& aCubit, const std::vector<PerturbableApreproVariable>& aVariables)
{
    for (const auto& aVariable : aVariables)
    {
        aCubit.addApreproVariable({aVariable.mName, aVariable.mValue.mValue});
    }
}

auto initial_guess(const input_parser::cubit_parameterized_shape& aInput) -> linear_algebra::DynamicVector<double>
{
    std::vector<double> tInitialValues;
    tInitialValues.reserve(aInput.aprepro_variables.value().mList.size());
    std::ranges::transform(aInput.aprepro_variables.value().mList, std::back_inserter(tInitialValues),
                           [](const auto& aBoundedApreproVariable) { return aBoundedApreproVariable.mInitialValue; });

    return linear_algebra::DynamicVector<double>{tInitialValues};
}

auto bounds(const input_parser::cubit_parameterized_shape& aInput)
    -> std::pair<std::vector<double>, std::vector<double>>
{
    std::vector<double> tLowerBounds, tUpperBounds;
    tLowerBounds.reserve(aInput.aprepro_variables.value().mList.size());
    tUpperBounds.reserve(aInput.aprepro_variables.value().mList.size());
    std::for_each(aInput.aprepro_variables.value().mList.begin(), aInput.aprepro_variables.value().mList.end(),
                  [&tLowerBounds, &tUpperBounds](const auto& aBoundedApreproVariable)
                  {
                      tLowerBounds.push_back(aBoundedApreproVariable.mBounds.mLower);
                      tUpperBounds.push_back(aBoundedApreproVariable.mBounds.mUpper);
                  });

    return {tLowerBounds, tUpperBounds};
}

void aprepro_variable_output(const std::filesystem::path& aFileName,
                             const linear_algebra::DynamicVector<double>& aSolution,
                             const std::vector<PerturbableApreproVariable>& aVariables)
{
    std::ofstream tOutfile(aFileName);
    for (const auto& [tVariable, tValue] : utilities::Zip(aVariables, aSolution.stdVector()))
    {
        tOutfile << "# " << BracedOutput{utilities::concatenate(tVariable.mName, " = ", tValue)} << std::endl;
    }
}

std::optional<std::string> validate_mesh_file_name(const input_parser::cubit_parameterized_shape& aInput)
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::cubit_parameterized_shape>(), aInput.mesh_file_name, "mesh_file_name");
}

std::optional<std::string> validate_output_file_name(const input_parser::cubit_parameterized_shape& aInput)
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::cubit_parameterized_shape>(), aInput.output_file_name,
        "output_file_name");
}

std::optional<std::string> validate_lower_bounds_less_than_upper(const input_parser::cubit_parameterized_shape& aInput)
{
    if (aInput.aprepro_variables)
    {
        std::vector<std::string> tInfeasibleBoundsMessages;
        tInfeasibleBoundsMessages.reserve(aInput.aprepro_variables.value().mList.size());

        utilities::transform_if(
            aInput.aprepro_variables.value().mList, std::back_inserter(tInfeasibleBoundsMessages),
            [](const auto& aBoundedApreproVariable)
            {
                return utilities::concatenate("Aprepro variable '", aBoundedApreproVariable.mName.mToken,
                                              "' lower bound must be less than upper bound.");
            },
            [](const auto& aBoundedApreproVariable)
            { return aBoundedApreproVariable.mBounds.mLower >= aBoundedApreproVariable.mBounds.mUpper; });
        if (!tInfeasibleBoundsMessages.empty())
        {
            return utilities::concatenate_container(tInfeasibleBoundsMessages, std::string_view{"\n"});
        }
    }
    return std::nullopt;
}

std::optional<std::string> validate_aprepro_variables_exists(const input_parser::cubit_parameterized_shape& aInput)
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::cubit_parameterized_shape>(), aInput.aprepro_variables,
        "aprepro_variables");
    return std::nullopt;
}

auto perturbation_scale(const input_parser::BoundedApreproVariable& aBoundedApreproVariable) -> double
{
    const auto tUpper = std::abs(aBoundedApreproVariable.mBounds.mUpper);
    const auto tLower = std::abs(aBoundedApreproVariable.mBounds.mLower);
    const auto tMinimum = std::min(tUpper, tLower);
    return std::max(tMinimum * kPerturbationScale, kPerturbationACISFloor);
}

}  // namespace plato::geometry::extension::cubit
