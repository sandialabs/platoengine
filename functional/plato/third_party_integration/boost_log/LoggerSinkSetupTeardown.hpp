#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_LOGGERSINKSETUPTEARDOWN
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_LOGGERSINKSETUPTEARDOWN

#include <boost/log/expressions/filter.hpp>
#include <boost/log/expressions/formatter.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>

namespace plato::third_party_integration::boost_log
{
/// @brief An RAII-style class to manage a boost::log sink. On construction, this adds a new sink to the boost logging
/// core using the ostream given on construction. On destruction, the sink is removed from the logging core.
class LoggerSinkSetupTeardown
{
   public:
    LoggerSinkSetupTeardown(const boost::shared_ptr<std::ostream>& aStreamSink,
                            const boost::log::formatter& aFormatter,
                            const boost::log::filter& aFilter);
    ~LoggerSinkSetupTeardown();

    LoggerSinkSetupTeardown(const LoggerSinkSetupTeardown&) = delete;
    LoggerSinkSetupTeardown(LoggerSinkSetupTeardown&&) = delete;
    auto operator=(const LoggerSinkSetupTeardown&) -> LoggerSinkSetupTeardown& = delete;
    auto operator=(LoggerSinkSetupTeardown&&) -> LoggerSinkSetupTeardown& = delete;

   private:
    using TextOstreamSink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>;

    boost::shared_ptr<TextOstreamSink> mSink = boost::make_shared<TextOstreamSink>();
};

}  // namespace plato::third_party_integration::boost_log

#endif
