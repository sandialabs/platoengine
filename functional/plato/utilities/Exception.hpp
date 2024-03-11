#ifndef PLATO_UTILITIES_EXCEPTION
#define PLATO_UTILITIES_EXCEPTION

#include <stdexcept>

namespace plato::utilities
{
class Exception : public std::runtime_error
{
   public:
    explicit Exception(const std::string& what) : std::runtime_error(what) {}
};
}  // namespace plato::utilities
#endif
