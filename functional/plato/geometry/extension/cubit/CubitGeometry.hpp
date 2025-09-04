#ifndef PLATO_GEOMETRY_EXTENSION_CUBITNGS_CUBITGEOMETRY
#define PLATO_GEOMETRY_EXTENSION_CUBITNGS_CUBITGEOMETRY

#include <boost/mpi.hpp>
#include <optional>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/geometry/extension/cubit/BoundedApreproVariableParser.hpp"
#include "plato/geometry/extension/cubit/CubitWrapper.hpp"
#include "plato/geometry/library/OutputInfo.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/utilities/NamedType.hpp"
#include "plato/utilities/StateCache.hpp"

// clang-format off
PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), cubit_parameterized_shape,
    (plato::input_parser::FileName, mesh_journal_file, "Required field specifying the cubit journal file to run. "
                                                                 "Do not define aprepro variables in the journal file that are listed in the Plato input as aprepro_variables. "
                                                                 "For fastest finite differencing, distinguish geometry operations from meshing. Use {IfDef(compute_mesh)} ... {EndIf} to enclose all meshing steps.")
    (plato::input_parser::FileName, output_file_name, "Required field specifying the file to write the final values of the aprepro variables.")
    (plato::input_parser::FileName, mesh_file_name, "Required field specifying the mesh file name.")
    (plato::input_parser::FileName, output_mesh_sensitivities_name, "Optional field specifying the mesh file name to output the sensitivities to.")
    (plato::input_parser::BoundedApreproVariableList,aprepro_variables,"Required fields specifying the aprepro variables in the mesh journal file. "
                                                                                 "Define as many as you like, comma separated, in this manner: aprepro_variables name_in_journal_file initial_value [lower_bound, upper_bound], ... . "
                                                                                 "This is an example list: aprepro_variables width 1 [0.1, 4], height 2 [0.1, 4]")
)
// clang-format on

namespace plato::input_parser
{
struct cubit_parameterized_shape;
}

namespace plato::mesh
{
class MeshOutput;
}

namespace plato::geometry::extension::cubit
{
using MeshCache = utilities::StateCache<analysis::AnalysisDomainMesh, const linear_algebra::DynamicVector<double>&>;
using ApreproVariableSensitivities = std::vector<CubitSensitivityMap>;
using PerturbationScale = utilities::NamedType<double, struct PerturbationScaleTag>;
using Value = utilities::NamedType<double, struct ValueTag>;

///@brief An extended version of what an third party integration aprepro variable is that can have a value and return a
/// perturbed value.
struct PerturbableApreproVariable
{
    std::string mName;
    Value mValue = Value{0.5};
    PerturbationScale mPerturbationScale = PerturbationScale{1e-5};
};

///@brief a Geometry class that takes design parameters that relate to aprepro variables in a cubit journal file and
/// can perturb them to numerically determine their sensitivities.
class CubitGeometry
{
   public:
    explicit CubitGeometry(const plato::input_parser::cubit_parameterized_shape& aInput);

    ///@brief Given design parameters @a aDesignParameters, generate an analysis domain mesh
    [[nodiscard]] auto generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameter)
        -> analysis::AnalysisDomainMesh;

    void outputMeshSensitivities(mesh::MeshOutput& aMeshOutput);

    ///@brief Compute the jacobian multiplier for a given design parameters @a aDesignParameters
    [[nodiscard]] auto jacobianMultiplier(const linear_algebra::DynamicVector<double>& aDesignParameter)
        -> linear_algebra::JacobianMultiplier;

    ///@brief Compute the adjoint jacobian multiplier for a given design parameters @a aDesignParameters
    [[nodiscard]] auto adjointJacobianMultiplier(const linear_algebra::DynamicVector<double>& aDesignParameter)
        -> linear_algebra::AdjointJacobianMultiplier;

   private:
    std::filesystem::path mBaseJournalFile;
    std::filesystem::path mMeshFile;

    std::vector<PerturbableApreproVariable> mVariables;
    CubitWrapper mCubit;

    MeshCache mMeshCache;
    boost::mpi::communicator mCommunicator;
};

namespace detail
{
using JournalFile = utilities::NamedType<std::filesystem::path, struct JournalFileTag>;
using ExodusFile = utilities::NamedType<std::filesystem::path, struct ExodusFileTag>;

///@brief Uses the Cubit library via @a aCubit to define a set of aprepro variables @a aVariables, run a journal file @a
/// aJournalFile and output an exodus file @a aExodusFile.
///@note Since aprepro variables are changed and meshing is done, the state of Cubit is not const and so must be passed
/// by reference.
void write_exodus_mesh(CubitWrapper& aCubit,
                       const std::vector<PerturbableApreproVariable>& aVariables,
                       const JournalFile& aJournalFile,
                       const ExodusFile& aExodusFile);

///@brief Uses Cubit @a aCubit and perturbs the set of aprepro variables @a aVariables before running the journal file
///@a JournalFile several times to compute the sensitivities with respect to each variable.
///@note Since aprepro variables are changed and meshing is done, the state of Cubit is not const and so must be passed
/// by reference.
[[nodiscard]] auto sensitivities(CubitWrapper& aCubit,
                                 const std::vector<PerturbableApreproVariable>& aVariables,
                                 const std::filesystem::path& aJournalFile) -> ApreproVariableSensitivities;

///@brief Given the input @a aInput, checks to make sure a journal file is specified and exists on disk.
[[nodiscard]] std::optional<std::string> validate_mesh_journal_file(
    const input_parser::cubit_parameterized_shape& aInput);

///@brief Given the input @a aInput, create an output function that generates the requested output when called.
[[nodiscard]] auto make_cubit_output(const input_parser::cubit_parameterized_shape& aInput)
    -> std::function<void(const linear_algebra::DynamicVector<double>&, const library::OutputInfo&)>;

}  // namespace detail
}  // namespace plato::geometry::extension::cubit

#endif
