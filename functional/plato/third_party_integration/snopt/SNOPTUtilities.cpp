#include "plato/third_party_integration/snopt/SNOPTUtilities.hpp"

#include <boost/numeric/conversion/cast.hpp>

namespace plato::third_party_integration::snopt
{
auto linear_constraint_jacobian_sparse_matrix(const SNOPTConstraints &aConstraintFunctions,
                                              const std::size_t aNumberOfDesignVariables)
    -> SparseMatrixBuilder<IndexType, double>
{
    using Row = typename SparseMatrixBuilder<IndexType, double>::Row;

    auto tSparseMatrix = SparseMatrixBuilder<IndexType, double>{};
    const auto tArbitraryDesignVariables = linear_algebra::DynamicVector(aNumberOfDesignVariables, 1.0);
    for (const auto tLinearConstraintIndex : utilities::IndexRange{aConstraintFunctions.numberOfLinearConstraints()})
    {
        const auto tSignedLinearConstraintIndex = boost::numeric_cast<int>(tLinearConstraintIndex);
        const auto &tLinearConstraint =
            *std::next(aConstraintFunctions.linearConstraintsBegin(), tSignedLinearConstraintIndex);
        const auto tConstraintGradient =
            tLinearConstraint.mFunction.template evaluate<core::evaluation::kFirstDerivative>(
                tArbitraryDesignVariables);
        tSparseMatrix.appendRow(Row{boost::numeric_cast<IndexType>(tLinearConstraintIndex)},
                                tConstraintGradient.stdVector());
    }
    return tSparseMatrix;
}

}  // namespace plato::third_party_integration::snopt
