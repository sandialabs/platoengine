#include "plato/geometry/extension/cubit/CubitWrapper.hpp"

#include <memory>

#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/services/SharedLibrarySetupTeardown.hpp"
#include "plato/third_party_integration/cubit/PlatoCubitInterface.hpp"

namespace plato::geometry::extension::cubit
{
namespace
{
constexpr std::string_view kCubitLibrary = "libPlatoFunctionalCubit.so";
constexpr std::string_view kCreateCubitFunctionName = "create_cubit";

auto create_library() -> services::SharedLibrarySetupTeardown
{
    const auto tSharedLibPath = services::plugin_directory_path().value() / std::filesystem::path{kCubitLibrary};
    return services::SharedLibrarySetupTeardown(tSharedLibPath);
}

auto get_cubit() -> std::unique_ptr<third_party_integration::cubit::PlatoCubitInterface>
{
    auto tLibrary = create_library();
    using FunctionSignature = std::unique_ptr<third_party_integration::cubit::PlatoCubitInterface>();
    return tLibrary.call<FunctionSignature>(kCreateCubitFunctionName);
}

}  // namespace

CubitWrapper::CubitWrapper() : mCubit(get_cubit()) {}
void CubitWrapper::addApreproVariable(const third_party_integration::cubit::ApreproVariable& aApreproVariable)
{
    mCubit->defineApreproVariable(aApreproVariable);
}

void CubitWrapper::reset() { mCubit->reset(); }

void CubitWrapper::playJournalFile(const std::filesystem::path& aFileName) { mCubit->playJournalFile(aFileName); }

void CubitWrapper::exportStepFile(const std::filesystem::path& aFileName) { mCubit->exportStepFile(aFileName); }

void CubitWrapper::importExodusFile(const std::filesystem::path& aFileName) { mCubit->importExodusFile(aFileName); };

void CubitWrapper::exportExodusFile(const std::filesystem::path& aFileName) { mCubit->exportExodusFile(aFileName); };

auto CubitWrapper::sensitivities(const std::filesystem::path& aJournalFileName,
                                 const third_party_integration::cubit::ApreproVariable& aApreproVariable,
                                 const double aPerturbationSize) -> CubitSensitivityMap
{
    return mCubit->determineSensitivity(aJournalFileName, aApreproVariable, aPerturbationSize);
}

}  // namespace plato::geometry::extension::cubit
