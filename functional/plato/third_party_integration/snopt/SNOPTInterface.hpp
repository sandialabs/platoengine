#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTINTERFACE
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTINTERFACE

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <optional>
#include <vector>

#include "plato/third_party_integration/snopt/SNOPTTypes.hpp"

class snoptProblemA;

namespace plato::output
{
class OutputManager;
}

namespace plato::third_party_integration::snopt
{
constexpr inline auto kTimeLimitName = std::string_view{"Time limit"};
constexpr inline auto kMajorIterationLimitName = std::string_view{"Major iterations limit"};

/// @brief A collection of options that are exposed in the plato input deck, they are all optional
struct SNOPTOptions
{
    std::optional<std::filesystem::path> mFilePath = std::nullopt;
    std::optional<unsigned int> mTimeLimitInMinutes = std::nullopt;
    std::optional<unsigned int> mMajorIterationLimit = std::nullopt;
    std::optional<bool> mOutputDesignHistory = std::nullopt;
};

/// @brief Solve optimization problem with SNOPT and return resulting design variables
/// @pre The size of @a aInitialGuess must be the same as the sizes of both vectors in @a aBoundConstraints.
[[nodiscard]] auto run_snopt_problem(const std::vector<double>& aInitialGuess,
                                     const SNOPTBounds& aBoundConstraints,
                                     ObjectiveType&& aObjective,
                                     const InterfaceConstraintVectorType& aConstraints,
                                     output::OutputManager&& aOutputManager,
                                     const std::filesystem::path& aLogFilePath,
                                     const SNOPTOptions& aOptions,
                                     const boost::mpi::communicator& aCommunicator) -> std::vector<double>;

namespace detail
{
/// @brief Apply a set of SNOPTOPtions @a aOptions to a given snoptProblemA @a aProblem.
/// Values specified in the input file mFilePath will be over written by any value given in the struct.
void apply_options(snoptProblemA& aProblem, const SNOPTOptions& aOptions);

/// @brief A helper struct to facilitate the output or no-output configuration for SNOPT on multiple ranks
struct SNOPTOutputConfig
{
    std::string mPrintFile;
    int mPrintUnit;
    std::string mSummaryFile;
    int mSummaryUnit;
};

/// @brief A helper function that takes a communicator @a aCommunicator and the designated log file path @a aLogFilePath
/// and populates a SNOPTOutputConfig that only writes to the log file on rank 0.
[[nodiscard]] auto make_snopt_output_config(const boost::mpi::communicator& aCommunicator,
                                            const std::filesystem::path& aLogFilePath) -> SNOPTOutputConfig;

}  // namespace detail

}  // namespace plato::third_party_integration::snopt

#endif
