#ifndef PLATO_FILTER_EXTENSION_KERNELFILTER
#define PLATO_FILTER_EXTENSION_KERNELFILTER

#include <boost/mpi/communicator.hpp>
#include <filesystem>

#include "plato/filter/extension/LinearMask.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/utilities/NamedType.hpp"
#include "plato/utilities/StateCache.hpp"

namespace plato::mesh
{
struct MeshProxy;
}

namespace plato::mesh
{
class Mesh;
}

namespace plato::input_parser
{
struct kernel_filter;
}

namespace plato::filter::extension
{
using FilterRadius = utilities::NamedType<double, struct FilterRadiusTag>;
using FilterCache = plato::utilities::StateCache<std::shared_ptr<library::FilterInterface>, const mesh::MeshProxy&>;

/// @brief An implementation of a kernel filter that relies on Tpetra and STK objects to conduct a search and create a
/// linear mask.
class KernelFilter : public library::FilterInterface
{
   public:
    /// @brief Construct a kernel filter object for a mesh @a aMeshFileName, with a filter radius of @a aFilterRadius,
    /// centered on the elements or nodes determined by @a aFilterCentering, using a communicator @a aCommunicator
    KernelFilter(const std::filesystem::path& aMeshFileName,
                 const FilterRadius aFilterRadius,
                 const input_parser::KernelFilterCenteringTypes aFilterCentering,
                 const boost::mpi::communicator& aCommunicator);

    /// @brief Apply the internal filter to the mesh specified in @a aMeshProxy and return a new MeshProxy object
    [[nodiscard]] mesh::MeshProxy filter(const mesh::MeshProxy& aMeshProxy) const override;

    /// @brief Return the Jacobian of the linear mask applied to a specific vector @a aV
    [[nodiscard]] linear_algebra::DynamicVector<double> jacobianTimesVector(
        const mesh::MeshProxy& aMeshProxy, const linear_algebra::DynamicVector<double>& aV) const override;

   private:
    LinearMask mLinearMask;
    input_parser::KernelFilterCenteringTypes mFilterCentering;
    boost::mpi::communicator mCommunicator;
};

namespace detail
{
std::optional<std::string> validate_kernel_filter_centering_type(const input_parser::kernel_filter& aInput);

/// @brief an empirically determined value for a uniform hex mesh and filter radii that are similar in size to the
/// element size.
// clang-format off
/// A value of 1.5 will cover FilterRadii that are 1.45*Element_Length and larger
/// A value of 1.2 will cover FilterRadii that are 2.6*Element_Length and larger
/// A value of 1.1 will cover FilterRadii that are 3.9*Element_Length and larger
// clang-format on
constexpr double kMaxMultiplier = 1.5;

/// @brief Compute the volume of a sphere with radius @a aFilterRadius
double filter_volume(const FilterRadius aFilterRadius);

/// @brief Compute the area of a circle with radius @a aFilterRadius
double filter_area(const FilterRadius aFilterRadius);

/// @brief Compute maximum expected connectivity in a row for mesh @a aMesh, with a filter sphere with radius
/// @a aFilterRadius
int maximum_connectivity_estimate(const mesh::Mesh& aMesh, const FilterRadius aFilterRadius);

/// @brief Create a LinearMask object a mesh @a aMeshFileName, with a filter sphere with radius @a aFilterRadius,
/// centered on the elements or nodes determined by @a aFilterCentering, using a communicator @a aCommunicator
LinearMask create_linear_mask(const std::filesystem::path& aMeshFileName,
                              const FilterRadius aFilterRadius,
                              const input_parser::KernelFilterCenteringTypes aFilterCentering,
                              const boost::mpi::communicator& aCommunicator);

/// @brief Create a StateCache object for constructing a shared pointer to a KernelFilter if the mesh coordinates have
/// changed (i.e. the mesh has changed)
[[nodiscard]] FilterCache create_filter_cache(const input_parser::kernel_filter& aInput);

}  // namespace detail
}  // namespace plato::filter::extension

#endif
