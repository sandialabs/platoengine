#include "plato/criteria/extension/VolumeCriterion.hpp"

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/mesh/CoordinateUtilities.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/third_party_integration/krino/KrinoLevelSetPolicy.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/PairWiseAccumulate.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::input_parser
{
template <>
struct InputTypeName<volume_criterion>
{
    static constexpr inline const char* name = "volume";
};
}  // namespace plato::input_parser

namespace plato::criteria::extension
{
namespace
{
[[nodiscard]] auto parse_ignore_void_blocks(const library::CriterionInput& aCriterionInput) -> bool
{
    bool tIgnoreVoid = true;
    if (aCriterionInput.mInputFiles.size() > 0)
    {
        tIgnoreVoid = detail::parse_input_block(aCriterionInput.mInputFiles.list().mList[0]).ignore_void_blocks;
    }

    if (tIgnoreVoid == true)
    {
        auto tLogger = services::component_logger(aCriterionInput.mComponentType, aCriterionInput.mName);
        tLogger.logWarning(std::format(
            "\nWarning: Unless using density topology, when computing volume criterion with name {}, blocks with "
            "'void' "
            "in their name are being ignored. This can be changed by adding an input file with the form:\nbegin "
            "volume \n  ignore_void_blocks false\nend",
            aCriterionInput.mName));
    }

    return tIgnoreVoid;
}

using Registration =
    library::CriterionRegistration<library::Parallelization::kSerial, library::FunctionDimension::kScalar>;

const auto kVolumeConfiguration = services::CriterionConfiguration{
    .mName = std::string{VolumeCriterion::kVolumeCriterionName}, .mIsParallelized = false, .mIsScalar = true};

[[maybe_unused]] static auto kVolumeConstraintRegistration =
    Registration{library::builtin_criterion_registration_name(VolumeCriterion::kVolumeCriterionName),
                 [](const library::CriterionInput& aCriterionInput)
                 {
                     return library::FunctionWithConfiguration{
                         .mFunction = make_volume_constraint_function(parse_ignore_void_blocks(aCriterionInput)),
                         .mConfiguration = kVolumeConfiguration};
                 }};

const auto kVolumeFractionConfiguration = services::CriterionConfiguration{
    .mName = std::string{VolumeCriterion::kVolumeCriterionName}, .mIsParallelized = false, .mIsScalar = true};

[[maybe_unused]] static auto kVolumeFractionConstraintRegistration = Registration{
    library::builtin_criterion_registration_name(VolumeCriterion::kVolumeFractionCriterionName),
    [](const library::CriterionInput& aCriterionInput)
    {
        return library::FunctionWithConfiguration{
            .mFunction = make_volume_fraction_constraint_function(parse_ignore_void_blocks(aCriterionInput)),
            .mConfiguration = kVolumeFractionConfiguration};
    }};

[[nodiscard]] auto all_block_names(const mesh::Mesh& aMesh) -> std::set<std::string>
{
    const auto tBlockNamesVector = mesh::MeshBlocks{aMesh}.blockNames();
    return std::set<std::string>{tBlockNamesVector.begin(), tBlockNamesVector.end()};
}

[[nodiscard]] auto evaluation_block_names(const mesh::Mesh& aMesh, bool aIgnoreVoidBlocks) -> std::set<std::string>
{
    auto tAllBlocks = all_block_names(aMesh);
    if (!aIgnoreVoidBlocks)
    {
        return tAllBlocks;
    }

    auto tEvaluationBlocks = utilities::reserved_container<std::vector<std::string>>(tAllBlocks.size());
    utilities::transform_if(
        tAllBlocks, std::back_inserter(tEvaluationBlocks), [](const auto& aBlockName) { return aBlockName; },
        [](const auto& tBlockName)
        { return tBlockName.find(third_party_integration::krino::void_phase_name()) == std::string::npos; });
    return std::set<std::string>{tEvaluationBlocks.begin(), tEvaluationBlocks.end()};
}

double fixed_domain_volume(const mesh::MeshQuantities& aMesh)
{
    const auto tElementVolumes = aMesh.fixedDomainElementVolumes();
    return utilities::pair_wise_accumulate(tElementVolumes);
}

bool is_element_based_density_topology(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
{
    if (aAnalysisDomainMesh.mBlockScalarField.empty())
    {
        return false;
    }
    else if (const bool tMeshFieldIsNotElementSized =
                 (analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh}.size() !=
                  mesh::EntityCounts{mesh::Mesh{aAnalysisDomainMesh}}.numberOfDesignDomainElements());
             tMeshFieldIsNotElementSized)
    {
        throw utilities::Exception{
            "Error: 'volume' and 'volume_fraction' criteria for density topology are currently only implemented for "
            "element-wise control fields."};
    }
    return true;
}
}  // namespace

double VolumeCriterion::f(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const auto tMesh = mesh::MeshQuantities{mesh::Mesh{aAnalysisDomainMesh}};
    if (is_element_based_density_topology(aAnalysisDomainMesh))
    {
        auto tScaledVolumes = tMesh.designDomainElementVolumes();
        const auto tMeshView = analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh};
        std::transform(tScaledVolumes.cbegin(), tScaledVolumes.cend(), tMeshView.begin(), tScaledVolumes.begin(),
                       [](const double aVolume, const analysis::ScalarFieldValue aDensity)
                       { return aDensity.mValue * aVolume; });

        return mScaleFactor * (utilities::pair_wise_accumulate(tScaledVolumes) + fixed_domain_volume(tMesh));
    }
    else
    {
        return mScaleFactor * utilities::pair_wise_accumulate(tMesh.specifiedDomainElementVolumes(
                                  evaluation_block_names(tMesh, mIgnoreVoidBlocks)));
    }
}

