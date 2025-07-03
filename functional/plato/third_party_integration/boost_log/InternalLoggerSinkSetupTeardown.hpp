#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_INTERNALLOGGERSETUPTEARDOWN
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_INTERNALLOGGERSETUPTEARDOWN

#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>

namespace plato::third_party_integration::boost_log
{
/// @brief An RAII-style class to manage a boost::log sink. On construction, this adds a new sink to the boost logging
/// core using the ostream given on construction. On destruction, the sink is removed from the logging core.
class InternalLoggerSinkSetupTeardown
{
   public:
    InternalLoggerSinkSetupTeardown(const std::shared_ptr<std::ostream>& aStreamSink);
    ~InternalLoggerSinkSetupTeardown();

    InternalLoggerSinkSetupTeardown(const InternalLoggerSinkSetupTeardown&) = delete;
    InternalLoggerSinkSetupTeardown(InternalLoggerSinkSetupTeardown&&) = delete;
    auto operator=(const InternalLoggerSinkSetupTeardown&) -> InternalLoggerSinkSetupTeardown& = delete;
    auto operator=(InternalLoggerSinkSetupTeardown&&) -> InternalLoggerSinkSetupTeardown& = delete;

   private:
    using TextOstreamSink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>;

    boost::shared_ptr<TextOstreamSink> mSink = boost::make_shared<TextOstreamSink>();
};

void initialize_internal_console_sink();

}  // namespace plato::third_party_integration::boost_log

#endif
