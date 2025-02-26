#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTUTILITIES
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTUTILITIES

#include <cassert>

#include "plato/geometry/library/OutputManager.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/third_party_integration/snopt/DataSingleton.hpp"
#include "plato/third_party_integration/snopt/ObjectiveConstraintArrayView.hpp"
#include "plato/third_party_integration/snopt/ObjectiveConstraintGradientArrayView.hpp"
#include "plato/third_party_integration/snopt/SNOPTConstraints.hpp"
#include "plato/third_party_integration/snopt/SNOPTTypes.hpp"
#include "plato/third_party_integration/snopt/SparseMatrixBuilder.hpp"
#include "plato/utilities/Enumerate.hpp"

namespace plato::third_party_integration::snopt
{
/// @brief Creates a sparse matrix representing the linear constraint Jacobians.
auto linear_constraint_jacobian_sparse_matrix(const SNOPTConstraints& aConstraintFunctions,
                                              std::size_t aNumberOfDesignVariables)
    -> SparseMatrixBuilder<IndexType, double>;

/// @brief Evaluates the objective into @a aObjectiveConstraintView.
///
/// The function used for evaluation is assumed to have been assigned to DataSingleton<FunctionType, FunctionTag>.
/// @note @a aObjectiveConstraintView is a view for the `F` array in the SNOPT interface.
template <typename FunctionTag>
void evaluateObjective(const linear_algebra::DynamicVector<double>& aDesignVariables,
                       ObjectiveConstraintArrayView<double> aObjectiveConstraintView);

/// @brief Evaluates the objective gradient into @a aObjectiveConstraintGradientView.
///
/// The function used for evaluation is assumed to have been assigned to DataSingleton<FunctionType, FunctionTag>.
/// @note @a aObjectiveConstraintGradientView is a view for the `G` array in the SNOPT interface.
template <typename FunctionTag>
void evaluateObjectiveGradient(const linear_algebra::DynamicVector<double>& aDesignVariables,
                               ObjectiveConstraintGradientArrayView<double> aObjectiveConstraintGradientView);

/// @brief Evaluates the constraints into @a aObjectiveConstraintView.
///
/// The functions used for evaluation are assumed to have been assigned to DataSingleton<FunctionType, FunctionTag>.
/// @note @a aObjectiveConstraintView is a view for the `F` array in the SNOPT interface.
template <typename FunctionTag>
void evaluateConstraints(const linear_algebra::DynamicVector<double>& aDesignVariables,
                         ObjectiveConstraintArrayView<double> aObjectiveConstraintView);

/// @brief Evaluates the constraint gradients into @a aObjectiveConstraintGradientView.
///
/// The functions used for evaluation are assumed to have been assigned to DataSingleton<FunctionType, FunctionTag>.
/// @note @a aObjectiveConstraintGradientView is a view for the `G` array in the SNOPT interface.
template <typename FunctionTag>
void evaluateConstraintGradients(const linear_algebra::DynamicVector<double>& aDesignVariables,
                                 ObjectiveConstraintGradientArrayView<double> aObjectiveConstraintGradientView);

template <typename FunctionTag>
void evaluateObjective(const linear_algebra::DynamicVector<double>& aDesignVariables,
                       ObjectiveConstraintArrayView<double> aObjectiveConstraintView)
{
    const auto& tSingleton = DataSingleton<ObjectiveType, FunctionTag>::constInstance();
    assert(tSingleton.hasData());
    const auto& tObjective = tSingleton.data();
    aObjectiveConstraintView.objective() = tObjective.template evaluate<core::evaluation::kFunction>(aDesignVariables);
}

template <typename FunctionTag>
void evaluateConstraints(const linear_algebra::DynamicVector<double>& aDesignVariables,
                         ObjectiveConstraintArrayView<double> aObjectiveConstraintView)
{
    const auto& tSingleton = DataSingleton<ConstraintVectorType, FunctionTag>::constInstance();
    assert(tSingleton.hasData());
    const auto& tNonlinearConstraints = tSingleton.data();
    std::transform(tNonlinearConstraints.begin(), tNonlinearConstraints.end(),
                   aObjectiveConstraintView.constraints().begin(),
                   [&aDesignVariables](const auto& tContraint)
                   { return tContraint.mFunction.template evaluate<core::evaluation::kFunction>(aDesignVariables); });
}

template <typename FunctionTag>
void evaluateObjectiveGradient(const linear_algebra::DynamicVector<double>& aDesignVariables,
                               ObjectiveConstraintGradientArrayView<double> aObjectiveConstraintGradientView)
{
    const auto& tObjectiveSingleton = DataSingleton<ObjectiveType, FunctionTag>::constInstance();
    assert(tObjectiveSingleton.hasData());
    const auto& tObjectiveGradient = tObjectiveSingleton.data();
    const auto tGradient = tObjectiveGradient.template evaluate<core::evaluation::kFirstDerivative>(aDesignVariables);
    std::copy(tGradient.stdVector().begin(), tGradient.stdVector().end(),
              aObjectiveConstraintGradientView.objectiveGradient().begin());

    auto& tOutputSingleton = DataSingleton<geometry::library::OutputManager, FunctionTag>::instance();
    assert(tOutputSingleton.hasData());
    auto& tOutputFunction = tOutputSingleton.data();
    tOutputFunction->output(aDesignVariables);
}

template <typename FunctionTag>
void evaluateConstraintGradient(const linear_algebra::DynamicVector<double>& aDesignVariables,
                                ObjectiveConstraintGradientArrayView<double> aObjectiveConstraintGradientView)
{
    const auto& tSingleton = DataSingleton<ConstraintVectorType, FunctionTag>::constInstance();
    assert(tSingleton.hasData());
    const auto& tNonlinearConstraints = tSingleton.data();
    for (const auto [tConstraintIndex, tConstraint] : utilities::enumerate(tNonlinearConstraints))
    {
        const auto tGradient =
            tConstraint.mFunction.template evaluate<core::evaluation::kFirstDerivative>(aDesignVariables);
        std::copy(tGradient.stdVector().begin(), tGradient.stdVector().end(),
                  aObjectiveConstraintGradientView.constraintGradient(ConstraintSizeType{tConstraintIndex}).begin());
    }
}

}  // namespace plato::third_party_integration::snopt

#endif
