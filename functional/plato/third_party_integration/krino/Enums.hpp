#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_ENUMS
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_ENUMS

namespace plato::third_party_integration::krino
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

}  // namespace plato::third_party_integration::krino

#endif  // PLATO_THIRD_PARTY_INTEGRATION_KRINO_ENUMS
