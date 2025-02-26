#ifndef PLATO_GEOMETRY_LIBRARY_GEOMETRYREGISTRATION
#define PLATO_GEOMETRY_LIBRARY_GEOMETRYREGISTRATION

#include <memory>
#include <string_view>

#include "plato/core/FactoryRegistration.hpp"
#include "plato/core/Function.hpp"
#include "plato/core/VariantInputBuilder.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::geometry::library
{
struct OutputInfo;
}

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::geometry::library
{
using GeometryFunction =
    core::Function<const linear_algebra::DynamicVector<double>&,
                   core::FunctionInfo<analysis::AnalysisDomainMesh, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>,
                   core::FunctionInfo<linear_algebra::AdjointJacobianMultiplier,
                                      core::evaluation::kFirstDerivative,
                                      core::MatrixOrdering::kAdjoint>>;

struct FactoryTypes
{
    using Compute = GeometryFunction;
    using InitialGuess = linear_algebra::DynamicVector<double>;
    using Bounds = std::pair<std::vector<double>, std::vector<double>>;
    using Output = std::function<void(const linear_algebra::DynamicVector<double>&, const OutputInfo&)>;

    Compute mCompute;
    InitialGuess mInitialGuess;
    Bounds mBounds;
    Output mOutput;
};

/// A `std::variant` with alternatives corresponding to input blocks
/// created using the PLATO_GEOMETRY_INPUT_BLOCK_STRUCT macro.
using GeometryInput = core::InputVariant<input_parser::ParsedInput, input_parser::IsGeometryInput>;
using ValidatedGeometryInput = core::ValidatedInputTypeWrapper<
    core::ValidatedInputVariant<input_parser::ParsedInput, input_parser::IsGeometryInput>>;
using GeometryRegistration = core::FactoryRegistration<FactoryTypes, ValidatedGeometryInput>;

/// @return A GeometryInput variant, which is the first non-empty geometry input block found in @a aInput.
/// @throw Exception If no geometry block was defined in @a aInput.
[[nodiscard]] library::GeometryInput first_geometry_input(const input_parser::ParsedInput& aInput);

[[nodiscard]] bool is_geometry_function_registered(const std::string_view aFunctionName);

/// @brief Helper to get the cross-referenced validated filter input block.
template <typename FilterInputType, typename Geometry>
[[nodiscard]] const FilterInputType get_cross_referenced_filter(const Geometry& aGeometry)
{
    return aGeometry.filter->mInputBlock.template get<FilterInputType>();
}
}  // namespace plato::geometry::library

#endif
