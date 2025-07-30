#include <gtest/gtest.h>

#include <boost/shared_ptr.hpp>
#include <filesystem>
#include <fstream>

#include "plato/services/ScopedExternalRedirectLogger.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"
#include "plato/third_party_integration/boost_log/SinkWithAttributeFormattersAndFilters.hpp"

namespace plato::services::unittest
{
namespace
{
constexpr auto kTestMessage = std::string_view{"Definitely not going to stdout."};

void check_redirected_log_output(std::ostream& aOutputStream, const test_utilities::TestContext& aTestContext)
{
    namespace tpi_bl = third_party_integration::boost_log;

    auto tStream = boost::make_shared<std::stringstream>();

    [[maybe_unused]] const auto tExternalSink =
        tpi_bl::sink_with_attribute_formatters_and_filters<tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kExternal>,
                                                           tpi_bl::ComponentTypeAndNameAttribute,
                                                           tpi_bl::SeverityAttribute>(tStream);

    {
        [[maybe_unused]] const auto tExternalLogCapture =
            ScopedExternalRedirectLogger{components::ComponentType::kObjective, "nodal-sum"};

        aOutputStream << kTestMessage;
    }

    EXPECT_NE(tStream->str().find(kTestMessage), std::string::npos) << aTestContext << "Log: " << tStream->str();
}
}  // namespace

TEST(ScopedExternalRedirectLogger, CapturesCout) { check_redirected_log_output(std::cout, TEST_CONTEXT("std::cout")); }

TEST(ScopedExternalRedirectLogger, CapturesCerr)
{
    // For cerr, we expect the error message to be sent to the console as well
    namespace tpi_bl = third_party_integration::boost_log;
    auto tInternalStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tInternalSink =
        tpi_bl::sink_with_attribute_formatters_and_filters<tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kInternal>,
                                                           tpi_bl::ComponentTypeAndNameAttribute,
                                                           tpi_bl::SeverityAttribute>(tInternalStream);

    check_redirected_log_output(std::cerr, TEST_CONTEXT("std::cerr"));

    EXPECT_NE(tInternalStream->str().find(kTestMessage), std::string::npos) << "Log: " << tInternalStream->str();
}

TEST(ScopedExternalRedirectLogger, ExceptionBehavior)
{
    namespace tpi_bl = third_party_integration::boost_log;

    auto tStream = boost::make_shared<std::stringstream>();
    {
        [[maybe_unused]] const auto tExternalSink = tpi_bl::sink_with_attribute_formatters_and_filters<
            tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kExternal> >(tStream);

        try
        {
            [[maybe_unused]] const auto tExternalLogCapture =
                ScopedExternalRedirectLogger{components::ComponentType::kGeometry, "brick"};

            std::cout << kTestMessage;
            throw std::runtime_error{"Throwing before redirect closes"};
        }
        catch (const std::runtime_error& aTestException)
        {
        }
    }
    EXPECT_NE(tStream->str().find(kTestMessage), std::string::npos) << "Log: " << tStream->str();
}
}  // namespace plato::services::unittest
