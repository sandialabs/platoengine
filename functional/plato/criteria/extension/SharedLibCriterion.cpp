#include "plato/criteria/extension/SharedLibCriterion.hpp"

#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/SharedLibraryUtilities.hpp"

namespace plato::criteria::extension
{
namespace
{
SharedLibCriterion make_shared_lib_criterion(const plato::criteria::library::CriterionInput& aInput)
{
    return SharedLibCriterion{aInput.mSharedLibraryPath.mName, aInput.mInputFiles.mList};
}

[[maybe_unused]] static auto kCustomAppRegistration = library::CriterionRegistration{
    input_parser::kCodeOptionsTable.toString(input_parser::CodeOptions::kCustomApp).value(),
    [](const plato::criteria::library::CriterionInput& aInput)
    { return make_shared_lib_function(make_shared_lib_criterion(aInput)); }};
}  // namespace

SharedLibCriterion::SharedLibCriterion(const std::filesystem::path& aSharedLibPath,
                                       const std::vector<std::string>& aFileNames)
    : mSharedLibPath(aSharedLibPath)
{
    using CreateCriterionFunction =
        std::add_pointer_t<std::unique_ptr<library::CriterionInterface>(const std::vector<std::string>&)>;

    void* const tSharedLibInterface = plato::utilities::load_shared_library(aSharedLibPath);
    const auto tCreateCriterionFunction = plato::utilities::load_function<CreateCriterionFunction>(
        tSharedLibInterface, library::kCreateCriterionFunctionName, aSharedLibPath);
    mCriterionFunction = tCreateCriterionFunction(aFileNames);
}

double SharedLibCriterion::f(const core::MeshProxy& aMesh) const { return mCriterionFunction->value(aMesh); }

linear_algebra::DynamicVector<double> SharedLibCriterion::df(const core::MeshProxy& aMesh) const
{
    std::vector<double> tGradient = mCriterionFunction->gradient(aMesh);
    return linear_algebra::DynamicVector<double>(std::move(tGradient));
}

auto make_shared_lib_function(const SharedLibCriterion& aSharedLibCriterion)
    -> core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>
{
    return core::make_function(
        [aSharedLibCriterion](const core::MeshProxy& mesh) { return aSharedLibCriterion.f(mesh); },
        [aSharedLibCriterion](const core::MeshProxy& mesh) { return aSharedLibCriterion.df(mesh); });
}

}  // namespace plato::criteria::extension
