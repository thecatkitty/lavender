# Build directory
set(CMAKE_BINARY_DIR "${CMAKE_ARGV3}")

# Assignments
set(ARGS_INDEX 4)
math(EXPR ARGS_COUNT "${CMAKE_ARGC}")

while(ARGS_INDEX LESS ARGS_COUNT)
    list(APPEND ASSIGNMENTS ${CMAKE_ARGV${ARGS_INDEX}})        
    math(EXPR ARGS_INDEX "${ARGS_INDEX}+1")
endwhile()


# Retrieve target
include(cmake/extensions/dotconfig.cmake)
retrieve_kconfig_target(KCONFIG_TARGET)


# Call setconfig
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
        TARGET=${KCONFIG_TARGET}
        setconfig
        --kconfig ${CMAKE_SOURCE_DIR}/Kconfig
        ${ASSIGNMENTS}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
