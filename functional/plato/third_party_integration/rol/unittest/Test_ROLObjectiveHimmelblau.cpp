#include <gtest/gtest.h>

#include <ROL_StdVector.hpp>
#include <memory>

#include "plato/core/Aggregate.hpp"
#include "plato/test_utilities/Himmelblau.hpp"
#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"
#include "plato/third_party_integration/rol/unittest/DynamicVectorHimmelblauFunction.hpp"
#include "plato/third_party_integration/rol/unittest/HimmelblauHelpers.hpp"

namespace plato::third_party_integration::rol::unittest
{
TEST(ROLObjectiveFunction, HimmelblauObjectiveValueAndGradient)
{
    const auto tArgument = make_himmelblau_dynamic_vector_function(plato::test_utilities::Himmelblau{});
    run_himmelblau_objective_test(tArgument);
}

TEST(ROLObjectiveFunction, AggregateTwoHimmelblauObjectives)
{
    namespace pt = plato::test_utilities;
    using HimmelblauF = std::decay_t<decltype(make_himmelblau_dynamic_vector_function(pt::Himmelblau{}))>;
    using HimmelblauFunctionAndWeight = std::vector<std::pair<HimmelblauF, double>>;
    const HimmelblauFunctionAndWeight tFunctionAndWeight{
        std::make_pair(make_himmelblau_dynamic_vector_function(pt::Himmelblau{}), .30),
        std::make_pair(make_himmelblau_dynamic_vector_function(pt::Himmelblau{}), .70)};

    const auto tArgument = core::make_aggregate_function(tFunctionAndWeight);
    run_himmelblau_objective_test(tArgument);
}
}  // namespace plato::third_party_integration::rol::unittest
