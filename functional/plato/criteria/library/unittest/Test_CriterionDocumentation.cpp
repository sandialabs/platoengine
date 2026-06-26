#include <gtest/gtest.h>

#include <algorithm>
#include <boost/mpi/communicator.hpp>
#include <cstddef>
#include <map>
#include <optional>
#include <string_view>
#include <vector>

#include "plato/criteria/library/CriterionDocumentation.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/CriterionTraits.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/services/AppConfiguration.hpp"

namespace plato::criteria::library::unittest
{
namespace
{
[[nodiscard]] auto criterion_function() -> plato::criteria::library::CriterionFunction
{
    return {[](const analysis::AnalysisDomainMesh&) { return 0.0; },
            [](const analysis::AnalysisDomainMesh&) { return linear_algebra::DynamicVector<double>{}; }};
}

[[nodiscard]] auto vector_criterion_function() -> plato::criteria::library::VectorCriterionFunction
{
    return {[](const analysis::AnalysisDomainMesh&) { return linear_algebra::DynamicVector<double>{}; },
            [](const analysis::AnalysisDomainMesh&) { return linear_algebra::JacobianMultiplier{}; },
            [](const analysis::AnalysisDomainMesh&) { return linear_algebra::AdjointJacobianMultiplier{}; }};
}

[[nodiscard]] auto criterion_name(const std::string_view aAppName, const std::string_view aCriterionName)
{
    return criteria::library::criterion_registration_name(input_parser::AppName{std::string{aAppName}},
                                                          input_parser::CriterionName{std::string{aCriterionName}});
}

constexpr auto kCarlCriterionName = std::string_view{"carl"};
const auto kCarlSerialConfiguration = services::CriterionConfiguration{.mName = std::string{kCarlCriterionName},
                                                                       .mIsParallelized = false,
                                                                       .mIsScalar = true,
                                                                       .mFunctionName = "",
                                                                       .mVectorComponents = std::nullopt};
const auto kCarlParallelConfiguration = services::CriterionConfiguration{.mName = std::string{kCarlCriterionName},
                                                                         .mIsParallelized = true,
                                                                         .mIsScalar = true,
                                                                         .mFunctionName = "",
                                                                         .mVectorComponents = std::nullopt};

constexpr auto kDonutCriterionName = std::string_view{"donut"};
const auto kDonutVectorComponents = std::map<std::size_t, std::string>{{0U, "tiara"}, {1U, "magic-missile"}};
const auto kDonutParallelConfiguration = services::CriterionConfiguration{.mName = std::string{kDonutCriterionName},
                                                                          .mIsParallelized = true,
                                                                          .mIsScalar = false,
                                                                          .mFunctionName = "",
                                                                          .mVectorComponents = kDonutVectorComponents};

constexpr auto kMordecaiCriterionName = std::string_view{"mordecai"};
const auto kMordecaiSerialConfiguration = services::CriterionConfiguration{.mName = std::string{kMordecaiCriterionName},
                                                                           .mIsParallelized = false,
                                                                           .mIsScalar = false,
                                                                           .mFunctionName = "",
                                                                           .mVectorComponents = std::nullopt};

constexpr auto kApp1Name = std::string_view{"dcc-b1"};
constexpr auto kApp2Name = std::string_view{"dcc-b2"};

[[maybe_unused]] const auto kCarlSerialCriterionRegistration =
    plato::criteria::library::CriterionRegistration<library::Parallelization::kSerial,
                                                    library::FunctionDimension::kScalar>{
        criterion_name(kApp1Name, kCarlCriterionName), [](const plato::criteria::library::CriterionInput&)
        { return FunctionWithConfiguration{criterion_function(), kCarlSerialConfiguration}; }};

[[maybe_unused]] const auto kCarlParallelCriterionRegistration =
    plato::criteria::library::CriterionRegistration<library::Parallelization::kParallel,
                                                    library::FunctionDimension::kScalar>{
        criterion_name(kApp1Name, kCarlCriterionName),
        [](const plato::criteria::library::CriterionInput&, const boost::mpi::communicator&)
        { return FunctionWithConfiguration{criterion_function(), kCarlParallelConfiguration}; }};

[[maybe_unused]] const auto kDonutParallelCriterionRegistration =
    plato::criteria::library::CriterionRegistration<library::Parallelization::kParallel,
                                                    library::FunctionDimension::kVector>{
        criterion_name(kApp1Name, kDonutCriterionName),
        [](const plato::criteria::library::CriterionInput&, const boost::mpi::communicator&)
        { return FunctionWithConfiguration{vector_criterion_function(), kDonutParallelConfiguration}; }};

[[maybe_unused]] const auto kMordecaiSerialCriterionRegistration =
    plato::criteria::library::CriterionRegistration<library::Parallelization::kSerial,
                                                    library::FunctionDimension::kVector>{
        criterion_name(kApp2Name, kMordecaiCriterionName), [](const plato::criteria::library::CriterionInput&)
        { return FunctionWithConfiguration{vector_criterion_function(), kMordecaiSerialConfiguration}; }};

}  // namespace

TEST(CriterionDocumentation, CriterionDocumentation)
{
    const auto tCriterionDocumentation = criterion_documentation();
    constexpr auto tNumberOfTestCriterion = 4U;
    ASSERT_GE(tCriterionDocumentation.size(), tNumberOfTestCriterion);

    const auto tConcurrencyLabel = std::string{"Parallelization"};
    const auto tSerialLabel = std::string{"serial"};
    const auto tParallelLabel = std::string{"parallel"};
    const auto tDimensionalityLabel = std::string{"Dimensionality"};
    const auto tScalarLabel = std::string{"scalar"};
    const auto tVectorLabel = std::string{"vector"};

    const auto tExpectedSerialCarl =
        CriterionDocumentation{.mAppName = std::string{kApp1Name},
                               .mCriterionName = std::string{kCarlCriterionName},
                               .mDescription = {},
                               .mVectorComponents = {},
                               .mTraits = {{tConcurrencyLabel, tSerialLabel}, {tDimensionalityLabel, tScalarLabel}}};
    const auto tExpectedParallelCarl =
        CriterionDocumentation{.mAppName = std::string{kApp1Name},
                               .mCriterionName = std::string{kCarlCriterionName},
                               .mDescription = {},
                               .mVectorComponents = {},
                               .mTraits = {{tConcurrencyLabel, tParallelLabel}, {tDimensionalityLabel, tScalarLabel}}};
    const auto tExpectedDonut =
        CriterionDocumentation{.mAppName = std::string{kApp1Name},
                               .mCriterionName = std::string{kDonutCriterionName},
                               .mDescription = {},
                               .mVectorComponents = {},
                               .mTraits = {{tConcurrencyLabel, tParallelLabel}, {tDimensionalityLabel, tVectorLabel}}};
    const auto tExpectedMordecai =
        CriterionDocumentation{.mAppName = std::string{kApp2Name},
                               .mCriterionName = std::string{kMordecaiCriterionName},
                               .mDescription = {},
                               .mVectorComponents = {},
                               .mTraits = {{tConcurrencyLabel, tSerialLabel}, {tDimensionalityLabel, tVectorLabel}}};
    const auto tAllExpected =
        std::vector{tExpectedSerialCarl, tExpectedParallelCarl, tExpectedDonut, tExpectedMordecai};

    for (const auto& tExpected : tAllExpected)
    {
        const auto tFound = std::ranges::any_of(tCriterionDocumentation,
                                                [&tExpected](const auto& aResult) { return aResult == tExpected; });
        EXPECT_TRUE(tFound) << tExpected.mAppName << " " << tExpected.mCriterionName;
    }
}
}  // namespace plato::criteria::library::unittest
