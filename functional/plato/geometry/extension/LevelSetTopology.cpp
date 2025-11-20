#include "plato/geometry/extension/LevelSetTopology.hpp"

#include <algorithm>
#include <boost/mpi/communicator.hpp>
#include <optional>
#include <ranges>

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
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/SphereFactory.hpp"
#include "plato/utilities/FileUtilities.hpp"
#include "plato/utilities/ParameterBounds.hpp"

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
        return library::FactoryTypes{make_level_set_geometry(aGeometryInput), tLevelSet.initialGuess(),
                                     tLevelSet.bounds(), make_topology_output(aGeometryInput)};
    }};

/// Static registration for input validation functions
[[maybe_unused]] static auto kLevelSetTopologyValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{
        [](const input_parser::level_set_topology& aInput) { return library::detail::validate_mesh_name(aInput); },
        [](const input_parser::level_set_topology& aInput)
        { return library::detail::validate_mesh_file_exists(aInput); },
        [](const input_parser::level_set_topology& aInput) { return library::detail::validate_output_name(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_lower_bound(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_upper_bound(aInput); },
        [](const input_parser::level_set_topology& aInput)
        {
            return input_validation::error_message_for_empty_parameter(
                input_parser::block_name<input_parser::level_set_topology>(), aInput.level_set_bounds,
                "level_set_bounds");
        },
        [](const input_parser::level_set_topology& aInput)
        { return detail::validate_max_snapping_edge_length(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_pattern_radius(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_pattern_spacing(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_pattern_bbox(aInput); },
        [](const input_parser::level_set_topology& aInput)
        { return detail::validate_sphere_pattern_spacing_greater_than_twice_radius(aInput); },
        [](const input_parser::level_set_topology& aInput) { return detail::validate_sphere_list_radii(aInput); },
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
    return tpik::SpherePatternData{{aInput.sphere_pattern.value().min.mX, aInput.sphere_pattern.value().min.mY,
                                    aInput.sphere_pattern.value().min.mZ},
                                   {aInput.sphere_pattern.value().max.mX, aInput.sphere_pattern.value().max.mY,
                                    aInput.sphere_pattern.value().max.mZ},
                                   aInput.sphere_pattern.value().radius,
                                   aInput.sphere_pattern.value().spacing};
}

auto void_phase(const input_parser::level_set_topology& aInput)
{
    return aInput.include_void_region.value() ? tpik::VoidPhase::kIncludeInMesh : tpik::VoidPhase::kExcludeFromMesh;
}

auto snapping_parameters_from_input(const input_parser::level_set_topology& aInput) -> tpik::SnappingParameters
{
    constexpr auto tDefaultSnappingEdgeLength = tpik::SnappingParameters{}.mMaxSnappingEdgeLength;
    return tpik::SnappingParameters{
        .mMaxSnappingEdgeLength = aInput.max_edge_length_percentage_for_snapping.value_or(tDefaultSnappingEdgeLength)};
}
}  // namespace

LevelSetTopology::LevelSetTopology(const input_parser::level_set_topology& aInput)
    : mInput{aInput},
      mBackgroundMesh(mesh_from_input(aInput)),
      mCutMesh(kKrinoCutMeshBaseName),
      mOutputMesh(aInput.output_name.value().mToken),
      mVoidRegion(void_phase(aInput)),
      mLevelSetBounds(std::make_pair(aInput.level_set_bounds.value().mLower, aInput.level_set_bounds.value().mUpper)),
      mKrinoWrapperCache{
          [mFixedBlocks = fixed_blocks(aInput), mSnappingParameters = snapping_parameters_from_input(aInput)](
              const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
          {
              return make_krino_wrapper_from_analysis_domain_mesh(aAnalysisDomainMesh, mFixedBlocks,
                                                                  mSnappingParameters);
          },
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

auto LevelSetTopology::initialGuess() const -> linear_algebra::DynamicVector<double>
{
    if (mInput.initial_field_name.has_value())
    {
        auto tValuesFromMesh = initial_field_from_mesh(mInput, mBackgroundMesh);
        const auto tMin = *std::min_element(tValuesFromMesh.begin(), tValuesFromMesh.end());
        const auto tMax = *std::max_element(tValuesFromMesh.begin(), tValuesFromMesh.end());
        return linear_algebra::DynamicVector<double>(detail::affine_transformation(
            std::move(tValuesFromMesh), detail::StartingLimits{std::make_pair(tMin, tMax)},
            detail::EndingLimits{mLevelSetBounds}));
    }
    else if (mInput.sphere_pattern.has_value())
    {
        const auto tLevelSetPrimitives = tpik::LevelSetPrimitives{{}, tpik::generate_spheres(sphere_pattern(mInput))};
        return linear_algebra::DynamicVector<double>{make_initial_guess_from_level_set_primitives(
            mBackgroundMesh.filePath(), tLevelSetPrimitives, fixed_blocks(mInput))};
    }
    else
    {
        const auto tLevelSetPrimitives = tpik::LevelSetPrimitives{{}, detail::generate_spheres_from_list(mInput)};
        return linear_algebra::DynamicVector<double>{make_initial_guess_from_level_set_primitives(
            mBackgroundMesh.filePath(), tLevelSetPrimitives, fixed_blocks(mInput))};
    }
}

auto LevelSetTopology::generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameters) const
    -> analysis::AnalysisDomainMesh
{
    [[maybe_unused]] const auto tTaskLogger = library::mesh_generation_task_log<input_parser::level_set_topology>();

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
            [[maybe_unused]] const auto tTaskLogger = library::jacobian_task_log<input_parser::level_set_topology>();

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
            [[maybe_unused]] const auto tTaskLogger =
                library::adjoint_jacobian_task_log<input_parser::level_set_topology>();

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
    [[maybe_unused]] const auto tTaskLogger = library::output_task_log<input_parser::level_set_topology>();

    const auto tMeshFieldOutput = MeshFieldOutputInfo{mesh_from_input(aInput),
                                                      restart_file_name(aInput),
                                                      fixed_blocks(aInput),
                                                      level_set_mesh_field_name(),
                                                      filtered_level_set_mesh_field_name(),
                                                      aInput.level_set_bounds.value().mUpper};
    const auto tFilteredField = output_nodal_field(tMeshFieldOutput, aFilterFunction, aSolution, aOutputInfo);

    make_krino_wrapper_from_analysis_domain_mesh(tFilteredField, fixed_blocks(aInput),
                                                 snapping_parameters_from_input(aInput))
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

namespace
{

template <typename U, typename SubAccessorFunction, typename T>
[[nodiscard]] auto validate_bounded_subinput(const boost::optional<U>& aParameter,
                                             const std::string& aInputName,
                                             const utilities::ParameterBounds<T>& aParameterBounds,
                                             const SubAccessorFunction& aSubAccessorFunction,
                                             const std::string& aMessage) -> std::optional<std::string>
{
    if (aParameter && !aParameterBounds.contains(aSubAccessorFunction(aParameter.value())))
    {
        return utilities::concatenate(input_parser::block_name<input_parser::level_set_topology>(), " entry \"",
                                      aInputName, "\" has value ", aSubAccessorFunction(aParameter.value()), aMessage);
    }
    return std::nullopt;
}
}  // namespace

auto validate_lower_bound(const input_parser::level_set_topology& aInput) -> std::optional<std::string>
{
    return validate_bounded_subinput(
        aInput.level_set_bounds, "level_set_bounds", utilities::upper_bounded(utilities::Exclusive{0.0}),
        [](const auto& aBounds) { return aBounds.mLower; }, " and needs to be less than zero.");
}

auto validate_upper_bound(const input_parser::level_set_topology& aInput) -> std::optional<std::string>
{
    return validate_bounded_subinput(
        aInput.level_set_bounds, "level_set_bounds", utilities::lower_bounded(utilities::Exclusive{0.0}),
        [](const auto& aBounds) { return aBounds.mUpper; }, " and needs to be greater than zero.");
}

auto validate_max_snapping_edge_length(const input_parser::level_set_topology& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::level_set_topology>(), aInput.max_edge_length_percentage_for_snapping,
        "max_edge_length_percentage_for_snapping", utilities::unit_bounded());
}

auto validate_sphere_pattern_spacing(const input_parser::level_set_topology& aInput) -> std::optional<std::string>
{
    return validate_bounded_subinput(
        aInput.sphere_pattern, "sphere_pattern", utilities::lower_bounded(utilities::Exclusive{1e-5}),
        [](const auto& aSpherePattern) { return aSpherePattern.spacing; },
        " and spacing needs to be greater than 1e-5.");
}

auto validate_sphere_pattern_radius(const input_parser::level_set_topology& aInput) -> std::optional<std::string>
{
    return validate_bounded_subinput(
        aInput.sphere_pattern, "sphere_pattern", utilities::lower_bounded(utilities::Exclusive{1e-5}),
        [](const auto& aSpherePattern) { return aSpherePattern.radius; }, " and radius needs to be greater than 1e-5.");
}

auto validate_sphere_pattern_bbox(const input_parser::level_set_topology& aInput) -> std::optional<std::string>
{
    if (aInput.sphere_pattern && (aInput.sphere_pattern.value().min.mX > aInput.sphere_pattern.value().max.mX ||
                                  aInput.sphere_pattern.value().min.mY > aInput.sphere_pattern.value().max.mY ||
                                  aInput.sphere_pattern.value().min.mZ > aInput.sphere_pattern.value().max.mZ))
    {
        return utilities::concatenate(input_parser::block_name<input_parser::level_set_topology>(),
                                      " Invalid sphere pattern bounding box was specified, check limits.");
    }

    return std::nullopt;
}

std::optional<std::string> validate_exactly_one_initial_level_set_specifier(
    const input_parser::level_set_topology& aInput)
{
    const int tSpherePattern = aInput.sphere_pattern.has_value() ? 1 : 0;
    const int tReadFieldSpecifier = aInput.initial_field_name.has_value() ? 1 : 0;
    const int tSphereList = aInput.sphere_list.has_value() ? 1 : 0;
    const auto tTallyOfSpecifiers = tSpherePattern + tReadFieldSpecifier + tSphereList;

    if (tTallyOfSpecifiers != 1)
    {
        return utilities::concatenate(input_parser::block_name<input_parser::level_set_topology>(),
                                      " Specify exactly one method to initialize the level set field, either a "
                                      "'sphere_pattern', 'sphere_list' or "
                                      "'initial_field_name'.");
    }

    return std::nullopt;
}

auto validate_sphere_pattern_spacing_greater_than_twice_radius(const input_parser::level_set_topology& aInput)
    -> std::optional<std::string>
{
    if (aInput.sphere_pattern.has_value() &&
        aInput.sphere_pattern.value().spacing < aInput.sphere_pattern.value().radius * 2.0)
    {
        return utilities::concatenate(input_parser::block_name<input_parser::level_set_topology>(),
                                      " 'sphere_pattern' contains a spacing that is less than twice the radius.");
    }
    return std::nullopt;
}

auto validate_sphere_list_radii(const input_parser::level_set_topology& aInput) -> std::optional<std::string>
{
    if (aInput.sphere_list)
    {
        const auto tList = aInput.sphere_list.value().mList;
        const auto tAnyInvalidRadii = std::ranges::any_of(
            tList, [](const auto aLevelSetSphereEntry) { return aLevelSetSphereEntry.radius <= 0; });
        if (tAnyInvalidRadii)
        {
            return utilities::concatenate(input_parser::block_name<input_parser::level_set_topology>(),
                                          " 'sphere_list' contains a radius that is not greater than 0.");
        }
    }
    return std::nullopt;
}

auto generate_spheres_from_list(const input_parser::level_set_topology& aInput)
    -> std::vector<third_party_integration::krino::Sphere>
{
    assert(aInput.sphere_list.has_value());
    namespace tpi = third_party_integration;
    const auto tSphereList = aInput.sphere_list.value().mList;
    auto tToKrinoSpheres =
        std::views::transform(tSphereList,
                              [](const auto& aLevelSetSphere)
                              {
                                  const auto tCenter = tpi::common::Coordinate{
                                      aLevelSetSphere.center.mX, aLevelSetSphere.center.mY, aLevelSetSphere.center.mZ};
                                  return tpi::krino::Sphere{tCenter, aLevelSetSphere.radius};
                              });
    return std::vector(tToKrinoSpheres.begin(), tToKrinoSpheres.end());
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
