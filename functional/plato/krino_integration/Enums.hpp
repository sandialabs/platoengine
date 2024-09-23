#ifndef PLATO_KRINO_INTEGRATION_ENUMS
#define PLATO_KRINO_INTEGRATION_ENUMS

namespace plato::krino_integration
{

enum class RunMode
{
    STAND_ALONE_PROMPT,
    STAND_ALONE_COMMAND_FILE,
    MPMD,
    UNKNOWN
};

enum class DataTransferMode
{
    IMPORT,
    EXPORT,
    UNKNOWN
};

}  // namespace plato::krino_integration

#endif  // PLATO_KRINO_INTEGRATION_ENUMS
