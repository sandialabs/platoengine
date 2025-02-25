###############################################################################
## require_feature( 
##    flag  == some flag that defines whether a feature is turned on or make fatal error
## )
###############################################################################
function(require_feature flag)
    if(NOT ${flag})
        message(FATAL_ERROR "Feature ${flag} is required but not enabled.")
    endif()
endfunction()
