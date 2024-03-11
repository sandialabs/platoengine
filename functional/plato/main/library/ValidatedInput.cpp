#include "plato/main/library/ValidatedInput.hpp"

#include <string>
#include <string_view>

#include "plato/criteria/library/ConstraintValidation.hpp"
#include "plato/criteria/library/ObjectiveValidation.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/input_parser/InputParser.hpp"
#include "plato/optimizer/OptimizerValidation.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::main::library
{
ValidatedInput::ValidatedInput(input_parser::ParsedInput aInput, Key) : mInput{std::move(aInput)} {}

ValidatedInput::Geometry ValidatedInput::geometry() const
{
    plato::geometry::library::GeometryInput tGeometryInput =
        plato::geometry::library::first_geometry_input(mInput);
    using ValidatedGeometryVariant =
        typename plato::geometry::library::ValidatedGeometryInput::RawInputType;
    // Use visit with a return value in c++20
    std::optional<ValidatedGeometryVariant> tValidatedGeometry;
    std::visit([&tValidatedGeometry](auto&& tGeometry)
               { tValidatedGeometry = core::ValidatedInputTypeWrapper{std::move(tGeometry)}; },
               std::move(tGeometryInput));
    assert(tValidatedGeometry);
    return core::ValidatedInputTypeWrapper{*tValidatedGeometry};
}

ValidatedInput::Objectives ValidatedInput::objectives() const
{
    return core::ValidatedInputTypeWrapper{validatedVector(mInput.mObjectives)};
}

ValidatedInput::Constraints ValidatedInput::constraints() const
{
    return core::ValidatedInputTypeWrapper{validatedVector(mInput.mConstraints)};
}

ValidatedInput::OptimizationParameters ValidatedInput::optimizationParameters() const
{
    return core::ValidatedInputTypeWrapper{mInput.mOptimizationParameters};
}

template <typename T>
std::vector<core::ValidatedInputTypeWrapper<T>> ValidatedInput::validatedVector(const std::vector<T>& aInputs)
{
    std::vector<core::ValidatedInputTypeWrapper<T>> tValidatedInputs;
    std::transform(aInputs.cbegin(), aInputs.cend(), std::back_inserter(tValidatedInputs),
                   [](T aT) { return core::ValidatedInputTypeWrapper{std::move(aT)}; });
    return tValidatedInputs;
}

ValidatedInput make_validated_input(input_parser::ParsedInput aInput)
{
    std::vector<std::string> tMessages;
    tMessages = plato::geometry::library::validate_geometry(aInput, std::move(tMessages));
    tMessages = plato::criteria::library::validate_objectives(aInput.mObjectives, std::move(tMessages));
    tMessages = plato::criteria::library::validate_constraints(aInput.mConstraints, std::move(tMessages));
    tMessages = plato::optimizer::validate_optimization_parameters(aInput.mOptimizationParameters,
                                                                               std::move(tMessages));
    if (!tMessages.empty())
    {
        throw plato::utilities::Exception(
            "Error: Could not validate input, the following errors were found: \n" + core::all_messages(tMessages));
    }
    return ValidatedInput{std::move(aInput), Key{}};
}

ValidatedInput parse_and_validate_from_file(const std::filesystem::path& aFileName)
{
    return make_validated_input(input_parser::parse_input_from_file(aFileName));
}

ValidatedInput parse_and_validate(const std::string_view aInput)
{
    return make_validated_input(input_parser::parse_input(aInput));
}

}  // namespace plato::main::library
