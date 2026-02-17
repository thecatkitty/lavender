function(prepare_dotconfig)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_BINARY_DIR}/.config)

    if("$ENV{LAV_DEFCONFIG}" STREQUAL "")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E env
                TARGET=${KCONFIG_TARGET}
                olddefconfig ${CMAKE_SOURCE_DIR}/Kconfig
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    else()
        set(defconfig "${CMAKE_SOURCE_DIR}/config/$ENV{LAV_DEFCONFIG}.defconfig")
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${defconfig})
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E env
                TARGET=${KCONFIG_TARGET}
                defconfig ${defconfig}
                --kconfig ${CMAKE_SOURCE_DIR}/Kconfig
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endif()

    install(FILES ${CMAKE_BINARY_DIR}/.config DESTINATION .)
endfunction()


function(import_dotconfig)
    file(STRINGS ${CMAKE_BINARY_DIR}/.config DOTCONFIG_FILE
        ENCODING "UTF-8")

    foreach(LINE ${DOTCONFIG_FILE})
        if("${LINE}" MATCHES "^(CONFIG_[^=]+)=([yn]|.+$)")
            set(KCONFIG_VARIABLE_NAME "${CMAKE_MATCH_1}")
            set(KCONFIG_VARIABLE_VALUE "${CMAKE_MATCH_2}")
        elseif("${LINE}" MATCHES "^# (CONFIG_[^ ]+) is not set")
            set(KCONFIG_VARIABLE_NAME "${CMAKE_MATCH_1}")
            set(KCONFIG_VARIABLE_VALUE "n")
        else()
            continue()
        endif()

        if("${KCONFIG_VARIABLE_VALUE}" STREQUAL "n")
            unset("${KCONFIG_VARIABLE_NAME}" PARENT_SCOPE)
        else()
            if("${KCONFIG_VARIABLE_VALUE}" MATCHES "^\"(.*)\"$")
                set(KCONFIG_VARIABLE_VALUE ${CMAKE_MATCH_1})
            endif()
            set("${KCONFIG_VARIABLE_NAME}" "${KCONFIG_VARIABLE_VALUE}" PARENT_SCOPE)
        endif()
    endforeach()
endfunction()


function(add_config_header)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/inc/generated)

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env
            TARGET=${KCONFIG_TARGET}
            genconfig
            --header-path inc/generated/config.h
            ${CMAKE_SOURCE_DIR}/Kconfig
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    include_directories(${CMAKE_BINARY_DIR}/inc)
endfunction()


function(add_menuconfig_target)
    add_custom_target(menuconfig
        COMMAND ${CMAKE_COMMAND} -E env
            TARGET=${KCONFIG_TARGET}
            menuconfig ${CMAKE_SOURCE_DIR}/Kconfig
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running menuconfig...")
endfunction()


function(retrieve_kconfig_target varname)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -LA -N ${CMAKE_BINARY_DIR}
        OUTPUT_VARIABLE CACHE_VARS)
    string(REPLACE "\n" ";" CACHE_VARS "${CACHE_VARS}")

    foreach(line IN LISTS CACHE_VARS)
        if(line MATCHES "^KCONFIG_TARGET:STRING=(.+)$")
            set(${varname} "${CMAKE_MATCH_1}")
        endif()
    endforeach()
endfunction()
