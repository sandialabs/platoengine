#include "plato/process_manager/library/ValidatedInput.hpp"

#include <iterator>
#include <string>
#include <string_view>

#include "plato/core/InputVariantUtilities.hpp"
#include "plato/criteria/library/ConstraintValidation.hpp"
#include "plato/criteria/library/ObjectiveValidation.hpp"
#include "plato/filter/library/FilterValidation.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/input_parser/InputParser.hpp"
#include "plato/process_manager/library/CrossReferenceUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerValidation.hpp"
#include "plato/process_manager/library/ValidateCrossReferences.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::process_manager::library
{
ValidatedInput::ValidatedInput(input_parser::ParsedInput aInput, const Key&) : mInput{std::move(aInput)}
{
    apply_to_cross_references(mInput, [](auto& aField, const auto& aInputBlock, const auto& aFullInput)
                              { ValidatedInput::fillCrossReference(aField, aInputBlock, aFullInput); });
}

ValidatedInput::Geometry ValidatedInput::geometry() const
{
    return core::ValidatedInputTypeWrapper{
        validatedVariant<typename Geometry::RawInputType>(geometry::library::first_geometry_input(mInput))};
}

ValidatedInput::Objectives ValidatedInput::objectives() const
{
    return core::ValidatedInputTypeWrapper{validatedVector(mInput.mObjectives)};
}

ValidatedInput::Constraints ValidatedInput::constraints() const
{
    return core::ValidatedInputTypeWrapper{validatedVector(mInput.mConstraints)};
}

ValidatedInput::ProcessManagers ValidatedInput::processManagers() const
{
    auto tRawProcessManagerInputs = core::all_input_blocks_in_variant<ProcessManagerInput>(mInput);
    using ProcessManagersRawInput = typename process_manager::library::ValidatedProcessManagerInputVector::RawInputType;
    using ProcessManagerValidatedVariant = typename ProcessManagersRawInput::value_type;
    auto tValidatedInputs = ProcessManagersRawInput{};
    std::transform(tRawProcessManagerInputs.begin(), tRawProcessManagerInputs.end(),
                   std::back_inserter(tValidatedInputs),
                   [](ProcessManagerInput& aRawInput)
                   { return validatedVariant<ProcessManagerValidatedVariant>(std::move(aRawInput)); });
    return core::ValidatedInputTypeWrapper{tValidatedInputs};
}

template <typename T>
std::vector<core::ValidatedInputTypeWrapper<T>> ValidatedInput::validatedVector(const std::vector<T>& aInputs)
{
    std::vector<core::ValidatedInputTypeWrapper<T>> tValidatedInputs;
    std::transform(aInputs.cbegin(), aInputs.cend(), std::back_inserter(tValidatedInputs),
                   [](T aT) { return core::ValidatedInputTypeWrapper{std::move(aT)}; });
    return tValidatedInputs;
}

template <typename ValidatedInputVariant, typename InputVariant>
ValidatedInputVariant ValidatedInput::validatedVariant(InputVariant aInputVariant)
{
    std::optional<ValidatedInputVariant> tValidatedInput;
    std::visit(
        [&tValidatedInput](auto&& tInput)
        {
            using InputType = std::decay_t<decltype(tInput)>;
            tValidatedInput = core::ValidatedInputTypeWrapper{std::forward<InputType>(tInput)};
        },
        std::move(aInputVariant));
    assert(tValidatedInput);
    return *tValidatedInput;
}

ValidatedInput make_validated_input(input_parser::ParsedInput aInput)
{
    auto tMessages = plato::geometry::library::validate_geometry(aInput, std::vector<std::string>{});
    tMessages = plato::filter::library::validate_filter(aInput, std::move(tMessages));
    tMessages = plato::criteria::library::validate_objectives(aInput.mObjectives, std::move(tMessages));
    tMessages = plato::criteria::library::validate_constraints(aInput.mConstraints, std::move(tMessages));
    tMessages = plato::process_manager::library::validate_process_managers(aInput, std::move(tMessages));
    tMessages = validate_cross_referenced_input(aInput, std::move(tMessages));

    if (!tMessages.empty())
    {
        throw plato::utilities::Exception("Error: Could not validate input, the following errors were found: \n" +
                                          utilities::concatenate_container(tMessages, "\n"));
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

}  // namespace plato::process_manager::library
