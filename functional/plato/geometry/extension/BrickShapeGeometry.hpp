#ifndef PLATO_GEOMETRY_EXTENSION_BRICKSHAPEGEOMETRY
#define PLATO_GEOMETRY_EXTENSION_BRICKSHAPEGEOMETRY

#include <filesystem>
#include <optional>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

// clang-format off
PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser),
    new_brick_shape_geometry,
    (plato::input_parser::FileName, mesh_name, "Required field specifying the exodus mesh file name to which the the brick mesh output is written."))
// clang-format on

namespace plato::geometry::extension
{
/// @brief Generates a valid brick shape geometry input struct for testing.
[[nodiscard]] auto create_valid_brick_shape_geometry_input() -> input_parser::new_brick_shape_geometry;

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
///
/// @note This geometry type generates a mesh used by downstream functions. The mesh file name is based on
/// the `mesh_name` input field, but with a unique hexadecimal value appended. This is to prevent name clashes
/// when running in parallel.
/// @note The file is maintained by this object and will be deleted when the object goes out of scope.
class BrickShapeGeometry
{
   public:
    /// @param aFileName The name of the file to write the mesh generated in generateMesh
    /// @param aDiscretizationSize When non-empty, this gives the approximate discretization size of the resulting mesh.
    /// I.e., the
    ///  number of elements is `ceil(dimension / discretizationSize)`.
    explicit BrickShapeGeometry(std::filesystem::path aFileName,
                                std::optional<double> aDiscretizationSize = std::nullopt);

    ~BrickShapeGeometry();

    [[nodiscard]] auto generateMesh(const BrickDesign& aDesignParameters) const -> analysis::AnalysisDomainMesh;

    [[nodiscard]] auto jacobian(const BrickDesign& aDesignParameters) const -> linear_algebra::JacobianColumnEvaluator;

    [[nodiscard]] auto adjointJacobian(const BrickDesign& aDesignParameters) const
        -> linear_algebra::JacobianColumnEvaluator;

    [[nodiscard]] static auto initialGuess() -> linear_algebra::DynamicVector<double>;

    [[nodiscard]] static auto bounds() -> std::pair<std::vector<double>, std::vector<double>>;

    static void output(const linear_algebra::DynamicVector<double>& aSolution, const library::OutputInfo& aOutputInfo);

   private:
    std::filesystem::path mFileName;
    std::optional<double> mDiscretizationSize;
};

/// @brief Generate a geometry function, that can be composed with an objective function.
[[nodiscard]] auto make_brick_shape_geometry(const BrickShapeGeometry& aBrickShapeGeometry)
    -> library::GeometryFunction;

namespace detail
{
[[nodiscard]] BrickDesign to_design_parameters(const linear_algebra::DynamicVector<double>& aDesignParameter);

void create_mesh(const BrickDesign& aDesign,
                 const std::filesystem::path& aOutputFile,
                 std::optional<double> aDiscretizationSize = std::nullopt);

[[nodiscard]] std::vector<double> sensitivities(unsigned int aParameterIndex);

[[nodiscard]] linear_algebra::DynamicVector<double> to_dynamic_vector(const BrickDesign& aDesignParameters);
}  // namespace detail

}  // namespace plato::geometry::extension
#endif
