#ifndef PLATO_PROCESSMANAGER_LIBRARY_VALIDATEDINPUT
#define PLATO_PROCESSMANAGER_LIBRARY_VALIDATEDINPUT

#include <filesystem>

#include "plato/core/InputVariantUtilities.hpp"
#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/process_manager/library/CrossReferenceUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

namespace plato::process_manager::library
{
class ValidatedInput;

struct Key
{
    friend ValidatedInput make_validated_input(input_parser::ParsedInput input);

   private:
    Key() {}
    Key(const Key&) {}
};

class ValidatedInput
{
   public:
    using Geometry = plato::geometry::library::ValidatedGeometryInput;
    using Objectives =
        core::ValidatedInputTypeWrapper<std::vector<core::ValidatedInputTypeWrapper<input_parser::objective>>>;
    using Constraints =
        core::ValidatedInputTypeWrapper<std::vector<core::ValidatedInputTypeWrapper<input_parser::constraint>>>;
    using OptimizationParameters = core::ValidatedInputTypeWrapper<input_parser::rol_optimization>;
    using ProcessManagers = plato::process_manager::library::ValidatedProcessManagerInputVector;

   public:
    ValidatedInput(input_parser::ParsedInput aInput, const Key&);

    [[nodiscard]] auto geometry() const -> Geometry;
    [[nodiscard]] auto objectives() const -> Objectives;
    [[nodiscard]] auto constraints() const -> Constraints;
    [[nodiscard]] auto processManagers() const -> ProcessManagers;

   private:
    template <typename T>
    [[nodiscard]] static std::vector<core::ValidatedInputTypeWrapper<T>> validatedVector(const std::vector<T>& aInputs);

    template <typename ValidatedInputVariant, typename InputVariant>
    [[nodiscard]] static ValidatedInputVariant validatedVariant(InputVariant aInputVariant);

    template <typename FieldType, typename InputBlock, typename FullInput>
    static void fillCrossReference(FieldType& aField, const InputBlock&, const FullInput& aFullInput);

   private:
    input_parser::ParsedInput mInput;
};

[[nodiscard]] ValidatedInput make_validated_input(input_parser::ParsedInput aInput);

/// @brief Parse input from file @a aInputFile and then validate the input
[[nodiscard]] ValidatedInput parse_and_validate_from_file(const std::filesystem::path& aFileName);

/// @brief Parse input from input string @a aInput and then validate the input
[[nodiscard]] ValidatedInput parse_and_validate(const std::string_view aInput);

template <typename FieldType, typename InputBlock, typename FullInput>
void ValidatedInput::fillCrossReference(FieldType& aField, const InputBlock&, const FullInput& aFullInput)
{
    using WrappedFieldType = typename plato::core::TypeOrOptional<std::decay_t<decltype(aField)>>::type;
    using VariantType = core::InputVariant<FullInput, WrappedFieldType::template IsVariantType>;
    using ValidatedVariantType = core::ValidatedInputVariant<FullInput, WrappedFieldType::template IsVariantType>;
    if (!aField.has_value())
    {
        aField.emplace();
        const auto aFirstInputBlock = core::first_input_block_in_variant<VariantType>(aFullInput);
        assert(aFirstInputBlock.has_value());
        aField->mInputBlock.set(core::ValidatedInputTypeWrapper{
            validatedVariant<ValidatedVariantType>(std::move(aFirstInputBlock).value())});
    }
    else
    {
        const auto tAllLinkableBlocks = core::all_input_blocks_in_variant<VariantType>(aFullInput);
        const auto& tNamedBlock = detail::find_cross_reference_named_block(aField.value(), tAllLinkableBlocks);
        assert(tNamedBlock != tAllLinkableBlocks.cend());
        aField->mInputBlock.set(core::ValidatedInputTypeWrapper{validatedVariant<ValidatedVariantType>(*tNamedBlock)});
    }
}

}  // namespace plato::process_manager::library

#endif
