#ifndef PLATO_THIRDPARTYINTEGRATION_CUBIT_CUBITINTERFACE
#define PLATO_THIRDPARTYINTEGRATION_CUBIT_CUBITINTERFACE

#include <filesystem>
#include <unordered_map>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::cubit
{

///@brief Struct defining connecting a name with a value in a simple struct so that it can be passed between PE and
/// cubit interface.
struct ApreproVariable
{
    std::string mName;
    double mValue;
};

using CubitGlobalId = int;
using NodalSensitivity = std::unordered_map<CubitGlobalId, ::plato::third_party_integration::common::Vector3>;

///@brief A virtual class for the cubit interface that could be included as a header only when interfacing PE with this
/// interface.
struct PlatoCubitInterface
{
    virtual void defineApreproVariable(const ApreproVariable& aApreproVariable) = 0;
    virtual void reset() = 0;
    virtual void playJournalFile(const std::filesystem::path& aFileName) = 0;
    virtual void exportStepFile(const std::filesystem::path& aFileName) = 0;
    virtual void importExodusFile(const std::filesystem::path& aFileName) = 0;
    virtual void exportExodusFile(const std::filesystem::path& aFileName) = 0;

    [[nodiscard]] virtual NodalSensitivity determineSensitivity(const std::filesystem::path& aJournalFileName,
                                                                const ApreproVariable& aApreproVariable,
                                                                const double aPerturbationSize) = 0;

    PlatoCubitInterface() = default;
    virtual ~PlatoCubitInterface() = default;
    PlatoCubitInterface(const PlatoCubitInterface&) = delete;
    PlatoCubitInterface& operator=(const PlatoCubitInterface&) = delete;
    PlatoCubitInterface(PlatoCubitInterface&&) = delete;
    PlatoCubitInterface& operator=(PlatoCubitInterface&&) = delete;
};

}  // namespace plato::third_party_integration::cubit

#endif
