#include "plato/geometry/extension/BrickShapeGeometry.hpp"

#include <filesystem>
#include <iomanip>
#include <sstream>

#include "plato/core/ValidationRegistration.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::geometry::extension
{
namespace
{
constexpr int kNumDims = 3;
constexpr int kNumDesignParameters = 6;
const std::vector<double> kLowerBounds = {-10.0, -10.0, -10.0, 1e-2, 1e-2, 1e-2};  // Arbitrary
const std::vector<double> kUpperBounds = {10.0, 10.0, 10.0, 1e2, 1e2, 1e2};        // Arbitrary

[[nodiscard]] std::filesystem::path mesh_path(const library::ValidatedGeometryInput& aGeometryInput)
{
    const auto& tInput = library::geometry_raw_input<input_parser::brick_shape_geometry>(aGeometryInput);
    return tInput.mesh_name.value().mName;
}

[[nodiscard]] std::function<void(const linear_algebra::DynamicVector<double>&)> make_output()
{
    return [](const linear_algebra::DynamicVector<double>& aSolution) { return BrickShapeGeometry::output(aSolution); };
}

[[maybe_unused]] static auto kBrickShapeGeometryRegistration =
    plato::geometry::library::GeometryRegistration{
        input_parser::block_name<input_parser::brick_shape_geometry>(),
        [](const library::ValidatedGeometryInput& aGeometryInput)
        {
            return library::FactoryTypes{make_brick_shape_geometry(BrickShapeGeometry{mesh_path(aGeometryInput)}),
                                         BrickShapeGeometry::initialGuess(), BrickShapeGeometry::bounds(),
                                         make_output()};
        }};

[[maybe_unused]] static auto kBrickShapeValidationRegistration =
    core::ValidationRegistration<input_parser::brick_shape_geometry>{
        [](const input_parser::brick_shape_geometry& aInput) { return library::detail::validate_mesh_name(aInput); }};
}  // namespace

BrickShapeGeometry::BrickShapeGeometry(std::filesystem::path aFileName, const std::optional<double> aDiscretizationSize)
    : mFileName(std::move(aFileName)), mDiscretizationSize(aDiscretizationSize)
{
}

core::MeshProxy BrickShapeGeometry::generateMesh(const BrickDesign& aDesignParameters) const
{
    std::shared_ptr<stk::mesh::BulkData> tMesh = detail::create_mesh(aDesignParameters, mDiscretizationSize);
    plato::utilities::write_mesh(mFileName, tMesh);
    return core::MeshProxy{mFileName, {}};
}

linear_algebra::JacobianColumnEvaluator BrickShapeGeometry::jacobian(const BrickDesign& aDesignParameters) const
{
    return linear_algebra::JacobianColumnEvaluator{
        /*.mColumns=*/kNumDesignParameters,
        /*.mX=*/detail::to_dynamic_vector(aDesignParameters),
        /*.mColumnFunction=*/[](unsigned int i, const linear_algebra::DynamicVector<double>&) {
            return linear_algebra::DynamicVector<double>(detail::sensitivities(i));
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

void BrickShapeGeometry::output(const linear_algebra::DynamicVector<double>& aSolution)
{
    std::cout << "centers: " << aSolution[0] << " " << aSolution[1] << " " << aSolution[2] << std::endl;
    std::cout << "dimensions: " << aSolution[3] << " " << aSolution[4] << " " << aSolution[5] << std::endl;
}

auto make_brick_shape_geometry(const BrickShapeGeometry& aBrickShapeGeometry)
    -> core::Function<core::MeshProxy, linear_algebra::JacobianMultiplier, const linear_algebra::DynamicVector<double>&>
{
    return core::make_function(
        [tBrickShapeGeometry = aBrickShapeGeometry](const linear_algebra::DynamicVector<double>& x)
        { return tBrickShapeGeometry.generateMesh(detail::to_design_parameters(x)); },
        [tBrickShapeGeometry = aBrickShapeGeometry](const linear_algebra::DynamicVector<double>& x)
        { return to_jacobian_multiplier(tBrickShapeGeometry.jacobian(detail::to_design_parameters(x))); });
}

namespace detail
{
std::shared_ptr<stk::mesh::BulkData> create_mesh(const BrickDesign& aDesign,
                                                 const std::optional<double> aDiscretizationSize)
{
    std::stringstream generationCommand;
    generationCommand << std::setprecision(16);
    generationCommand << "generated:";
    if (aDiscretizationSize)
    {
        const auto tNx = static_cast<int>(std::ceil(aDesign.dimension_x / aDiscretizationSize.value()));
        const auto tNy = static_cast<int>(std::ceil(aDesign.dimension_y / aDiscretizationSize.value()));
        const auto tNz = static_cast<int>(std::ceil(aDesign.dimension_z / aDiscretizationSize.value()));
        generationCommand << tNx << "x" << tNy << "x" << tNz;
    }
    else
    {
        generationCommand << "1x1x1";
    }
    generationCommand << "|bbox:";
    const double xmin = aDesign.center_x - aDesign.dimension_x / 2.0;
    const double ymin = aDesign.center_y - aDesign.dimension_y / 2.0;
    const double zmin = aDesign.center_z - aDesign.dimension_z / 2.0;
    generationCommand << xmin << "," << ymin << "," << zmin << ",";
    const double xmax = aDesign.center_x + aDesign.dimension_x / 2.0;
    const double ymax = aDesign.center_y + aDesign.dimension_y / 2.0;
    const double zmax = aDesign.center_z + aDesign.dimension_z / 2.0;
    generationCommand << xmax << "," << ymax << "," << zmax;
    generationCommand << "|sideset:Z|nodeset:Y";
    std::cout << generationCommand.str() << std::endl;
    return plato::utilities::create_mesh(generationCommand.str());
}

std::vector<double> sensitivities(const unsigned int aParameterIndex)
{
    // design parameters are center (x,y,z), dimension (x,y,z)
    // All nodes or just corners? ESP is based on surface nodeset not all interior nodes, settle for corners right now
    // Assuming nodes are min(x),min(y),min(z) -> increasing x, increasing y, increasing z
    std::vector<double> base = {1, 1, 1, 1, 1, 1, 1, 1};
    const std::vector<double> xdim = {-0.5, 0.5, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5};
    const std::vector<double> ydim = {-0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5};
    const std::vector<double> zdim = {-0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5};

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
