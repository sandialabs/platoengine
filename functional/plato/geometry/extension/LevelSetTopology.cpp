#include "plato/geometry/extension/LevelSetTopology.hpp"

#include <algorithm>
#include <boost/mpi/communicator.hpp>
#include <optional>

#include "plato/analysis/Utilities.hpp"
#include "plato/core/Compose.hpp"
#include "plato/geometry/extension/FixedBlockUtilities.hpp"
#include "plato/geometry/extension/MeshValidationUtilities.hpp"
#include "plato/geometry/extension/OutputUtilities.hpp"
#include "plato/geometry/library/GeometryFilterUtilities.hpp"
#include "plato/geometry/library/GeometryLogger.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/geometry/library/OutputInfo.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/mesh/DesignVariableAdapter.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"
#include "plato/third_party_integration/krino/SphereFactory.hpp"
#include "plato/utilities/FileUtilities.hpp"

namespace plato::geometry::extension
{

namespace
{
namespace tpik = third_party_integration::krino;

constexpr auto kKrinoLogFileName = std::string_view{"Krino_Output.txt"};
constexpr auto kKrinoCutMeshBaseName = std::string_view{"krino_cut_mesh.exo"};

constexpr auto kMeshNameAccessor =
    [](const input_parser::level_set_topology& aInput) -> const boost::optional<input_parser::FileName>&
{ return aInput.mesh_name; };

[[nodiscard]] auto make_topology_output(const library::ValidatedGeometryInput& aGeometryInput)
    -> library::FactoryTypes::Output
{
    return
        [aGeometryInput](const linear_algebra::DynamicVector<double>& aSolution, const library::OutputInfo& aOutputInfo)
    {
        const auto& tInput = input_validation::get_input_block<input_parser::level_set_topology>(aGeometryInput);
        return LevelSetTopology::output(
            tInput, library::make_filter_from_geometry_input<input_parser::level_set_topology>(aGeometryInput),
            aSolution, aOutputInfo);
    };
}

void initialize_krino()
{
    static bool tIsInitialized = false;
    if (!tIsInitialized)
    {
        tIsInitialized = true;
        tpik::initialize_environment_for_krino(kKrinoLogFileName, MPI_COMM_WORLD);
    }
}

auto make_level_set_geometry(const library::ValidatedGeometryInput& aGeometryInput) -> library::GeometryFunction
{
    const auto& tInput = input_validation::get_input_block<input_parser::level_set_topology>(aGeometryInput);
    return make_level_set_geometry(
        std::make_shared<LevelSetTopology>(tInput),
        library::make_filter_from_geometry_input<input_parser::level_set_topology>(aGeometryInput));
}

[[nodiscard]] auto any_sphere_pattern_specifiers(const input_parser::level_set_topology& aInput) -> bool
{
    return aInput.sphere_pattern_bbox_max_x.has_value() || aInput.sphere_pattern_bbox_max_y.has_value() ||
           aInput.sphere_pattern_bbox_max_z.has_value() || aInput.sphere_pattern_bbox_min_x.has_value() ||
           aInput.sphere_pattern_bbox_min_y.has_value() || aInput.sphere_pattern_bbox_min_z.has_value() ||
           aInput.sphere_pattern_radius.has_value() || aInput.sphere_pattern_spacing.has_value();
}

[[nodiscard]] auto all_sphere_pattern_bounding_box_specifiers(const input_parser::level_set_topology& aInput) -> bool
{
    return aInput.sphere_pattern_bbox_max_x.has_value() && aInput.sphere_pattern_bbox_max_y.has_value() &&
           aInput.sphere_pattern_bbox_max_z.has_value() && aInput.sphere_pattern_bbox_min_x.has_value() &&
           aInput.sphere_pattern_bbox_min_y.has_value() && aInput.sphere_pattern_bbox_min_z.has_value();
}

/// Static registration for parser
[[maybe_unused]] static auto kLevelSetTopologyParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::level_set_topology>{};

/// Static registration for library
[[maybe_unused]] static auto kLevelSetTopologyRegistration = plato::geometry::library::GeometryRegistration{
    input_parser::block_name<input_parser::level_set_topology>(),
    [](const library::ValidatedGeometryInput& aGeometryInput)
    {
        initialize_krino();
        const auto& tInput = input_validation::get_input_block<input_parser::level_set_topology>(aGeometryInput);
        auto tLevelSet = LevelSetTopology{tInput};
        return library::FactoryTypes{make_level_set_geometry(aGeometryInput), tLevelSet.initialGuess(tInput),
                                     tLevelSet.bounds(), make_topology_output(aGeometryInput)};
    }};

/// Static registration for input validation functions
[[maybe_unused]] static auto kLevelSetTopologyValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{
        [](const input_parser::level_set_topology& aInput) { return library::detail::validate_mesh_name(aInput); },
        [](const input_parser::level_set_topology& aInput) { return library::detail::validate_output_name(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_lower_bound(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_upper_bound(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_pattern_radius(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_pattern_spacing(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_pattern_bbox(aInput); },
        [](const input_parser::level_set_topology& aInput)
        { return validate_unique_fixed_block_names(aInput, kMeshNameAccessor); },
        [](const input_parser::level_set_topology& aInput)
        { return validate_fixed_block_names_exist(aInput, kMeshNameAccessor); },
        [](const input_parser::level_set_topology& aInput)
        { return validate_at_least_one_design_block(aInput, kMeshNameAccessor); },
        [](const input_parser::level_set_topology& aInput)
        { return detail::validate_exactly_one_initial_level_set_specifier(aInput); },
        [](const input_parser::level_set_topology& aInput) { return validate_initial_field_source(aInput); },
    };

auto sphere_pattern(const input_parser::level_set_topology& aInput) -> tpik::SpherePatternData
{
    return tpik::SpherePatternData{{aInput.sphere_pattern_bbox_min_x.value(), aInput.sphere_pattern_bbox_min_y.value(),
                                    aInput.sphere_pattern_bbox_min_z.value()},
                                   {aInput.sphere_pattern_bbox_max_x.value(), aInput.sphere_pattern_bbox_max_y.value(),
                                    aInput.sphere_pattern_bbox_max_z.value()},
                                   aInput.sphere_pattern_radius.value(),
                                   aInput.sphere_pattern_spacing.value()};
}

auto void_phase(const input_parser::level_set_topology& aInput)
{
    return aInput.include_void_region.value() ? tpik::VoidPhase::kIncludeInMesh : tpik::VoidPhase::kExcludeFromMesh;
}

}  // namespace

LevelSetTopology::LevelSetTopology(const input_parser::level_set_topology& aInput)
    : mBackgroundMesh(mesh_from_input(aInput)),
      mCutMesh(kKrinoCutMeshBaseName),
      mOutputMesh(aInput.output_name.value().mToken),
      mVoidRegion(void_phase(aInput)),
      mLevelSetBounds(std::make_pair(aInput.level_set_lower_bound.value(), aInput.level_set_upper_bound.value())),
      mKrinoWrapperCache{[tFixedValue = mLevelSetBounds.second](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
                         { return make_krino_wrapper_from_analysis_domain_mesh(aAnalysisDomainMesh, tFixedValue); },
                         [](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
                         { return analysis::hash_value(aAnalysisDomainMesh); }}
{
}

LevelSetTopology::~LevelSetTopology() { std::filesystem::remove(mCutMesh); }

auto LevelSetTopology::bounds() const -> std::pair<std::vector<double>, std::vector<double>>
{
    const unsigned int tNumNodes = mesh::EntityCounts{mBackgroundMesh}.numberOfDesignDomainNodes();
    return {std::vector<double>(tNumNodes, mLevelSetBounds.first),
            std::vector<double>(tNumNodes, mLevelSetBounds.second)};
}

auto LevelSetTopology::initialGuess(const input_parser::level_set_topology& aInput) const
    -> linear_algebra::DynamicVector<double>
{
    if (aInput.initial_field_name.has_value())
    {
        auto tValuesFromMesh = initial_field_from_mesh(aInput, mBackgroundMesh);
        const auto tMin = *std::min_element(tValuesFromMesh.begin(), tValuesFromMesh.end());
        const auto tMax = *std::max_element(tValuesFromMesh.begin(), tValuesFromMesh.end());
        return linear_algebra::DynamicVector<double>(detail::affine_transformation(
            std::move(tValuesFromMesh), detail::StartingLimits{std::make_pair(tMin, tMax)},
            detail::EndingLimits{mLevelSetBounds}));
    }
    else
    {
        const auto tLevelSetPrimitives = tpik::LevelSetPrimitives{{}, tpik::generate_spheres(sphere_pattern(aInput))};
        const auto tDesignDomainNodeIds = mesh::EntityRetrieval{mBackgroundMesh}.designDomainNodeIDs();
        return linear_algebra::DynamicVector<double>{make_initial_guess_from_level_set_primitives(
            mBackgroundMesh.filePath(), tLevelSetPrimitives, tDesignDomainNodeIds)};
    }
}

auto LevelSetTopology::generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameters) const
    -> analysis::AnalysisDomainMesh
{
    [[maybe_unused]] const auto tTaskLogger =
        services::TaskLogSetupTeardown{"Mesh generation", library::geometry_logger<input_parser::level_set_topology>()};

    const auto tAnalysisMesh = mesh::DesignVariablesConversion{mBackgroundMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{aDesignParameters.stdVector()});

    mKrinoWrapperCache.compute(tAnalysisMesh).writeCutMesh(mCutMesh, mVoidRegion);
    return analysis::AnalysisDomainMesh{mCutMesh, {}};
}

auto LevelSetTopology::jacobian(const linear_algebra::DynamicVector<double>& aDesignParameters) const
    -> linear_algebra::JacobianMultiplier
{
    return linear_algebra::JacobianMultiplier{
        [this, aDesignParameters](
            const linear_algebra::DynamicVector<double>& aVector) -> linear_algebra::DynamicVector<double>
        {
            [[maybe_unused]] const auto tTaskLogger = services::TaskLogSetupTeardown{
                "Vector-Jacobian product", library::geometry_logger<input_parser::level_set_topology>()};

            auto tBackgroundMeshWithLevelSetField =
                mesh::DesignVariablesConversion{mBackgroundMesh}.nodalFieldToAnalysisDomainMesh(
                    mesh::NodalFieldVectorReference{aDesignParameters.stdVector()});

            return linear_algebra::DynamicVector<double>{
                mKrinoWrapperCache.compute(tBackgroundMeshWithLevelSetField)
                    .rowVectorJacobianProduct(aVector.stdVector(), mVoidRegion)};
        }};
}

auto LevelSetTopology::adjointJacobian(const linear_algebra::DynamicVector<double>& aDesignParameters) const
    -> linear_algebra::AdjointJacobianMultiplier
{
    return linear_algebra::AdjointJacobianMultiplier{linear_algebra::JacobianMultiplier{
        [this, aDesignParameters](
            const linear_algebra::DynamicVector<double>& aVector) -> linear_algebra::DynamicVector<double>
        {
            [[maybe_unused]] const auto tTaskLogger = services::TaskLogSetupTeardown{
                "Vector-adjoint-Jacobian product", library::geometry_logger<input_parser::level_set_topology>()};

            const auto tDesignVariableConverter = mesh::DesignVariablesConversion{mBackgroundMesh};

            const auto tBackgroundMeshWithLevelSets = tDesignVariableConverter.nodalFieldToAnalysisDomainMesh(
                mesh::NodalFieldVectorReference{aDesignParameters.stdVector()});

            return linear_algebra::DynamicVector<double>{
                mKrinoWrapperCache.compute(tBackgroundMeshWithLevelSets)
                    .rowVectorAdjointJacobianProduct(aVector.stdVector(), mVoidRegion)};
        }}};
}

void LevelSetTopology::output(const input_parser::level_set_topology& aInput,
                              const filter::library::FilterFunction& aFilterFunction,
                              const linear_algebra::DynamicVector<double>& aSolution,
                              const library::OutputInfo& aOutputInfo)
{
    [[maybe_unused]] const auto tTaskLogger =
        services::TaskLogSetupTeardown{"Writing output", library::geometry_logger<input_parser::level_set_topology>()};

    const auto tMeshFieldOutput = MeshFieldOutputInfo{mesh_from_input(aInput),
                                                      restart_file_name(aInput),
                                                      fixed_blocks(aInput),
                                                      level_set_mesh_field_name(),
                                                      filtered_level_set_mesh_field_name(),
                                                      aInput.level_set_upper_bound.value()};
    const auto tFilteredField = output_nodal_field(tMeshFieldOutput, aFilterFunction, aSolution, aOutputInfo);

    make_krino_wrapper_from_analysis_domain_mesh(tFilteredField, tMeshFieldOutput.mFixedFieldValue)
        .writeCutMesh(aInput.output_name->mToken, void_phase(aInput));
}

auto LevelSetTopology::backgroundMesh() const -> const mesh::Mesh& { return mBackgroundMesh; }

auto make_level_set_geometry(const std::shared_ptr<LevelSetTopology>& aLevelSetTopology,
                             const filter::library::FilterFunction& aFilterFunction) -> library::GeometryFunction
{
    auto tGeometryFunction = library::GeometryFunction{
        [aLevelSetTopology](const linear_algebra::DynamicVector<double>& x)
        { return aLevelSetTopology->generateMesh(x); },
        [aLevelSetTopology](const linear_algebra::DynamicVector<double>& x) { return aLevelSetTopology->jacobian(x); },
        [aLevelSetTopology](const linear_algebra::DynamicVector<double>& x)
        { return linear_algebra::AdjointJacobianMultiplier{aLevelSetTopology->adjointJacobian(x)}; }};

    const auto tAdaptedFilter = library::adapt_filter(aFilterFunction, aLevelSetTopology->backgroundMesh());
    return core::compose(tGeometryFunction, tAdaptedFilter);
}

auto restart_file_name(const input_parser::level_set_topology& aInput) -> std::filesystem::path
{
    constexpr auto tRestartFileNamePrefix = std::string_view{"restart_"};
    return std::filesystem::path{std::string{tRestartFileNamePrefix} + aInput.output_name->mToken};
}

namespace detail
{

auto validate_lower_bound(const input_parser::level_set_topology& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_parameter_out_of_bounds(
        input_parser::block_name<input_parser::level_set_topology>(), aInput.level_set_lower_bound,
        "level_set_lower_bound", utilities::upper_bounded(utilities::Exclusive{0.0}));
}

auto validate_upper_bound(const input_parser::level_set_topology& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_parameter_out_of_bounds(
        input_parser::block_name<input_parser::level_set_topology>(), aInput.level_set_upper_bound,
        "level_set_upper_bound", utilities::lower_bounded(utilities::Exclusive{0.0}));
}

auto validate_sphere_pattern_spacing(const input_parser::level_set_topology& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::level_set_topology>(), aInput.sphere_pattern_spacing,
        "sphere_pattern_spacing", utilities::lower_bounded(utilities::Exclusive{1e-5}));
}

auto validate_sphere_pattern_radius(const input_parser::level_set_topology& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::level_set_topology>(), aInput.sphere_pattern_radius,
        "sphere_pattern_radius", utilities::lower_bounded(utilities::Exclusive{1e-5}));
}

auto validate_sphere_pattern_bbox(const input_parser::level_set_topology& aInput) -> std::optional<std::string>
{
    if (!all_sphere_pattern_bounding_box_specifiers(aInput) && !aInput.initial_field_name.has_value())
    {
        return std::string("Missing entries in sphere pattern bbox specification.");
    }

    if (all_sphere_pattern_bounding_box_specifiers(aInput) &&
        (aInput.sphere_pattern_bbox_max_x.value() < aInput.sphere_pattern_bbox_min_x.value() ||
         aInput.sphere_pattern_bbox_max_y.value() < aInput.sphere_pattern_bbox_min_y.value() ||
         aInput.sphere_pattern_bbox_max_z.value() < aInput.sphere_pattern_bbox_min_z.value()))
    {
        return std::string("Invalid sphere pattern bounding box was specified, check limits.");
    }

    return std::nullopt;
}

std::optional<std::string> validate_exactly_one_initial_level_set_specifier(
    const input_parser::level_set_topology& aInput)
{
    const bool tAnySpherePatterns = any_sphere_pattern_specifiers(aInput);
    const bool tReadFieldSpecifier = aInput.initial_field_name.has_value();

    if (tReadFieldSpecifier && tAnySpherePatterns)
    {
        return std::string(
            "Specify only one method to initialize the level set field, either a sphere pattern or "
            "'initial_field_name'.");
    }
    if (!tReadFieldSpecifier && !tAnySpherePatterns)
    {
        return std::string(
            "Specify some method to initialize the level set field, either a sphere pattern or "
            "'initial_field_name'.");
    }
    return std::nullopt;
}

auto affine_transformation(std::vector<double> aVector,
                           const StartingLimits& aStartingLimits,
                           const EndingLimits& aEndingLimits) -> std::vector<double>
{
    assert(aStartingLimits.mValue.second > aStartingLimits.mValue.first);
    assert(aEndingLimits.mValue.second > aEndingLimits.mValue.first);
    const double tStartingScale = aStartingLimits.mValue.second - aStartingLimits.mValue.first;
    const double tStartingMin = aStartingLimits.mValue.first;
    const double tEndingScale = aEndingLimits.mValue.second - aEndingLimits.mValue.first;
    const double tEndingMin = aEndingLimits.mValue.first;

    std::transform(aVector.begin(), aVector.end(), aVector.begin(),
                   [tStartingMin, tStartingScale, tEndingScale, tEndingMin](const auto tEntry)
                   { return (tEntry - tStartingMin) / tStartingScale * tEndingScale + tEndingMin; });

    return aVector;
}

}  // namespace detail

}  // namespace plato::geometry::extension
