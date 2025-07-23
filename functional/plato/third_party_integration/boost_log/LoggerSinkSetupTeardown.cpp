#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"

#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <iostream>

namespace plato::third_party_integration::boost_log
{
LoggerSinkSetupTeardown::LoggerSinkSetupTeardown(const std::shared_ptr<std::ostream>& aStreamSink,
                                                 const boost::log::formatter& aFormatter,
                                                 const boost::log::filter& aFilter)
{
    mSink->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(aStreamSink.get(), boost::null_deleter()));
    mSink->set_formatter(aFormatter);
    mSink->set_filter(aFilter);
    boost::log::core::get()->add_sink(mSink);
}

LoggerSinkSetupTeardown::~LoggerSinkSetupTeardown() { boost::log::core::get()->remove_sink(mSink); }

}  // namespace plato::third_party_integration::boost_log
