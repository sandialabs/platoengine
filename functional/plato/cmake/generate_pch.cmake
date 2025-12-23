
macro(remove_pch_excludes)
    set(PCH_EXCLUDE_LIST "boost/spirit/include/qi.hpp" "boost/spirit/home/qi/nonterminal/rule.hpp")
    foreach(BAD_HEADER ${PCH_EXCLUDE_LIST})
        list(FILTER RAW_HEADER_LIST EXCLUDE REGEX ${BAD_HEADER})
    endforeach()
endmacro(remove_pch_excludes)

# Generates a list of headers to precompile. The headers are all those included with angle brackets,
# as this is our standard for external library includes.
function(pch_list ALL_FILES OUT_PCH_LIST)
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
            list(APPEND RAW_HEADER_LIST "<${REGEX_LINE}$<ANGLE-R>")
        endforeach()
    endforeach()

    list(REMOVE_DUPLICATES RAW_HEADER_LIST)

    remove_pch_excludes()

    set(${OUT_PCH_LIST} "${RAW_HEADER_LIST}" PARENT_SCOPE)

endfunction(pch_list)
