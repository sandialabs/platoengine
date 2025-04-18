#include <gtest/gtest.h>

#include <boost/optional/optional_io.hpp>

#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/CrossLinkedInput.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidatedInputTypeWrapper.hpp"

namespace
{
using FilterNewCrossReference = plato::input_parser::NewCrossReference<plato::input_parser::ComponentType::kFilter>;
}

// clang-format off
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         land_creatures,
                         plato::input_parser::ComponentType::kGeometry,
                         (FilterNewCrossReference, my_sea_creature, ""))

PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         sea_creatures, plato::input_parser::ComponentType::kFilter,
                         (double, octopus, "")
                         (unsigned int, squid, ""))
// clang-format on

namespace plato::input_validation::unittest
{
TEST(ValidatedInputTypeWrapper, GetCrossReference)
{
    [[maybe_unused]] const auto kLandCreaturesParserRegistration =
        input_parser::ComponentParserRegistration<input_parser::land_creatures>{};

    [[maybe_unused]] const auto kSeaCreaturesParserRegistration =
        input_parser::ComponentParserRegistration<input_parser::sea_creatures>{};

    const auto tLandCreature = plato::input_parser::land_creatures{};
    const auto tParsedLandCreature = input_parser::InputDataBlock{
        input_parser::ComponentType::kGeometry, "land_creatures", input_parser::CrossReferencedInput{tLandCreature}};

    const auto tSeaCreature = plato::input_parser::sea_creatures{/*.octopus=*/42.0, /*.squid=*/100};
    const auto tParsedSeaCreature = input_parser::InputDataBlock{input_parser::ComponentType::kFilter, "sea_creatures",
                                                                 input_parser::CrossReferencedInput{tSeaCreature}};
    const auto tParsedInput = std::vector<input_parser::InputDataBlock>{tParsedLandCreature, tParsedSeaCreature};

    const auto tCrossLinkedInput = input_parser::make_cross_linked_input(input_parser::ParsedInput{tParsedInput});
    ASSERT_TRUE(tCrossLinkedInput.hasValue()) << tCrossLinkedInput.error();

    const auto tValidatedInputOrError = make_validated_input(tCrossLinkedInput.value());
    ASSERT_TRUE(tValidatedInputOrError.hasValue()) << tValidatedInputOrError.error();
    const auto& tValidatedInput = tValidatedInputOrError.value().get<input_parser::ComponentType::kGeometry>();

    const auto tValidatedSeaCreatures = validated_cross_reference<input_parser::ComponentType::kFilter>(
        tValidatedInput,
        [](const input_parser::land_creatures& aLandCreature) { return aLandCreature.my_sea_creature; });

    EXPECT_EQ(tValidatedSeaCreatures.rawInput().mInput.get<input_parser::sea_creatures>().squid, 100U);
}
}  // namespace plato::input_validation::unittest
