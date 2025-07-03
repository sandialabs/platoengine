#include "plato/third_party_integration/boost_log/InternalLoggerSinkSetupTeardown.hpp"

#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <iostream>

namespace plato::third_party_integration::boost_log
{
InternalLoggerSinkSetupTeardown::InternalLoggerSinkSetupTeardown(const std::shared_ptr<std::ostream>& aStreamSink)
{
    mSink->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(aStreamSink.get(), boost::null_deleter()));
    mSink->set_formatter(boost::log::expressions::stream << boost::log::expressions::smessage);
    boost::log::core::get()->add_sink(mSink);
}

InternalLoggerSinkSetupTeardown::~InternalLoggerSinkSetupTeardown() { boost::log::core::get()->remove_sink(mSink); }

void initialize_internal_console_sink()
{
    [[maybe_unused]] static auto tConsoleSink =
        InternalLoggerSinkSetupTeardown{std::shared_ptr<std::ostream>{&std::cout, boost::null_deleter()}};
}

}  // namespace plato::third_party_integration::boost_log
