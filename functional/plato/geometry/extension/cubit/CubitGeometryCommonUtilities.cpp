#include "plato/geometry/extension/cubit/CubitGeometryCommonUtilities.hpp"

#include <algorithm>
#include <format>
#include <iterator>

#include "plato/geometry/extension/cubit/BracedOutput.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/StringUtilities.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::geometry::extension::cubit
{

namespace
{
/// Each aprepro variable is defined with a valid range. kPerturbationScale is used to take this range and come up with
/// a suitable perturbation that can be communicated to Cubit with enough digits that it is a meaningful perturbation
/// but also not so large that the nearest node projection gives erroneous results.
constexpr auto kPerturbationScale = 1e-5;
/// After a variable scaling is determined from the perturbation scale, it is compared against the ACIS floor. If the
/// perturbation size is below the floor, it is brought back up to the floor.
constexpr auto kPerturbationACISFloor = 1e-5;

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
    std::ofstream tOutfile(aFileName, std::ios::app);
    for (const auto& [tVariable, tValue] : utilities::Zip(aVariables, aSolution.stdVector()))
    {
        tOutfile << "# " << BracedOutput{utilities::concatenate(tVariable.mName, " = ", std::format("{:.16g}", tValue))}
                 << std::endl;
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

namespace
{
const auto kErrorMessageStringForApreproVariableScaling = [](const auto& aBoundedApreproVariable)
{
    return utilities::concatenate(
        "Aprepro variable '", aBoundedApreproVariable.mName.mToken,
        "' is too small or has a range that is too small. The range (", aBoundedApreproVariable.mBounds.mUpper, "-",
        aBoundedApreproVariable.mBounds.mLower, ") times ", kPerturbationScale, " must be larger than ",
        kPerturbationACISFloor, ". Calculated a value of: ", perturbation_scale(aBoundedApreproVariable),
        ". For accurate results, change your unit system to make all design variables larger than 1.");
};

const auto kPredicateTestForApreproVariableScale = [](const auto& aBoundedApreproVariable)
{ return perturbation_scale(aBoundedApreproVariable) < kPerturbationACISFloor; };

}  // namespace

std::optional<std::string> validate_aprepro_variables_perturbation_scale(
    const input_parser::cubit_parameterized_shape& aInput)
{
    if (aInput.aprepro_variables)
    {
        auto tTruncationErrorMessages =
            utilities::reserved_container<std::vector<std::string>>(aInput.aprepro_variables.value().mList.size());
        utilities::transform_if(aInput.aprepro_variables.value().mList, std::back_inserter(tTruncationErrorMessages),
                                kErrorMessageStringForApreproVariableScaling, kPredicateTestForApreproVariableScale);
        if (!tTruncationErrorMessages.empty())
        {
            return utilities::concatenate_container(tTruncationErrorMessages, std::string_view{"\n"});
        }
    }
    return std::nullopt;
}

auto perturbation_scale(const input_parser::BoundedApreproVariable& aBoundedApreproVariable) -> double
{
    return (aBoundedApreproVariable.mBounds.mUpper - aBoundedApreproVariable.mBounds.mLower) * kPerturbationScale;
}

auto perturbation_scale_multiplier() -> double { return kPerturbationScale; }

}  // namespace plato::geometry::extension::cubit
