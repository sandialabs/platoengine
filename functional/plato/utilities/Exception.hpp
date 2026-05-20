#ifndef PLATO_UTILITIES_EXCEPTION
#define PLATO_UTILITIES_EXCEPTION

#include <stdexcept>

namespace plato::utilities
{
class Exception : public std::runtime_error
{
   public:
    explicit Exception(const std::string& aWhat) : std::runtime_error(aWhat) {}
};
}  // namespace plato::utilities
#endif
