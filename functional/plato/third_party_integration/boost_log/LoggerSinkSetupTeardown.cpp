#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"

#include <boost/log/expressions/filter.hpp>
#include <boost/log/expressions/formatter.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <iostream>

namespace plato::third_party_integration::boost_log
{
LoggerSinkSetupTeardown::LoggerSinkSetupTeardown(const boost::shared_ptr<std::ostream>& aStreamSink,
                                                 const boost::log::formatter& aFormatter,
                                                 const boost::log::filter& aFilter)
{
    mSink->locked_backend()->add_stream(aStreamSink);
    mSink->set_formatter(aFormatter);
    mSink->set_filter(aFilter);
    mSink->locked_backend()->auto_flush(true);
    boost::log::core::get()->add_sink(mSink);
}

LoggerSinkSetupTeardown::~LoggerSinkSetupTeardown() { boost::log::core::get()->remove_sink(mSink); }

}  // namespace plato::third_party_integration::boost_log
