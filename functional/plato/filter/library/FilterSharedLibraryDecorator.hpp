#ifndef PLATO_FILTER_LIBRARY_FILTERSHAREDLIBRARYDECORATOR
#define PLATO_FILTER_LIBRARY_FILTERSHAREDLIBRARYDECORATOR

#include <memory>

#include "plato/filter/library/FilterInterface.hpp"
#include "plato/services/SharedLibraryObject.hpp"

namespace plato::filter::library
{
class FilterSharedLibraryDecorator : public FilterInterface
{
   public:
    FilterSharedLibraryDecorator(
        services::SharedLibraryObject<std::unique_ptr<FilterInterface>>&& aSharedLibraryObject);

    [[nodiscard]] auto filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
        -> analysis::AnalysisDomainMesh override;

    [[nodiscard]] auto rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                              const linear_algebra::DynamicVector<double>& aRowVector) const
        -> linear_algebra::DynamicVector<double> override;

    [[nodiscard]] auto rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const plato::linear_algebra::DynamicVector<double>& aRowVector)
        const -> plato::linear_algebra::DynamicVector<double> override;

   private:
    services::SharedLibraryObject<std::unique_ptr<FilterInterface>> mSharedLibraryObject;
};
}  // namespace plato::filter::library

#endif
