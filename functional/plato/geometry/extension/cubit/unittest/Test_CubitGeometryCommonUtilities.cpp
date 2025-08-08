#include <gtest/gtest.h>

#include "plato/geometry/extension/cubit/CubitGeometryCommonUtilities.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::geometry::extension::cubit::unittest
{
namespace
{

const auto kRadius =
    input_parser::BoundedApreproVariable{input_parser::ApreproString{"radius"}, 1, plato::input_parser::Bounds{1, 2}};
const auto kHeight =
    input_parser::BoundedApreproVariable{input_parser::ApreproString{"height"}, 1, plato::input_parser::Bounds{0.5, 2}};
const auto kXShift = input_parser::BoundedApreproVariable{input_parser::ApreproString{"xshift"}, 0.5,
                                                          plato::input_parser::Bounds{0., 2}};

const auto kCubit = input_parser::cubit_parameterized_shape{
    /*.mesh_journal_file=*/input_parser::FileName{"cylinder.jou"},
    /*.output_file_name=*/input_parser::FileName{"output.txt"},
    /*.mesh_file_name=*/input_parser::FileName{"mesh.exo"},
    /*.output_mesh_sensitivities_name=*/boost::none,
    /*.aprepro_variables=*/input_parser::BoundedApreproVariableList{{kRadius, kHeight, kXShift}}};

}  // namespace

TEST(CubitGeometryCommon, InitializeVariables)
{
    const auto tResult = initialize_variables(kCubit);
    ASSERT_EQ(tResult.size(), 3U);
    EXPECT_EQ(tResult.front().mName, kRadius.mName.mToken);
    EXPECT_EQ(tResult.front().mPerturbationScale.mValue,
              perturbation_scale(kCubit.aprepro_variables.value().mList.front()));
    EXPECT_EQ(tResult.front().mValue.mValue, 1);
    EXPECT_EQ(tResult.back().mName, kXShift.mName.mToken);
    EXPECT_EQ(tResult.back().mPerturbationScale.mValue,
              perturbation_scale(kCubit.aprepro_variables.value().mList.back()));
    EXPECT_EQ(tResult.back().mValue.mValue, .50);
}

TEST(CubitGeometryCommon, UpdateVariables)
{
    auto tApreproVariables = initialize_variables(kCubit);
    const auto tDesignVariables = linear_algebra::DynamicVector<double>{0.2, 0.3, 0.4};
    update_variables(tApreproVariables, tDesignVariables);
    ASSERT_EQ(tApreproVariables.size(), 3U);
    EXPECT_EQ(tApreproVariables.front().mValue.mValue, .2);
    EXPECT_EQ(tApreproVariables.back().mValue.mValue, 0.4);

    EXPECT_EQ(tApreproVariables.front().mName, kRadius.mName.mToken);
    EXPECT_EQ(tApreproVariables.front().mPerturbationScale.mValue,
              perturbation_scale(kCubit.aprepro_variables.value().mList.front()));
    EXPECT_EQ(tApreproVariables.back().mName, kXShift.mName.mToken);
    EXPECT_EQ(tApreproVariables.back().mPerturbationScale.mValue,
              perturbation_scale(kCubit.aprepro_variables.value().mList.back()));
}

TEST(CubitGeometryCommon, InitialGuess)
{
    const auto tResult = initial_guess(kCubit);
    EXPECT_EQ(tResult.size(), 3U);
    EXPECT_EQ(tResult.stdVector().front(), 1.0);
    EXPECT_EQ(tResult.stdVector().back(), .50);
}

TEST(CubitGeometryCommon, Bounds)
{
    const auto tBounds = bounds(kCubit);
    ASSERT_EQ(tBounds.first.size(), 3U);
    EXPECT_EQ(tBounds.first.front(), 1);
    EXPECT_EQ(tBounds.first.back(), 0);
    ASSERT_EQ(tBounds.second.size(), 3U);
    EXPECT_EQ(tBounds.second.front(), 2.0);
    EXPECT_EQ(tBounds.second.back(), 2.0);
}

TEST(CubitGeometryCommonValidation, ValidateOutputFileName)
{
    auto tCubit = kCubit;
    EXPECT_FALSE(validate_output_file_name(tCubit).has_value());
    tCubit.output_file_name = boost::none;
    EXPECT_TRUE(validate_output_file_name(tCubit).has_value());
}

TEST(CubitGeometryCommonValidation, ValidateMeshFileName)
{
    auto tCubit = kCubit;
    EXPECT_FALSE(validate_mesh_file_name(tCubit).has_value());
    tCubit.mesh_file_name = boost::none;
    EXPECT_TRUE(validate_mesh_file_name(tCubit).has_value());
}

TEST(CubitGeometryCommonValidation, ApreproVariableOutput)
{
    const std::filesystem::path tFileName = "outputfile.txt";
    const auto tSolution = linear_algebra::DynamicVector<double>({1.1, 2.2});
    const auto tVariables = std::vector<PerturbableApreproVariable>{PerturbableApreproVariable{"first"},
                                                                    PerturbableApreproVariable{"second"}};

    aprepro_variable_output(tFileName, tSolution, tVariables);

    plato::test_utilities::test_for_existence_and_remove({tFileName}, TEST_CONTEXT("Remove output file."));
}

TEST(CubitGeometryCommonValidation, ValidateLowerBoundsLessThanUpper)
{
    {
        EXPECT_FALSE(validate_lower_bounds_less_than_upper(kCubit).has_value());
    }
    {
        auto tCubit = kCubit;
        tCubit.aprepro_variables.value().mList.front().mBounds.mLower = 20;
        tCubit.aprepro_variables.value().mList.back().mBounds.mLower = 40;
        EXPECT_TRUE(validate_lower_bounds_less_than_upper(tCubit).has_value());
    }
    {
        auto tCubit = kCubit;
        tCubit.aprepro_variables.value().mList.front().mBounds.mUpper = -20;
        tCubit.aprepro_variables.value().mList.back().mBounds.mUpper = -40;
        EXPECT_TRUE(validate_lower_bounds_less_than_upper(tCubit).has_value());
    }
}

TEST(CubitGeometryCommonValidation, ApreproVariablesExist)
{
    auto tCubit = kCubit;
    EXPECT_FALSE(validate_aprepro_variables_exists(tCubit));
    tCubit.aprepro_variables = boost::none;
    EXPECT_TRUE(validate_aprepro_variables_exists(tCubit));
}

TEST(CubitGeometryCommonUtilities, PerturbationScale)
{
    constexpr auto tScaleFactor = 1.0e-4;
    constexpr auto tACISFloor = 1.0e-6;
    {
        const auto tVariable = input_parser::BoundedApreproVariable{input_parser::ApreproString{"var"}, 0,
                                                                    plato::input_parser::Bounds{1, 2}};
        const auto tPerturbationScale = perturbation_scale(tVariable);
        EXPECT_EQ(tPerturbationScale, tScaleFactor) << "Lower bounds times scale factor is scale.";
    }
    {
        const auto tVariable = input_parser::BoundedApreproVariable{input_parser::ApreproString{"var"}, 0,
                                                                    plato::input_parser::Bounds{-1000, 2}};
        const auto tPerturbationScale = perturbation_scale(tVariable);
        EXPECT_EQ(tPerturbationScale, 2 * tScaleFactor) << "Upper bounds times scale factor is scale.";
    }
    {
        const auto tVariable = input_parser::BoundedApreproVariable{input_parser::ApreproString{"var"}, 0,
                                                                    plato::input_parser::Bounds{1e-3, 2}};
        const auto tPerturbationScale = perturbation_scale(tVariable);
        EXPECT_EQ(tPerturbationScale, tACISFloor) << "Acis floor is scale.";
    }
}

}  // namespace plato::geometry::extension::cubit::unittest
