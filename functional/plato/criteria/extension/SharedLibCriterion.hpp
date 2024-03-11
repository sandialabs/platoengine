#ifndef PLATO_CRITERIA_EXTENSION_SHAREDLIBCRITERION
#define PLATO_CRITERIA_EXTENSION_SHAREDLIBCRITERION

#include <filesystem>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "plato/core/Function.hpp"
#include "plato/core/MeshProxy.hpp"
#include "plato/criteria/library/CriterionInterface.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::criteria::extension
{
/// @brief for a criterion that is loaded from a shared library.
///
/// A shared library path is given on construction from which to load
/// a CriterionInterface object.
class SharedLibCriterion
{
   public:
    SharedLibCriterion(const std::filesystem::path& aSharedLibPath, const std::vector<std::string>& aFileNames);

    [[nodiscard]] double f(const core::MeshProxy& aMesh) const;

    [[nodiscard]] linear_algebra::DynamicVector<double> df(const core::MeshProxy& aMesh) const;

   private:
    std::filesystem::path mSharedLibPath;
    std::shared_ptr<library::CriterionInterface> mCriterionFunction;
};

[[nodiscard]] auto make_shared_lib_function(const SharedLibCriterion& aSharedLibCriterion)
    -> core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>;

}  // namespace plato::criteria::extension

#endif