#pragma once

#include "Plato_BatchSampleEvaluator.hpp"
#include "Plato_Interface.hpp"
#include "Plato_DistributedVectorROL.hpp"
#include "Plato_OptimizerEngineStageData.hpp"
#include "Plato_ReducedObjectiveROL.hpp"
#include "Plato_ROLSampleGeneratorResponseCache.hpp"
#include "Plato_StochasticSample.hpp"
#include "Plato_SampleParameterMap.hpp"

#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>

namespace Plato
{
/// Implements the ROL::Objective interface, specialized for stochastic problems.
/// In particular, maps stochastic parameters from ROL::Objective::getParameter
/// to internal parameters set on Operations. These are communicated through
/// shared data.
///
/// All samples from the ROL::SampleGenerator are evaluated as specified in the value/gradient stage. 
/// In particular, MultiOperation may be used with multiple performers to define a pool
/// of performers for parallelization. This pool is used to compute all samples generated
/// by ROL::SampleGenerator all at once, and the results are stored in ROLSampleGeneratorResponseCache.
template<typename ScalarType>
class ReducedStochasticObjectiveROL : public ReducedObjectiveROL<ScalarType>
{
public:
    using GradientType = std::vector<ScalarType>;

public:
    ReducedStochasticObjectiveROL(
        const Plato::OptimizerEngineStageData & aInputData, 
        Plato::Interface* aInterface,
        const std::unordered_map<std::string, unsigned int>& aDistributionMap,
        ROL::Ptr<ROL::SampleGenerator<ScalarType>> aSampleGenerator);

    ScalarType value(const ROL::Vector<ScalarType> & aControl, ScalarType & aTolerance) override;
    void gradient(ROL::Vector<ScalarType> & aGradient, const ROL::Vector<ScalarType> & aControl, ScalarType & aTolerance) override;

private:
    template<typename ComputedType>
    BatchSampleEvaluator<ComputedType> batchSampleEvaluator(
        const std::unordered_map<std::string, unsigned int>& aDistributionMap);

private:
    Plato::ROLSampleGeneratorResponseCache<ScalarType> mValueResponseCache;
    Plato::ROLSampleGeneratorResponseCache<GradientType> mGradientResponseCache;
};

template<typename ScalarType>
ReducedStochasticObjectiveROL<ScalarType>::ReducedStochasticObjectiveROL(
    const Plato::OptimizerEngineStageData & aInputData, 
    Plato::Interface* aInterface,
    const std::unordered_map<std::string, unsigned int>& aDistributionMap,
    ROL::Ptr<ROL::SampleGenerator<ScalarType>> aSampleGenerator) :
    ReducedObjectiveROL<ScalarType>(aInputData, aInterface),
    mValueResponseCache(
        batchSampleEvaluator<ScalarType>(aDistributionMap), aSampleGenerator),
    mGradientResponseCache(
        batchSampleEvaluator<GradientType>(aDistributionMap), aSampleGenerator)
{
}

template<typename ScalarType>
ScalarType ReducedStochasticObjectiveROL<ScalarType>::value(
    const ROL::Vector<ScalarType> & aControl, ScalarType & /*aTolerance*/)
{
    const Plato::DistributedVectorROL<ScalarType> & tControl =
          dynamic_cast<const Plato::DistributedVectorROL<ScalarType>&>(aControl);
    return mValueResponseCache(tControl, Plato::Sample{this->getParameter()});
}

template<typename ScalarType>
void ReducedStochasticObjectiveROL<ScalarType>::gradient(
    ROL::Vector<ScalarType> & aGradient, const ROL::Vector<ScalarType> & aControl, ScalarType & /*aTolerance*/)
{
    const Plato::DistributedVectorROL<ScalarType> & tControl =
          dynamic_cast<const Plato::DistributedVectorROL<ScalarType>&>(aControl);
    GradientType tGradient = mGradientResponseCache(tControl, Plato::Sample{this->getParameter()});
    Plato::DistributedVectorROL<ScalarType> & tOutputGradient =
            dynamic_cast<Plato::DistributedVectorROL<ScalarType>&>(aGradient);
    std::copy(tGradient.begin(), tGradient.end(), tOutputGradient.vector().begin());
}

namespace detail
{
template<typename ScalarType, typename ComputedType>
constexpr bool is_computed_type_gradient()
{
    return !std::is_same_v<ScalarType, ComputedType>;
}
}

template<typename ScalarType>
template<typename ComputedType>
BatchSampleEvaluator<ComputedType> ReducedStochasticObjectiveROL<ScalarType>::batchSampleEvaluator(
    const std::unordered_map<std::string, unsigned int>& aDistributionMap)
{
    constexpr size_t tCONTROL_VECTOR_INDEX = 0;
    std::string tControlName = this->engineInputData().getControlName(tCONTROL_VECTOR_INDEX);
    const std::vector<Plato::StochasticSampleSharedDataNames>& tSampleSharedData =
        this->engineInputData().getStochasticSampleSharedDataNames();
    std::vector<Plato::SampleParameterMap> tParameterMaps = 
        detail::is_computed_type_gradient<ScalarType, ComputedType>() ? 
        shared_data_parameter_maps_for_gradient(tSampleSharedData, aDistributionMap) :
        shared_data_parameter_maps_for_value(tSampleSharedData, aDistributionMap);
    std::string tStageName = 
        detail::is_computed_type_gradient<ScalarType, ComputedType>() ? 
        this->engineInputData().getObjectiveGradientStageName() :
        this->engineInputData().getObjectiveValueStageName();
    return Plato::BatchSampleEvaluator<ComputedType>(
        *this->interface(), 
        std::move(tStageName), 
        std::move(tParameterMaps), 
        std::move(tControlName));
}

}
// namespace Plato
