#include <gtest/gtest.h>

#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{
TEST(KernelFilterCenteringTypes, EnumTable)
{
    EXPECT_TRUE(input_parser::kKernelFilterCenteringTypesTable.toString(
        input_parser::KernelFilterCenteringTypes::kElementCentered));
    EXPECT_EQ(input_parser::kKernelFilterCenteringTypesTable
                  .toString(input_parser::KernelFilterCenteringTypes::kElementCentered)
                  .value(),
              "element");
    EXPECT_TRUE(input_parser::kKernelFilterCenteringTypesTable.toString(
        input_parser::KernelFilterCenteringTypes::kNodeCentered));
    EXPECT_EQ(
        input_parser::kKernelFilterCenteringTypesTable.toString(input_parser::KernelFilterCenteringTypes::kNodeCentered)
            .value(),
        "node");
}
}  // namespace plato::filter::extension::kernel_filters::unittest
