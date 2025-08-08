#ifndef PLATO_GEOMETRY_EXTENSION_CUBITNGS_CUBITWRAPPER
#define PLATO_GEOMETRY_EXTENSION_CUBITNGS_CUBITWRAPPER

#include <filesystem>
#include <unordered_map>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/cubit/PlatoCubitInterface.hpp"

namespace plato::third_party_integration::cubit
{
struct ApreproVariable;
}  // namespace plato::third_party_integration::cubit

namespace plato::geometry::extension::cubit
{

using CubitSensitivityMap =
    std::unordered_map<third_party_integration::cubit::CubitGlobalId, third_party_integration::common::Vector3>;

///@brief A class that wraps the functionality of the cubit library interface and ensures the instance of cubit is
/// controlled
class CubitWrapper
{
   public:
    ///@brief Construct a CubitWrapper
    CubitWrapper();

    ///@brief Define an aprepro variable @a aApreproVariable in cubit's environment
    void addApreproVariable(const third_party_integration::cubit::ApreproVariable& aApreproVariable);

    ///@brief Reset cubit's environment
    void reset();

    ///@brief Play a journal file @a aFileName in cubit's current environment
    void playJournalFile(const std::filesystem::path& aFileName);

    ///@brief Export a step file @a aFileName based on the geometry in cubit's current environment
    void exportStepFile(const std::filesystem::path& aFileName);

    ///@brief Import an exodus file @a aFileName into cubit's environment
    void importExodusFile(const std::filesystem::path& aFileName);

    ///@brief Export an exodus file @a aFileName based on the geometry in cubit's current environment
    ///@pre Cubit's environment must have blocks defined or the mesh written will contain no data.
    void exportExodusFile(const std::filesystem::path& aFileName);

    ///@brief Runs the journal file @a aJournalFileName twice. First with the aprepro variable defined @a
    /// aApreproVariable, and a second time with that same variable perturbed @a aPerturbationSize.
    /// Returns a map of the surface mesh global ids to the sensitivity determined by a finite difference
    ///@pre All aprepro variables that are needed for the journal file have been defined.
    auto sensitivities(const std::filesystem::path& aJournalFileName,
                       const third_party_integration::cubit::ApreproVariable& aApreproVariable,
                       const double aPerturbationSize) -> CubitSensitivityMap;

   private:
    std::unique_ptr<plato::third_party_integration::cubit::PlatoCubitInterface> mCubit;
};

}  // namespace plato::geometry::extension::cubit

#endif
