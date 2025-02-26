#ifndef PLATO_GEOMETRY_LIBRARY_OUTPUTINFO
#define PLATO_GEOMETRY_LIBRARY_OUTPUTINFO

#include <cstddef>

namespace plato::geometry::library
{
/// @brief A struct to hold information for writing output.
/// This is used to indicate to the geometry classes what iteration the fields should be written for and whether the
/// output file should be overwritten
struct OutputInfo
{
    bool mOverwrite;
    std::size_t mIteration;
};

/// @brief A default @a OutputInfo struct for the case where the output file is to be overwritten to contain a single
/// iteration
constexpr inline OutputInfo kOverwriteInfo{true, 1U};
}  // namespace plato::geometry::library

#endif
