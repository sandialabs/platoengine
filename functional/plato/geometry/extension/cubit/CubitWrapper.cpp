#include "plato/geometry/extension/cubit/CubitWrapper.hpp"

#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/third_party_integration/cubit/PlatoCubitInterface.hpp"

namespace plato::geometry::extension::cubit
{
namespace
{
constexpr std::string_view kCubitLibrary = "libPlatoFunctionalCubit.so";
constexpr std::string_view kCreateCubitFunctionName = "create_cubit";

[[nodiscard]] auto cubit()
{
    const auto tSharedLibPath = services::plugin_directory_path().value() / std::filesystem::path{kCubitLibrary};
    return services::make_shared_library_object<CubitInterfaceFunction()>(tSharedLibPath, kCreateCubitFunctionName);
}

}  // namespace

CubitWrapper::CubitWrapper() : mSharedLibraryObject(cubit()) {}
void CubitWrapper::addApreproVariable(const third_party_integration::cubit::ApreproVariable& aApreproVariable)
{
    mSharedLibraryObject.object()->defineApreproVariable(aApreproVariable);
}

void CubitWrapper::reset() { mSharedLibraryObject.object()->reset(); }

void CubitWrapper::playJournalFile(const std::filesystem::path& aFileName)
{
    mSharedLibraryObject.object()->playJournalFile(aFileName);
}

void CubitWrapper::exportStepFile(const std::filesystem::path& aFileName)
{
    mSharedLibraryObject.object()->exportStepFile(aFileName);
}

void CubitWrapper::importExodusFile(const std::filesystem::path& aFileName)
{
    mSharedLibraryObject.object()->importExodusFile(aFileName);
};

void CubitWrapper::exportExodusFile(const std::filesystem::path& aFileName)
{
    mSharedLibraryObject.object()->exportExodusFile(aFileName);
};

auto CubitWrapper::sensitivities(const std::filesystem::path& aJournalFileName,
                                 const third_party_integration::cubit::ApreproVariable& aApreproVariable,
                                 const double aPerturbationSize) -> CubitSensitivityMap
{
    return mSharedLibraryObject.object()->determineSensitivity(aJournalFileName, aApreproVariable, aPerturbationSize);
}

}  // namespace plato::geometry::extension::cubit
