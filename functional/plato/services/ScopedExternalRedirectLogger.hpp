#ifndef PLATO_SERVICES_SCOPEDEXTERNALREDIRECTLOGGER
#define PLATO_SERVICES_SCOPEDEXTERNALREDIRECTLOGGER

#include <sstream>
#include <string>

#include "plato/components/ComponentType.hpp"

namespace plato::services
{
/// @brief A logging source that captures stdout and stderr and redirects to an external log sink.
///
/// This is an RAII-style object that, on construction, captures all `stdout` and `sterr` output and on destruction,
/// writes the captured output to the external log sink. `stdout` output will be labeled with the `info` attribute, and
/// `stderr` output will be labeled with the `error` attribute.
template <components::ComponentType kComponentType>
class ScopedExternalRedirectLogger
{
   public:
    ScopedExternalRedirectLogger(std::string aComponentName);
    ~ScopedExternalRedirectLogger();

    ScopedExternalRedirectLogger(const ScopedExternalRedirectLogger&) = delete;
    ScopedExternalRedirectLogger(ScopedExternalRedirectLogger&&) = delete;
    auto operator=(const ScopedExternalRedirectLogger&) -> ScopedExternalRedirectLogger& = delete;
    auto operator=(ScopedExternalRedirectLogger&&) -> ScopedExternalRedirectLogger& = delete;

   private:
    std::string mComponentName;
    std::streambuf* mOriginalCoutBuffer;
    std::streambuf* mOriginalCerrBuffer;
    std::stringstream mRedirectedCoutStream;
    std::stringstream mRedirectedCerrStream;
};

}  // namespace plato::services

#endif
