#ifndef PLATO_OUTPUT_OUTPUTMANAGER
#define PLATO_OUTPUT_OUTPUTMANAGER

#include <cstdint>
#include <functional>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/output/OutputInfo.hpp"

namespace plato::output
{
enum struct OutputMode : std::uint8_t
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
    using OutputFunction = std::function<void(const linear_algebra::DynamicVector<double>&, const OutputInfo&)>;
    OutputManager() = default;

    OutputManager(OutputFunction aOutputFunction, OutputMode aOutputMode);

    /// @brief Call the output function for the design represented by @a aDesignVariables
    void output(const linear_algebra::DynamicVector<double>& aDesignVariables);

   private:
    OutputFunction mOutputFunction;
    OutputMode mOutputMode{OutputMode::kNever};
    std::size_t mIteration{0u};
};
}  // namespace plato::output
#endif
