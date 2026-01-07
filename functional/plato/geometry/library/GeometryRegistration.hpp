#ifndef PLATO_GEOMETRY_LIBRARY_GEOMETRYREGISTRATION
#define PLATO_GEOMETRY_LIBRARY_GEOMETRYREGISTRATION

#include <memory>
#include <string_view>

#include "plato/core/FactoryRegistration.hpp"
#include "plato/core/Function.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/output/OutputManager.hpp"

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
    using Output = typename output::OutputManager::OutputFunction;

    Compute mCompute;
    InitialGuess mInitialGuess;
    Bounds mBounds;
    Output mOutput;
};

using ValidatedGeometryInput = input_validation::ValidatedInputDataBlock<components::ComponentType::kGeometry>;
using GeometryRegistration = core::FactoryRegistration<FactoryTypes, ValidatedGeometryInput>;

[[nodiscard]] auto is_geometry_function_registered(std::string_view aFunctionName) -> bool;

/// @brief Helper to get the cross-referenced validated filter input block.
template <typename FilterInputType, typename Geometry>
[[nodiscard]] auto get_cross_referenced_filter(const Geometry& aGeometry) -> FilterInputType
{
    assert(aGeometry.filter->mInputBlock.has_value());
    assert(aGeometry.filter->mInputBlock.template holdsExpectedType<FilterInputType>());
    return aGeometry.filter->mInputBlock.template get<FilterInputType>();
}
}  // namespace plato::geometry::library

#endif
