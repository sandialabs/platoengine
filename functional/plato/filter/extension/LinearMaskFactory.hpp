#ifndef PLATO_FILTER_EXTENSION_LINEARMASKFACTORY
#define PLATO_FILTER_EXTENSION_LINEARMASKFACTORY

#include <boost/mpi/communicator.hpp>

#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/third_party_integration/tpetra/Utilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::mesh
{
class Mesh;
}

namespace plato::filter::extension
{

using SearchRadius = utilities::NamedType<double, struct SearchRadiusTag>;
using Distance = utilities::NamedType<double, struct DistanceTag>;

using NodalVector =
    utilities::NamedType<std::vector<third_party_integration::common::Coordinate>, struct NodalVectorTag>;
using CenterVector =
    utilities::NamedType<std::vector<third_party_integration::common::Coordinate>, struct CenterVectorTag>;

using TpetraGlobalOrdinalVector = std::vector<third_party_integration::tpetra::TpetraGlobalOrdinal>;
using TpetraScalarVector = std::vector<third_party_integration::tpetra::TpetraScalar>;

/// @brief A mask generation class for the Kernel filter.
///
/// The intent is that this object gets created during some initialization phase and does not get updated thereafter.
class LinearMaskFactory
{
   public:
    /// @brief Construction from a mesh.
    ///
    /// This will create a filter that expects a nodal field defined at all nodes and will filter to either nodes or
    /// element centroids as given by @a aCenteringType.
    LinearMaskFactory(const mesh::Mesh& aMesh,
                      input_parser::KernelFilterCenteringTypes aCenteringType,
                      const SearchRadius aSearchRadius,
                      const boost::mpi::communicator& aCommunicator);

    /// @brief Construction directly from vectors of points.
    ///
    /// @param aNodalCoordinates is a vector of nodal coordinates, presumably from the mesh directly.
    /// @param aCenters is a vector of the element centroids if it is element centered or nodes if it is nodal.
    /// @param aSearchRadius is the search radius the distance map will be calculated over. It is used in a STK search
    /// as well as the linear function to determine the weight.
    /// @param aMaximumConnectivityEstimate is an estimate provided to the Tpetra CRS Matrix during allocation. It
    /// should be a maximum expected to avoid any additional allocation time.
    LinearMaskFactory(const NodalVector& aNodalCoordinates,
                      CenterVector aCenters,
                      const SearchRadius aSearchRadius,
                      const int aMaximumConnectivityEstimate,
                      const boost::mpi::communicator& aCommunicator);

    /// @brief Return a reference to the distance mask.
    [[nodiscard]] auto returnMask() const -> const third_party_integration::tpetra::TpetraCRSMatrix&;

   private:
    /// @brief create nodal coordinate tpetra container of  @a aNodalCoordinates
    third_party_integration::tpetra::TpetraMultiVector createNodalCoordinates(
        const std::vector<third_party_integration::common::Coordinate>& aNodalCoordinates);

    /// @brief Perform a parallel computation of the row calculation assuming some center @a aCenter.
    ///  Return a pair of vectors one with the global ordinals and the other the corresponding normalized weights.
    ///  An individual row should sum to 1.
    [[nodiscard]] auto generateRow(third_party_integration::common::Coordinate aCenter)
        -> std::pair<TpetraGlobalOrdinalVector, TpetraScalarVector>;

    /// @brief Perform the calculation of the full NxM distance map.
    void generateDistanceMap();

   private:
    boost::mpi::communicator mCommunicator;

    double mSearchRadius = 1;
    unsigned int mMaximumConnectivityEstimate = 1;

    std::vector<third_party_integration::common::Coordinate> mRowCenterCoordinates;
    third_party_integration::tpetra::TpetraMultiVector mNodalCoordinates;

    Teuchos::RCP<third_party_integration::tpetra::TpetraCRSMatrix> mLinearMask;

    std::vector<third_party_integration::stk_search::SearchPointWithIdentifier> mLocalSearchPointWithIdentifiers;
};

namespace detail
{
/// @brief an empirically determined value for a uniform hex mesh and filter radii that are similar in size to the
/// element size.
// clang-format off
/// A value of 1.5 will cover FilterRadii that are 1.45*Element_Length and larger
/// A value of 1.2 will cover FilterRadii that are 2.6*Element_Length and larger
/// A value of 1.1 will cover FilterRadii that are 3.9*Element_Length and larger
// clang-format on
constexpr double kMaxMultiplier = 1.5;

using RowSum = utilities::NamedType<double, struct RowSumTag>;
using EstimatedConnectivity = utilities::NamedType<unsigned int, struct EstimatedConnectivityTag>;

/// @brief Computes the linear tophat function based on a distance @a aDistance and a search radius @a aSearchRadius
[[nodiscard]] double linear_ramp_weight(const Distance aDistance, const SearchRadius aSearchRadius);

/// @brief Compute the volume of a sphere with radius @a aFilterRadius
[[nodiscard]] double filter_volume(const SearchRadius aSearchRadius);

/// @brief Compute the area of a circle with radius @a aFilterRadius
[[nodiscard]] double filter_area(const SearchRadius aSearchRadius);

/// @brief Compute maximum expected connectivity in a row for mesh @a aMesh, with a filter sphere with radius
/// @a aFilterRadius
[[nodiscard]] unsigned int maximum_connectivity_estimate(const mesh::Mesh& aMesh, const SearchRadius aFilterRadius);

/// @brief Helper function that takes a tpetra vector @a RowVector and its sum of values @a aRowSum, and normalizes and
/// populates the non-zero weights into a pair of vectors storing the global indices and weights.
[[nodiscard]] auto normalize_nonzero_weights(const third_party_integration::tpetra::TpetraVector& aRowVector,
                                             const RowSum aRowSum,
                                             const EstimatedConnectivity aEstimatedConnectivity)
    -> std::pair<TpetraGlobalOrdinalVector, TpetraScalarVector>;

/// @brief Helper function to take a multivector @a aNodalCoordinates and a given rank @a aRank will generate the STK
/// formatted Search point equal to the node and assigned a local id and rank.
[[nodiscard]] std::vector<third_party_integration::stk_search::SearchPointWithIdentifier> stk_search_points(
    const third_party_integration::tpetra::TpetraMultiVector& aNodalCoordinates, const int aRank);

}  // namespace detail

}  // namespace plato::filter::extension

#endif
