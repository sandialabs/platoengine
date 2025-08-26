#include "plato/geometry/extension/BrickShapeGeometry.hpp"

#include <filesystem>

#include "plato/geometry/library/GeometryLogger.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/geometry/library/OutputInfo.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/FileUtilities.hpp"
#include "plato/utilities/FixedWidthFloatingPointOutput.hpp"

namespace plato::geometry::extension
{
namespace
{

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

[[nodiscard]] library::FactoryTypes::Output make_output()
{
    return [](const linear_algebra::DynamicVector<double>& aSolution, const library::OutputInfo& aOutputInfo)
    { return BrickShapeGeometry::output(aSolution, aOutputInfo); };
}

[[maybe_unused]] static auto kBrickShapeGeometryParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::brick_shape_geometry>{};

[[maybe_unused]] static auto kBrickShapeGeometryRegistration = plato::geometry::library::GeometryRegistration{
    input_parser::block_name<input_parser::brick_shape_geometry>(),
    [](const library::ValidatedGeometryInput& aGeometryInput)
    {
        return library::FactoryTypes{make_brick_shape_geometry(BrickShapeGeometry{mesh_path(aGeometryInput)}),
                                     BrickShapeGeometry::initialGuess(), BrickShapeGeometry::bounds(), make_output()};
    }};

[[maybe_unused]] static auto kBrickShapeInputValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{[](const input_parser::brick_shape_geometry& aInput)
                                                         { return library::detail::validate_mesh_name(aInput); }};

}  // namespace

BrickShapeGeometry::BrickShapeGeometry(std::filesystem::path aFileName, const std::optional<double> aDiscretizationSize)
    : mFileName(utilities::make_filename_unique(std::move(aFileName))), mDiscretizationSize(aDiscretizationSize)
{
}

BrickShapeGeometry::~BrickShapeGeometry() { std::filesystem::remove(mFileName); }

analysis::AnalysisDomainMesh BrickShapeGeometry::generateMesh(const BrickDesign& aDesignParameters) const
{
    [[maybe_unused]] const auto tTaskLogger = library::mesh_generation_task_log<input_parser::brick_shape_geometry>();

    detail::create_mesh(aDesignParameters, mFileName, mDiscretizationSize);
    return analysis::AnalysisDomainMesh{mFileName, {}};
}

linear_algebra::JacobianColumnEvaluator BrickShapeGeometry::jacobian(const BrickDesign& aDesignParameters) const
{
    return linear_algebra::JacobianColumnEvaluator{
        /*.mColumns=*/kNumDesignParameters,
        /*.mX=*/detail::to_dynamic_vector(aDesignParameters),
        /*.mColumnFunction=*/[](unsigned int aColumnIndex, const linear_algebra::DynamicVector<double>&)
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

linear_algebra::DynamicVector<double> BrickShapeGeometry::initialGuess()
{
    return detail::to_dynamic_vector(BrickDesign{});
}

std::pair<std::vector<double>, std::vector<double>> BrickShapeGeometry::bounds()
{
    return {kLowerBounds, kUpperBounds};
}

void BrickShapeGeometry::output(const linear_algebra::DynamicVector<double>& aSolution,
                                const library::OutputInfo& aOutputInfo)
{
    using TableOutput = utilities::FixedWidthFloatingPointOutput<double, kPrecision, kPrintWidth>;

    auto tBrickParameterTable = std::stringstream{};
    tBrickParameterTable << "\nBrick geometry parameters:\n";
    tBrickParameterTable << "iteration: " << aOutputInfo.mIteration << "\n";
    tBrickParameterTable << "centers: " << TableOutput{aSolution[0]} << TableOutput{aSolution[1]}
                         << TableOutput{aSolution[2]} << "\n";
    tBrickParameterTable << "dimensions: " << TableOutput{aSolution[3]} << TableOutput{aSolution[4]}
                         << TableOutput{aSolution[5]} << "\n";

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

std::vector<double> sensitivities(const unsigned int aParameterIndex)
{
    assert(aParameterIndex < kNumDesignParameters);
    // design parameters are center (x,y,z), dimension (x,y,z)
    // All nodes or just corners? ESP is based on surface nodeset not all interior nodes, settle for corners right now
    // Assuming nodes are min(x),min(y),min(z) -> increasing x, increasing y, increasing z
    auto base = std::array{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    constexpr auto xdim = std::array{-0.5, 0.5, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5};
    constexpr auto ydim = std::array{-0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5};
    constexpr auto zdim = std::array{-0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5};

    switch (aParameterIndex)
    {
        case 3:
            base = xdim;
            break;
        case 4:
            base = ydim;
            break;
        case 5:
            base = zdim;
            break;
        default:
            break;
    }

    std::vector<double> sensitive(base.size() * kNumDims, 0);
    const unsigned int shift = aParameterIndex % 3;
    for (unsigned int b = 0; b < base.size(); ++b)
    {
        sensitive[kNumDims * b + shift] = base[b];
    }
    return sensitive;
}

linear_algebra::DynamicVector<double> to_dynamic_vector(const BrickDesign& aDesignParameters)
{
    return linear_algebra::DynamicVector<double>{aDesignParameters.center_x,    aDesignParameters.center_y,
                                                 aDesignParameters.center_z,    aDesignParameters.dimension_x,
                                                 aDesignParameters.dimension_y, aDesignParameters.dimension_z};
}

BrickDesign to_design_parameters(const linear_algebra::DynamicVector<double>& aDesignParameter)
{
    assert(aDesignParameter.size() == kNumDesignParameters);
    return BrickDesign{/*.center_x=*/aDesignParameter[0],
                       /*.center_y=*/aDesignParameter[1],
                       /*.center_z=*/aDesignParameter[2],
                       /*.dimension_x=*/aDesignParameter[3],
                       /*.dimension_y=*/aDesignParameter[4],
                       /*.dimension_z=*/aDesignParameter[5]};
}
}  // namespace detail

}  // namespace plato::geometry::extension
