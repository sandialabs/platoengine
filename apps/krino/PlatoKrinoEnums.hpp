#pragma once 

namespace Plato::Krino 
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
}

