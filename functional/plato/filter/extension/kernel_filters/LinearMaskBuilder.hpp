#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_LINEARMASKBUILDER
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_LINEARMASKBUILDER

#include <boost/mpi/communicator.hpp>
#include <functional>
#include <memory>

#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"
#include "plato/filter/extension/kernel_filters/SphericalMaskUtilities.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/third_party_integration/tpetra/Utilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::mesh
{
class Mesh;
}

namespace plato::filter::extension::kernel_filters
{

/// @brief a struct which defines the key pieces of a kernel filter. The SearchFunction and UnnormalizedWeightFunction
/// are the key pieces that define a symmetry filter by defining relationships between the source and target mesh and
/// how to populate the weights in a row of the kernel filter.
struct KernelFilterFunctions
{
    /// @brief a function that takes target and source vectors returns specialized search results
    using SearchFunction = std::function<third_party_integration::stk_search::SearchResults(
        const TargetRowVector&, const SourceColumnVector&, const boost::mpi::communicator&)>;

    /// @brief a function that returns a weight for a given target and source point
    using UnnormalizedWeightFunction = std::function<double(const SourcePoint&, const TargetPoint&)>;

    double mRadius;
    input_parser::KernelFilterCenteringTypes mFilterCentering;
    SearchFunction mSearchFunction;
    UnnormalizedWeightFunction mUnnormalizedWeightFunction;
    std::string mLoggingName;
};

/// @brief Container for a kernel filter's search and weight functions and other defining properties, along with a
/// function that generates the target mesh.
template <typename InputParserType>
struct KernelFilterAndMeshFunctions
{
    /// @brief a function that returns the target mesh
    using TargetMeshFunction = std::function<mesh::Mesh(const InputParserType&)>;

    KernelFilterFunctions mKernelFilterFunctions;
    TargetMeshFunction mTargetMeshFunction;
};

using SourceMesh = utilities::NamedType<mesh::Mesh, struct SourceMeshTag>;
using TargetMesh = utilities::NamedType<mesh::Mesh, struct TargetMeshTag>;

/// @brief A mask generation class for the filters that are based on a linear mask, kernel filter and symmetry filters.
///
/// The intent is that this object gets created during some initialization phase and does not get updated thereafter.
class LinearMaskBuilder
{
   public:
    /// @brief Construction from a source mesh @a aSourceMesh and @a aTargetMesh, given the filter info @a aFilterInfo
    ///
    /// This will create a filter that expects a nodal field defined at all nodes and will filter to either nodes or
    /// element centroids as given by @a aCenteringType.
    LinearMaskBuilder(const SourceMesh& aSourceMesh,
                      const TargetMesh& aTargetMesh,
                      input_parser::KernelFilterCenteringTypes aCenteringType,
                      const KernelFilterFunctions& aFilterType,
                      const boost::mpi::communicator& aCommunicator);

    /// @brief Construction directly from vectors of points.
    ///
    /// @param aNodalCoordinates is a vector of nodal coordinates, presumably from the mesh directly.
    /// @param aCenters is a vector of the element centroids if it is element centered or nodes if it is nodal.
    /// @param aFilterInfo all the information required to create this mask
    /// @param aMaximumConnectivityEstimate is an estimate provided to the Tpetra CRS Matrix during allocation. It
    /// should be a maximum expected to avoid any additional allocation time.
    LinearMaskBuilder(const SourceColumnVector& aNodalCoordinates,
                      TargetRowVector aCenters,
                      const KernelFilterFunctions& aFilterType,
                      const int aMaximumConnectivityEstimate,
                      const boost::mpi::communicator& aCommunicator);

    /// @brief Return a reference to the distance mask.
    [[nodiscard]] auto mask() const -> const third_party_integration::tpetra::TpetraCRSMatrix&;

   private:
    /// @brief Perform the calculation of the full NxM distance map.
    void generateDistanceMap();

    /// @brief Given a sphere ID @a aSphereID and a node entry @a aNodeID call the detail functions to determine the
    /// weight with the filter parameters
    [[nodiscard]] double unnormalized_weight(const RowSearchEntityGlobalID aSphereID, const ColumnNodeGlobalID aNodeID);

    /// @brief Take the RowMap data structure @a aRowMap and convert it to a CRS matrix
    void create_linear_mask(const RowMap& aRowMap);

    /// @brief Take the search results from STK @a aSearchResults and convert them to a RowMap, a map that takes Tpetra
    /// global ID and maps it to the details regarding that row (nonzero entries, their values, and their total)
    [[nodiscard]] RowMap create_normalized_row_map(
        const third_party_integration::stk_search::SearchResults& aSearchResults);

    boost::mpi::communicator mCommunicator;
    std::vector<third_party_integration::common::Coordinate> mGlobalRowCenterCoordinates;
    std::vector<third_party_integration::common::Coordinate> mGlobalColumnCenterCoordinates;
    unsigned int mMaximumConnectivityEstimate = 1;
    KernelFilterFunctions mFilterType;
    std::unique_ptr<third_party_integration::tpetra::TpetraCRSMatrix> mLinearMask;
};

namespace detail
{
///@brief Given a mesh @a aMesh, return the nodal coordinates or the element centroids depending on the value of @a
/// aCenteringType
[[nodiscard]] auto center_coordinates(const mesh::Mesh& aMesh, input_parser::KernelFilterCenteringTypes aCenteringType)
    -> std::vector<third_party_integration::common::Coordinate>;

}  // namespace detail

}  // namespace plato::filter::extension::kernel_filters
#endif
