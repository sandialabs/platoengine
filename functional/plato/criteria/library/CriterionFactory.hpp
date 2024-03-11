#ifndef PLATO_CRITERIA_LIBRARY_CRITERIONFACTORY
#define PLATO_CRITERIA_LIBRARY_CRITERIONFACTORY

#include <string>

#include "plato/core/Function.hpp"
#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::criteria::library
{
using CriterionFunction = core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>;

/// @brief Converts either objective or constraint input objects to a common CriterionInput struct
/// @tparam Input Must be either input_parser::objective or input_parser::constraint input structs
template <typename Input>
[[nodiscard]] CriterionInput to_criterion_input(const Input& aInput);

/// @brief Creates a criterion Function object from either objective or constraint input objects.
/// @tparam Input Must be either input_parser::objective or input_parser::constraint input structs
template <typename Input>
[[nodiscard]] CriterionFunction make_criterion_function(const Input& aInput);

template <typename Input>
CriterionFunction make_criterion_function(const Input& aValidatedInput)
{
    static_assert(std::is_same_v<Input, core::ValidatedInputTypeWrapper<input_parser::objective>> ||
                      std::is_same_v<Input, core::ValidatedInputTypeWrapper<input_parser::constraint>>,
                  "make_criterion_function must only be called with input_parser::objective or "
                  "input_parser::constraint wrapped in "
                  "ValidatedInputTypeWrapper");

    const auto& tRawInput = aValidatedInput.rawInput();
    const std::string tAppName = input_parser::kCodeOptionsTable.toString(tRawInput.app.value()).value();
    const std::optional<CriterionFunction> tCriterion =
        core::create_object_from_factory<CriterionFunction, CriterionInput>(tAppName,
                                                                            to_criterion_input(aValidatedInput));
    if (tCriterion)
    {
        return std::move(tCriterion).value();
    }
    else
    {
        throw utilities::Exception("App not supported.");
    }
}

template <typename Input>
CriterionInput to_criterion_input(const Input& aInput)
{
    static_assert(
        std::is_same_v<Input, core::ValidatedInputTypeWrapper<input_parser::objective>> ||
            std::is_same_v<Input, core::ValidatedInputTypeWrapper<input_parser::constraint>>,
        "to_criterion_input must only be called with input_parser::objective or input_parser::constraint wrapped in "
        "ValidatedInputTypeWrapper");
    return CriterionInput{
        /*.mSharedLibraryPath=*/aInput.rawInput().shared_library_path.value_or(input_parser::FileName{}),
        /*.mNumberOfProcessors=*/aInput.rawInput().number_of_processors.value_or(1),
        /*.mInputFiles=*/aInput.rawInput().input_files.value_or(input_parser::FileList{})};
}

}  // namespace plato::criteria::library

#endif
