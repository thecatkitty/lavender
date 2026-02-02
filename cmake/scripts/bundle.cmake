foreach(v EXECUTABLE_PATH ARCHIVE_PATH BUNDLE_PATH)
  if(NOT DEFINED ${v})
    message(FATAL_ERROR "Missing -D${v}=...")
  endif()
endforeach()


execute_process(
    COMMAND ${CMAKE_COMMAND} -E cat
        "${EXECUTABLE_PATH}" "${ARCHIVE_PATH}"
    ENCODING NONE
    OUTPUT_FILE "${BUNDLE_PATH}"
    COMMAND_ERROR_IS_FATAL ANY)

if(NOT "${MAX_SIZE}" STREQUAL "")
    math(EXPR MAX_SIZE_DEC "${MAX_SIZE}")
    file(SIZE "${BUNDLE_PATH}" size)

    if(size GREATER ${MAX_SIZE_DEC})
        message(FATAL_ERROR "Bundle '${BUNDLE_PATH}' is too large!")
    endif()
endif()
