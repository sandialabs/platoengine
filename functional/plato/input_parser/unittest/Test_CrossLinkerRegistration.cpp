#include <gtest/gtest.h>

#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/CrossReference.hpp"

namespace
{
using GeometryNewCrossReference = plato::input_parser::NewCrossReference<plato::input_parser::ComponentType::kGeometry>;
}

// clang-format off
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         block_with_geometry_cross_reference,
                         (GeometryNewCrossReference, my_geom, "help"))
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         geometry_block,
                         (int, a_parameter, "help"))
// clang-format off

namespace plato::input_parser::unittest
{
namespace
{
[[maybe_unused]] static const auto kCrossLinkerRegistration = plato::input_parser::ComponentParserRegistration<block_with_geometry_cross_reference, ComponentType::kProcessManager>{};
}

TEST(CrossLinkerRegistration, Register)
{
    auto tBlockWithGeometryCrossReference=
        InputDataBlock{/*.mComponentType=*/ComponentType::kProcessManager,
                       /*.mBlockName=*/"block_with_geometry_cross_reference", /*.mInput=*/CrossReferencedInput{block_with_geometry_cross_reference{}}};
    constexpr auto tParameterValue = 64;
    const auto tGeometryBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"geometry_block", /*.mInput=*/CrossReferencedInput{geometry_block{/*.a_parameter=*/tParameterValue}}};
   
    const auto tParsedInput = NewParsedInput{{tBlockWithGeometryCrossReference, tGeometryBlock}};

    for(const auto& tCrossLinker : registered_cross_linkers())
    {
        tCrossLinker.crossLink(tBlockWithGeometryCrossReference, tParsedInput);
    }

    const auto& tResultBlock = tBlockWithGeometryCrossReference.mInput.get<block_with_geometry_cross_reference>();

    ASSERT_TRUE(tResultBlock.my_geom.has_value());
    ASSERT_TRUE(tResultBlock.my_geom.value().mInputBlock.has_value());
    ASSERT_TRUE(tResultBlock.my_geom.value().mInputBlock.holds_expected_type<geometry_block>());
    ASSERT_TRUE(tResultBlock.my_geom.value().mInputBlock.get<geometry_block>().a_parameter.has_value());
    EXPECT_EQ(tResultBlock.my_geom.value().mInputBlock.get<geometry_block>().a_parameter.value(), tParameterValue);
}
}
