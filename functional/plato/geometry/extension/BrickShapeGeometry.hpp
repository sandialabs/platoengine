#ifndef PLATO_GEOMETRY_EXTENSION_BRICKSHAPEGEOMETRY
#define PLATO_GEOMETRY_EXTENSION_BRICKSHAPEGEOMETRY

#include <filesystem>
#include <optional>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"

// clang-format off
PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser),
    brick_shape_geometry,
    "Help placeholder for brick_shape_geometry",
    (plato::input_parser::FileName, mesh_name, "Required field specifying the exodus mesh file name to which the the brick mesh output is written."))
// clang-format on

namespace plato::geometry::extension
{
/// @brief Design parameters for BrickShapeGeometry
struct BrickDesign
{
    double mCenterX = 0.0;
    double mCenterY = 0.0;
    double mCenterZ = 0.0;
    double mDimensionX = 1.0;
    double mDimensionY = 1.0;
    double mDimensionZ = 1.0;
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

    BrickShapeGeometry(const BrickShapeGeometry& aBrickShapeGeometry);
    BrickShapeGeometry(BrickShapeGeometry&& aBrickShapeGeometry) noexcept;
    auto operator=(const BrickShapeGeometry& aBrickShapeGeometry) -> BrickShapeGeometry&;
    auto operator=(BrickShapeGeometry&& aBrickShapeGeometry) noexcept -> BrickShapeGeometry&;

    [[nodiscard]] auto generateMesh(const BrickDesign& aDesignParameters) const -> analysis::AnalysisDomainMesh;

    [[nodiscard]] static auto jacobian(const BrickDesign& aDesignParameters) -> linear_algebra::JacobianColumnEvaluator;

    [[nodiscard]] static auto adjointJacobian(const BrickDesign& aDesignParameters)
        -> linear_algebra::JacobianColumnEvaluator;

    /// @brief Returns the file name at which that this object will write its mesh.
    /// @note This is different than the file passed on construction, it will have a unique string appended.
    [[nodiscard]] auto fileName() const -> const std::filesystem::path&;

    [[nodiscard]] static auto initialGuess() -> linear_algebra::DynamicVector<double>;

    [[nodiscard]] static auto bounds() -> std::pair<std::vector<double>, std::vector<double>>;

    static void output(const linear_algebra::DynamicVector<double>& aSolution, const output::OutputInfo& aOutputInfo);

   private:
    std::filesystem::path mFileName;
    std::optional<double> mDiscretizationSize;
};

/// @brief Generate a geometry function, that can be composed with an objective function.
[[nodiscard]] auto make_brick_shape_geometry(const BrickShapeGeometry& aBrickShapeGeometry)
    -> library::GeometryFunction;

namespace detail
{
[[nodiscard]] auto to_design_parameters(const linear_algebra::DynamicVector<double>& aDesignParameter) -> BrickDesign;

void create_mesh(const BrickDesign& aDesign,
                 const std::filesystem::path& aOutputFile,
                 std::optional<double> aDiscretizationSize = std::nullopt);

[[nodiscard]] auto sensitivities(unsigned int aParameterIndex) -> std::vector<double>;

[[nodiscard]] auto to_dynamic_vector(const BrickDesign& aDesignParameters) -> linear_algebra::DynamicVector<double>;
}  // namespace detail

}  // namespace plato::geometry::extension
#endif
