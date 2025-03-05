###############################################################################
## require_feature( 
##    flag  == some flag that defines whether a feature is turned - if not skip this folder
## )
###############################################################################
macro(require_feature flag)
    if(NOT ${flag})
        set(CUR_PATH ${CMAKE_CURRENT_SOURCE_DIR})
        message(STATUS "${CUR_PATH} not included because following flags are missing or off:\n${flag}")
        return()
    endif()
endmacro()