linear_algebra::DynamicVector<double> VolumeCriterion::df(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const auto tMesh = mesh::MeshQuantities{mesh::Mesh{aAnalysisDomainMesh}};
    if (is_element_based_density_topology(aAnalysisDomainMesh))
    {
        auto tJacobian = linear_algebra::DynamicVector<double>{tMesh.designDomainElementVolumes()};
        return mScaleFactor * std::move(tJacobian);
    }
    else
    {
        return mScaleFactor * mesh::nodal_vector_field_to_dynamic_vector(
                                  tMesh.volumeNodalSensitivities(evaluation_block_names(tMesh, mIgnoreVoidBlocks)),
                                  mesh::EntityCounts{tMesh}.spatialDimensions());
    }
}

auto make_volume_constraint_function(const bool aIgnoreVoidBlocks) -> library::CriterionFunction
{
    return core::make_function_with_first_derivative(
        [aIgnoreVoidBlocks](const analysis::AnalysisDomainMesh& mesh)
        { return VolumeCriterion{.mScaleFactor = 1.0, .mIgnoreVoidBlocks = aIgnoreVoidBlocks}.f(mesh); },
        [aIgnoreVoidBlocks](const analysis::AnalysisDomainMesh& mesh)
        { return VolumeCriterion{.mScaleFactor = 1.0, .mIgnoreVoidBlocks = aIgnoreVoidBlocks}.df(mesh); });
}

auto make_volume_fraction_constraint_function(const bool aIgnoreVoidBlocks) -> library::CriterionFunction
{
    return core::make_function_with_first_derivative(
        [aIgnoreVoidBlocks](const analysis::AnalysisDomainMesh& mesh)
        {
            const double tVolumeTotal = third_party_integration::stk_io::mesh_volume(
                *third_party_integration::stk_io::read_mesh_bulk_data(mesh.mFileName));
            return VolumeCriterion{.mScaleFactor = 1.0 / tVolumeTotal, .mIgnoreVoidBlocks = aIgnoreVoidBlocks}.f(mesh);
        },
        [aIgnoreVoidBlocks](const analysis::AnalysisDomainMesh& mesh)
        {
            const double tVolumeTotal = third_party_integration::stk_io::mesh_volume(
                *third_party_integration::stk_io::read_mesh_bulk_data(mesh.mFileName));
            return VolumeCriterion{.mScaleFactor = 1.0 / tVolumeTotal, .mIgnoreVoidBlocks = aIgnoreVoidBlocks}.df(mesh);
        });
}

namespace detail
{
auto parse_input_block(const std::filesystem::path& aFilePath) -> input_parser::volume_criterion
{
    if (!std::filesystem::exists(aFilePath))
    {
        auto tReturn = input_parser::volume_criterion{};
        tReturn.ignore_void_blocks = true;
        return tReturn;
    }

    auto tInputs = input_parser::volume_criterion{};
    auto tInputStream = std::ifstream{aFilePath};
    const auto tInputFileString =
        std::string(std::istreambuf_iterator<char>(tInputStream), std::istreambuf_iterator<char>());

    const auto tParser = input_parser::BlockStructRule<std::string::const_iterator, input_parser::volume_criterion>{};
    const auto tSkipper = input_parser::SkipperRule<std::string::const_iterator>{};

    auto tInputIterator = tInputFileString.begin();
    [[maybe_unused]] const auto tParseSucceeded =
        phrase_parse(tInputIterator, tInputFileString.cend(), tParser.mBlockRule, tSkipper.skipperRule(), tInputs);

    return tInputs;
}
}  // namespace detail

}  // namespace plato::criteria::extension
