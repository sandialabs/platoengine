#ifndef PLATO_GEOMETRY_LIBRARY_GEOMETRYREGISTRATION
#define PLATO_GEOMETRY_LIBRARY_GEOMETRYREGISTRATION

#include <memory>
#include <string_view>

#include "plato/core/FactoryRegistration.hpp"
#include "plato/core/Function.hpp"
#include "plato/core/VariantInputBuilder.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::linear_algebra
{
struct JacobianMultiplier;
}

namespace plato::geometry::library
{
using GeometryFunction = core::Function<analysis::AnalysisDomainMesh,
                                        linear_algebra::JacobianMultiplier,
                                        const linear_algebra::DynamicVector<double>&>;

struct FactoryTypes
{
    using Compute = GeometryFunction;
    using InitialGuess = linear_algebra::DynamicVector<double>;
    using Bounds = std::pair<std::vector<double>, std::vector<double>>;
    using Output = std::function<void(const linear_algebra::DynamicVector<double>&)>;

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
