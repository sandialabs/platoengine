#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_OutputManager
#define PLATO_THIRDPARTYINTEGRATION_COMMON_OutputManager

#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::geometry::library
{
enum struct OutputMode
{
    kNever,
    kEveryIterationOverwrite,
    kEveryIterationAppend
};

/// @brief Class for managing output of optimization design variables to an exodus mesh.
///
/// Stores and calls the output function implemented by the geometry class used during optimization.
class OutputManager
{
   public:
    using OutputFunction = FactoryTypes::Output;
    OutputManager() = default;

    OutputManager(OutputFunction aOutputFunction, OutputMode aOutputMode);

    /// @brief Call the output function for the design represented by @a aDesignVariables
    void output(const linear_algebra::DynamicVector<double>& aDesignVariables);

   private:
    OutputFunction mOutputFunction;
    OutputMode mOutputMode{OutputMode::kNever};
    std::size_t mIteration{0u};
};
}  // namespace plato::geometry::library
#endif
