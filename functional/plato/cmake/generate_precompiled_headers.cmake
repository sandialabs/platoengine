# Generates a list of headers to precompile. The headers are all those included with angle brackets,
# as this is our standard for external library includes.
function(precompiled_header_list ALL_FILES OUT_precompiled_header_list)
    unset(RAW_HEADER_LIST)
    foreach(SRC ${ALL_FILES})
        if(NOT IS_ABSOLUTE ${SRC})
            get_filename_component(SRC ${SRC} ABSOLUTE ${CMAKE_CURRENT_LIST_DIR})
        endif()
        file(STRINGS "${SRC}" MATCHED_INCLUDE_LINES REGEX "#include[ \t]*<[^>]+>" )  
        foreach(INCLUDE_LINE IN LISTS MATCHED_INCLUDE_LINES)
            string(REGEX REPLACE
                ".*#include[ \t]*<([^>]+)>.*"
                "\\1"
                REGEX_LINE
                "${INCLUDE_LINE}"
            )
            if("${REGEX_LINE}" MATCHES "^plato\/.*")
                message(SEND_ERROR "Found an internal plato header included with angle-brackets.\nHeader: ${REGEX_LINE}\nFile: ${SRC}")
            else()
                list(APPEND RAW_HEADER_LIST "<${REGEX_LINE}$<ANGLE-R>")
            endif()
        endforeach()

        file(STRINGS "${SRC}" MATCHED_INCLUDE_LINES REGEX "#include[ \t]*\"[^>]+\"" )
        foreach(INCLUDE_LINE IN LISTS MATCHED_INCLUDE_LINES)
            if(NOT "${INCLUDE_LINE}" MATCHES "#include[ \t]*\"plato\/.*" )
                message(SEND_ERROR "Found an apparently external header included with quotes.\nHeader: ${INCLUDE_LINE}\nFile: ${SRC}")
            endif()
        endforeach()

    endforeach()

    list(REMOVE_DUPLICATES RAW_HEADER_LIST)

    set(${OUT_precompiled_header_list} "${RAW_HEADER_LIST}" PARENT_SCOPE)

endfunction(precompiled_header_list)
