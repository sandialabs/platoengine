#ifndef PLATO_THIRDPARTYINTEGRATION_ROL_OPTIMIZATIONPARAMETERS
#define PLATO_THIRDPARTYINTEGRATION_ROL_OPTIMIZATIONPARAMETERS

#include <ROL_ParameterList.hpp>
#include <boost/optional.hpp>
#include <filesystem>
#include <optional>

#include "plato/third_party_integration/rol/ParameterBuilder.hpp"

namespace plato::third_party_integration::rol
{

///@brief A class to handle the state of the ROL::Parameters list
class OptimizationParameters
{
   public:
    ///@brief Constructor for OptimizationParameters class from loading optional @a aInputFile, if no input file is
    /// provided this function will load the input file from the plato data folder
    OptimizationParameters(const std::optional<std::filesystem::path>& aInputFile = std::nullopt);

    ///@brief Makes the parameter list report the highest level of output
    void verbose();

    ///@brief Changes the parameter list to override the Hessian calculation and instead approximate it
    void approximateHessian();

    ///@brief Changes the parameter for that specifies the maximum iteration in an optimization to @a aMaximumIterations
    void maximumIterations(const std::optional<unsigned int> aMaximumIterations);

    ///@brief Changes the parameter for that specifies the gradient tolerance in an optimization to @a
    /// aGradientTolerance
    ///@note Convergence in ROL is dictated by the norm of the gradient which grows with controls like sqrt(N)
    void gradientTolerance(const std::optional<double> aGradientTolerance);

    ///@brief Changes the parameter for that specifies the step tolerance in an optimization to @a aStepTolerance
    ///@note Convergence in ROL is dictated by the norm of the step tolerance which grows with controls like sqrt(N)
    void stepTolerance(const std::optional<double> aStepTolerance);

    ///@brief Changes the parameter for that specifies the initial search radius in an optimization to @a
    /// aInitialSearchRadius
    ///@note Search radius in ROL is dictated by the norm of the gradient which grows with controls like sqrt(N)
    void initialSearchRadius(const std::optional<double> aInitialSearchRadius);

    ///@brief Return the ROL::ParameterList stored in the class member mParameterList
    [[nodiscard]] const ROL::ParameterList& parameters() const;

    ///@brief Write the ROL::ParameterList stored in the class member mParameterList to file @a aPath
    void writeParameters(const std::filesystem::path& aPath) const;

   private:
    ROL::ParameterList mParameterList;
};

namespace detail
{

///@brief Makes the parameter list @a aParameterList report the highest level of output
void make_verbose(ROL::ParameterList& aParameterList);

///@brief Changes the parameter list @a aParameterList to override the Hessian calculation and instead approximate it
void approximate_hessian(ROL::ParameterList& aParameterList);

///@brief Load a ParameterList from file @a aInputFile
[[nodiscard]] ROL::ParameterList load_from_file(const std::filesystem::path& aInputFile);

///@brief Traverse the parameter list @a aParlist and set the value specified by ParameterAndValue @a aParameterAndValue
template <typename Type>
void set_parameter(ROL::ParameterList& aParlist, const ParameterAndValue<Type>& aParameterAndValue);

///@brief Traverse the parameter list @a aParameterList according to the sequence of strings @a aSequence and return
/// the lowest sublist
auto retrieve_sublist_from_sequence(ROL::ParameterList& aParameterList, const std::vector<std::string>& aSequence)
    -> std::reference_wrapper<ROL::ParameterList>;

template <typename Type>
void set_parameter(ROL::ParameterList& aParlist, const ParameterAndValue<Type>& aParameterAndValue)
{
    auto tCurrentSublist = detail::retrieve_sublist_from_sequence(aParlist, aParameterAndValue.mSublistNames);
    assert(tCurrentSublist.get().isParameter(aParameterAndValue.mParameterName));
    tCurrentSublist.get().template set<Type>(aParameterAndValue.mParameterName, aParameterAndValue.mValue);
}

}  // namespace detail
}  // namespace plato::third_party_integration::rol

#endif
