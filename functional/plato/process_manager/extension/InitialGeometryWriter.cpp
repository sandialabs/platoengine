#include "plato/process_manager/extension/InitialGeometryWriter.hpp"

#include "plato/output/OutputManager.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerLogger.hpp"

namespace plato::process_manager::extension
{
namespace
{
[[nodiscard]] auto make_initial_geometry_writer_process_manager() -> library::StageAndProcessManager
{
    return {library::RunStage::kValidate, [](const library::ProcessManagerData& aProcessManagerData)
            { InitialGeometryWriter{}.run(aProcessManagerData); }};
}

[[maybe_unused]] static const auto kConformingDensityMeshGeneratorParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::initial_geometry_writer>{};

[[maybe_unused]] static const auto kConformingDensityMeshGeneratorRegistration =
    library::ProcessManagerRegistration{input_parser::block_name<input_parser::initial_geometry_writer>(),
                                        [](const library::ValidatedProcessManagerInput& /*aValidInput*/)
                                        { return make_initial_geometry_writer_process_manager(); }};
}  // namespace

void InitialGeometryWriter::run(const library::ProcessManagerData& aProcessManagerData) const
{
    [[maybe_unused]] const auto tTaskLogger = library::run_task_log<input_parser::initial_geometry_writer>();
    auto tOutputManager =
        output::OutputManager{aProcessManagerData.mGeometry.mOutput, output::OutputMode::kEveryIterationOverwrite};
    tOutputManager.output(aProcessManagerData.mGeometry.mInitialGuess);
}
}  // namespace plato::process_manager::extension
