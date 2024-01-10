#pragma once

#include "Plato_Interface.hpp"
#include "Plato_SampleIndexing.hpp"
#include "Plato_SampleParameterMap.hpp"
#include "Plato_StochasticSample.hpp"

#include <functional>
#include <string>
#include <vector>

namespace Plato
{

/// @brief Class template for evaluating a batch of stochastic samples in parallel, using a stage set on construction.
///
/// The template is likely either `double` or `std::vector<double>`, with `double` resulting from an objective
/// function evaluation and `std::vector<double>` resulting from a gradient computation. Templating on 
/// `vector` will assume the sizes of the resulting vectors to be equal to the size of the control variable vector.
///
/// @tparam ComputedType The type computed by the stage evaluated by the stage set on construction.
template<typename ComputedType>
class BatchSampleEvaluator
{
public:
    /// @param aInterface Plato interface
    /// @param aStageName The name of the stage to evaluate.
    /// @param aSampleParameterMap Data structure holding the mapping from sample indices to parameter names.
    /// @param aControlName The name of the control vector SharedData object. If this class is templated with `std::vector`,
    ///  the result vectors will have the same size as the control vector.
    BatchSampleEvaluator(
        Interface& aInterface, 
        std::string aStageName, 
        std::vector<Plato::SampleParameterMap> aSampleParameterMap, 
        std::string aControlName);

    /// Evaluate a batch of stochastic samples held in @a aSamples with control vector @a aControl
    /// @return Results from evaluating the stage given on construction. If templated on `std::vector`,
    ///  the sizes of the resulting vectors will be equal to the size of @a aControl.
    [[nodiscard]] 
    std::vector<ComputedType> operator()(
        const Plato::DistributedVectorROL<double>& aControl, const std::vector<Plato::Sample>& aSamples);

private:
    void setupSampleDataForBatch(
        Plato::SampleBatch aSampleIndexing,
        const std::vector<Plato::Sample>& aSamples,
        Teuchos::ParameterList& aParameterList,
        std::vector<ComputedType>& aResults,
        std::size_t aAllocationSize);

private:
    std::reference_wrapper<Plato::Interface> mInterface;
    std::string mStageName;
    std::vector<Plato::SampleParameterMap> mSampleParameterMap;
    std::string mControlName;
};

namespace detail
{
void assign_parameters(
    Plato::Interface& aInterface, const Plato::Sample& aSample, const SampleParameterMap& aSampleParameterMap)
{
    for(std::size_t i = 0; i < aSample.mValue.size(); ++i)
    {
        const double tStochasticParameterValue = aSample.mValue[i];
        const std::string& tStochasticParameterName = aSampleParameterMap.mParameterIndexToParameterName.at(i);
        aInterface.setParameterInDataLayer(tStochasticParameterName, tStochasticParameterValue);
    }
}

[[nodiscard]]
Teuchos::ParameterList parameter_list_with_control_vector(
    const Plato::DistributedVectorROL<double>& aControl, const std::string& aControlName)
{
    Teuchos::ParameterList tParameterList;
    tParameterList.set(aControlName, aControl.vector().data());
    return tParameterList;
}

/// @brief Compile-time constant for checking if a type `T` is a specialization of `std::vector`
template <typename T>
constexpr bool kIsVector = false;

template <typename T, typename A>
constexpr bool kIsVector<std::vector<T, A>> = true;

}

template<typename ComputedType>
BatchSampleEvaluator<ComputedType>::BatchSampleEvaluator(
    Interface& aInterface,
    std::string aStageName,
    std::vector<Plato::SampleParameterMap> aSampleParameterMap,
    std::string aControlName) :
    mInterface(aInterface),
    mStageName(std::move(aStageName)),
    mSampleParameterMap(std::move(aSampleParameterMap)),
    mControlName(std::move(aControlName))
{}

template<typename ComputedType>
std::vector<ComputedType> BatchSampleEvaluator<ComputedType>::operator()(
    const Plato::DistributedVectorROL<double>& aControl, 
    const std::vector<Plato::Sample>& aSamples)
{
    Teuchos::ParameterList tParameterList = detail::parameter_list_with_control_vector(aControl, mControlName);
    const Plato::SampleIndexing tSampleIndexing{
        Plato::NumPerformers{mSampleParameterMap.size()}, 
        Plato::NumSamples{aSamples.size()}};
    std::vector<ComputedType> tResults(aSamples.size());
    for(const Plato::SampleBatch tBatch : tSampleIndexing)
    {
        setupSampleDataForBatch(tBatch, aSamples, tParameterList, tResults, aControl.vector().size());
        mInterface.get().compute(mStageName, tParameterList);
    }
    return tResults;
}

template<typename ComputedType>
void BatchSampleEvaluator<ComputedType>::setupSampleDataForBatch(
    const Plato::SampleBatch aSampleBatch,
    const std::vector<Plato::Sample>& aSamples,
    Teuchos::ParameterList& aParameterList,
    std::vector<ComputedType>& aResults,
    const std::size_t aAllocationSize)
{
    for(const auto [tPerformerIndex, tSampleIndex] : aSampleBatch)
    {
        detail::assign_parameters(mInterface, aSamples.at(tSampleIndex), mSampleParameterMap[tPerformerIndex]);
        if constexpr(detail::kIsVector<ComputedType>)
        {
            // For std::vector, allocate storage for the vector and give member `data` as the address
            aResults[tSampleIndex].resize(aAllocationSize);
            aParameterList.set(mSampleParameterMap[tPerformerIndex].mOutputSharedDataName, aResults[tSampleIndex].data());
        }
        else
        {
            aParameterList.set(mSampleParameterMap[tPerformerIndex].mOutputSharedDataName, &aResults[tSampleIndex]);
        }
    }
}

}
