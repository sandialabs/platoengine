#include "plato/geometry/extension/BrickShapeGeometry.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <utility>
#include <vector>

#include "plato/geometry/library/GeometryLogger.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/output/OutputInfo.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/FileUtilities.hpp"
#include "plato/utilities/FixedWidthFloatingPointOutput.hpp"

namespace plato::geometry::extension
{
namespace
{
constexpr auto kXCenterIndex = 0U;
constexpr auto kYCenterIndex = 1U;
constexpr auto kZCenterIndex = 2U;
constexpr auto kXLengthIndex = 3U;
constexpr auto kYLengthIndex = 4U;
constexpr auto kZLengthIndex = 5U;

constexpr auto kPrecision = std::size_t{16};
constexpr auto kPrintWidth = std::size_t{24};
constexpr auto kNumDims = std::size_t{3};
constexpr auto kNumNodes = std::size_t{8};
constexpr auto kNumDesignParameters = std::size_t{6};
const std::vector<double> kLowerBounds = {-10.0, -10.0, -10.0, 1e-2, 1e-2, 1e-2};  // Arbitrary
const std::vector<double> kUpperBounds = {10.0, 10.0, 10.0, 1e2, 1e2, 1e2};        // Arbitrary

[[nodiscard]] auto mesh_path(const library::ValidatedGeometryInput& aGeometryInput) -> std::filesystem::path
{
    const auto& tInput = input_validation::get_input_block<input_parser::brick_shape_geometry>(aGeometryInput);
    return tInput.mesh_name.value().mToken;
}

[[nodiscard]] auto make_output() -> library::FactoryTypes::Output
{
    return [](const linear_algebra::DynamicVector<double>& aSolution, const output::OutputInfo& aOutputInfo)
    { return BrickShapeGeometry::output(aSolution, aOutputInfo); };
}

[[maybe_unused]] static const auto kBrickShapeGeometryParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::brick_shape_geometry>{};

[[maybe_unused]] static const auto kBrickShapeGeometryRegistration = plato::geometry::library::GeometryRegistration{
    input_parser::block_name<input_parser::brick_shape_geometry>(),
    [](const library::ValidatedGeometryInput& aGeometryInput)
    {
        return library::FactoryTypes{make_brick_shape_geometry(BrickShapeGeometry{mesh_path(aGeometryInput)}),
                                     BrickShapeGeometry::initialGuess(), BrickShapeGeometry::bounds(), make_output()};
    }};

[[maybe_unused]] static const auto kBrickShapeInputValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{[](const input_parser::brick_shape_geometry& aInput)
                                                         { return library::detail::validate_mesh_name(aInput); }};

}  // namespace

BrickShapeGeometry::BrickShapeGeometry(std::filesystem::path aFileName, const std::optional<double> aDiscretizationSize)
    : mFileName(utilities::make_filename_unique(std::move(aFileName))), mDiscretizationSize(aDiscretizationSize)
{
}

BrickShapeGeometry::~BrickShapeGeometry()
{
    if (!mFileName.empty())
    {
        std::filesystem::remove(mFileName);
    }
}

BrickShapeGeometry::BrickShapeGeometry(const BrickShapeGeometry& aBrickShapeGeometry)
    : mFileName{utilities::make_filename_unique(aBrickShapeGeometry.mFileName)},
      mDiscretizationSize{aBrickShapeGeometry.mDiscretizationSize}
{
}

BrickShapeGeometry::BrickShapeGeometry(BrickShapeGeometry&& aBrickShapeGeometry) noexcept
    : mFileName{std::move(aBrickShapeGeometry.mFileName)}, mDiscretizationSize{aBrickShapeGeometry.mDiscretizationSize}
{
    aBrickShapeGeometry.mFileName.clear();
}

auto BrickShapeGeometry::operator=(const BrickShapeGeometry& aBrickShapeGeometry) -> BrickShapeGeometry&
{
    if (&aBrickShapeGeometry == this)
    {
        return *this;
    }

    mFileName = utilities::make_filename_unique(aBrickShapeGeometry.mFileName);
    mDiscretizationSize = aBrickShapeGeometry.mDiscretizationSize;

    return *this;
}

auto BrickShapeGeometry::operator=(BrickShapeGeometry&& aBrickShapeGeometry) noexcept -> BrickShapeGeometry&
{
    if (&aBrickShapeGeometry == this)
    {
        return *this;
    }

    mFileName = std::move(aBrickShapeGeometry.mFileName);
    mDiscretizationSize = aBrickShapeGeometry.mDiscretizationSize;
    aBrickShapeGeometry.mFileName.clear();

    return *this;
}

auto BrickShapeGeometry::generateMesh(const BrickDesign& aDesignParameters) const -> analysis::AnalysisDomainMesh
{
    [[maybe_unused]] const auto tTaskLogger = library::mesh_generation_task_log<input_parser::brick_shape_geometry>();

    detail::create_mesh(aDesignParameters, mFileName, mDiscretizationSize);
    return analysis::AnalysisDomainMesh{mFileName, {}};
}

auto BrickShapeGeometry::jacobian(const BrickDesign& aDesignParameters) const -> linear_algebra::JacobianColumnEvaluator
{
    return linear_algebra::JacobianColumnEvaluator{
        /*.mColumns=*/kNumDesignParameters,
        /*.mX=*/detail::to_dynamic_vector(aDesignParameters),
        /*.mColumnFunction=*/
        [](unsigned int aColumnIndex, const linear_algebra::DynamicVector<double>&)
        {
            [[maybe_unused]] const auto tTaskLogger = library::jacobian_task_log<input_parser::brick_shape_geometry>();
            return linear_algebra::DynamicVector<double>(detail::sensitivities(aColumnIndex));
        }};
}

auto BrickShapeGeometry::adjointJacobian(const BrickDesign& aDesignParameters) const
    -> linear_algebra::JacobianColumnEvaluator
{
    return linear_algebra::JacobianColumnEvaluator{
        kNumDims * kNumNodes, detail::to_dynamic_vector(aDesignParameters),
        [](unsigned int aAdjointColumnIndex, const linear_algebra::DynamicVector<double>&)
        {
            [[maybe_unused]] const auto tTaskLogger =
                library::adjoint_jacobian_task_log<input_parser::brick_shape_geometry>();

            auto tAdjointColumn = std::vector<double>(kNumDesignParameters);
            std::generate(tAdjointColumn.begin(), tAdjointColumn.end(),
                          [tAdjointRowIndex = 0, aAdjointColumnIndex]() mutable
                          { return detail::sensitivities(tAdjointRowIndex++)[aAdjointColumnIndex]; });
            return linear_algebra::DynamicVector(std::move(tAdjointColumn));
        }};
}

auto BrickShapeGeometry::initialGuess() -> linear_algebra::DynamicVector<double>
{
    return detail::to_dynamic_vector(BrickDesign{});
}

auto BrickShapeGeometry::bounds() -> std::pair<std::vector<double>, std::vector<double>>
{
    return {kLowerBounds, kUpperBounds};
}

auto BrickShapeGeometry::file_name() const -> const std::filesystem::path& { return mFileName; }

void BrickShapeGeometry::output(const linear_algebra::DynamicVector<double>& aSolution,
                                const output::OutputInfo& aOutputInfo)
{
    using TableOutput = utilities::FixedWidthFloatingPointOutput<double, kPrecision, kPrintWidth>;

    auto tBrickParameterTable = std::stringstream{};
    tBrickParameterTable << "\nBrick geometry parameters:\n";
    tBrickParameterTable << "iteration: " << aOutputInfo.mIteration << "\n";
    tBrickParameterTable << "centers: " << TableOutput{aSolution[kXCenterIndex]}
                         << TableOutput{aSolution[kYCenterIndex]} << TableOutput{aSolution[kZCenterIndex]} << "\n";
    tBrickParameterTable << "dimensions: " << TableOutput{aSolution[kXLengthIndex]}
                         << TableOutput{aSolution[kYLengthIndex]} << TableOutput{aSolution[kZLengthIndex]} << "\n";

    auto tLogger = geometry::library::geometry_logger<input_parser::brick_shape_geometry>();
    tLogger.logInfo(tBrickParameterTable.str());
}

auto make_brick_shape_geometry(const BrickShapeGeometry& aBrickShapeGeometry) -> library::GeometryFunction
{
    return library::GeometryFunction{
        [tBrickShapeGeometry = aBrickShapeGeometry](const linear_algebra::DynamicVector<double>& x)
        { return tBrickShapeGeometry.generateMesh(detail::to_design_parameters(x)); },
        [tBrickShapeGeometry = aBrickShapeGeometry](const linear_algebra::DynamicVector<double>& x)
        { return to_jacobian_multiplier(tBrickShapeGeometry.jacobian(detail::to_design_parameters(x))); },
        [tBrickShapeGeometry = aBrickShapeGeometry](const linear_algebra::DynamicVector<double>& x)
        {
            return linear_algebra::AdjointJacobianMultiplier{
                to_jacobian_multiplier(tBrickShapeGeometry.adjointJacobian(detail::to_design_parameters(x)))};
        }};
}

namespace detail
{
void create_mesh(const BrickDesign& aDesign,
                 const std::filesystem::path& aOutputFile,
                 const std::optional<double> aDiscretizationSize)
{
    namespace tpistkio = third_party_integration::stk_io;
    const auto tLowerBounds = tpistkio::CommandBounds{aDesign.center_x - aDesign.dimension_x / 2.0,
                                                      aDesign.center_y - aDesign.dimension_y / 2.0,
                                                      aDesign.center_z - aDesign.dimension_z / 2.0};
    const auto tUpperBounds = tpistkio::CommandBounds{aDesign.center_x + aDesign.dimension_x / 2.0,
                                                      aDesign.center_y + aDesign.dimension_y / 2.0,
                                                      aDesign.center_z + aDesign.dimension_z / 2.0};
    auto tNumberOfElements = tpistkio::CommandNumberOfElements{1, 1, 1};
    if (aDiscretizationSize)
    {
        const auto tNx = static_cast<unsigned int>(std::ceil(aDesign.dimension_x / aDiscretizationSize.value()));
        const auto tNy = static_cast<unsigned int>(std::ceil(aDesign.dimension_y / aDiscretizationSize.value()));
        const auto tNz = static_cast<unsigned int>(std::ceil(aDesign.dimension_z / aDiscretizationSize.value()));
        tNumberOfElements = {tNx, tNy, tNz};
    }

    const auto tSidesets = tpistkio::NodeSetSideSetIdentifiers{tpistkio::UseLowerX{false}, tpistkio::UseUpperX{false},
                                                               tpistkio::UseLowerY{false}, tpistkio::UseUpperY{false},
                                                               tpistkio::UseLowerZ{false}, tpistkio::UseUpperZ{true}};
    const auto tNodesets = tpistkio::NodeSetSideSetIdentifiers{tpistkio::UseLowerX{false}, tpistkio::UseUpperX{false},
                                                               tpistkio::UseLowerY{false}, tpistkio::UseUpperY{true},
                                                               tpistkio::UseLowerZ{false}, tpistkio::UseUpperZ{true}};
    constexpr int tPrecision = 16;

    const tpistkio::CommandGenerator tGenerator{
        tNumberOfElements, tLowerBounds, tUpperBounds, tpistkio::CommandElementType::Hex,
        tNodesets,         tSidesets,    tPrecision};
    tpistkio::write_mesh(aOutputFile, tGenerator);
}

auto sensitivities(const unsigned int aParameterIndex) -> std::vector<double>
{
    assert(aParameterIndex < kNumDesignParameters);
    // design parameters are center (x,y,z), dimension (x,y,z)
    // All nodes or just corners? ESP is based on surface nodeset not all interior nodes, settle for corners right now
    // Assuming nodes are min(x),min(y),min(z) -> increasing x, increasing y, increasing z
    auto tBase = std::array{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    constexpr auto xdim = std::array{-0.5, 0.5, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5};
    constexpr auto ydim = std::array{-0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5};
    constexpr auto zdim = std::array{-0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5};

    switch (aParameterIndex)
    {
        case kXLengthIndex:
            tBase = xdim;
            break;
        case kYLengthIndex:
            tBase = ydim;
            break;
        case kZLengthIndex:
            tBase = zdim;
            break;
        default:
            break;
    }

    std::vector<double> sensitive(tBase.size() * kNumDims, 0);
    const unsigned int shift = aParameterIndex % 3;
    for (unsigned int b = 0; b < tBase.size(); ++b)
    {
        sensitive[kNumDims * b + shift] = tBase.at(b);
    }
    return sensitive;
}

auto to_dynamic_vector(const BrickDesign& aDesignParameters) -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{aDesignParameters.center_x,    aDesignParameters.center_y,
                                                 aDesignParameters.center_z,    aDesignParameters.dimension_x,
                                                 aDesignParameters.dimension_y, aDesignParameters.dimension_z};
}

auto to_design_parameters(const linear_algebra::DynamicVector<double>& aDesignParameter) -> BrickDesign
{
    assert(aDesignParameter.size() == kNumDesignParameters);
    return BrickDesign{.center_x = aDesignParameter[kXCenterIndex],
                       .center_y = aDesignParameter[kYCenterIndex],
                       .center_z = aDesignParameter[kZCenterIndex],
                       .dimension_x = aDesignParameter[kXLengthIndex],
                       .dimension_y = aDesignParameter[kYLengthIndex],
                       .dimension_z = aDesignParameter[kZLengthIndex]};
}
}  // namespace detail

}  // namespace plato::geometry::extension
