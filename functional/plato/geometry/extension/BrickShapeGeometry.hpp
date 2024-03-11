#ifndef PLATO_GEOMETRY_EXTENSION_BRICKSHAPEGEOMETRY
#define PLATO_GEOMETRY_EXTENSION_BRICKSHAPEGEOMETRY

#include <filesystem>
#include <optional>

#include "plato/core/Function.hpp"
#include "plato/core/MeshProxy.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace input_parser
{
struct brick_shape_geometry;
}

namespace stk::mesh
{
class BulkData;
}

namespace plato::geometry::extension
{
/// @brief Design parameters for BrickShapeGeometry
struct BrickDesign
{
    double center_x = 0.0;
    double center_y = 0.0;
    double center_z = 0.0;
    double dimension_x = 1.0;
    double dimension_y = 1.0;
    double dimension_z = 1.0;
};

/// @brief Prototype example of a geometry function that maps a center location and dimensions to a brick.
///
/// The purpose of this class is to demonstrate a geometry function. It uses six design parameters,
/// center coordinates and dimensions, and maps to a hex mesh. The goal is to demonstrate a shape-optimization-like
/// capability similar to ESP for testing purposes.
class BrickShapeGeometry
{
   public:
    /// @param aFileName The name of the file to write the mesh generated in generateMesh
    /// @param aDiscretizationSize When non-empty, this gives the approximate discretization size of the resulting mesh.
    /// I.e., the
    ///  number of elements is `ceil(dimension / discretizationSize)`.
    explicit BrickShapeGeometry(std::filesystem::path aFileName,
                                std::optional<double> aDiscretizationSize = std::nullopt);

    [[nodiscard]] core::MeshProxy generateMesh(const BrickDesign& aDesignParameters) const;

    [[nodiscard]] linear_algebra::JacobianColumnEvaluator jacobian(const BrickDesign& aDesignParameters) const;

    [[nodiscard]] static linear_algebra::DynamicVector<double> initialGuess();

    [[nodiscard]] static std::pair<std::vector<double>, std::vector<double>> bounds();

    static void output(const linear_algebra::DynamicVector<double>& aSolution);

   private:
    std::filesystem::path mFileName;
    std::optional<double> mDiscretizationSize;
};

/// @brief Generate a geometry function, that can be composed with an objective function.
[[nodiscard]] auto make_brick_shape_geometry(const BrickShapeGeometry& aBrickShapeGeometry) -> core::
    Function<core::MeshProxy, linear_algebra::JacobianMultiplier, const linear_algebra::DynamicVector<double>&>;

namespace detail
{
[[nodiscard]] BrickDesign to_design_parameters(const linear_algebra::DynamicVector<double>& aDesignParameter);

[[nodiscard]] std::shared_ptr<stk::mesh::BulkData> create_mesh(
    const BrickDesign& aDesign, std::optional<double> aDiscretizationSize = std::nullopt);

[[nodiscard]] std::vector<double> sensitivities(unsigned int aParameterIndex);

[[nodiscard]] linear_algebra::DynamicVector<double> to_dynamic_vector(const BrickDesign& aDesignParameters);
}  // namespace detail

}  // namespace plato::geometry::extension
#endif
