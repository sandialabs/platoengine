#include <gtest/gtest.h>

#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/CrossLinkedInput.hpp"
#include "plato/input_parser/CrossReference.hpp"

namespace
{
using GeometryCrossReference = plato::input_parser::CrossReference<plato::components::ComponentType::kGeometry>;
}

// clang-format off
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         block_with_geometry_cross_reference,
                         plato::components::ComponentType::kProcessManager,
                         (GeometryCrossReference, my_geom, "help"))
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         geometry_block,
                         plato::components::ComponentType::kGeometry,
                         (int, a_parameter, "help"))
// clang-format off

namespace plato::input_parser::unittest
{
namespace
{
[[maybe_unused]] static const auto kCrossLinkerRegistration = plato::input_parser::ComponentParserRegistration<block_with_geometry_cross_reference>{};
}

TEST(CrossLinkedInput, MakeCrossLinkedInput)
{
    auto tBlockWithGeometryCrossReference=
        InputDataBlock{.mComponentType=components::ComponentType::kProcessManager,
                       .mBlockName="block_with_geometry_cross_reference", .mInput=InputBlockWrapper{block_with_geometry_cross_reference{}}};
    constexpr auto tParameterValue = 64;
    const auto tGeometryBlock =
        InputDataBlock{.mComponentType=components::ComponentType::kGeometry,
                       .mBlockName="geometry_block", .mInput=InputBlockWrapper{geometry_block{/*.a_parameter=*/tParameterValue}}};
   
    const auto tParsedInput = ParsedInput{{tBlockWithGeometryCrossReference, tGeometryBlock}};
    const auto tCrossLinkedInputOrError = make_cross_linked_input(tParsedInput);

    ASSERT_TRUE(tCrossLinkedInputOrError.hasValue());
    const auto& tCrossLinkedInput = tCrossLinkedInputOrError.value();
    ASSERT_EQ(tCrossLinkedInput.rawInput().get<components::ComponentType::kProcessManager>().size(), 1U);
    const auto& tResultBlock = tCrossLinkedInput.rawInput().get<components::ComponentType::kProcessManager>().front().mInput.get<block_with_geometry_cross_reference>();

    ASSERT_TRUE(tResultBlock.my_geom.has_value());
    ASSERT_TRUE(tResultBlock.my_geom.value().mInputBlock.hasValue());
    ASSERT_TRUE(tResultBlock.my_geom.value().mInputBlock.holdsExpectedType<geometry_block>());
    ASSERT_TRUE(tResultBlock.my_geom.value().mInputBlock.get<geometry_block>().a_parameter.has_value());
    EXPECT_EQ(tResultBlock.my_geom.value().mInputBlock.get<geometry_block>().a_parameter.value(), tParameterValue);
}

TEST(CrossLinkedInput, MakeCrossLinkedInputWithError)
{
    auto tBlockWithGeometryCrossReference=
        InputDataBlock{.mComponentType=components::ComponentType::kProcessManager,
                       .mBlockName="block_with_geometry_cross_reference", .mInput=InputBlockWrapper{block_with_geometry_cross_reference{}}};
   
    const auto tParsedInput = ParsedInput{{tBlockWithGeometryCrossReference}};
    const auto tCrossLinkedInputOrError = make_cross_linked_input(tParsedInput);
    EXPECT_TRUE(tCrossLinkedInputOrError.hasError());
}
}
